/*
 * select.c - Builtin functions to provide select I/O multiplexing.
 */

/*
 * Copyright (C) 2013 the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#include "common.h"
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_SELECT) && defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

static const char *signal2name[] = {
#define init_sig(A, B) [A] = B,
#include "name2signal.h"
#undef init_sig
};
#define NUM_SIG2NAME  (sizeof(signal2name)/sizeof(signal2name[0]))

static const struct {
   const char *name;
   int n;
} name2signal[] = {
#define init_sig(A, B) { B, A },
#include "name2signal.h"
#undef init_sig
};
#define NUM_NAME2SIG  (sizeof(name2signal)/sizeof(name2signal[0]))

#define MIN_VALID_SIGNAL  1   /* 0 is not allowed! */
static struct {
   int min;
   int max;
} sigrange;

static int
signame2num(const char *name)
{
  size_t i;

  if (strncasecmp(name, "sig", 3) == 0)
    /* skip "sig" prefix */
    name += 3;
  for (i = 0; i < NUM_NAME2SIG; i++) {
    if (!strcasecmp(name2signal[i].name, name)) {
      /*
       * some signal numbers have multiple names, so we save the version that
       * was used so that the inverse mapping is consistent.
       */
      if (name2signal[i].n < (int)NUM_SIG2NAME)
	signal2name[name2signal[i].n] = name2signal[i].name;
      return name2signal[i].n;
    }
  }
  return -1;
}

static volatile struct {
   int flag;
   sigset_t mask;
} caught;

static void
signal_handler(int signum)
{
  /*
   * All signals should be blocked, so we do not have to worry about
   * whether sigaddset is thread-safe.  It is documented to be
   * async-signal-safe.
   */
  sigaddset(& caught.mask, signum);
  caught.flag = 1;
#ifndef HAVE_SIGACTION
  /*
   * On platforms without sigaction, we do not know how the legacy
   * signal API will behave.  There does not appear to be an autoconf
   * test for whether the signal handler is reset to default each time
   * a signal is trapped, so we do this to be safe.
   */
  signal(signum, signal_handler);
#endif
}

static int
integer_string(const char *s, long *x)
{
  char *endptr;

  *x = strtol(s, & endptr, 10);
  return ((endptr != s) && (*endptr == '\0')) ? 0 : -1;
}

static int
get_signal_number(awk_value_t signame, int *signum)
{
  switch (signame.val_type) {
  case AWK_NUMBER:
    *signum = signame.num_value;
    if (*signum != signame.num_value) {
      update_ERRNO_string(_("invalid signal number"));
      return -1;
    }
    return 0;
  case AWK_STRING:
    if ((*signum = signame2num(signame.str_value.str)) >= 0)
      return 0;
    {
      long z;
      if (integer_string(signame.str_value.str, &z) == 0) {
        *signum = z;
        return 0;
      }
    }
    update_ERRNO_string(_("invalid signal name"));
    return -1;
  default:
    update_ERRNO_string(_("signal name argument must be string or numeric"));
    return -1;
  }
}

static awk_value_t *
signal_result(awk_value_t *result, void (*func)(int))
{
  awk_value_t override;

  if (func == SIG_DFL)
    return make_const_string("default", 7, result);
  if (func == SIG_IGN)
    return make_const_string("ignore", 6, result);
  if (func == signal_handler)
    return make_const_string("trap", 4, result);
  if (get_argument(2, AWK_NUMBER, & override) && override.num_value)
    return make_const_string("unknown", 7, result);
  /* need to roll it back! */
  update_ERRNO_string(_("select_signal: override not requested for unknown signal handler"));
  make_null_string(result);
  return NULL;
}

/*  do_signal --- trap signals */

static awk_value_t *
do_signal(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG)
{
  awk_value_t signame, disposition;
  int signum;
  void (*func)(int);

#if gawk_api_major_version < 2
  if (do_lint && nargs > 3)
    lintwarn(ext_id, _("select_signal: called with too many arguments"));
#endif
  if (! get_argument(0, AWK_UNDEFINED, & signame)) {
    update_ERRNO_string(_("select_signal: missing required signal name argument"));
    return make_null_string(result);
  }
  if (get_signal_number(signame, &signum) < 0)
    return make_null_string(result);
  if (signum < MIN_VALID_SIGNAL) {
    update_ERRNO_string(_("invalid signal number"));
    return make_null_string(result);
  }
  if (! get_argument(1, AWK_STRING, & disposition)) {
    update_ERRNO_string(_("select_signal: missing required signal disposition argument"));
    return make_null_string(result);
  }
  if (strcasecmp(disposition.str_value.str, "default") == 0)
    func = SIG_DFL;
  else if (strcasecmp(disposition.str_value.str, "ignore") == 0)
    func = SIG_IGN;
  else if (strcasecmp(disposition.str_value.str, "trap") == 0)
    func = signal_handler;
  else {
    update_ERRNO_string(_("select_signal: invalid disposition argument"));
    return make_null_string(result);
  }

#ifdef HAVE_SIGPROCMASK
/* Temporarily block this signal in case we need to roll back the handler! */
#define PROTECT \
    sigset_t set, oldset; \
    sigemptyset(& set); \
    sigaddset(& set, signum); \
    sigprocmask(SIG_BLOCK, &set, &oldset);
#define RELEASE sigprocmask(SIG_SETMASK, &oldset, NULL);
#else
/* Brain-damaged platform, so we will have to live with the race condition. */
#define PROTECT
#define RELEASE
#endif

#define UPDATE_RANGE \
  if (func == signal_handler) { \
    if (!sigrange.min) \
      sigrange.min = sigrange.max = signum; \
    else { \
      if (sigrange.min > signum) \
	sigrange.min = signum; \
      if (sigrange.max < signum) \
	sigrange.max = signum; \
    } \
  }

#ifdef HAVE_SIGACTION
  {
    struct sigaction sa, prev;
    sa.sa_handler = func;
    sigfillset(& sa.sa_mask);  /* block all signals in handler */
    sa.sa_flags = SA_RESTART;
    {
      PROTECT
      if (sigaction(signum, &sa, &prev) < 0) {
        update_ERRNO_int(errno);
        RELEASE
        return make_null_string(result);
      }
      if (signal_result(result, prev.sa_handler)) {
        RELEASE
	UPDATE_RANGE
        return result;
      }
      /* roll it back! */
      sigaction(signum, &prev, NULL);
      RELEASE
      return result;
    }
  }
#else
  /*
   * Fall back to signal; this is available on all platforms.  We can
   * only hope that it does the right thing.
   */
  {
    void (*prev)(int);
    PROTECT
    if ((prev = signal(signum, func)) == SIG_ERR) {
      update_ERRNO_int(errno);
      RELEASE
      return make_null_string(result);
    }
    if (signal_result(result, prev)) {
      RELEASE
      UPDATE_RANGE
      return result;
    }
    /* roll it back! */
    signal(signum, prev);
    RELEASE
    return result;
  }
#endif
}

/*  do_kill --- send a signal */

static awk_value_t *
do_kill(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG)
{
#ifdef HAVE_KILL
  awk_value_t pidarg, signame;
  pid_t pid;
  int signum;
  int rc;

#if gawk_api_major_version < 2
  if (do_lint && nargs > 2)
    lintwarn(ext_id, _("kill: called with too many arguments"));
#endif
  if (! get_argument(0, AWK_NUMBER, & pidarg)) {
    update_ERRNO_string(_("kill: missing required pid argument"));
    return make_number(-1, result);
  }
  pid = pidarg.num_value;
  if (pid != pidarg.num_value) {
    update_ERRNO_string(_("kill: pid argument must be an integer"));
    return make_number(-1, result);
  }
  if (! get_argument(1, AWK_UNDEFINED, & signame)) {
    update_ERRNO_string(_("kill: missing required signal name argument"));
    return make_number(-1, result);
  }
  if (get_signal_number(signame, &signum) < 0)
    return make_number(-1, result);
  if ((rc = kill(pid, signum)) < 0)
    update_ERRNO_int(errno);
  return make_number(rc, result);
#else
  update_ERRNO_string(_("kill: not supported on this platform"));
  return make_number(-1, result);
#endif
}

static int
grabfd(int i, const awk_input_buf_t *ibuf, const awk_output_buf_t *obuf, const char *fnm, const char *ftp)
{
  switch (i) {
  case 0: /* read */
    return ibuf ? ibuf->fd : -1;
  case 1: /* write */
    return obuf ? fileno(obuf->fp) : -1;
  case 2: /* except */
    if (ibuf) {
      if (obuf && ibuf->fd != fileno(obuf->fp))
        warning(ext_id, _("select: `%s', `%s' in `except' array has clashing fds, using input %d, not output %d"), fnm, ftp, ibuf->fd, fileno(obuf->fp));
      return ibuf->fd;
    }
    if (obuf)
      return fileno(obuf->fp);
    break;
  }
  return -1;
}

static int
get_numeric_argument(size_t count, double *x)
{
  awk_value_t val;

  if (! get_argument(count, AWK_UNDEFINED, &val))
    return 0;
  switch (val.val_type) {
  case AWK_NUMBER:
    *x = val.num_value;
    return 1;
  case AWK_STRNUM:
    /* we know it's numeric, but the value comes back as a string. */
    /* FALLTHRU */
  case AWK_STRING:
    /* it could be a numeric string */
    {
      char junk[3];
      return (sscanf(val.str_value.str, "%lf%1s", x, junk) == 1);
    }
  default:
    return 0;
  }
}

/*  do_select --- I/O multiplexing */

static awk_value_t *
do_select(int nargs, awk_value_t *result API_FINFO_ARG)
{
  static const char *argname[] = { "read", "write", "except" };
  struct {
    awk_value_t array;
    awk_flat_array_t *flat;
    fd_set bits;
    int *array2fd;
  } fds[3];
  double secs;
  u_int i;
  struct timeval maxwait;
  struct timeval *timeout;
  int nfds = 0;
  int rc;
  awk_value_t sigarr;
  int dosig;

#if gawk_api_major_version < 2
  if (do_lint && nargs > 5)
    lintwarn(ext_id, _("select: called with too many arguments"));
#endif
#define EL  fds[i].flat->elements[j]
  if (nargs >= 5) {
    dosig = 1;
    if (! get_argument(4, AWK_ARRAY, &sigarr)) {
      warning(ext_id, _("select: the signal argument must be an array"));
      update_ERRNO_string(_("select: bad signal parameter"));
      return make_number(-1, result);
    }
    clear_array(sigarr.array_cookie);
  }
  else
    dosig = 0;

  for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
    size_t j;

    if (! get_argument(i, AWK_ARRAY, & fds[i].array)) {
      warning(ext_id, _("select: bad array parameter `%s'"), argname[i]);
      update_ERRNO_string(_("select: bad array parameter"));
      return make_number(-1, result);
    }
    /* N.B. flatten_array fails for empty arrays, so that's OK */
    FD_ZERO(&fds[i].bits);
    if (flatten_array(fds[i].array.array_cookie, &fds[i].flat)) {
      emalloc(fds[i].array2fd, int *, fds[i].flat->count*sizeof(int), "select");
      for (j = 0; j < fds[i].flat->count; j++) {
        long x;
        fds[i].array2fd[j] = -1;
        /* note: the index is always delivered as a string */

        if (((EL.value.val_type == AWK_UNDEFINED) || ((EL.value.val_type == AWK_STRING) && ! EL.value.str_value.len)) && (integer_string(EL.index.str_value.str, &x) == 0) && (x >= 0))
          fds[i].array2fd[j] = x;
        else if (EL.value.val_type == AWK_STRING) {
          const awk_input_buf_t *ibuf;
          const awk_output_buf_t *obuf;
          int fd;
          if (get_file(EL.index.str_value.str, EL.index.str_value.len, EL.value.str_value.str, -1, &ibuf, &obuf) && ((fd = grabfd(i, ibuf, obuf, EL.index.str_value.str, EL.value.str_value.str)) >= 0))
            fds[i].array2fd[j] = fd;
          else
            warning(ext_id, _("select: get_file(`%s', `%s') failed in `%s' array"), EL.index.str_value.str, EL.value.str_value.str, argname[i]);
        }
        else
          warning(ext_id, _("select: command type should be a string for `%s' in `%s' array"), EL.index.str_value.str, argname[i]);
        if (fds[i].array2fd[j] < 0) {
          update_ERRNO_string(_("select: get_file failed"));
          if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
            warning(ext_id, _("select: release_flattened_array failed"));
          free(fds[i].array2fd);
          return make_number(-1, result);
        }
        FD_SET(fds[i].array2fd[j], &fds[i].bits);
        if (nfds <= fds[i].array2fd[j])
          nfds = fds[i].array2fd[j]+1;
      }
    }
    else
      fds[i].flat = NULL;
  }
  if (dosig && caught.flag) {
    /* take a quick poll, but do not block, since signals have been trapped */
    maxwait.tv_sec = maxwait.tv_usec = 0;
    timeout = &maxwait;
  }
  else if (get_numeric_argument(3, &secs)) {
    if (secs < 0) {
      warning(ext_id, _("select: treating negative timeout as zero"));
      secs = 0;
    }
    maxwait.tv_sec = secs;
    maxwait.tv_usec = (secs-(double)maxwait.tv_sec)*1000000.0;
    timeout = &maxwait;
  } else
    timeout = NULL;

  if ((rc = select(nfds, &fds[0].bits, &fds[1].bits, &fds[2].bits, timeout)) < 0)
    update_ERRNO_int(errno);

  if (dosig && caught.flag) {
    int s;
    sigset_t trapped;
#ifdef HAVE_SIGPROCMASK
    /*
     * Block signals while we copy and reset the mask to eliminate
     * a race condition whereby a signal could be missed.
     */
    sigset_t set, oldset;
    sigfillset(& set);
    sigprocmask(SIG_SETMASK, &set, &oldset);
#endif
    /*
     * Reset flag to 0 first.  If we don't have sigprocmask,
     * that may reduce the chance of missing a signal.
     */
    caught.flag = 0;
    trapped = caught.mask;
    sigemptyset(& caught.mask);
#ifdef HAVE_SIGPROCMASK
    sigprocmask(SIG_SETMASK, &oldset, NULL);
#endif
    /* populate sigarr with trapped signals */
    /* XXX this is very inefficient!  */
    for (s = sigrange.min; s <= sigrange.max; s++) {
      if (sigismember(& trapped, s) > 0) {
        awk_value_t idx, val;
        if ((s < (int)NUM_SIG2NAME) && signal2name[s])
          set_array_element(sigarr.array_cookie, make_number(s, &idx), make_const_string(signal2name[s], strlen(signal2name[s]), &val));
        else
          set_array_element(sigarr.array_cookie, make_number(s, &idx), make_null_string(&val));
      }
    }
  }

  if (rc < 0) {
    /* bit masks are undefined, so delete all array entries */
    for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
      if (fds[i].flat) {
        size_t j;
        for (j = 0; j < fds[i].flat->count; j++)
          EL.flags |= AWK_ELEMENT_DELETE;
        if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
          warning(ext_id, _("select: release_flattened_array failed"));
        free(fds[i].array2fd);
      }
    }
    return make_number(rc, result);
  }

  for (i = 0; i < sizeof(fds)/sizeof(fds[0]); i++) {
    if (fds[i].flat) {
      size_t j;
      /* remove array elements not set in the bit mask */
      for (j = 0; j < fds[i].flat->count; j++) {
        if (! FD_ISSET(fds[i].array2fd[j], &fds[i].bits))
          EL.flags |= AWK_ELEMENT_DELETE;
      }
      if (! release_flattened_array(fds[i].array.array_cookie, fds[i].flat))
        warning(ext_id, _("select: release_flattened_array failed"));
      free(fds[i].array2fd);
    }
  }
#undef EL

  /* Set the return value */
  return make_number(rc, result);
}

static int
set_non_blocking(int fd)
{
#if defined(HAVE_FCNTL) && defined(O_NONBLOCK)
  int flags;

  if (((flags = fcntl(fd, F_GETFL)) == -1) ||
    (fcntl(fd, F_SETFL, (flags|O_NONBLOCK)) == -1)) {
    update_ERRNO_int(errno);
    return -1;
  }
  return 0;
#else
  update_ERRNO_string(_("set_non_blocking: not supported on this platform"));
  return -1;
#endif
}

static void
set_retry(const char *name)
{
  static const char suffix[] = "RETRY";
  static awk_array_t procinfo;
  static char *subsep;
  static size_t subsep_len;
  awk_value_t idx, val;
  char *s;
  size_t len;

  if (!subsep) {
    /* initialize cached values for PROCINFO and SUBSEP */
    awk_value_t res;

    if (!gawk_varinit_array("PROCINFO", 0, &procinfo)) {
      warning(ext_id, _("set_non_blocking: could not install PROCINFO array; unable to configure PROCINFO RETRY for `%s'"), name);
      return;
    }

    if (! sym_lookup("SUBSEP", AWK_STRING, & res)) {
      warning(ext_id, _("set_non_blocking: sym_lookup(`%s') failed; unable to configure PROCINFO RETRY for `%s'"), "SUBSEP", name);
      return;
    }
    subsep = strdup(res.str_value.str);
    subsep_len = res.str_value.len;
  }

  len = strlen(name)+subsep_len+sizeof(suffix)-1;
  emalloc(s, char *, len+1, "set_non_blocking");
  sprintf(s, "%s%s%s", name, subsep, suffix);

  if (! set_array_element(procinfo, make_malloced_string(s, len, &idx), make_null_string(&val)))
    warning(ext_id, _("set_non_blocking: unable to configure PROCINFO RETRY for `%s'"), name);
}

/*  do_set_non_blocking --- Set a file to be non-blocking */

static awk_value_t *
do_set_non_blocking(int nargs, awk_value_t *result API_FINFO_ARG)
{
  awk_value_t cmd, cmdtype;
  int fd;

#if gawk_api_major_version < 2
  if (do_lint && nargs > 2)
    lintwarn(ext_id, _("set_non_blocking: called with too many arguments"));
#endif
  /*
   * N.B. If called with a single "" arg, we want it to work!  In that
   * case, the 1st arg is an empty string, and get_argument fails on the
   * 2nd arg.  Note that API get_file promises not to access the type
   * argument if the name argument is an empty string.
   */
  if (get_argument(0, AWK_NUMBER, & cmd) &&
    (cmd.num_value == (fd = cmd.num_value)) && 
    ! get_argument(1, AWK_STRING, & cmdtype))
    return make_number(set_non_blocking(fd), result);
  else if (get_argument(0, AWK_STRING, & cmd) &&
    (get_argument(1, AWK_STRING, & cmdtype) ||
      (! cmd.str_value.len && (nargs == 1)))) {
    const awk_input_buf_t *ibuf;
    const awk_output_buf_t *obuf;
    if (get_file(cmd.str_value.str, cmd.str_value.len, cmdtype.str_value.str, -1, &ibuf, &obuf)) {
      int rc = set_non_blocking(ibuf ? ibuf->fd : fileno(obuf->fp));
      if (rc == 0 && ibuf)
        set_retry(ibuf->name);
      return make_number(rc, result);
    }
    warning(ext_id, _("set_non_blocking: get_file(`%s', `%s') failed"), cmd.str_value.str, cmdtype.str_value.str);
  } else if (do_lint) {
    if (nargs < 2)
      lintwarn(ext_id, _("set_non_blocking: called with too few arguments"));
    else
      lintwarn(ext_id, _("set_non_blocking: called with inappropriate argument(s)"));
  }
  return make_number(-1, result);
}

/*  do_input_fd --- Return command's input file descriptor */

static awk_value_t *
do_input_fd(int nargs, awk_value_t *result API_FINFO_ARG)
{
  awk_value_t cmd, cmdtype;

#if gawk_api_major_version < 2
  if (do_lint && nargs > 2)
    lintwarn(ext_id, _("input_fd: called with too many arguments"));
#endif
  /*
   * N.B. If called with a single "" arg, we want it to work!  In that
   * case, the 1st arg is an empty string, and get_argument fails on the
   * 2nd arg.  Note that API get_file promises not to access the type
   * argument if the name argument is an empty string.
   */
  if (get_argument(0, AWK_STRING, & cmd) &&
      (get_argument(1, AWK_STRING, & cmdtype) ||
       (! cmd.str_value.len && (nargs == 1)))) {
    const awk_input_buf_t *ibuf;
    const awk_output_buf_t *obuf;
    if (get_file(cmd.str_value.str, cmd.str_value.len, cmdtype.str_value.str, -1, &ibuf, &obuf) && ibuf)
      return make_number(ibuf->fd, result);
    warning(ext_id, _("input_fd: get_file(`%s', `%s') failed to return an input descriptor"), cmd.str_value.str, cmdtype.str_value.str);
  } else if (do_lint) {
    if (nargs < 2)
      lintwarn(ext_id, _("input_fd: called with too few arguments"));
    else
      lintwarn(ext_id, _("input_fd: called with inappropriate argument(s)"));
  }
  return make_number(-1, result);
}

/*  do_output_fd --- Return command's output file descriptor */

static awk_value_t *
do_output_fd(int nargs, awk_value_t *result API_FINFO_ARG)
{
  awk_value_t cmd, cmdtype;

#if gawk_api_major_version < 2
  if (do_lint && nargs > 2)
    lintwarn(ext_id, _("output_fd: called with too many arguments"));
#endif
  /*
   * N.B. If called with a single "" arg, it will not work, since there
   * is no output fd associated the current input file.
   */
  if (get_argument(0, AWK_STRING, & cmd) &&
      get_argument(1, AWK_STRING, & cmdtype)) {
    const awk_input_buf_t *ibuf;
    const awk_output_buf_t *obuf;
    if (get_file(cmd.str_value.str, cmd.str_value.len, cmdtype.str_value.str, -1, &ibuf, &obuf) && obuf)
      return make_number(fileno(obuf->fp), result);
    warning(ext_id, _("output_fd: get_file(`%s', `%s') failed to return an output descriptor"), cmd.str_value.str, cmdtype.str_value.str);
  } else if (do_lint) {
    if (nargs < 2)
      lintwarn(ext_id, _("output_fd: called with too few arguments"));
    else
      lintwarn(ext_id, _("output_fd: called with inappropriate argument(s)"));
  }
  return make_number(-1, result);
}

static awk_ext_func_t func_table[] = {
  API_FUNC_MAXMIN("select", do_select, 5, 3)
  API_FUNC_MAXMIN("select_signal", do_signal, 3, 2)
  API_FUNC_MAXMIN("set_non_blocking", do_set_non_blocking, 2, 1)
  API_FUNC("kill", do_kill, 2)
  API_FUNC_MAXMIN("input_fd", do_input_fd, 2, 1)
  API_FUNC("output_fd", do_output_fd, 2)
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, select, "")
