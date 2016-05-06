/*
 * lmdb.c - Builtin functions to implement lmdb.
 */

/*
 * Copyright (C) 2016 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "common.h"
#include <lmdb.h>
#include <assert.h>

#define API_ERROR (MDB_LAST_ERRCODE-1)

static const struct {
   const char *name;
   int val;
} mdbdef[] = {
#define init_lmdb(A, B, C) { #B, C },
#include "lmdb_header.h"
#undef init_lmdb
};

static struct {
  struct namespace {
    strhash *ht;
    char name[4];
  } env, txn, dbi;
} mdb = {
  .env = { .name = "env" },
  .txn = { .name = "txn" },
  .dbi = { .name = "dbi" },
};

static awk_value_t *
do_mdb_strerror(int nargs, awk_value_t *result)
{
  awk_value_t err;
  const char *s;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  
  if (!get_argument(0, AWK_NUMBER, &err)) {
    set_ERRNO(_("mdb_sterror: cannot get error number argument"));
    RET_NULSTR;
  }
  if (err.num_value == API_ERROR)
    s = "API_ERROR: internal error in gawk lmdb API";
  else
    s = mdb_strerror(err.num_value);
  return make_string_malloc(s, strlen(s), result);
}

static strhash_entry *
get_handle(struct namespace *ns, size_t argnum,
	   awk_value_t *name, const char *funcname)
{
  strhash_entry *hte;

  if (!get_argument(argnum, AWK_UNDEFINED, name)) {
    char emsg[256];
    snprintf(emsg, sizeof(emsg), _("%s: %s name required in argument #%zu"),
	     funcname, ns->name, argnum+1);
    set_ERRNO(emsg);
    return NULL;
  }
  if ((name->val_type != AWK_STRING) || !name->str_value.len) {
    char emsg[256];
    snprintf(emsg, sizeof(emsg),
	     _("%s: argument #%zu must be a non-empty string identifying "
	       "the %s"), funcname, argnum+1, ns->name);
    set_ERRNO(emsg);
    return NULL;
  }
  hte = strhash_get(ns->ht, name->str_value.str, name->str_value.len, 1);
  if (hte->data) {
    char emsg[256];
    snprintf(emsg, sizeof(emsg),
	     _("%s: %s name in argument #%zu is already in use"),
	     funcname, ns->name, argnum+1);
    set_ERRNO(emsg);
    return NULL;
  }
  return hte;
}

static void
release_handle(struct namespace *ns, awk_value_t *name, const char *funcname)
{
  if (strhash_delete(ns->ht, name->str_value.str, name->str_value.len,
		     NULL, NULL) < 0)
    fatal(ext_id, _("%s: unable to release %s handle `%s'"),
	  funcname, ns->name, name->str_value.str);
}

static void *
lookup_handle(struct namespace *ns, size_t argnum, awk_value_t *key,
	      awk_bool_t empty_ok, const char *funcname)
{
  strhash_entry *hte;
  awk_value_t keybuf;
  
  if (!key)
    /* some callers need the name so they can release the handle */
    key = &keybuf;
  if (!get_argument(argnum, AWK_STRING, key)) {
    char emsg[256];
    snprintf(emsg, sizeof(emsg),
	     _("%s: argument #%zu must be a string identifying the %s"),
	     funcname, argnum+1, ns->name);
    set_ERRNO(emsg);
    return NULL;
  }
  if (!key->str_value.len) {
    if (!empty_ok) {
      char emsg[256];
      snprintf(emsg, sizeof(emsg),
	       _("%s: argument #%zu empty string invalid as a %s handle"),
	       funcname, argnum+1, ns->name);
      set_ERRNO(emsg);
    }
    return NULL;
  }
  if (!(hte = strhash_get(ns->ht, key->str_value.str, key->str_value.len, 0))) {
    char emsg[256];
    snprintf(emsg, sizeof(emsg),
	     _("%s: argument #%zu `%s' does not map to a known %s handle"),
	     funcname, argnum+1, key->str_value.str, ns->name);
    set_ERRNO(emsg);
    return NULL;
  }
  if (!hte->data)
    fatal(ext_id,
	  _("%s: corruption detected: %s handle `%s' maps to a NULL pointer"),
	  funcname, ns->name, key->str_value.str);
  return hte->data;
}

static awk_value_t *
do_mdb_env_create(int nargs, awk_value_t *result)
{
  awk_value_t name;
  int rc;
  MDB_env *env;
  strhash_entry *hte;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(hte = get_handle(&mdb.env, 0, &name, __func__+3)))
    RET_NUM(API_ERROR);
  if ((rc = mdb_env_create(&env)) != MDB_SUCCESS) {
    release_handle(&mdb.env, &name, __func__+3);
    set_ERRNO(_("mdb_env_create failed"));
    RET_NUM(rc);
  }
  hte->data = env;
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_env_close(int nargs, awk_value_t *result)
{
  awk_value_t name;
  MDB_env *env;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(env = lookup_handle(&mdb.env, 0, &name, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  mdb_env_close(env);
  release_handle(&mdb.env, &name, __func__+3);
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_env_set_mapsize(int nargs, awk_value_t *result)
{
  awk_value_t msize;
  int rc;
  MDB_env *env;

  if (do_lint && nargs > 2)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(env = lookup_handle(&mdb.env, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if (!get_argument(1, AWK_NUMBER, &msize)) {
    set_ERRNO(_("mdb_env_set_mapsize: 2nd argument must be the mapsize"));
    RET_NUM(API_ERROR);
  }
  rc = mdb_env_set_mapsize(env, msize.num_value);
  RET_NUM(rc);
}

static awk_value_t *
do_mdb_env_open(int nargs, awk_value_t *result)
{
  awk_value_t path, flags, mode;
  int rc;
  MDB_env *env;

  if (do_lint && nargs > 4)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(env = lookup_handle(&mdb.env, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if (!get_argument(1, AWK_STRING, &path)) {
    set_ERRNO(_("mdb_env_open: 2nd argument must be a string path value"));
    RET_NUM(API_ERROR);
  }
  if (!get_argument(2, AWK_NUMBER, &flags)) {
    set_ERRNO(_("mdb_env_open: 3rd argument must be a numeric flags value"));
    RET_NUM(API_ERROR);
  }
  if (!get_argument(3, AWK_NUMBER, &mode)) {
    set_ERRNO(_("mdb_env_open: 4th argument must be a numeric mode value"));
    RET_NUM(API_ERROR);
  }
  rc = mdb_env_open(env, path.str_value.str, flags.num_value, mode.num_value);
  RET_NUM(rc);
}

static awk_value_t *
do_mdb_txn_begin(int nargs, awk_value_t *result)
{
  awk_value_t name, flags;
  int rc;
  MDB_env *env;
  MDB_txn *parent;
  MDB_txn *txn;
  strhash_entry *hte;

  if (do_lint && nargs > 4)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(env = lookup_handle(&mdb.env, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  {
    awk_value_t pname;
    if (!(parent = lookup_handle(&mdb.txn, 1, &pname, awk_true, __func__+3)) &&
	pname.str_value.len)
      RET_NUM(API_ERROR);
  }
  if (!get_argument(2, AWK_NUMBER, &flags)) {
    set_ERRNO(_("mdb_txn_begin: 3rd argument must be a numeric flags value"));
    RET_NUM(API_ERROR);
  }
  if (!(hte = get_handle(&mdb.txn, 3, &name, __func__+3)))
    RET_NUM(API_ERROR);
  if ((rc = mdb_txn_begin(env, parent, flags.num_value, &txn)) != MDB_SUCCESS) {
    release_handle(&mdb.txn, &name, __func__+3);
    set_ERRNO(_("mdb_txn_begin failed"));
    RET_NUM(rc);
  }
  hte->data = txn;
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_txn_id(int nargs, awk_value_t *result)
{
  MDB_txn *txn;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  RET_NUM(mdb_txn_id(txn));
}

static awk_value_t *
do_mdb_txn_commit(int nargs, awk_value_t *result)
{
  awk_value_t name;
  MDB_txn *txn;
  int rc;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, &name, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if ((rc = mdb_txn_commit(txn)) == MDB_SUCCESS)
    release_handle(&mdb.txn, &name, __func__+3);
  else
    set_ERRNO(_("mdb_txn_commit failed"));
  RET_NUM(rc);
}

static awk_value_t *
do_mdb_txn_abort(int nargs, awk_value_t *result)
{
  awk_value_t name;
  MDB_txn *txn;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, &name, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  mdb_txn_abort(txn);
  release_handle(&mdb.txn, &name, __func__+3);
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_txn_reset(int nargs, awk_value_t *result)
{
  MDB_txn *txn;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  mdb_txn_reset(txn);
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_txn_renew(int nargs, awk_value_t *result)
{
  MDB_txn *txn;

  if (do_lint && nargs > 1)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  RET_NUM(mdb_txn_renew(txn));
}

static awk_value_t *
do_mdb_dbi_open(int nargs, awk_value_t *result)
{
  awk_value_t name, flags;
  int rc;
  MDB_txn *txn;
  MDB_dbi *dbi;
  strhash_entry *hte;

  if (do_lint && nargs > 4)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if (!get_argument(1, AWK_STRING, &name)) {
    set_ERRNO(_("mdb_dbi_open: 3rd argument must be a numeric flags value"));
    RET_NUM(API_ERROR);
  }
  if (!get_argument(2, AWK_NUMBER, &flags)) {
    set_ERRNO(_("mdb_dbi_open: 3rd argument must be a numeric flags value"));
    RET_NUM(API_ERROR);
  }
  if (!(hte = get_handle(&mdb.dbi, 3, &name, __func__+3)))
    RET_NUM(API_ERROR);
  if (!(dbi = (MDB_dbi *)malloc(sizeof(*dbi))))
    fatal(ext_id, _("%s: dbi malloc failed"), __func__+3);
  if ((rc = mdb_dbi_open(txn, (name.str_value.len ? name.str_value.str : NULL),
			 flags.num_value, dbi)) != MDB_SUCCESS) {
    release_handle(&mdb.dbi, &name, __func__+3);
    set_ERRNO(_("mdb_dbi_open failed"));
    RET_NUM(rc);
  }
  hte->data = dbi;
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_dbi_close(int nargs, awk_value_t *result)
{
  awk_value_t name;
  MDB_env *env;
  MDB_dbi *dbi;

  if (do_lint && nargs > 2)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(env = lookup_handle(&mdb.env, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if (!(dbi = lookup_handle(&mdb.dbi, 1, &name, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  mdb_dbi_close(env, *dbi);
  free(dbi);
  release_handle(&mdb.dbi, &name, __func__+3);
  RET_NUM(MDB_SUCCESS);
}

static awk_value_t *
do_mdb_put(int nargs, awk_value_t *result)
{
  awk_value_t key, data, flags;
  MDB_txn *txn;
  MDB_dbi *dbi;
  MDB_val mdbkey, mdbdata;

  if (do_lint && nargs > 5)
    lintwarn(ext_id, _("%s: called with too many arguments"), __func__+3);
  if (!(txn = lookup_handle(&mdb.txn, 0, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if (!(dbi = lookup_handle(&mdb.dbi, 1, NULL, awk_false, __func__+3)))
    RET_NUM(API_ERROR);
  if (!get_argument(2, AWK_STRING, &key)) {
    set_ERRNO(_("mdb_put: 3rd argument must be the key string"));
    RET_NUM(API_ERROR);
  }
  if (!get_argument(3, AWK_STRING, &data)) {
    set_ERRNO(_("mdb_put: 4th argument must be the data string"));
    RET_NUM(API_ERROR);
  }
  if (!get_argument(4, AWK_NUMBER, &flags)) {
    set_ERRNO(_("mdb_put: 5th argument must a numeric flags value"));
    RET_NUM(API_ERROR);
  }
  mdbkey.mv_size = key.str_value.len;
  mdbkey.mv_data = key.str_value.str;
  mdbdata.mv_size = data.str_value.len;
  mdbdata.mv_data = data.str_value.str;
  RET_NUM(mdb_put(txn, *dbi, &mdbkey, &mdbdata, flags.num_value));
}

static awk_ext_func_t func_table[] = {
  { "mdb_strerror", do_mdb_strerror, 1 },
  { "mdb_env_create", do_mdb_env_create, 1 },
  { "mdb_env_set_mapsize", do_mdb_env_set_mapsize, 2 },
  { "mdb_env_open", do_mdb_env_open, 4 },
  { "mdb_env_close", do_mdb_env_close, 1 },
  { "mdb_txn_begin", do_mdb_txn_begin, 4 },
  { "mdb_txn_id", do_mdb_txn_id, 1 },
  { "mdb_txn_commit", do_mdb_txn_commit, 1 },
  { "mdb_txn_abort", do_mdb_txn_abort, 1 },
  { "mdb_txn_reset", do_mdb_txn_reset, 1 },
  { "mdb_txn_renew", do_mdb_txn_renew, 1 },
  { "mdb_dbi_open", do_mdb_dbi_open, 4 },
  { "mdb_dbi_close", do_mdb_dbi_close, 2 },
  { "mdb_put", do_mdb_put, 5 },
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT

  assert(MDB_SUCCESS != API_ERROR);	/* sanity check */
  /* strhash_create exits on failure, so no need to check return code */
  mdb.env.ht = strhash_create(0);
  mdb.txn.ht = strhash_create(0);
  mdb.dbi.ht = strhash_create(0);
  {
    awk_value_t x;
    awk_scalar_t cookie;
    if (!gawk_varinit_constant("MDB_SUCCESS", make_number(MDB_SUCCESS, &x),
			       &cookie))
      fatal(ext_id, _("lmdb: unable to initialize MDB_SUCCESS"));
  }
  {
    /* check version */
    int major, minor;
    const char *verstr = mdb_version(&major, &minor, NULL);
    if ((MDB_VERSION_MAJOR != major) || (MDB_VERSION_MINOR > minor))
      fatal(ext_id,
	    _("lmdb compile-time version `%s' inconsistent with run-time "
	      "library version `%s'"), MDB_VERSION_STRING, verstr);
  }
  {
    awk_array_t mc;
    size_t i;

    if (!gawk_varinit_array("MDB", 1, &mc))
      fatal(ext_id, _("lmdb: unable to create MDB array"));
    for (i = 0; i < sizeof(mdbdef)/sizeof(mdbdef[0]); i++) {
      awk_value_t idx, val;
      if (!set_array_element(mc,
			     make_string_malloc(mdbdef[i].name,
			     			strlen(mdbdef[i].name), &idx),
			     make_number(mdbdef[i].val, &val)))
	fatal(ext_id, _("lmdb: unable to initialize MDB[%s]"), mdbdef[i].name);
    }
  }

  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, lmdb, "")
