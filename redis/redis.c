/*
 *  redis.c - Builtin functions that provide interface to the Redis server,
 *  using the hiredis library.
 *
 * 
 *  Author: Paulino Huerta Sanchez, 2014-08-22
 */

/*
 *  Copyright (C) 1986, 1988, 1989, 1991-2004 the Free Software Foundation, Inc.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */
#if defined(__GNUC__) && ((__GNUC__==4 && __GNUC_MINOR__>=2) || (__GNUC__>4))
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

#define GAWKEXTLIB_NOT_NEEDED
#include "common.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hiredis/hiredis.h>


#include <sys/types.h>
#include <sys/stat.h>

#define TOPC   100 //Top Connection
#define INCRPIPE   1000 //the pipeline increments

char **mem_cdo(char **,const char *,int);
char *mem_str(char **,const char *,int);
void  free_mem_str(char **,int);

redisReply * rCommand(int, int, int, const char **);

enum format_type {
   INDEF, CONN, NUMBER, STRING, ARRAY, ST_AR, ST_NUM
};
enum resultArray {
  KEYSTRING, KEYNUMBER
};

struct command {
   char name[90];
   int num;
   enum format_type type[10];
};
int validate(struct command,char *,int *,enum format_type *);
int validate_conn(int,char *,const char *,int *);

awk_value_t * tipoKeys(int,awk_value_t *,const char *);
awk_value_t * tipoPubsub(int,awk_value_t *,const char *);
awk_value_t * tipoGeohash(int,awk_value_t *,const char *);
awk_value_t * tipoInfo(int,awk_value_t *,const char *);
awk_value_t * tipoExec(int,awk_value_t *,const char *);
awk_value_t * tipoSlowlog(int,awk_value_t *,const char *);
awk_value_t * tipoEvalsha(int,awk_value_t *,const char *);
awk_value_t * tipoZrangebylex(int,awk_value_t *,const char *);
awk_value_t * tipoConnect(int,awk_value_t *,const char *);
awk_value_t * tipoScript(int,awk_value_t *,const char *);
awk_value_t * tipoScard(int,awk_value_t *,const char *);
awk_value_t * tipoSpop(int,awk_value_t *,const char *);
awk_value_t * tipoRandomkey(int,awk_value_t *,const char *);
awk_value_t * tipoPipeline(int,awk_value_t *,const char *);
awk_value_t * tipoHincrby(int,awk_value_t *,const char *);
awk_value_t * tipoSismember(int,awk_value_t *,const char *);
awk_value_t * tipoObject(int,awk_value_t *,const char *);
awk_value_t * tipoClientOne(int,awk_value_t *,const char *);
awk_value_t * tipoClientTwo(int,awk_value_t *,const char *);
awk_value_t * tipoSadd(int,awk_value_t *,const char *);
awk_value_t * tipoBitcount(int,awk_value_t *,const char *);
awk_value_t * tipoBitpos(int,awk_value_t *,const char *);
awk_value_t * tipoBitop(int,awk_value_t *,const char *);
awk_value_t * tipoBrpop(int,awk_value_t *,const char *);
awk_value_t * tipoSetbit(int,awk_value_t *,const char *);
awk_value_t * tipoSetrange(int,awk_value_t *,const char *);
awk_value_t * tipoSinter(int,awk_value_t *,const char *);
awk_value_t * tipoSmove(int,awk_value_t *,const char *);
awk_value_t * tipoZincrby(int,awk_value_t *,const char *);
awk_value_t * tipoRestore(int,awk_value_t *,const char *);
awk_value_t * tipoSrandmember(int,awk_value_t *,const char *);
awk_value_t * tipoScan(int,awk_value_t *,const char *);
awk_value_t * tipoLinsert(int,awk_value_t *,const char *);
awk_value_t * tipoGeodist(int,awk_value_t *,const char *);
awk_value_t * tipoGeoradius(int,awk_value_t *,const char *);
awk_value_t * tipoGeoradiusbymember(int,awk_value_t *,const char *);
awk_value_t * tipoGeoradiusWD(int,awk_value_t *,const char *);
awk_value_t * tipoGeoradiusbymemberWD(int,awk_value_t *,const char *);
awk_value_t * tipoSscan(int,awk_value_t *,const char *);
awk_value_t * tipoGetReply(int,awk_value_t *,const char *);
awk_value_t * tipoGetReplyMass(int,awk_value_t *,const char *);
awk_value_t * tipoSelect(int,awk_value_t *,const char *);
awk_value_t * tipoExpire(int,awk_value_t *,const char *);
awk_value_t * tipoGetrange(int,awk_value_t *,const char *);
awk_value_t * tipoSet(int,awk_value_t *,const char *);
awk_value_t * tipoSubscribe(int,awk_value_t *,const char *);
awk_value_t * tipoGetMessage(int,awk_value_t *,const char *);
awk_value_t * tipoZadd(int,awk_value_t *,const char *);
awk_value_t * tipoZrange(int,awk_value_t *,const char *);
awk_value_t * tipoZunionstore(int,awk_value_t *,const char *);
awk_value_t * tipoUnsubscribe(int,awk_value_t *,const char *);
awk_value_t * tipoHmset(int,awk_value_t *,const char *);
awk_value_t * tipoMset(int,awk_value_t *,const char *);
awk_value_t * tipoMget(int,awk_value_t *,const char *);
awk_value_t * tipoHmget(int,awk_value_t *,const char *);
awk_value_t * tipoSort(int,awk_value_t *,const char *);
awk_value_t * tipoSortLimit(int,awk_value_t *,const char *);

awk_value_t * processREPLY(awk_array_t *,awk_value_t *,redisContext *,const char *);

awk_value_t * theReply(awk_value_t *, redisContext *);
char ** getArrayContent(awk_array_t, size_t, const char *, int *);
char ** getArrayContentCont(awk_array_t, size_t, const char *, int *,int);
int getArrayContentSecond(awk_array_t, int, char **);

int theReplyArrayS(awk_array_t);
int theReplyToArray(awk_array_t,const char*,const char);
int theReplyArray(awk_array_t, enum resultArray, size_t);
int theReplyArrayK1(awk_array_t, redisReply *);
int theReplyArray1(awk_array_t, enum resultArray, size_t);
int theReplyScan(awk_array_t,char *);

static  long long pipel[TOPC][2];

static  redisContext *c[TOPC];
static  redisReply *reply;
static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t ext_id;
static awk_bool_t init_redis(void);
static awk_bool_t (*init_func)(void) = init_redis;
static const char *ext_version = PACKAGE_STRING;

static void
array_set(awk_array_t array, const char *sub, awk_value_t *value)
{
	awk_value_t idx;
	set_array_element(array,make_const_string(sub, strlen(sub), & idx),value);
}

int plugin_is_GPL_compatible;

static awk_value_t * do_disconnect(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   int ret=1;   
   int ival;
   awk_value_t val;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_close/redis_disconnect: called with too many arguments"));
    }
#endif
   if(nargs==1) {
    if(!get_argument(0, AWK_NUMBER, & val)) {
     set_ERRNO(_("disconnect/close: argument 1 is present but not is a number"));
     ret=-1;
     return make_number(ret, result);
    }
    ival=val.num_value;
    if(ival >= 0 && ival < TOPC) {
     if(c[ival]!=NULL) {
       redisFree(c[ival]);
       c[ival]=(redisContext *)NULL;
       ret=1;
     }
     else {
       set_ERRNO(_("disconnect/close: the argument does not correspond to a connection"));
       ret=-11;
     }
    }
    else {
     set_ERRNO(_("disconnect/close: argument out of range"));
     ret=-1;
    }
   }
   else {
     set_ERRNO(_("disconnect/close: needs one argument"));
     ret=-1;
   }
   return make_number(ret, result);
}

redisReply * rCommand(int tcdo, int ind, int count, const char ** sts) {
   if(tcdo==-1)  {
     return redisCommandArgv(c[ind],count,sts,NULL);
   }
   else {
     redisAppendCommandArgv(c[tcdo],count,sts,NULL);
     pipel[tcdo][1]++;
     return NULL;
   }
}

static awk_value_t * do_hvals(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hvals: called with too many arguments"));
    }
#endif
   p_value_t=tipoKeys(nargs,result,"hvals");
   return p_value_t;
}

static awk_value_t * do_srandmember(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_srandmember: called with too many arguments"));
    }
#endif
   if(nargs==2) {
     p_value_t=tipoScard(nargs,result,"srandmember");
   }
   else {
     p_value_t=tipoSrandmember(nargs,result,"srandmember");
   }
   return p_value_t;
}

static awk_value_t * do_hscan(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_hscan: called with too many arguments"));
    }
#endif
   p_value_t=tipoSscan(nargs,result,"hscan");
   return p_value_t;
}

static awk_value_t * do_zscan(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zscan: called with too many arguments"));
    }
#endif
   p_value_t=tipoSscan(nargs,result,"zscan");
   return p_value_t;
}

static awk_value_t * do_sscan(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_sscan: called with too many arguments"));
    }
#endif
   p_value_t=tipoSscan(nargs,result,"sscan");
   return p_value_t;
}

static awk_value_t * do_scan(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_scan: called with too many arguments"));
    }
#endif
   p_value_t=tipoScan(nargs,result,"scan");
   return p_value_t;
}

static awk_value_t * do_lpop(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_lpop: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"lpop");
   return p_value_t;
}

static awk_value_t * do_rpop(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_rpop: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"rpop");
   return p_value_t;
}

static awk_value_t * do_spop(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_spop: called with too many arguments"));
    }
#endif
   p_value_t=tipoSpop(nargs,result,"spop");
   return p_value_t;
}

static awk_value_t * do_incrby(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_incrby: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"incrby");
   return p_value_t;
}

static awk_value_t * do_lindex(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_lindex: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"lindex");
   return p_value_t;
}

static awk_value_t * do_decrby(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_decrby: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"decrby");
   return p_value_t;
}

static awk_value_t * do_incrbyfloat(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_incrbyfloat: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"incrbyfloat");
   return p_value_t;
}

static awk_value_t * do_getbit(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_getbit: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"getbit");
   return p_value_t;
}

static awk_value_t * do_bitpos(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_bitpos: called with too many arguments"));
    }
#endif
   p_value_t=tipoBitpos(nargs,result,"bitpos");
   return p_value_t;
}

static awk_value_t * do_setbit(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_setbit: called with too many arguments"));
    }
#endif
   p_value_t=tipoSetbit(nargs,result,"setbit");
   return p_value_t;
}

static awk_value_t * do_pexpire(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_pexpire: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"pexpire");
   return p_value_t;
}

static awk_value_t * do_expire(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_expire: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"expire");
   return p_value_t;
}

static awk_value_t * do_hstrlen(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hstrlen: called with too many arguments"));
    }
#endif
   p_value_t=tipoExpire(nargs,result,"hstrlen");
   return p_value_t;
}

static awk_value_t * do_decr(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_decr: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"decr");
   return p_value_t;
}

static awk_value_t * do_incr(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_incr: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"incr");
   return p_value_t;
}

static awk_value_t * do_persist(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_persist: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"persist");
   return p_value_t;
}

static awk_value_t * do_pttl(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_pttl: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"pttl");
   return p_value_t;
}

static awk_value_t * do_ttl(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_ttl: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"ttl");
   return p_value_t;
}

static awk_value_t * do_type(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_type: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"type");
   return p_value_t;
}

static awk_value_t * do_llen(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_llen: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"llen");
   return p_value_t;
}
static awk_value_t * do_hlen(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_hlen: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"hlen");
   return p_value_t;
}

static awk_value_t * do_strlen(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_strlen: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"strlen");
   return p_value_t;
}

static awk_value_t * do_script(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_script: called with too many arguments"));
    }
#endif
   p_value_t=tipoScript(nargs,result,"script");
   return p_value_t;
}

static awk_value_t * do_bitop(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_bitop: called with too many arguments"));
    }
#endif
   p_value_t=tipoBitop(nargs,result,"bitop");
   return p_value_t;
}

static awk_value_t * do_bitcount(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_bitcount: called with too many arguments"));
    }
#endif
   p_value_t=tipoBitcount(nargs,result,"bitcount");
   return p_value_t;

}
static awk_value_t * do_zcard(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_zcard: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"zcard");
   return p_value_t;
}

static awk_value_t * do_scard(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_scard: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"scard");
   return p_value_t;
}

static awk_value_t * do_echo(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_echo: called with too many arguments"));
    }
#endif
   p_value_t=tipoScard(nargs,result,"echo");
   return p_value_t;
}

static awk_value_t * do_hkeys(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hkeys: called with too many arguments"));
    }
#endif
   p_value_t=tipoKeys(nargs,result,"hkeys");
   return p_value_t;
}

static awk_value_t * do_zrevrank(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_zrevrank: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"zrevrank");
   return p_value_t;
}

static awk_value_t * do_zscore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_zscore: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"zscore");
   return p_value_t;
}

static awk_value_t * do_zrank(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_zrank: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"zrank");
   return p_value_t;
}

static awk_value_t * do_smembers(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_smembers: called with too many arguments"));
    }
#endif
   p_value_t=tipoKeys(nargs,result,"smembers");
   return p_value_t;
}

static awk_value_t * do_pubsub(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_pubsub: called with too many arguments"));
    }
#endif
   p_value_t=tipoPubsub(nargs,result,"pubsub");
   return p_value_t;
}



static awk_value_t * do_hget(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hget: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"hget");
   return p_value_t;
}

static awk_value_t * do_hexists(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hexists: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"hexists");
   return p_value_t;
}

static awk_value_t * do_getset(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_getset: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"getset");
   return p_value_t;
}

static awk_value_t * do_set(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 6)) {
      lintwarn(ext_id, _("redis_set: called with too many arguments"));
    }
#endif
   p_value_t=tipoSet(nargs,result,"set");
   return p_value_t;
}

static awk_value_t * do_publish(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_publish: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"publish");
   return p_value_t;
}

static awk_value_t * do_append(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_append: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"append");
   return p_value_t;
}

static awk_value_t * do_rpoplpush(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_rpoplpush: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"rpoplpush");
   return p_value_t;
}

static awk_value_t * do_rpushx(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_rpushx: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"rpushx");
   return p_value_t;
}

static awk_value_t * do_lpushx(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_lpushx: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"lpushx");
   return p_value_t;
}

static awk_value_t * do_sortStore(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_sortStore: called with too many arguments"));
    }
#endif
   p_value_t=tipoSort(nargs,result,"sortStore");
   return p_value_t;
}

static awk_value_t * do_sortLimitStore(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 6)) {
      lintwarn(ext_id, _("redis_sortLimitStore: called with too many arguments"));
    }
#endif
   p_value_t=tipoSortLimit(nargs,result,"sortLimitStore");
   return p_value_t;
}

static awk_value_t * do_sortLimit(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 6)) {
      lintwarn(ext_id, _("redis_sortLimit: called with too many arguments"));
    }
#endif
   p_value_t=tipoSortLimit(nargs,result,"sortLimit");
   return p_value_t;
}

static awk_value_t * do_sort(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_sort: called with too many arguments"));
    }
#endif
   p_value_t=tipoSort(nargs,result,"sort");
   return p_value_t;
}

static awk_value_t * do_object(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_object: called with too many arguments"));
    }
#endif
   p_value_t=tipoObject(nargs,result,"object");
   return p_value_t;
}

static awk_value_t * do_sismember(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sismember: called with too many arguments"));
    }
#endif
   p_value_t=tipoSismember(nargs,result,"sismember");
   return p_value_t;
}

static awk_value_t * do_lrange(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_lrange: called with too many arguments"));
    }
#endif
   p_value_t=tipoZrange(nargs,result,"lrange");
   return p_value_t;
}

static awk_value_t * do_ltrim(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_ltrim: called with too many arguments"));
    }
#endif
   p_value_t=tipoSetbit(nargs,result,"ltrim");
   return p_value_t;
}

static awk_value_t * do_zcount(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zcount: called with too many arguments"));
    }
#endif
   p_value_t=tipoSmove(nargs,result,"zcount");
   return p_value_t;
}

static awk_value_t * do_zrangeWithScores(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrangeWithScores: called with too many arguments"));
    }
#endif
   p_value_t=tipoZrange(nargs,result,"zrangeWithScores");
   return p_value_t;
}

static awk_value_t * do_zrevrangeWithScores(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrevrangeWithScores: called with too many arguments"));
    }
#endif
   p_value_t=tipoZrange(nargs,result,"zrevrangeWithScores");
   return p_value_t;
}

static awk_value_t * do_zrange(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrange: called with too many arguments"));
    }
#endif
   p_value_t=tipoZrange(nargs,result,"zrange");
   return p_value_t;
}

static awk_value_t * do_zrevrange(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrevrange: called with too many arguments"));
    }
#endif
   p_value_t=tipoZrange(nargs,result,"zrevrange");
   return p_value_t;
}

static awk_value_t * do_hmset(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hmset: called with too many arguments"));
    }
#endif
   p_value_t=tipoHmset(nargs,result,"hmset");
   return p_value_t;
}

static awk_value_t * do_hmget(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_hmget: called with too many arguments"));
    }
#endif
   p_value_t=tipoHmget(nargs,result,"hmget");
   return p_value_t;
}

static awk_value_t * do_mset(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_mset: called with too many arguments"));
    }
#endif
   p_value_t=tipoMset(nargs,result,"mset");
   return p_value_t;
}

static awk_value_t * do_msetnx(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_msetnx: called with too many arguments"));
    }
#endif
   p_value_t=tipoMset(nargs,result,"msetnx");
   return p_value_t;
}

static awk_value_t * do_mget(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_mget: called with too many arguments"));
    }
#endif
   p_value_t=tipoMget(nargs,result,"mget");
   return p_value_t;
}

static awk_value_t * do_connectRedis(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_connect: called with too many arguments"));
    }
#endif
   p_value_t=tipoConnect(nargs,result,"connect");
   return p_value_t;
}

static awk_value_t * do_info(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_info: called with too many arguments"));
    }
#endif
   p_value_t=tipoInfo(nargs,result,"info");
   return p_value_t;
}

static awk_value_t * do_keys(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_keys: called with too many arguments"));
    }
#endif
   p_value_t=tipoKeys(nargs,result,"keys");
   return p_value_t;
}

void free_mem_str(char **p,int n) {
  int i;
  for(i=0;i<n;i++){
    free((void *)p[i]);
  }
  free((void *)p);
}

char **mem_cdo(char **p,const char *st,int i) {
  if(p == NULL){
    p=(char **)malloc(25*sizeof(char *));
  }
  p[i]=(char *)malloc((strlen(st)+1)*sizeof(char));
  strcpy(p[i],st);
  return p;
}

char *mem_str(char **p,const char *st,int i) {
  p[i]=(char *)malloc((strlen(st)+1)*sizeof(char));
  strcpy(p[i],st);
  return p[i];
}

int validate_conn(int conn,char *str,const char *command,int *pconn){
  int i;
  if(conn>=INCRPIPE && pipel[conn-INCRPIPE][0]) {
    conn-=INCRPIPE;
    *pconn=conn;
  }
  for(i=0;i<TOPC;i++) {
    if(conn==i){
      break;
    }
  }
  if(i==TOPC) {
   sprintf(str,"%s: connection %d out of range",command,conn);
   return 0;
  } 
  if(c[conn]==(redisContext *)NULL){
   sprintf(str,"%s: connection error for number %d",command,conn);
   return 0;
  }
  return 1;
}

int getArrayContentSecond(awk_array_t array,int cf,char **sts){
  size_t i,j,count;
  awk_value_t idx,val;
  get_element_count(array, &count);
  j=cf-1;
  for(i=cf;i<cf+count;i++) {
     get_array_element(array,make_number(i-j,&idx),AWK_STRING,&val);
     mem_str(sts,val.str_value.str,i);
  }
  return i;
}

char **getArrayContent(awk_array_t array,size_t from,const char *command, int *cnt) {
   awk_value_t idx,val;
   char **sts;
   size_t i,j,count;
   j=from-1;
   get_element_count(array, &count);
   sts=(char **)malloc((from+count)*sizeof(char *));
   mem_str(sts,command,0);
   for(i=from;i<from+count;i++) {
     get_array_element(array,make_number(i-j,&idx),AWK_STRING,&val);
     mem_str(sts,val.str_value.str,i);
   }
   *cnt=from+count;
   return sts;
}

char **getArrayContentCont(awk_array_t array,size_t from,const char *command, int *cnt, int add) {
   char **sts;
   awk_value_t idx,val;
   size_t i,j,count;
   j=from-1;
   get_element_count(array, &count);
   sts=(char **)malloc((from+count+add)*sizeof(char *));
   mem_str(sts,command,0);
   for(i=from;i<from+count;i++) {
     get_array_element(array,make_number(i-j,&idx),AWK_STRING,&val);
     mem_str(sts,val.str_value.str,i);
   }
   *cnt=from+count;
   return sts;
}

int theReplyArrayS(awk_array_t array){
    size_t j;
    char str[15];
    awk_value_t tmp;
    if(reply->element[1]->elements > 0) {
      sprintf(str,"%d",1);
      array_set(array,str,
         make_const_user_input(reply->element[0]->str,reply->element[0]->len, & tmp));
      for(j=0; j < reply->element[1]->elements ;j++) {
        sprintf(str,"%zu",j+2);
	array_set(array,str,
	make_const_user_input(reply->element[1]->element[j]->str,reply->element[1]->element[j]->len, & tmp));
      }
      if(strcmp(reply->element[0]->str,"0")==0) {
        return 0;
      }
    }
    else {
      if(strcmp(reply->element[0]->str,"0")==0) {
        return 0;
      }
      else {
       sprintf(str,"%d",1);
       array_set(array,str,
         make_const_user_input(reply->element[0]->str,reply->element[0]->len, & tmp));
      }
    }
    return 1;
}

int theReplyArrayK1(awk_array_t array, redisReply *rep ){
    size_t j;
    char str[15],str1[15]; 
    awk_value_t tmp,value,ind;
    awk_array_t a_cookie;
    if(rep->elements==0) {
      return 0;
    }
    if(rep->elements>0) {
     for (j = 0; j < rep->elements; j++) {
         sprintf(str, "%zu", j+1);
	 if(rep->element[j]->type==REDIS_REPLY_ARRAY) {
	   make_const_string(str,strlen(str), & ind);
	   a_cookie = create_array();
	   value.val_type = AWK_ARRAY;
	   value.array_cookie = a_cookie;
           set_array_element(array,&ind,&value);
	   a_cookie = value.array_cookie;
	   theReplyArrayK1(a_cookie,rep->element[j]);
	 }
	 if(rep->element[j]->type==REDIS_REPLY_INTEGER) {
	   sprintf(str1, "%lld", rep->element[j]->integer);
           array_set(array,str,make_const_string(str1,strlen(str1),& tmp));
         }
         if(rep->element[j]->type==REDIS_REPLY_STATUS){
	   if(strcmp(rep->element[j]->str,"OK")==0) {
             array_set(array,str,make_const_string("1",1, & tmp));
	   }
         }
	 if(rep->element[j]->type==REDIS_REPLY_STRING) {
	   if(rep->element[j]->str==NULL) {
             array_set(array,str,make_null_string(& tmp));
	   }
	   else {
             array_set(array,str,
                make_const_user_input(rep->element[j]->str,rep->element[j]->len, & tmp));
	   }
         }
       }
    }
    return 1;
}

int theReplyToArray(awk_array_t array,const char* RS,const char FS){
    char str[240], *pstr, *pch, *psep, *pkey, *pval;
    awk_value_t tmp;
    if(reply->str==NULL) {
      return 0;
    }
    // string to array: record separator in string is '\r\n'
    pstr=reply->str;
    pch = strtok(pstr,RS);
    while(pch!=NULL) {
      // make key/value from each record
      // obtain pkey and pval
      strcpy(str,pch);
      psep=strchr(str,FS); 
      if(psep!=NULL) {
        *psep='\0';
        pkey=str;
        pval=psep+1;
        array_set(array,pkey,make_const_string(pval,strlen(pval), & tmp));
      }
      pch = strtok(NULL,RS);
    }
    return 1;
}

int theReplyArray(awk_array_t array, enum resultArray r,size_t incr){
    size_t j;
    char str[15],str1[15]; 
    awk_value_t tmp, idx, val2;
    if(reply->elements==0) {
      return 0;
    }
    if(reply->elements>0) {
     for (j = 0; j < reply->elements; j+=incr) {
       if(r==KEYNUMBER) {
         sprintf(str, "%zu", j+1);
	 if(reply->element[j]->type==REDIS_REPLY_INTEGER) {
	   sprintf(str1, "%lld", reply->element[j]->integer);
           array_set(array,str,make_const_string(str1,strlen(str1),& tmp));
         }
	 if(reply->element[j]->type==REDIS_REPLY_STRING || reply->element[j]->type==REDIS_REPLY_STATUS) {
	   if(reply->element[j]->str==NULL) {
             array_set(array,str,make_null_string(& tmp));
	   }
	   else {
             array_set(array,str,
                make_const_user_input(reply->element[j]->str,reply->element[j]->len, & tmp));
	   }
         }
       }
       else {
        if(r==KEYSTRING) {
	   if(reply->element[j+1]->type==REDIS_REPLY_INTEGER) {
              set_array_element(array,make_const_string(reply->element[j]->str,reply->element[j]->len,&idx),
                  make_number(reply->element[j+1]->integer, &val2));  
           }
	   if(reply->element[j+1]->type==REDIS_REPLY_STRING) {
              set_array_element(array,make_const_string(reply->element[j]->str,reply->element[j]->len,&idx),
                  make_const_string(reply->element[j+1]->str,reply->element[j+1]->len,&val2));
           }
	}
       }
     }
    }
    return 1;
}

int theReplyArray1(awk_array_t array, enum resultArray r,size_t incr){
    size_t j;
    char str[15]; 
    awk_value_t tmp;
    for (j = 0; j < reply->elements; j+=incr) {
       if(r==KEYNUMBER) {
         sprintf(str, "%zu", j+1);
	 if(reply->element[j]->str==NULL) {
           array_set(array,str,make_null_string(& tmp));
	 }
	 else {
           array_set(array,str,
                make_const_user_input(reply->element[j]->str,reply->element[j]->len, & tmp));
         }
       }
       else {
        if(r==KEYSTRING) {
            array_set(array,reply->element[j]->str,
               make_const_user_input(reply->element[j+1]->str,reply->element[j+1]->len, & tmp));
	}
       }
    }
    return 1;
}

int theReplyScan(awk_array_t array, char *first) {
   size_t j;
   char str[15];
   awk_value_t tmp;
   strcpy(first,reply->element[0]->str);
   for(j=0; j < reply->element[1]->elements ;j++) { 
    sprintf(str,"%zu",j+1);
    array_set(array,str,
      make_const_user_input(reply->element[1]->element[j]->str,reply->element[1]->element[j]->len, & tmp));
   }
   return 1;
}

awk_value_t * processREPLY(awk_array_t *array, awk_value_t *result, redisContext *context, const char *command) {
   awk_value_t *pstr;
   int ret=1;
   enum resultArray k=KEYNUMBER;
   pstr=NULL;
   pstr=theReply(result,context);
   if((pstr!=NULL) && (command==NULL)) {
     freeReplyObject(reply);
     return pstr;
   }
   if ((reply->type == REDIS_REPLY_ARRAY) && (command == NULL)) {
     freeReplyObject(reply);
     return NULL;
   }
   if (reply->type == REDIS_REPLY_ARRAY || strcmp(command,"tipoInfo") == 0 || strcmp(command,"tipoClient") == 0) {
     if(strcmp(command,"tipoExec")==0) {
        ret=theReplyArrayK1(array,reply);
     }
     if(strcmp(command,"tipoScan")==0) {
        ret=theReplyArrayS(array);
     }
     if(strcmp(command,"theRest1")==0) {  // for the pubsub function
       ret=theReplyArray(array,KEYSTRING,2);
     }
     if(strcmp(command,"theRest")==0) {  // for the rest
       ret=theReplyArray(array,k,1);
     }
     if(strcmp(command,"tipoInfo")==0) {
        ret=theReplyToArray(array,"\r\n",':');
     }
     if(strcmp(command,"tipoClient")==0) {
        ret=theReplyToArray(array,"\n",' ');
     }
     if(ret==1) {
       pstr=make_number(1, result);
     }
     else {
       pstr=make_number(0, result);
     }
   }
   freeReplyObject(reply);
   return pstr;
}

awk_value_t * theReply(awk_value_t *result, redisContext *conn) {
    awk_value_t *pstr;
    pstr=NULL;
    
    if(conn->err!=0){
      set_ERRNO(_(conn->errstr));
      return make_number(-1, result);
    }

    if(reply->type==REDIS_REPLY_STATUS || reply->type==REDIS_REPLY_STRING){
       if(reply->type==REDIS_REPLY_STATUS){
	 if(strcmp(reply->str,"OK")==0) {
           pstr=make_number(1,result);
	 }
       }
       if(pstr==NULL) {
         if(reply->str==NULL) {
           pstr=make_nul_string(result);
         }
	 else {
           pstr=make_user_input_malloc(reply->str,reply->len,result);
         }
       }
    }
    if(reply->type==REDIS_REPLY_ERROR){
      set_ERRNO(_(reply->str));
      pstr=make_number(-1, result);
    }
    if(reply->type==REDIS_REPLY_NIL){
      pstr=make_nul_string(result);
    }
    if(reply->type==REDIS_REPLY_INTEGER) {
      pstr=make_number(reply->integer, result);
    }
    return pstr;
}

awk_value_t * tipoExec(int nargs,awk_value_t *result,const char *command) {
  int r,ival;
  struct command valid;
  char str[240];
  awk_value_t val,  array_param, *pstr;
  awk_array_t array_ou;
  enum format_type there[2];
  int pconn=-1;
  pstr=NULL;
  if(nargs==2) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=ARRAY;
    valid.num=2;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s",command);
      pstr=processREPLY(array_ou,result,c[ival],"tipoExec");
    }
    else {
      redisAppendCommand(c[pconn],"%s",command);
      pipel[pconn][1]++;
      return make_number(1, result);
    }
  }
  else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSlowlog(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt;
  struct command valid;
  char str[240], **sts;
  size_t getlen,get;
  awk_value_t val,  array_param, *pstr;
  awk_array_t array_ou;
  enum format_type there[4];
  pconn=-1;
  getlen=get=cnt=0;
  pstr=NULL;
  sts=(char **)NULL;
  array_ou=NULL;
  if(nargs>=2 && nargs<=4) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.num=2;
    if(nargs==3){
      valid.type[2]=ARRAY;
      get=1;
      valid.num=3;
    }
    if(nargs==4){
      valid.type[2]=STRING;
      valid.type[3]=ARRAY;
      getlen=1;      
      valid.num=4;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    if(get){ 
      get_argument(2, AWK_ARRAY, & array_param);
      array_ou = array_param.array_cookie;
    }
    if(getlen){ 
      get_argument(2, AWK_STRING, & val);
      mem_cdo(sts,val.str_value.str,++cnt);
      get_argument(3, AWK_ARRAY, & array_param);
      array_ou = array_param.array_cookie;
    }
    
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);

    if(pconn==-1) {
      if(getlen || get) {
        pstr=processREPLY(array_ou,result,c[ival],"tipoExec");
      }
      else {
        pstr=processREPLY(NULL,result,c[ival],NULL);
      }
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs between two and four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoEvalsha(int nargs,awk_value_t *result,const char *command) {
  int count,r,ival;
  struct command valid;
  char str[240],**sts;
  awk_value_t val, val1, val2, array_param, *pstr;
  awk_array_t array_in, array_ou;
  enum format_type there[5];
  int pconn=-1;
  pstr=NULL;
  sts=NULL;
  if(nargs==5) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=ARRAY;
    valid.type[4]=ARRAY;
    valid.num=5;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_ARRAY, & array_param);
    array_in = array_param.array_cookie;
    get_argument(4, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    sts=getArrayContent(array_in,3,command,&count);
    mem_str(sts,val1.str_value.str,1);
    mem_str(sts,val2.str_value.str,2);
    if(pconn==-1) {
      reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      pstr=processREPLY(array_ou,result,c[ival],"tipoExec");
    }
    else {
      redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      pipel[pconn][1]++;
      return make_number(1, result);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s needs five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZrangebylex(int nargs,awk_value_t *result,const char *command) {
  int r,ival;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, array_param, *pstr;
  awk_array_t array;
  enum format_type there[5];
  int pconn=-1;

  if(nargs==5) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=STRING;
    valid.type[4]=STRING;
    valid.num=5;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_argument(3, AWK_STRING, & val2);
    get_argument(4, AWK_STRING, & val3);
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      return make_number(1, result);
    }
  }
  else {
    sprintf(str,"%s needs five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSscan(int nargs,awk_value_t *result,const char *command) {
  int r,ival,ival2;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, array_param, *pstr;
  awk_array_t array;
  enum format_type there[5];
  int pconn=-1;

  if(nargs==4 || nargs==5) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=ARRAY;
    if(nargs==5) {
     valid.type[4]=STRING;
     valid.num=5;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_NUMBER, & val2);
    ival2=val2.num_value;
    get_argument(3, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    if(nargs==5) {
      get_argument(4, AWK_STRING, & val3);
      if(pconn==-1) {
        reply = redisCommand(c[ival],"%s %s %d MATCH %s",command,val1.str_value.str,ival2,val3.str_value.str);
      }
      else {
        redisAppendCommand(c[pconn],"%s %s %d MATCH %s",command,val1.str_value.str,ival2,val3.str_value.str);
        pipel[pconn][1]++;
        return make_number(1, result);
      }
    }
    else {
      if(pconn==-1) {
        reply = redisCommand(c[ival],"%s %s %d",command,val1.str_value.str,ival2);
      }
      else {
        redisAppendCommand(c[pconn],"%s %s %d",command,val1.str_value.str,ival2);
        pipel[pconn][1]++;
        return make_number(1, result);
      }
    }
    pstr=processREPLY(array,result,c[ival],"tipoScan");
  }
  else {
    sprintf(str,"%s needs three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoScan(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, array_param, *pstr;
  awk_array_t array;
  enum format_type there[4];
  pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==3 || nargs==4) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=NUMBER;
    valid.type[2]=ARRAY;
    if(nargs==4) {
     valid.type[3]=STRING;
     valid.num=4;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    if(nargs==4) {
      get_argument(3, AWK_STRING, & val2);
      mem_cdo(sts,"match",++cnt);
      mem_cdo(sts,val2.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"tipoScan");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

static awk_value_t * do_getReply(int nargs, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_getReply: called with too many arguments"));
    }
#endif
   p_value_t=tipoGetReply(nargs,result,"getReply");
   return p_value_t;
}

static awk_value_t * do_getReplyInfo(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_getReplyInfo: called with too many arguments"));
    }
#endif
   p_value_t=tipoGetReply(nargs,result,"getReplyInfo");
   return p_value_t;
}

static awk_value_t * do_getReplyMass(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) { 
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_getReplyMass: called with too many arguments"));
    }
#endif
   p_value_t=tipoGetReplyMass(nargs,result,"getReplyMass");
   return p_value_t;
}

static awk_value_t * do_pipeline(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
   awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_pipeline: called with too many arguments"));
    }
#endif
   p_value_t=tipoPipeline(nargs,result,"pipeline");
   return p_value_t;
}

awk_value_t * tipoPipeline(int nargs,awk_value_t *result,const char *command) {
  int ret,r,ival;
  struct command valid;
  char str[240];
  awk_value_t val;
  enum format_type there[1];
  int pconn=-1;

  if(nargs==1) {
    strcpy(valid.name,command); 
    valid.num=1;
    valid.type[0]=CONN;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }

    if(pipel[ival][0]) {
      sprintf(str,"%s: exists already a pipe for this connection", command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    pipel[ival][0]=1;
    ret=ival+INCRPIPE;
  }
  else {
    sprintf(str,"%s needs one argument",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return make_number(ret, result);
}

awk_value_t * tipoSelect(int nargs,awk_value_t *result,const char *command) {
  int r,ival,ival1;
  struct command valid;
  char str[240];
  awk_value_t val,val1, *pstr;
  enum format_type there[2];
  int pconn=-1;

  if(nargs==2) {
    strcpy(valid.name,command); 
    valid.num=2;
    valid.type[0]=CONN;
    valid.type[1]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_NUMBER, & val1);
    ival1=val1.num_value;
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %d",command,ival1);
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    else {
      redisAppendCommand(c[pconn],"%s %d",command,ival1);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoRandomkey(int nargs,awk_value_t *result,const char *command) {
  int r,ival,cnt;
  struct command valid;
  char str[240], **sts;
  size_t config;
  config=cnt=0;
  awk_value_t val, *pstr;
  enum format_type there[1];
  int pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==1) {
    strcpy(valid.name,command); 
    valid.num=1;
    valid.type[0]=CONN;
    if(strcmp(command,"configResetStat")==0) {
       config = 1;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(!config) {
      sts=mem_cdo(sts,command,cnt);
    }
    else {
      sts=mem_cdo(sts,"config",cnt);
      mem_cdo(sts,"resetstat",++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs one argument",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoClientOne(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt;
  struct command valid;
  char str[240], the_command[25], **sts;
  awk_value_t val, *pstr;
  enum format_type there[1];
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==1) {
   valid.num=1;
   valid.type[0]=CONN;
   strcpy(valid.name,command); 
   if(strcmp(command,"clientGetName")==0) {
     strcpy(the_command,"getname");
   }
   if(!validate(valid,str,&r,there)) {
     set_ERRNO(_(str));
     return make_number(-1, result);
   }
   get_argument(0, AWK_NUMBER, & val);
   ival=val.num_value;
   if(!validate_conn(ival,str,command,&pconn)) {
       set_ERRNO(_(str));
       return make_number(-1, result);
   }
   sts=mem_cdo(sts,"client",cnt);
   mem_cdo(sts,the_command,++cnt);
   reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
   if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
   }
   free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs one argument",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoClientTwo(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt;
  struct command valid;
  char str[240], the_command[25], the_command_arg[25], **sts;
  awk_value_t val, array_param, *pstr;
  awk_array_t array;
  enum format_type there[2];
  array=NULL;
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  the_command_arg[0]='\0';
  pstr=make_number(1, result);
  if(nargs==2) {
   valid.num=2;
   valid.type[0]=CONN;
   valid.type[1]=ST_AR;
   strcpy(valid.name,command); 
   if(strcmp(command,"clientPause")==0) {
     strcpy(the_command,"pause");
   }
   if(strcmp(command,"clientSetName")==0) {
     strcpy(the_command,"setname");
   }
   if(strcmp(command,"clientList")==0) {
     strcpy(the_command,"list");
   }
   if(strcmp(command,"clientKillAddr")==0) {
     strcpy(the_command,"kill");
     strcpy(the_command_arg,"addr");
   }
   if(strcmp(command,"clientKillId")==0) {
     strcpy(the_command,"kill");
     strcpy(the_command_arg,"id");
   }
   if(strcmp(command,"clientKillType")==0) {
     strcpy(the_command,"kill");
     strcpy(the_command_arg,"type");
   }
   if(!validate(valid,str,&r,there)) {
     set_ERRNO(_(str));
     return make_number(-1, result);
   }
   get_argument(0, AWK_NUMBER, & val);
   ival=val.num_value;
   if(!validate_conn(ival,str,command,&pconn)) {
       set_ERRNO(_(str));
       return make_number(-1, result);
   }
   sts=mem_cdo(sts,"client",cnt);
   mem_cdo(sts,the_command,++cnt);
   if(the_command_arg[0]!='\0') {
     mem_cdo(sts,the_command_arg,++cnt);
   }

   if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & val);
      mem_cdo(sts,val.str_value.str,++cnt);
   }
   else { // there[1]==ARRAY
      get_argument(1, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
   }
   reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
   if(pconn==-1) {
      if(strcmp(the_command,"list") == 0) {
        pstr=processREPLY(array,result,c[ival],"tipoClient");
      }
      else {
          pstr=processREPLY(NULL,result,c[ival],NULL);
      }
   }
   free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoConnect(int nargs,awk_value_t *result,const char *command) {
  int ret,r,port=6379;
  size_t i;
  struct command valid;
  char str[240];
  awk_value_t val, val1;
  enum format_type there[2];
  char address[16]="127.0.0.1";
  if(nargs==0||nargs==1||nargs==2) {
    strcpy(valid.name,command); 
    if(nargs==1){
      valid.num=1;
      valid.type[0]=STRING;
    }
    if(nargs==2){
      valid.num=2;
      valid.type[0]=STRING;
      valid.type[1]=NUMBER;
    }
    if(nargs!=0) {
     if(!validate(valid,str,&r,there)) {
       set_ERRNO(_(str));
       return make_number(-1, result);
     }
    }
    if(nargs>=1) {
     get_argument(0, AWK_STRING, & val);
     strcpy(address,val.str_value.str);
    }
    if(nargs==2) {
     get_argument(1, AWK_NUMBER, & val1);
     port=val1.num_value;
    }
    for(i=0;i<TOPC;i++) {
      if(!c[i]) {
        break;
      }
    }
    if(i==TOPC) {
     set_ERRNO(_("connection: not possible, exceeds the connection limit"));
     ret=-1;
     return make_number(ret, result);
    }
    c[i] = redisConnect((const char*)address, port);
    if (c[i]->err) {
      sprintf(str,"connection error: %s\n", c[i]->errstr);
      set_ERRNO(_(str));
      c[i]=(redisContext *)NULL;
      ret=-1;
    }
    else {
      ret=i;
    }
    return make_number(ret, result);
  }
  else {
    sprintf(str,"%s maximum of two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
}

awk_value_t * tipoScript(int nargs,awk_value_t *result,const char *command) {
  int r,pconn,ival,flush,kill,load,exists,count,earg,cnt=0;
  struct command valid;
  char str[240],**sts;
  awk_value_t val0,val,val1,array_param, *pstr;
  awk_array_t array_in,array_ou;
  enum format_type there[4];
  earg=flush=kill=load=exists=0;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  pconn=-1;
  array_ou=0;
  if(nargs>=2 && nargs<=4) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    get_argument(1, AWK_STRING, & val);
    if((strcmp(val.str_value.str,"exists")==0)) {
      if(nargs==4) {
        exists=1;
        valid.type[2]=ARRAY;
        valid.type[3]=ARRAY;
        valid.num=4;
      }
      else {
        earg=1;
      }
    }
    else if((strcmp(val.str_value.str,"load")==0)) {
      if(nargs==3) {
        load=1;
        valid.type[2]=STRING;
        valid.num=3;
      }
      else {
        earg=1;
      }
    }
    else if((strcmp(val.str_value.str,"kill")==0)) {
      if(nargs==2) {
        kill=1;
        valid.num=2;
      }
      else {
        earg=1;
      }
    }
    else if((strcmp(val.str_value.str,"flush")==0)) {
      if(nargs==2) {
        flush=1;
        valid.num=2;
      }
      else {
        earg=1;
      }
    }
    else { 
      sprintf(str,"%s needs a valid subcommand",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(earg) {
      sprintf(str,"%s %s, wrong number of arguments",command,val.str_value.str);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val0);
    ival=val0.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(exists){
      get_argument(2, AWK_ARRAY, & array_param);
      array_in = array_param.array_cookie;
      sts=getArrayContent(array_in,2,command,&count);
      mem_str(sts,val.str_value.str,1);
      get_argument(3, AWK_ARRAY, & array_param);
      array_ou = array_param.array_cookie;
      cnt=count-1;
    }
    if(load){
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,"LOAD",++cnt);
      get_argument(2, AWK_STRING, & val1);
      mem_cdo(sts,val1.str_value.str,++cnt);
    }
    if(flush) {
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,"FLUSH",++cnt);
    }
    if(kill) {
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,"KILL",++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
       pstr=processREPLY(array_ou,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs two, three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoScard(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, *pstr;
  enum format_type there[2];
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==2) {
    strcpy(valid.name,command); 
    valid.num=2;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoPubsub(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt=0;
  size_t channels, numpat, numsub, i, nkeys_w;
  struct command valid;
  awk_array_t array, array_w;
  char str[240], **sts;
  awk_value_t val, val1, val2, idx, array_param, array_param_w, *pstr;
  enum format_type there[3];
  channels = numpat = numsub = 0;
  pstr=make_number(1, result);
  pconn=-1;
  sts=(char **)NULL;
  array=NULL;
  if(nargs >= 2 && nargs <= 4) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    if(nargs == 2) {
      valid.num=2;
    }
    if(nargs == 4) {
      valid.num = 4;
      valid.type[2]=ST_AR;
      valid.type[3]=ARRAY;
    }
    if(nargs==3) {
      valid.num=3;
      valid.type[2]=ARRAY;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    if (strcmp(val.str_value.str,"channels") == 0) {
      channels = 1;
    }
    if (strcmp(val.str_value.str,"numsub") == 0) {
      numsub = 1;
    }
    if (strcmp(val.str_value.str,"numpat") == 0) {
      numpat = 1;
    }
    if(!(numsub || numpat || channels)) {
      sprintf(str,"%s needs a second argument with a value between numsub, numpat or channels",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if (numpat && nargs != 2) {
      sprintf(str,"%s with numpat subcommand needs only two arguments",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if (channels && nargs == 2) {
      sprintf(str,"%s with channels subcommand needs three or four arguments",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if (numsub && nargs != 4) {
      sprintf(str,"%s with numsub subcommand needs four arguments",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    if(numsub && nargs == 4) {
      get_argument(2, AWK_ARRAY, & array_param_w); // getting channels
      get_argument(3, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      array_w = array_param_w.array_cookie;
      get_element_count(array_w, &nkeys_w);
      for(i=1; i <= nkeys_w; i++) {
        get_array_element(array_w,make_number(i,&idx),AWK_STRING,&val2);
        mem_cdo(sts,val2.str_value.str,++cnt);
      }
    }
    if(channels && nargs == 4) {
      get_argument(2, AWK_STRING, & val1);
      get_argument(3, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      mem_cdo(sts,val1.str_value.str,++cnt);
    }
    if(channels && nargs == 3) {
      get_argument(2, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      if(channels) {
        pstr=processREPLY(array,result,c[ival],"theRest");
      }
      if(numsub) {
        pstr=processREPLY(array,result,c[ival],"theRest1");
      }
      if(numpat) {
        pstr=processREPLY(NULL,result,c[ival],NULL);
      }
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs between two and four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}
awk_value_t * tipoSpop(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,cnt=0;
  size_t withcount=0;
  struct command valid;
  awk_array_t array;
  char str[240], **sts;
  awk_value_t val, val1, array_param, *pstr;
  enum format_type there[3];
  pstr=make_number(1, result);
  pconn=-1;
  sts=(char **)NULL;
  if(nargs==2 || nargs==4) {
    strcpy(valid.name,command); 
    valid.num=2;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    if(nargs==4) {
      valid.num=4;
      valid.type[2]=NUMBER;
      valid.type[3]=ARRAY;
      withcount=1;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    if(withcount) {
      get_argument(2, AWK_STRING, & val1);
      get_argument(3, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      mem_cdo(sts,val1.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      if(withcount) {
        pstr=processREPLY(array,result,c[ival],"theRest");
      }
      else {
        pstr=processREPLY(NULL,result,c[ival],NULL);
      }
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs two or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBrpop(int nargs,awk_value_t *result,const char *command) {
   int count,r,ival,pconn,cnt=0;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, val1, val2, array_param, *pstr;
   awk_array_t array_in, array_ou;
   enum format_type there[4];
   pconn=-1;   
   pstr=make_number(1, result);
   sts=(char **)NULL;
   if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=ST_AR;
    valid.type[2]=ARRAY;
    valid.type[3]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(3, AWK_STRING, & val2);
    if(there[1]==ARRAY){
      get_argument(1, AWK_ARRAY, & array_param);
      array_in = array_param.array_cookie;
      sts=getArrayContent(array_in,1,command,&count);
      mem_str(sts,val2.str_value.str,count++);
      cnt=count-1;
    }
    if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & val1);
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,val1.str_value.str,++cnt);
      mem_cdo(sts,val2.str_value.str,++cnt);
    }
    get_argument(2, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array_ou,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSinter(int nargs,awk_value_t *result,const char *command) {
   int count,r,ival;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, array_param, *pstr;
   awk_array_t array_in, array_ou;
   enum format_type there[3];
   int pconn=-1;   
   pstr=make_number(1, result);
   if(nargs==3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=ARRAY;
    valid.type[2]=ARRAY;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_ARRAY, & array_param);
    array_in = array_param.array_cookie;
    sts=getArrayContent(array_in,1,command,&count);
    get_argument(2, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    reply = (redisReply *)rCommand(pconn,ival,count,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array_ou,result,c[ival],"theRest");
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoUnsubscribe(int nargs,awk_value_t *result,const char *command) {
  int count,r,pconn,cnt=0;
  char **sts;
  int ival;
  struct command valid;
  char str[240];
  awk_value_t val, array_param, name_channel, *pstr;
  awk_array_t array;
  enum format_type there[2];
  pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==2 || nargs==1) {
    valid.num=1;
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    if(nargs==2) {
      valid.num=2;
      valid.type[1]=ST_AR;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(nargs==2) {
     if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & name_channel);
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,name_channel.str_value.str,++cnt);
     }
     else { // there[1]==ARRAY
      get_argument(1, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,1,command,&count);
      cnt=count-1;
     }
    }
    else {
      sts=mem_cdo(sts,command,cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    free_mem_str(sts,cnt+1);
    pstr=make_number(1, result);
  }
  else {
    sprintf(str,"%s needs one or two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSubscribe(int nargs,awk_value_t *result,const char *command) {
  int r,count,pconn,cnt,ival;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, name_channel, *pstr;
  awk_array_t array;
  enum format_type there[3];
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  strcpy(valid.name,command); 
  valid.type[0]=CONN;
  if(nargs==2) {
    valid.num=2;
    valid.type[0]=CONN;
    valid.type[1]=ST_AR;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & name_channel);
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,name_channel.str_value.str,++cnt);
    }
    else {
      // is an array calling to CommandArgv
      get_argument(1, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,1,command,&count);
      cnt=count-1;  
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
        pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSadd(int nargs,awk_value_t *result,const char *command) {
  int r,count,cnt,ival,pconn;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, name_set, *pstr;
  awk_array_t array;
  enum format_type there[3];
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=NUMBER;
    valid.type[1]=STRING;
    valid.type[2]=ST_AR;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    sts=mem_cdo(sts,command,cnt);
    get_argument(1, AWK_STRING, & name_set);
    if(there[2]==STRING) {
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,name_set.str_value.str,++cnt);
      get_argument(2, AWK_STRING, & val);
      mem_cdo(sts,val.str_value.str,++cnt);
      count=cnt+1;
    }
    else {
      // is an array calling to commandARGV
      get_argument(2, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,2,command,&count);
      mem_str(sts,name_set.str_value.str,1);
    }  
    reply = (redisReply *)rCommand(pconn,ival,count,(const char **)sts);
    if(pconn==-1) {
        pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGetrange(int nargs,awk_value_t *result,const char *command) {
  int cnt,ival,r;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  int pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
     sts=mem_cdo(sts,command,cnt);
     mem_cdo(sts,val1.str_value.str,++cnt);
     mem_cdo(sts,val2.str_value.str,++cnt);
     mem_cdo(sts,val3.str_value.str,++cnt);
     reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
     if(pconn==-1) {
       pstr=processREPLY(NULL,result,c[ival],NULL);
     }
     free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZadd(int nargs,awk_value_t *result,const char *command) {
  int ival,r,count,cnt;
  struct command valid;
  char str[240], **sts;
  awk_value_t array_param, val, val1, val2, val3, *pstr;
  awk_array_t array;
  enum format_type there[4];
  int pconn=-1;
  sts=(char **)NULL;
  cnt=0;
  pstr=make_number(1,result);
  strcpy(valid.name,command);
  valid.type[0]=CONN;
  valid.type[1]=STRING;
  if(nargs==4) {
    valid.num=4;
    valid.type[2]=NUMBER;
    valid.type[3]=STRING;
  }
  if(nargs==3) {
    valid.num=3;
    valid.type[2]=ARRAY;
  }
  if(nargs!=3 && nargs!=4) {
    sprintf(str,"%s needs three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  if(!validate(valid,str,&r,there)) {
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  get_argument(0, AWK_NUMBER, & val);
  ival=val.num_value;
  if(!validate_conn(ival,str,command,&pconn)) {
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  get_argument(1, AWK_STRING, & val1);
  if(nargs==4) {
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
       pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    sts=getArrayContent(array,2,command,&count);
    mem_str(sts,val1.str_value.str,1);
    reply = (redisReply *)rCommand(pconn,ival,count,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,count);
  }
  return pstr;
}

awk_value_t * tipoSetrange(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0; 
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBitcount(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  pconn=-1;
  pstr=make_number(1, result);
  sts=(char **)NULL;
  if(nargs==4||nargs==2) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.num=2;
    if(nargs==4) {
      valid.num=4;
      valid.type[2]=NUMBER;
      valid.type[3]=NUMBER;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    if(nargs==4) {
      get_argument(2, AWK_STRING, & val2);
      get_argument(3, AWK_STRING, & val3);
      mem_cdo(sts,val2.str_value.str,++cnt);
      mem_cdo(sts,val3.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs two or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBitpos(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, val4, *pstr;
  enum format_type there[5];
  pconn=-1;
  pstr=make_number(1, result);
  sts=(char **)NULL;
  if(nargs==3||nargs==4||nargs==5) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    if(nargs==4){
     valid.num=4;
     valid.type[3]=NUMBER;
    }
    if(nargs==5){
     valid.num=5;
     valid.type[3]=NUMBER;
     valid.type[4]=NUMBER;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    if(nargs==4){
      get_argument(3, AWK_NUMBER, & val3);
      mem_cdo(sts,val3.str_value.str,++cnt);
    }
    if(nargs==5){
      get_argument(3, AWK_NUMBER, & val3);
      get_argument(4, AWK_NUMBER, & val4);
      mem_cdo(sts,val3.str_value.str,++cnt);
      mem_cdo(sts,val4.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs three, four or five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGeodist(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, val4, *pstr;
  enum format_type there[5];
  pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==4||nargs==5) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    valid.type[3]=STRING;
    if(nargs==5){
     valid.num=5;
     valid.type[4]=STRING;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    if(nargs==5){
      get_argument(4, AWK_STRING, & val4);
      mem_cdo(sts,val4.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four or five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGeoradius(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  size_t withboth,without;
  struct command valid;
  char str[240], **sts;
  awk_array_t array;
  awk_value_t val, val1, val3, val4, val5, val6, val7, val8, array_param, *pstr;
  enum format_type there[9];
  without=withboth=0;
  pconn=-1;
  pstr=make_number(1, result);
  sts=(char **)NULL;
  if(nargs==7 || nargs==8 || nargs==9) {
    strcpy(valid.name,command); 
    valid.num=7;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=NUMBER;
    valid.type[4]=NUMBER;
    valid.type[5]=NUMBER;
    valid.type[6]=STRING;
    if(nargs==7) {
      without=1;
    }
    if(nargs==8) {
      valid.num=8;
      valid.type[7]=STRING;
    }
    if(nargs==9) {
      withboth=1;
      valid.num=9;
      valid.type[7]=STRING;
      valid.type[8]=NUMBER;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_argument(3, AWK_STRING, & val3);
    get_argument(4, AWK_STRING, & val4);
    get_argument(5, AWK_STRING, & val5);
    get_argument(6, AWK_STRING, & val6);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    mem_cdo(sts,val4.str_value.str,++cnt);
    mem_cdo(sts,val5.str_value.str,++cnt);
    mem_cdo(sts,val6.str_value.str,++cnt);
    if(!without) {  
      get_argument(7, AWK_STRING, & val7);
      if(withboth) {  
        mem_cdo(sts,val7.str_value.str,++cnt);
        get_argument(8, AWK_STRING, & val8);
        mem_cdo(sts,"count",++cnt);
        mem_cdo(sts,val8.str_value.str,++cnt);
      }
      else {
        if(strcmp(val7.str_value.str,"asc") == 0 || strcmp(val7.str_value.str,"desc") == 0 ||
           strcmp(val7.str_value.str,"km") == 0 || strcmp(val7.str_value.str,"mi") == 0 ||
           strcmp(val7.str_value.str,"m") == 0 || strcmp(val7.str_value.str,"ft") == 0 ) {
              mem_cdo(sts,val7.str_value.str,++cnt);
        }
        else {
          mem_cdo(sts,"count",++cnt);
          mem_cdo(sts,val7.str_value.str,++cnt);
        }
      }
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs seven, eight or nine arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGeoradiusbymember(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  size_t withboth,without;
  struct command valid;
  char str[240], **sts;
  awk_array_t array;
  awk_value_t val, val1, val3, val4, val5, val6, val7, array_param, *pstr=NULL;
  enum format_type there[8];
  without=withboth=0;
  pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==6 || nargs==7 || nargs==8) {
    strcpy(valid.name,command); 
    valid.num=6;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=STRING;
    valid.type[4]=NUMBER;
    valid.type[5]=STRING;
    if(nargs==6) {
      without=1;
    }
    if(nargs==7) {
      valid.num=7;
      valid.type[6]=STRING;
    }
    if(nargs==8) {
      withboth=1;
      valid.num=8;
      valid.type[6]=STRING;
      valid.type[7]=NUMBER;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_argument(3, AWK_STRING, & val3);
    get_argument(4, AWK_STRING, & val4);
    get_argument(5, AWK_STRING, & val5);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    mem_cdo(sts,val4.str_value.str,++cnt);
    mem_cdo(sts,val5.str_value.str,++cnt);
    if(!without) {  
      get_argument(6, AWK_STRING, & val6);
      if(withboth) {  
        get_argument(7, AWK_STRING, & val7);
        mem_cdo(sts,val6.str_value.str,++cnt);
        mem_cdo(sts,"count",++cnt);
        mem_cdo(sts,val7.str_value.str,++cnt);
      }
      else {
         if(strcmp(val6.str_value.str,"asc") == 0 || strcmp(val6.str_value.str,"desc") == 0) {
          mem_cdo(sts,val6.str_value.str,++cnt);
         }
         else {
          mem_cdo(sts,"count",++cnt);
          mem_cdo(sts,val6.str_value.str,++cnt);
        }
      }
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs six, seven or eigth arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGeoradiusWD(int nargs,awk_value_t *result,const char *command) {
  int ival,r,cnt,pconn;
  size_t withboth,without,WD,WDWC,WC;
  struct command valid;
  char str[240], thecommand[]="georadius", **sts;
  awk_array_t array;
  awk_value_t val, val1, val3, val4, val5, val6, val7, val8, array_param, *pstr;
  enum format_type there[9];
  without=withboth=WD=WDWC=WC=0;
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==7 || nargs==8 || nargs==9) {
    strcpy(valid.name,"georadius"); 
    if(strcmp(command,"WD")==0) {
      WD=1;
    }
    if(strcmp(command,"WC")==0) {
      WC=1;
    }
    if(strcmp(command,"WDWC")==0) {
      WDWC=1;
    }
    valid.num=7;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=NUMBER;
    valid.type[4]=NUMBER;
    valid.type[5]=NUMBER;
    valid.type[6]=STRING;
    if(nargs==7) {
      without=1;
    }
    if(nargs==8) {
      valid.num=8;
      valid.type[7]=STRING;
    }
    if(nargs==9) {
      withboth=1;
      valid.num=9;
      valid.type[7]=STRING;
      valid.type[8]=NUMBER;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,thecommand,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_argument(3, AWK_STRING, & val3);
    get_argument(4, AWK_STRING, & val4);
    get_argument(5, AWK_STRING, & val5);
    get_argument(6, AWK_STRING, & val6);
    sts=mem_cdo(sts,thecommand,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    mem_cdo(sts,val4.str_value.str,++cnt);
    mem_cdo(sts,val5.str_value.str,++cnt);
    mem_cdo(sts,val6.str_value.str,++cnt);
    if(without) {  
      if(WD) {
        mem_cdo(sts,"withdist",++cnt);
      }
      if(WDWC) {
        mem_cdo(sts,"withdist",++cnt);
        mem_cdo(sts,"withcoord",++cnt);
      }
      if(WC) {
        mem_cdo(sts,"withcoord",++cnt);
      }
    }
    else {
       get_argument(7, AWK_STRING, & val7);
       if(withboth) {  
        mem_cdo(sts,val7.str_value.str,++cnt);
        mem_cdo(sts,"count",++cnt);
        get_argument(8, AWK_STRING, & val8);
        mem_cdo(sts,val8.str_value.str,++cnt);
        if(WD) {
         mem_cdo(sts,"withdist",++cnt);
        }
        if(WDWC) {
         mem_cdo(sts,"withdist",++cnt);
         mem_cdo(sts,"withcoord",++cnt);
        }
        if(WC) {
         mem_cdo(sts,"withcoord",++cnt);
        }
       }
       else {
        if(strcmp(val7.str_value.str,"asc") == 0 || strcmp(val7.str_value.str,"desc") == 0) {
         mem_cdo(sts,val7.str_value.str,++cnt);
         if(WD) {
          mem_cdo(sts,"withdist",++cnt);
         }
         if(WDWC) {
          mem_cdo(sts,"withdist",++cnt);
          mem_cdo(sts,"withcoord",++cnt);
         }
         if(WC) {
          mem_cdo(sts,"withcoord",++cnt);
         }
        }
        else {
         mem_cdo(sts,"count",++cnt);
         mem_cdo(sts,val7.str_value.str,++cnt);
         if(WD) {
          mem_cdo(sts,"withdist",++cnt);
         }
         if(WDWC) {
          mem_cdo(sts,"withdist",++cnt);
          mem_cdo(sts,"withcoord",++cnt);
         }
         if(WC) {
          mem_cdo(sts,"withcoord",++cnt);
         }
        }
       }
      }
      reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
      if(pconn==-1) {
        pstr=processREPLY(array,result,c[ival],"tipoExec");
      }
      free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs seven, eight or nine arguments",thecommand);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGeoradiusbymemberWD(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt;
  size_t withboth,without,WD,WDWC,WC;
  struct command valid;
  char str[240], thecommand[]="georadiusbymember", **sts;
  awk_array_t array;
  awk_value_t val, val1, val3, val4, val5, val6, val7, array_param, *pstr;
  enum format_type there[8];
  without=withboth=WD=WDWC=WC=0;
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==6 || nargs==7 || nargs==8) {
    strcpy(valid.name,"georadiusbymember"); 
    if(strcmp(command,"WD")==0) {
      WD=1;
    }
    if(strcmp(command,"WC")==0) {
      WC=1;
    }
    if(strcmp(command,"WDWC")==0) {
      WDWC=1;
    }
    valid.num=6;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=STRING;
    valid.type[4]=NUMBER;
    valid.type[5]=STRING;
    if(nargs==6) {
      without=1;
    }
    if(nargs==7) {
      valid.num=7;
      valid.type[6]=STRING;
    }
    if(nargs==8) {
      withboth=1;
      valid.num=8;
      valid.type[6]=STRING;
      valid.type[7]=NUMBER;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,thecommand,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_argument(3, AWK_STRING, & val3);
    get_argument(4, AWK_STRING, & val4);
    get_argument(5, AWK_STRING, & val5);
    sts=mem_cdo(sts,thecommand,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    mem_cdo(sts,val4.str_value.str,++cnt);
    mem_cdo(sts,val5.str_value.str,++cnt);
    if(without) {
      if(WD) {
        mem_cdo(sts,"withdist",++cnt);
       }
       if(WDWC) {
        mem_cdo(sts,"withdist",++cnt);
        mem_cdo(sts,"withcoord",++cnt);
       }
       if(WC) {
        mem_cdo(sts,"withcoord",++cnt);
       }
    }
    else {
       get_argument(6, AWK_STRING, & val6);
       if(withboth) {  
         mem_cdo(sts,val6.str_value.str,++cnt);
         mem_cdo(sts,"count",++cnt);
         get_argument(7, AWK_STRING, & val7);
         mem_cdo(sts,val7.str_value.str,++cnt);
         if(WD) {
          mem_cdo(sts,"withdist",++cnt);
         }
         if(WDWC) {
          mem_cdo(sts,"withdist",++cnt);
          mem_cdo(sts,"withcoord",++cnt);
         }
         if(WC) {
          mem_cdo(sts,"withcoord",++cnt);
         }
       }
       else {
        if(strcmp(val6.str_value.str,"asc") == 0 || strcmp(val6.str_value.str,"desc") == 0) {
         mem_cdo(sts,val6.str_value.str,++cnt);
         if(WD) {
          mem_cdo(sts,"withdist",++cnt);
         }
         if(WDWC) {
          mem_cdo(sts,"withdist",++cnt);
          mem_cdo(sts,"withcoord",++cnt);
         }
         if(WC) {
          mem_cdo(sts,"withcoord",++cnt);
         }
        }        
        else {
         mem_cdo(sts,"count",++cnt);
         mem_cdo(sts,val6.str_value.str,++cnt);
         if(WD) {
          mem_cdo(sts,"withdist",++cnt);
         }
         if(WDWC) {
          mem_cdo(sts,"withdist",++cnt);
          mem_cdo(sts,"withcoord",++cnt);
         }
         if(WC) {
          mem_cdo(sts,"withcoord",++cnt);
         }
        }
       }
   }
   reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
   if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"tipoExec");
   }
   free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs six, seven or eight",thecommand);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSetbit(int nargs,awk_value_t *result,const char *command) {
  int ival,r; 
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  int pconn=-1;
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pstr=theReply(result,c[ival]);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBitop(int nargs,awk_value_t *result,const char *command) {
  size_t i;
  int count,ival,r,pconn,cnt=0;
  struct command valid;
  
  enum BitOperator { 
    AND, OR, XOR, NOT
  };     
  const char* BitOperator_Strings[] = {"AND", "OR", "XOR", "NOT"}; 
  enum BitOperator bop[4]; 
  bop[0] = AND; 
  bop[1] = OR; 
  bop[2] = XOR; 
  bop[3] = NOT; 

  char str[240],**sts=NULL;
  awk_value_t val, val1, val2, val3, array_param, *pstr;
  awk_array_t array;
  enum format_type there[4];
  pconn=-1;
  pstr=make_number(1, result);
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    valid.type[3]=ST_AR;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    // validate operation AND OR XOR and NOT
    for(i=0;i<4;i++){
      if(strcmp(BitOperator_Strings[i],val1.str_value.str)==0) {
        break;
      }
    }
    if(i==4) {
     sprintf(str,"%s Operator argument must be AND, OR, XOR, NOT",command);
     set_ERRNO(_(str));
     return make_number(-1, result);
    }
    get_argument(2, AWK_STRING, & val2);
    if(there[3]==STRING) {
      get_argument(3, AWK_STRING, & val3);
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,val1.str_value.str,++cnt);
      mem_cdo(sts,val2.str_value.str,++cnt);
      mem_cdo(sts,val3.str_value.str,++cnt);
    }
    if(bop[i]==NOT && there[3]==ARRAY) {
     sprintf(str,"%s Operator NOT, needs only one source key",command);
     set_ERRNO(_(str));
     return make_number(-1, result);
    }
    if(there[3]==ARRAY) {
      get_argument(3, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,3,command,&count);
      mem_str(sts,val1.str_value.str,1);
      mem_str(sts,val2.str_value.str,2);
      cnt=count-1;
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoLinsert(int nargs,awk_value_t *result,const char *command) {
  int ival,r,cnt, pconn;
  struct command valid;
  char **sts, str[240],cmd[]="linsert", where[10]="AFTER";
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    valid.type[3]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(strcmp(command,"linsertBefore")==0) {
      strcpy(where,"BEFORE");
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    sts=mem_cdo(sts,cmd,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,where,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    pstr=make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSmove(int nargs,awk_value_t *result,const char *command) {
  int ival,r,cnt,pconn;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  pconn=-1;
  cnt=0;
  sts=(char **)NULL;
  pstr=make_number(1,result);
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    valid.type[3]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZincrby(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  pconn=-1;
  sts=(char **)NULL;
  pstr=make_number(1, result);
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);

    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoHincrby(int nargs,awk_value_t *result,const char *command) {
  int ival,r,pconn,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  pstr=make_number(1, result);
  pconn=-1;
  sts=(char **)NULL;
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    valid.type[3]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoRestore(int nargs,awk_value_t *result,const char *command) {
  int ival,r;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  int pconn=-1;
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    valid.type[3]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    get_argument(3, AWK_STRING, & val3);
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s %s %b",command,val1.str_value.str,val2.str_value.str,val3.str_value.str,val3.str_value.len);
      pstr=theReply(result,c[ival]);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %b",command,val1.str_value.str,val2.str_value.str,val3.str_value.str,val3.str_value.len);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoExpire(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   struct command valid;
   char str[240];
   awk_value_t val, val1, val2, *pstr;
   enum format_type there[3];
   int pconn=-1;
   pstr=(awk_value_t *)NULL;
   if(nargs==3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_STRING, & val2);
    if(pconn==-1){ 
      reply = redisCommand(c[ival],"%s %s %s",command,val1.str_value.str,val2.str_value.str);
      pstr=theReply(result,c[ival]);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s",command,val1.str_value.str,val2.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSet(int nargs,awk_value_t *result,const char *command) {
   int r,ival,pconn,cnt;
   int i;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, mbr, mbr1, *pstr;
   enum format_type there[3];
   pconn=-1;
   cnt=0;
   sts=(char **)NULL;
   pstr=make_number(1,result);
   if(nargs >= 3 && nargs <=6) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_STRING, & mbr);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    mem_cdo(sts,mbr.str_value.str,++cnt);
    if(nargs > 3) {
     for(i=3;i<nargs;i++) {
       get_argument(i, AWK_STRING, & mbr1);
       mem_cdo(sts,mbr1.str_value.str,++cnt);
     }
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s: arguments must be between three and six",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSismember(int nargs,awk_value_t *result,const char *command) {
   int r,ival,cnt,pconn;
   size_t config;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, mbr, *pstr;
   enum format_type there[3];
   pconn=-1;
   config=cnt=0;
   sts=(char **)NULL;
   pstr=make_number(1, result);
   if(nargs==3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_STRING, & mbr);
    if(strcmp(command,"configSet")==0) {
      config=1;
    }
    if(!config) {
      sts=mem_cdo(sts,command,cnt);
    }
    else {
      sts=mem_cdo(sts,"config",cnt);
      mem_cdo(sts,"set",++cnt);
    }
    mem_cdo(sts,val.str_value.str,++cnt);
    mem_cdo(sts,mbr.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoObject(int nargs,awk_value_t *result,const char *command) {
   int r,ival,cnt, pconn;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, mbr, *pstr;
   enum format_type there[3];
   pconn=-1;
   cnt=0;
   sts=(char **)NULL;
   pstr=make_number(1, result);
   if(nargs==3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=STRING;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    if((strcmp(val.str_value.str,"refcount")==0)) {
    }
    else if((strcmp(val.str_value.str,"idletime")==0)) {
    }
    else if((strcmp(val.str_value.str,"encoding")==0)) {
    }
    else { 
      sprintf(str,"%s needs a valid command refcount|encoding|idletime",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(2, AWK_STRING, & mbr);
    sts=mem_cdo(sts,command,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    mem_cdo(sts,mbr.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGetReply(int nargs,awk_value_t *result,const char *command) {
   int r,ival,ret;
   struct command valid;
   char str[240];
   awk_value_t val, array_param, *pstr;
   awk_array_t array;
   enum format_type there[2];
   pstr=NULL;
   int pconn=-1;
   if(nargs==2 || nargs==1) {
     strcpy(valid.name,command); 
     valid.type[0]=CONN;
     if(nargs==2) {
       valid.num=2;
       valid.type[1]=ARRAY;
     }
     else {
       valid.num=1;
     }
     if(!validate(valid,str,&r,there)) {
        set_ERRNO(_(str));
        return make_number(-1, result);
     }
     get_argument(0, AWK_NUMBER, & val);
     ival=val.num_value;
     if(!validate_conn(ival,str,command,&pconn)) {
        set_ERRNO(_(str));
        return make_number(-1, result);
     }
     if(nargs==2) {
       get_argument(1, AWK_ARRAY, & array_param);
       array = array_param.array_cookie;
     }
     if(pconn==-1 || pipel[pconn][1]<=0) {
       sprintf(str,"%s: No such reply, nothing to getReply\n",command);
       set_ERRNO(_(str));
       return make_number(-1, result);
     }
     if((ret=redisGetReply(c[pconn],(void **)&reply))==REDIS_OK) {
       pipel[pconn][1]--;
       if(nargs==2) {
        if(strcmp(command,"getReplyInfo")==0){
          pstr=processREPLY(array,result,c[pconn],"tipoInfo");
        }
        else {
          pstr=processREPLY(array,result,c[pconn],"tipoExec");
        }
       }
       else {
         pstr=processREPLY(NULL,result,c[pconn],NULL);
         if(!pstr) {
           sprintf(str,"%s (%s)","getReply function needs an array as an argument", "the function pipelined returns an array");
           set_ERRNO(_(str));
           pstr=make_number(-1, result);
         }
       }
     }
     if(ret==REDIS_ERR) {
       if(c[pconn]->err) {
           sprintf(str,"%s: error %s\n",command,c[pconn]->errstr);
	   set_ERRNO(_(str));
	   c[pconn]=(redisContext *)NULL;
           return make_number(-1, result);
       }
     }
   }
   else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
   }
   return pstr;
}

awk_value_t * tipoGetReplyMass(int nargs,awk_value_t *result,const char *command) {
   int r,ival,ret;
   long long replies = 0;
   struct command valid;
   char str[240];
   awk_value_t val;
   enum format_type there[1];
   int pconn=-1;
   if(nargs==1) {
    strcpy(valid.name,command); 
    valid.num=1;
    valid.type[0]=CONN;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(pconn==-1 || pipel[pconn][1]<=0) {
      sprintf(str,"%s: No such reply, nothing to getReplyMassive\n",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    replies=pipel[pconn][1];
    while(pipel[pconn][1] > 0 && (ret=redisGetReply(c[pconn],(void **)&reply)) == REDIS_OK) {
      freeReplyObject(reply);
      pipel[pconn][1]--;
    }
    if(ret==REDIS_ERR) {
      if(c[pconn]->err) {
        sprintf(str,"%s: error %s\n",command,c[pconn]->errstr);
	set_ERRNO(_(str));
	c[pconn]=(redisContext *)NULL;
        return make_number(-1, result);
      }
    }
  }
  else {
    sprintf(str,"%s needs one argument",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return make_number(replies - pipel[pconn][1],result);
}

awk_value_t * tipoGetMessage(int nargs,awk_value_t *result,const char *command) {
   int r,ival,ret;
   struct command valid;
   char str[240];
   awk_value_t val, array_param, *pstr=NULL;
   awk_array_t array;
   enum format_type there[2];
   int pconn=-1;
   if(nargs==2) {
    strcpy(valid.name,command); 
    valid.num=2;
    valid.type[0]=CONN;
    valid.type[1]=ARRAY;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    if(pconn==-1) {
     if((ret=redisGetReply(c[ival],(void **)&reply)) == REDIS_OK) {
      pstr=processREPLY(array,result,c[ival],"theRest");
     }
     if(ret==REDIS_ERR) {
       if(c[ival]->err) {
         sprintf(str,"%s: error %s\n",command,c[ival]->errstr);
         set_ERRNO(_(str));
         c[ival]=(redisContext *)NULL;
         return make_number(-1, result);
       }
     }
    }
    else {
      redisAppendCommand(c[pconn],"%s %s",command,val.str_value.str);
      pipel[pconn][1]++;
    }
  }
  else {
    sprintf(str,"%s needs two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZrange(int nargs,awk_value_t *result,const char *command) {
   int r,pconn,cnt,ival,with=0;
   struct command valid;
   char str[240],cmd[30], **sts;
   awk_value_t val, val1, val2, val3, array_param, *pstr;
   awk_array_t array;
   enum format_type there[5];
   pstr=make_number(1, result);
   sts=(char **)NULL; 
   pconn=-1;
   cnt=0;
   if(nargs==5) {
    strcpy(valid.name,command); 
    valid.num=5;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=NUMBER;
    valid.type[4]=NUMBER;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val1);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_argument(3, AWK_STRING, & val2);
    get_argument(4, AWK_STRING, & val3);
    if (strstr(command,"WithScores") != NULL) {
       if (strstr(command,"rev") != NULL) {
         strcpy(cmd,"zrevrange");
       }
       else {
         strcpy(cmd,"zrange");
       }
       with=1;
    }
    else {
      strcpy(cmd,command);
    }
    sts=mem_cdo(sts,cmd,cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    mem_cdo(sts,val3.str_value.str,++cnt);
    if(with) {
      mem_cdo(sts,"WITHSCORES",++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr; 
}

awk_value_t * tipoZunionstore(int nargs,awk_value_t *result,const char *command) {
   int r,ival,count,pconn;
   size_t nkeys,nkeys_w;
   struct command valid;
   char str[240], st_nkeys[15], **sts, *pt;
   awk_value_t val, val1, array_param, array_param_w, *pstr;
   awk_array_t array, array_w;
   enum format_type there[5];
   pconn=-1;
   sts=(char **)NULL;
   pstr=make_number(1, result);
   if(nargs==3 || nargs==4 || nargs==5) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.num=3;
    if(nargs==4) {
      valid.type[3]=ST_AR;
      valid.num=4;
    }
    if(nargs==5) {
      valid.type[3]=ARRAY;
      valid.type[4]=STRING;
      valid.num=5;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    get_element_count(array, &nkeys);
    if(nargs==3){
     sts=getArrayContent(array,3,command,&count);
    }
    if(nargs==4 && there[3]==STRING) {
     get_argument(3, AWK_STRING, & val1);
     sts=getArrayContentCont(array,3,command,&count,2);
     pt=strchr(val1.str_value.str,' ');
     *pt='\0';
     mem_str(sts,val1.str_value.str,count++);
     mem_str(sts,pt+1,count++);
    }
    if((nargs==4 && there[3]==ARRAY) || nargs==5) {
      get_argument(3, AWK_ARRAY, & array_param_w);
      array_w = array_param_w.array_cookie;
      get_element_count(array_w, &nkeys_w);
      if(nargs==5){
        get_argument(4, AWK_STRING, & val1);
        sts=getArrayContentCont(array,3,command,&count,nkeys_w+1+2);
        mem_str(sts,"weights",count++);
        count=getArrayContentSecond(array_w,count,sts);
        pt=strchr(val1.str_value.str,' ');
        *pt='\0';
        mem_str(sts,val1.str_value.str,count++);
        mem_str(sts,pt+1,count++);
      }
      else{
        sts=getArrayContentCont(array,3,command,&count,nkeys_w+1);
        mem_str(sts,"weights",count++);
        count=getArrayContentSecond(array_w,count,sts);
      }
    }
    mem_str(sts,val.str_value.str,1);
    sprintf(st_nkeys,"%zu",nkeys);
    mem_str(sts,st_nkeys,2); // passing a string nkeys
    reply = (redisReply *)rCommand(pconn,ival,count,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s needs three, four or five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSortLimit(int nargs,awk_value_t *result,const char *command) {
   int r,ival,pconn,cnt,ret=1;
   size_t store;
   struct command valid;
   char cmd[]="sort", str[240], *pch, tmp_val3[240], **sts;
   awk_value_t val, val1, val2, val3, array_param, valstore, *pstr;
   awk_array_t array=NULL;
   enum format_type there[6];
   pconn=-1;
   sts=(char **)NULL;
   cnt=0;
   store=0;
   pstr=make_number(ret, result);
   if(nargs==5 || nargs==6) {
    if(strcmp(command,"sortLimitStore")==0) {
      store=1;
    }
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    if(store){
      valid.type[2]=STRING;
    }
    else {
      valid.type[2]=ARRAY;
    }
    valid.type[3]=NUMBER;
    valid.type[4]=NUMBER;
    if(nargs==5) {
      valid.num=5;
    }
    else {
      valid.num=6;
      valid.type[5]=STRING;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    if(store) {
      get_argument(2, AWK_STRING, & valstore);
    }
    else {
      get_argument(2, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
    }
    get_argument(3, AWK_STRING, & val1);
    get_argument(4, AWK_STRING, & val2);
    sts=mem_cdo(sts,cmd,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    mem_cdo(sts,"LIMIT",++cnt);
    mem_cdo(sts,val1.str_value.str,++cnt);
    mem_cdo(sts,val2.str_value.str,++cnt);
    if(nargs==6) {
      get_argument(5, AWK_STRING, & val3);
      strcpy(tmp_val3,val3.str_value.str);
      pch = strtok (tmp_val3," ");
      while (pch != NULL) {
        mem_cdo(sts,pch,++cnt);
	pch = strtok (NULL," ");
      }
    }
    if(store) {
      mem_cdo(sts,"STORE",++cnt);
      mem_cdo(sts,valstore.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs five or six arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSort(int nargs,awk_value_t *result,const char *command) {
   int r,ival,pconn,cnt=0;
   size_t store;
   struct command valid;
   char str[240], *pch, cmd[]="sort", tmp_val1[240], **sts;
   awk_value_t val, val1, array_param, valstore, *pstr;
   awk_array_t array=0;
   enum format_type there[4];
   pconn=-1;
   store=0;
   sts=(char **)NULL;
   pstr=make_number(1, result);
   if(nargs==3 || nargs==4) {
    if(strcmp(command,"sortStore")==0) {
       store=1;
    }
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    if(store) {
      valid.type[2]=STRING;
    }
    else {
      valid.type[2]=ARRAY;
    }
    if(nargs==3) {
      valid.num=3;
    }
    else {
      valid.num=4;
      valid.type[3]=STRING;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    if(store) {
      get_argument(2, AWK_STRING, & valstore);
    }
    else {
      get_argument(2, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
    }
    sts=mem_cdo(sts,cmd,cnt);
    mem_cdo(sts,val.str_value.str,++cnt);
    if(nargs==4) {
      get_argument(3, AWK_STRING, & val1);
      strcpy(tmp_val1,val1.str_value.str);
      pch = strtok (tmp_val1," ");
      while (pch != NULL) {
        mem_cdo(sts,pch,++cnt);
	pch = strtok (NULL," ");
      }
    }
    if(store) {
      mem_cdo(sts,"STORE",++cnt);
      mem_cdo(sts,valstore.str_value.str,++cnt);
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoKeys(int nargs,awk_value_t *result,const char *command) {
   int r,ival,cnt,pconn;
   size_t config;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, array_param, *pstr;
   awk_array_t array;
   enum format_type there[3];
   pconn=-1;
   config = cnt = 0;
   sts=(char **)NULL;
   pstr=make_number(1,result);
   if(nargs == 3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    if(strcmp(command,"configGet")==0) {
       config = 1;
    }
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    if(!config) {
      sts=mem_cdo(sts,command,cnt);
    }
    else {
      sts=mem_cdo(sts,"config",cnt);
      mem_cdo(sts,"get",++cnt);
    }
    mem_cdo(sts,val.str_value.str,++cnt);
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
   }
   else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
   }
   return pstr;
}

awk_value_t * tipoGeohash(int nargs,awk_value_t *result,const char *command) {
   int r,ival,count,pconn;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, array_param, array_input, *pstr;
   awk_array_t array, arrayin;
   enum format_type there[4];
   pconn=-1;
   sts=(char **)NULL;
   pstr=make_number(1,result);
   if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    valid.type[3]=ARRAY;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_ARRAY, & array_input);
    get_argument(3, AWK_ARRAY, & array_param);
    arrayin = array_input.array_cookie;
    array = array_param.array_cookie;
    sts=getArrayContent(arrayin,2,command,&count);
    mem_str(sts,val.str_value.str,1);
    reply = (redisReply *)rCommand(pconn,ival,count,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    free_mem_str(sts,count);
   }
   else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
   }
   return pstr;
}

awk_value_t * tipoInfo(int nargs,awk_value_t *result,const char *command) {
   int r,ival,pconn,cnt=0;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, array_param, *pstr;
   awk_array_t array;
   enum format_type there[3];
   pstr=make_number(1, result);
   sts=(char **)NULL;
   pconn=-1;
   if(nargs==2||nargs==3) {
     valid.num=2;
     strcpy(valid.name,command); 
     valid.type[0]=CONN;
     valid.type[1]=ARRAY;
     if(nargs==3) {
      valid.type[2]=STRING;
      valid.num=3;
     }
     if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
       return make_number(-1, result);
     }
     get_argument(0, AWK_NUMBER, & val);
     ival=val.num_value;
     if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
     }
     get_argument(1, AWK_ARRAY, & array_param);
     array = array_param.array_cookie;
     sts=mem_cdo(sts,command,cnt);
     if(nargs==3) {
      get_argument(2, AWK_STRING, & val);
      mem_cdo(sts,val.str_value.str,++cnt);
     }
     reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
     if(pconn==-1) {
       pstr=processREPLY(array,result,c[ival],"tipoInfo");
     }
     free_mem_str(sts,cnt+1);
   }
   else {
    sprintf(str,"%s needs two or three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
   }
   return pstr;
}

awk_value_t * tipoSrandmember(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   struct command valid;
   char str[240];
   awk_value_t val,val1, array_param, *pstr;
   awk_array_t array;
   enum format_type there[4];
   int pconn=-1;
   pstr=NULL;
   if(nargs==4) {
    strcpy(valid.name,command); 
    valid.num=4;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=NUMBER;
    valid.type[3]=ARRAY;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_STRING, & val1);
    get_argument(3, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s %s",command,val.str_value.str,val1.str_value.str);
      pstr=processREPLY(array,result,c[ival],"theRest");
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s",command,val.str_value.str,val1.str_value.str);
      pipel[pconn][1]++;
    }
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

int validate(struct command valid,char *str,int *r,enum format_type *there){
  int i;
  awk_value_t val, array_param;
  enum format_type t=INDEF;

  for(i=0; i < valid.num; i++) {
    if(valid.type[i]==CONN) {
      if(!get_argument(i, AWK_NUMBER, & val)) {
       sprintf(str,"%s: argument %d does not have a valid connection format",valid.name,i+1);
       *r=i;
       return 0;
      }
    }
    if(valid.type[i]==NUMBER) {
      if(!get_argument(i, AWK_NUMBER, & val)) {
       sprintf(str,"%s: argument %d is present but not is a number",valid.name,i+1);
       *r=i;
       return 0;
      }
      else {
       t=NUMBER;
      }
    }
    if(valid.type[i]==STRING) {
      if(!get_argument(i, AWK_STRING, & val)) {
       sprintf(str,"%s: argument %d is present but not is a string",valid.name,i+1);
       *r=i;
       return 0;
      }
      else {
       t=STRING;
      }
    }
    if(valid.type[i]==ARRAY) {
      if(!get_argument(i, AWK_ARRAY, & array_param)) {
       sprintf(str,"%s: argument %d is present but not is an array",valid.name,i+1);
       *r=i;
       return 0;
      }
      else {
       t=ARRAY;
      }
    }
    if(valid.type[i]==ST_AR) {
      if(!get_argument(i, AWK_STRING, & val)) {
        if(!get_argument(i, AWK_ARRAY, & val)) {
         sprintf(str,"%s: argument %d is present but not is either an array or a string",valid.name,i+1);
         *r=i;
	 return 0;
	}
        else {
         t=ARRAY;
	}
      }
      else {
       t=STRING;
      }
    }
    if(valid.type[i]==ST_NUM) {
      if(!get_argument(i, AWK_STRING, & val)) {
        if(!get_argument(i, AWK_NUMBER, & val)) {
         sprintf(str,"%s: argument %d is present but not is either a number or a string",valid.name,i+1);
         *r=i;
	 return 0;
	}
        else {
         t=NUMBER;
	}
      }
      else {
       t=STRING;
      }
    }
    there[i]=t;
  }
  return 1;
}

static awk_value_t * do_hgetall(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hgetall: called with too many arguments"));
    }
#endif
  p_value_t=tipoKeys(nargs,result,"hgetall");
  return p_value_t;
}

awk_value_t * tipoMset(int nargs,awk_value_t *result,const char *command) {
  int r,ival,count,pconn;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, *pts;
  awk_array_t array;
  enum format_type there[2];
  pts=make_number(1, result);
  pconn=-1;
  sts=(char **)NULL; 
  if(nargs==2) {
    strcpy(valid.name,command); 
    valid.num=2;
    valid.type[0]=CONN;
    valid.type[1]=ARRAY;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    sts=getArrayContent(array,1,command,&count);
    reply = (redisReply *)rCommand(pconn,ival,count,(const char **)sts);
    if(pconn==-1) {
      pts=processREPLY(NULL,result,c[ival],NULL);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pts;
}

awk_value_t * tipoHmset(int nargs,awk_value_t *result,const char *command) {
  int r,ival,count;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, *pts;
  awk_array_t array;
  enum format_type there[3];
  int pconn=-1;
  
  if(nargs==3) {
    strcpy(valid.name,command); 
    valid.num=3;
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ARRAY;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    sts=getArrayContent(array,2,"HMSET",&count);
    mem_str(sts,val.str_value.str,1);
    if(pconn==-1) {
      reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      pts=theReply(result,c[ival]);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      pipel[pconn][1]++;
      pts=make_number(1, result);
    }
    free(sts);
  }
  else {
    sprintf(str,"%s needs three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pts;
}

awk_value_t * tipoHmget(int nargs,awk_value_t *result,const char *command) {
  int r,ival,count;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, val1, array_param, *pstr;
  awk_array_t array_in,array_ou;
  enum format_type there[4];
  pstr=NULL;
  int pconn=-1;
  if(nargs==4) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=STRING;
    valid.type[2]=ST_AR;
    valid.type[3]=ARRAY;
    valid.num=4;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_STRING, & val);
    get_argument(3, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    if(there[2]==STRING) {
      get_argument(2, AWK_STRING, & val1);
      if(pconn==-1) {
       reply = redisCommand(c[ival],"%s %s %s",command,val.str_value.str,val1.str_value.str);
      }
      else {
       redisAppendCommand(c[pconn],"%s %s %s",command,val.str_value.str,val1.str_value.str);
       pipel[pconn][1]++;
       return make_number(1, result);
      }
    }
    else {
      get_argument(2, AWK_ARRAY, & array_param);
      array_in = array_param.array_cookie;
      sts=getArrayContent(array_in,2,command,&count);
      mem_str(sts,val.str_value.str,1);
      if(pconn==-1) {
        reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      }
      else {
        redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
        pipel[pconn][1]++;
        return make_number(1, result);
      }
      free(sts);
    }
    if(strcmp(command,"geopos")==0) {
      pstr=processREPLY(array_ou,result,c[ival],"tipoExec");
    }
    else {
      pstr=processREPLY(array_ou,result,c[ival],"theRest");
    }
  }
  else {
    sprintf(str,"%s needs four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoMget(int nargs,awk_value_t *result,const char *command) {
  int r,ival,pconn,count,cnt=0;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, *pstr;
  awk_array_t array_in,array_ou;
  enum format_type there[3];
  sts=(char **)NULL;
  pstr=make_number(1, result);
  pconn=-1;
  if(nargs==3) {
    strcpy(valid.name,command); 
    valid.type[0]=CONN;
    valid.type[1]=ST_AR;
    valid.type[2]=ARRAY;
    valid.num=3;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(0, AWK_NUMBER, & val);
    ival=val.num_value;
    if(!validate_conn(ival,str,command,&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(2, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & val);
      sts=mem_cdo(sts,command,cnt);
      mem_cdo(sts,val.str_value.str,++cnt);
    }
    else {
      get_argument(1, AWK_ARRAY, & array_param);
      array_in = array_param.array_cookie;
      sts=getArrayContent(array_in,1,command,&count);
      cnt=count-1;
    }
    reply = (redisReply *)rCommand(pconn,ival,cnt+1,(const char **)sts);
    if(pconn==-1) {
      pstr=processREPLY(array_ou,result,c[ival],"theRest");
    }
    free_mem_str(sts,cnt+1);
  }
  else {
    sprintf(str,"%s needs three arguments\n",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

static awk_value_t * do_select(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_select: called with too many arguments"));
    }
#endif
  p_value_t=tipoSelect(nargs,result,"select");
  return p_value_t;
}

static awk_value_t * do_randomkey(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_randomkey: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"randomkey");
  return p_value_t;
}

static awk_value_t * do_clientSetName(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_clientSetName: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientTwo(nargs,result,"clientSetName");
  return p_value_t;
}

static awk_value_t * do_clientPause(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_clientPause: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientTwo(nargs,result,"clientPause");
  return p_value_t;
}

static awk_value_t * do_clientKillAddr(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_clientKillAddr: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientTwo(nargs,result,"clientKillAddr");
  return p_value_t;
}

static awk_value_t * do_clientKillType(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_clientKillType: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientTwo(nargs,result,"clientKillType");
  return p_value_t;
}

static awk_value_t * do_clientKillId(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_clientKillId: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientTwo(nargs,result,"clientKillId");
  return p_value_t;
}

static awk_value_t * do_clientGetName(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_clientGetName: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientOne(nargs,result,"clientGetName");
  return p_value_t;
}
static awk_value_t * do_clientList(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_clientList: called with too many arguments"));
    }
#endif
  p_value_t=tipoClientTwo(nargs,result,"clientList");
  return p_value_t;
}

static awk_value_t * do_dbsize(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_dbsize: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"dbsize");
  return p_value_t;
}

static awk_value_t * do_ping(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_ping: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"ping");
  return p_value_t;
}

static awk_value_t * do_multi(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_multi: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"multi");
  return p_value_t;
}

static awk_value_t * do_discard(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_discard: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"discard");
  return p_value_t;
}

static awk_value_t * do_unwatch(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_unwatch: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"unwatch");
  return p_value_t;
}

static awk_value_t * do_exec(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_exec: called with too many arguments"));
    }
#endif
  p_value_t=tipoExec(nargs,result,"exec");
  return p_value_t;
}

static awk_value_t * do_configSet(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_configSet: called with too many arguments"));
    }
#endif
  p_value_t=tipoSismember(nargs,result,"configSet");
  return p_value_t;
}

static awk_value_t * do_configGet(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_configGet: called with too many arguments"));
    }
#endif
  p_value_t=tipoKeys(nargs,result,"configGet");
  return p_value_t;
}

static awk_value_t * do_slowlog(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_slowlog: called with too many arguments"));
    }
#endif
  p_value_t=tipoSlowlog(nargs,result,"slowlog");
  return p_value_t;
}

static awk_value_t * do_flushdb(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_flushdb: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"flushdb");
  return p_value_t;
}

static awk_value_t * do_flushall(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_flushall: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"flushall");
  return p_value_t;
}

static awk_value_t * do_auth(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_auth: called with too many arguments"));
    }
#endif
  p_value_t=tipoScard(nargs,result,"auth");
  return p_value_t;
}

static awk_value_t * do_bgsave(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_bgsave: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"bgsave");
  return p_value_t;
}

static awk_value_t * do_configResetStat(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_configResetStat: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"configResetStat");
  return p_value_t;
}


static awk_value_t * do_lastsave(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 1)) {
      lintwarn(ext_id, _("redis_lastsave: called with too many arguments"));
    }
#endif
  p_value_t=tipoRandomkey(nargs,result,"lastsave");
  return p_value_t;
}

static awk_value_t * do_exists(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_exists: called with too many arguments"));
    }
#endif
  p_value_t=tipoScard(nargs,result,"exists");
  return p_value_t;
}

static awk_value_t * do_get(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_get: called with too many arguments"));
    }
#endif
  p_value_t=tipoScard(nargs,result,"get");
  return p_value_t;
}

static awk_value_t * do_sdiff(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sdiff: called with too many arguments"));
    }
#endif
  p_value_t=tipoSinter(nargs,result,"sdiff");
  return p_value_t;
}

static awk_value_t * do_sunion(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sunion: called with too many arguments"));
    }
#endif
  p_value_t=tipoSinter(nargs,result,"sunion");
  return p_value_t;
}

static awk_value_t * do_sdiffstore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sdiffstore: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"sdiffstore");
  return p_value_t;
}

static awk_value_t * do_sunionstore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sunionstore: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"sunionstore");
  return p_value_t;
}

static awk_value_t * do_getMessage(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_getMessage: called with too many arguments"));
    }
#endif
  p_value_t=tipoGetMessage(nargs,result,"getMessage");
  return p_value_t;
}

static awk_value_t * do_punsubscribe(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_punsubscribe: called with too many arguments"));
    }
#endif
  p_value_t=tipoUnsubscribe(nargs,result,"punsubscribe");
  return p_value_t;
}

static awk_value_t * do_unsubscribe(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_unsubscribe: called with too many arguments"));
    }
#endif
  p_value_t=tipoUnsubscribe(nargs,result,"unsubscribe");
  return p_value_t;
}

static awk_value_t * do_psubscribe(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_psubscribe: called with too many arguments"));
    }
#endif
  p_value_t=tipoMget(nargs,result,"psubscribe");
  return p_value_t;
}

static awk_value_t * do_del(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_del: called with too many arguments"));
    }
#endif
  p_value_t=tipoSubscribe(nargs,result,"del");
  return p_value_t;
}

static awk_value_t * do_pfcount(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_pfcount: called with too many arguments"));
    }
#endif
  p_value_t=tipoSubscribe(nargs,result,"pfcount");
  return p_value_t;
}

static awk_value_t * do_subscribe(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_subscribe: called with too many arguments"));
    }
#endif
  p_value_t=tipoMget(nargs,result,"subscribe");
  return p_value_t;
}

static awk_value_t * do_watch(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_watch: called with too many arguments"));
    }
#endif
  p_value_t=tipoSubscribe(nargs,result,"watch");
  return p_value_t;
}

static awk_value_t * do_geohash(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_geohash: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeohash(nargs,result,"geohash");
  return p_value_t;
}

static awk_value_t * do_geoadd(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_geoadd: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"geoadd");
  return p_value_t;
}

static awk_value_t * do_geodist(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_geodist: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeodist(nargs,result,"geodist");
  return p_value_t;
}

static awk_value_t * do_geopos(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_geopos: called with too many arguments"));
    }
#endif
  p_value_t=tipoHmget(nargs,result,"geopos");
  return p_value_t;
}

static awk_value_t * do_georadius(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 9)) {
      lintwarn(ext_id, _("redis_georadius: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradius(nargs,result,"georadius");
  return p_value_t;
}

static awk_value_t * do_georadiusbymember(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 8)) {
      lintwarn(ext_id, _("redis_georadiusbymember: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusbymember(nargs,result,"georadiusbymember");
  return p_value_t;
}

static awk_value_t * do_georadiusWDWC(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 9)) {
      lintwarn(ext_id, _("redis_georadiusWDWC: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusWD(nargs,result,"WDWC");
  return p_value_t;
}

static awk_value_t * do_georadiusbymemberWDWC(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 8)) {
      lintwarn(ext_id, _("redis_georadiusbymemberWDWC: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusbymemberWD(nargs,result,"WDWC");
  return p_value_t;
}

static awk_value_t * do_georadiusWC(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 9)) {
      lintwarn(ext_id, _("redis_georadiusWC: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusWD(nargs,result,"WC");
  return p_value_t;
}

static awk_value_t * do_georadiusbymemberWC(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 8)) {
      lintwarn(ext_id, _("redis_georadiusbymemberWC: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusbymemberWD(nargs,result,"WC");
  return p_value_t;
}

static awk_value_t * do_georadiusWD(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 9)) {
      lintwarn(ext_id, _("redis_georadiusWD: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusWD(nargs,result,"WD");
  return p_value_t;
}

static awk_value_t * do_georadiusbymemberWD(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if(do_lint && (nargs > 8)) {
      lintwarn(ext_id, _("redis_georadiusbymemberWD: called with too many arguments"));
    }
#endif
  p_value_t=tipoGeoradiusbymemberWD(nargs,result,"WD");
  return p_value_t;
}

static awk_value_t * do_sinterstore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sinterstore: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"sinterstore");
  return p_value_t;
}

static awk_value_t * do_sinter(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sinter: called with too many arguments"));
    }
#endif
  p_value_t=tipoSinter(nargs,result,"sinter");
  return p_value_t;
}

static awk_value_t * do_hdel(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_hdel: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"hdel");
  return p_value_t;
}

static awk_value_t * do_hincrby(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_hincrby: called with too many arguments"));
    }
#endif
  p_value_t=tipoHincrby(nargs,result,"hincrby");
  return p_value_t;
}

static awk_value_t * do_brpoplpush(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_brpoplpush: called with too many arguments"));
    }
#endif
  p_value_t=tipoHincrby(nargs,result,"brpoplpush");
  return p_value_t;
}

static awk_value_t * do_zincrby(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zincrby: called with too many arguments"));
    }
#endif
  p_value_t=tipoZincrby(nargs,result,"zincrby");
  return p_value_t;
}

static awk_value_t * do_hincrbyfloat(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_hincrbyfloat: called with too many arguments"));
    }
#endif
  p_value_t=tipoHincrby(nargs,result,"hincrbyfloat");
  return p_value_t;
}

static awk_value_t * do_hset(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_hset: called with too many arguments"));
    }
#endif
  p_value_t=tipoSmove(nargs,result,"hset");
  return p_value_t;
}

static awk_value_t * do_hsetnx(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_hsetnx: called with too many arguments"));
    }
#endif
  p_value_t=tipoSmove(nargs,result,"hsetnx");
  return p_value_t;
}

static awk_value_t * do_lrem(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_lrem: called with too many arguments"));
    }
#endif
  p_value_t=tipoSetrange(nargs,result,"lrem");
  return p_value_t;
}

static awk_value_t * do_blpop(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_blpop: called with too many arguments"));
    }
#endif
  p_value_t=tipoBrpop(nargs,result,"blpop");
  return p_value_t;
}

static awk_value_t * do_brpop(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_brpop: called with too many arguments"));
    }
#endif
  p_value_t=tipoBrpop(nargs,result,"brpop");
  return p_value_t;
}

static awk_value_t * do_lset(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_lset: called with too many arguments"));
    }
#endif
  p_value_t=tipoSetrange(nargs,result,"lset");
  return p_value_t;
}

static awk_value_t * do_setrange(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_setrange: called with too many arguments"));
    }
#endif
  p_value_t=tipoSetrange(nargs,result,"setrange");
  return p_value_t;
}

static awk_value_t * do_rename(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_rename: called with too many arguments"));
    }
#endif
  p_value_t=tipoSismember(nargs,result,"rename");
  return p_value_t;
}

static awk_value_t * do_renamenx(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_renamenx: called with too many arguments"));
    }
#endif
  p_value_t=tipoSismember(nargs,result,"renamenx");
  return p_value_t;
}

static awk_value_t * do_restore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_restore: called with too many arguments"));
    }
#endif
  p_value_t=tipoRestore(nargs,result,"restore");
  return p_value_t;
}

static awk_value_t * do_dump(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 2)) {
      lintwarn(ext_id, _("redis_dump: called with too many arguments"));
    }
#endif
  p_value_t=tipoScard(nargs,result,"dump");
  return p_value_t;
}

static awk_value_t * do_move(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_move: called with too many arguments"));
    }
#endif
  p_value_t=tipoSismember(nargs,result,"move");
  return p_value_t;
}

static awk_value_t * do_getrange(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_getrange: called with too many arguments"));
    }
#endif
  p_value_t=tipoSetbit(nargs,result,"getrange");
  return p_value_t;
}

static awk_value_t * do_zremrangebyrank(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zremrangebyrank: called with too many arguments"));
    }
#endif
  p_value_t=tipoSetbit(nargs,result,"zremrangebyrank");
  return p_value_t;
}

static awk_value_t * do_linsertBefore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_linsertBefore: called with too many arguments"));
    }
#endif
  p_value_t=tipoLinsert(nargs,result,"linsertBefore");
  return p_value_t;
}

static awk_value_t * do_linsertAfter(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_linsertAfter: called with too many arguments"));
    }
#endif
  p_value_t=tipoLinsert(nargs,result,"linsertAfter");
  return p_value_t;
}

static awk_value_t * do_zremrangebylex(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zremrangebylex: called with too many arguments"));
    }
#endif
  p_value_t=tipoSmove(nargs,result,"zremrangebylex");
  return p_value_t;
}

static awk_value_t * do_zremrangebyscore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zremrangebyscore: called with too many arguments"));
    }
#endif
  p_value_t=tipoSmove(nargs,result,"zremrangebyscore");
  return p_value_t;
}

static awk_value_t * do_zlexcount(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zlexcount: called with too many arguments"));
    }
#endif
  p_value_t=tipoSmove(nargs,result,"zlexcount");
  return p_value_t;
}

static awk_value_t * do_zrangebyscore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrangebyscore: called with too many arguments"));
    }
#endif
  p_value_t=tipoZrangebylex(nargs,result,"zrangebyscore");
  return p_value_t;
}

static awk_value_t * do_zrevrangebyscore(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrevrangebyscore: called with too many arguments"));
    }
#endif
  p_value_t=tipoZrangebylex(nargs,result,"zrevrangebyscore");
  return p_value_t;
}

static awk_value_t * do_zrangebylex(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zrangebylex: called with too many arguments"));
    }
#endif
  p_value_t=tipoZrangebylex(nargs,result,"zrangebylex");
  return p_value_t;
}

static awk_value_t * do_evalsha(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_evalsha: called with too many arguments"));
    }
#endif
  p_value_t=tipoEvalsha(nargs,result,"evalsha");
  return p_value_t;
}

static awk_value_t * do_eval(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_eval: called with too many arguments"));
    }
#endif
  p_value_t=tipoEvalsha(nargs,result,"eval");
  return p_value_t;
}

static awk_value_t * do_smove(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_smove: called with too many arguments"));
    }
#endif
  p_value_t=tipoSmove(nargs,result,"smove");
  return p_value_t;
}

static awk_value_t * do_srem(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_srem: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"srem");
  return p_value_t;
}

static awk_value_t * do_rpush(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_rpush: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"rpush");
  return p_value_t;
}

static awk_value_t * do_lpush(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_lpush: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"lpush");
  return p_value_t;
}

static awk_value_t * do_zinterstore(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zinterstore: called with too many arguments"));
    }
#endif
  p_value_t=tipoZunionstore(nargs,result,"zinterstore");
  return p_value_t;
}

static awk_value_t * do_zunionstore(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 5)) {
      lintwarn(ext_id, _("redis_zunionstore: called with too many arguments"));
    }
#endif
  p_value_t=tipoZunionstore(nargs,result,"zunionstore");
  return p_value_t;
}

static awk_value_t * do_zadd(int nargs, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 4)) {
      lintwarn(ext_id, _("redis_zadd: called with too many arguments"));
    }
#endif
  p_value_t=tipoZadd(nargs,result,"zadd");
  return p_value_t;
}

static awk_value_t * do_zrem(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_zrem: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"zrem");
  return p_value_t;
}
static awk_value_t * do_sadd(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_sadd: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"sadd");
  return p_value_t;
}

static awk_value_t * do_pfadd(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_pfadd: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"pfadd");
  return p_value_t;
}

static awk_value_t * do_pfmerge(int nargs __UNUSED_V2, awk_value_t *result API_FINFO_ARG) {
  awk_value_t *p_value_t;
#if gawk_api_major_version < 2
    if (do_lint && (nargs > 3)) {
      lintwarn(ext_id, _("redis_pfmerge: called with too many arguments"));
    }
#endif
  p_value_t=tipoSadd(nargs,result,"pfmerge");
  return p_value_t;
}

static awk_bool_t
init_redis(void)
{
   GAWKEXTLIB_COMMON_INIT
   return awk_true;
}

static awk_ext_func_t func_table[] = {
	API_FUNC_MAXMIN("redis_connect", do_connectRedis, 2, 0)
	API_FUNC("redis_scan", do_scan, 4)
	API_FUNC("redis_sadd", do_sadd, 3)
	API_FUNC("redis_smembers", do_smembers, 3)
	API_FUNC("redis_scard", do_scard, 2)
	API_FUNC_MAXMIN("redis_spop", do_spop, 4, 2)
	API_FUNC("redis_sinter", do_sinter, 3)
        API_FUNC("redis_sinterstore",do_sinterstore, 3)
	API_FUNC("redis_sunion", do_sunion, 3)
	API_FUNC("redis_sunionstore", do_sunionstore, 3)
	API_FUNC("redis_sdiffstore", do_sdiffstore, 3)
	API_FUNC("redis_sdiff",	do_sdiff, 3)
	API_FUNC("redis_sismember",do_sismember,3)
	API_FUNC("redis_smove",do_smove, 4)
	API_FUNC("redis_srem",do_srem, 3)
        API_FUNC_MAXMIN("redis_srandmember",do_srandmember, 4, 2)
	API_FUNC_MAXMIN("redis_sscan",do_sscan, 5, 4)
	API_FUNC("redis_disconnect", do_disconnect, 1)
	API_FUNC("redis_close",	do_disconnect, 1)
	API_FUNC("redis_keys", do_keys, 3)
	API_FUNC("redis_mget", do_mget, 3)
	API_FUNC("redis_mset", do_mset, 2)
	API_FUNC("redis_msetnx", do_msetnx, 2)
	API_FUNC("redis_pfadd", do_pfadd, 3)
	API_FUNC("redis_pfcount", do_pfcount, 2)
	API_FUNC("redis_pfmerge", do_pfmerge, 3)
	API_FUNC("redis_hmget", do_hmget, 4)
	API_FUNC("redis_hgetall", do_hgetall, 3)
	API_FUNC("redis_hkeys", do_hkeys, 3)
	API_FUNC("redis_hexists", do_hexists, 3)
	API_FUNC("redis_hvals", do_hvals, 3)
	API_FUNC("redis_hmset", do_hmset, 3)
	API_FUNC("redis_hsetnx", do_hsetnx, 4)
	API_FUNC("redis_hset", do_hset, 4)
	API_FUNC("redis_hget", do_hget, 3)
	API_FUNC("redis_hdel", do_hdel, 3)
	API_FUNC("redis_hincrby", do_hincrby, 4 )
	API_FUNC("redis_hincrbyfloat", do_hincrbyfloat, 4 )
	API_FUNC("redis_hlen", do_hlen, 2 )
	API_FUNC_MAXMIN("redis_hscan", do_hscan, 5, 4)
	API_FUNC("redis_publish", do_publish, 3)
	API_FUNC("redis_subscribe", do_subscribe, 3)
	API_FUNC("redis_psubscribe", do_psubscribe, 3)
	API_FUNC_MAXMIN("redis_unsubscribe", do_unsubscribe, 2, 1)
	API_FUNC_MAXMIN("redis_punsubscribe", do_punsubscribe, 2, 1)
	API_FUNC("redis_getMessage", do_getMessage, 2)
	API_FUNC("redis_object", do_object, 3)
	API_FUNC("redis_select", do_select, 2)
	API_FUNC("redis_geoadd", do_geoadd,3)
	API_FUNC("redis_geohash", do_geohash, 4)
	API_FUNC("redis_geopos", do_geopos, 4)
	API_FUNC_MAXMIN("redis_geodist", do_geodist, 5, 4)
	API_FUNC_MAXMIN("redis_georadius", do_georadius, 9, 7)
	API_FUNC_MAXMIN("redis_georadiusWD", do_georadiusWD, 9, 7)
	API_FUNC_MAXMIN("redis_georadiusWC", do_georadiusWC, 9, 7)
	API_FUNC_MAXMIN("redis_georadiusWDWC", do_georadiusWDWC, 9, 7)
	API_FUNC_MAXMIN("redis_georadiusbymember", do_georadiusbymember, 8, 6)
	API_FUNC_MAXMIN("redis_georadiusbymemberWD", do_georadiusbymemberWD, 8, 6)
	API_FUNC_MAXMIN("redis_georadiusbymemberWC", do_georadiusbymemberWC, 8, 6)
	API_FUNC_MAXMIN("redis_georadiusbymemberWDWC", do_georadiusbymemberWDWC, 8, 6)
	API_FUNC("redis_get", do_get, 2)
	API_FUNC("redis_del", do_del, 2)
	API_FUNC("redis_exists", do_exists, 2)
	API_FUNC("redis_lrange", do_lrange, 5)
	API_FUNC("redis_llen", do_llen, 2)
	API_FUNC("redis_lindex", do_lindex, 3)
	API_FUNC("redis_lpop", do_lpop, 2)
	API_FUNC("redis_lpush",	do_lpush, 3)
	API_FUNC("redis_lset", do_lset, 4)
	API_FUNC("redis_brpop",	do_brpop, 4)
	API_FUNC("redis_blpop",	do_blpop, 4)
	API_FUNC("redis_brpoplpush", do_brpoplpush, 4)
	API_FUNC("redis_linsertBefore",do_linsertBefore, 4 )
	API_FUNC("redis_linsertAfter",do_linsertAfter, 4 )
	API_FUNC("redis_lrem", do_lrem, 4 )
	API_FUNC("redis_ltrim",	do_ltrim, 4 )
	API_FUNC("redis_lpushx", do_lpushx, 3 )
	API_FUNC("redis_rpush",	do_rpush, 3 )
	API_FUNC("redis_rpushx", do_rpushx, 3 )
	API_FUNC("redis_rpop", do_rpop, 2 )
	API_FUNC("redis_rpoplpush",do_rpoplpush, 3 )
	API_FUNC("redis_pipeline",do_pipeline, 1 )
	API_FUNC_MAXMIN("redis_getReply", do_getReply, 2, 1 )
	API_FUNC("redis_getReplyInfo", do_getReplyInfo, 2 )
	API_FUNC("redis_getReplyMass", do_getReplyMass, 1 )
	API_FUNC("redis_type", do_type, 2 )
	API_FUNC("redis_incr", do_incr, 2 )
	API_FUNC("redis_incrbyfloat", do_incrbyfloat, 3 )
	API_FUNC("redis_pttl", do_pttl, 2 )
	API_FUNC("redis_persist", do_persist, 2 )
	API_FUNC("redis_pexpire", do_pexpire, 3 )
	API_FUNC("redis_setbit", do_setbit, 4 )
	API_FUNC("redis_getbit", do_getbit, 3 )
	API_FUNC_MAXMIN("redis_bitpos", do_bitpos, 5, 3)
	API_FUNC("redis_incrby", do_incrby, 3)
	API_FUNC("redis_decrby", do_decrby, 3)
	API_FUNC("redis_decr", do_decr, 2)
	API_FUNC("redis_ttl", do_ttl, 2)
	API_FUNC("redis_expire", do_expire, 3)
	API_FUNC("redis_hstrlen", do_hstrlen, 3)
	API_FUNC("redis_move", do_move, 3 )
	API_FUNC("redis_echo",do_echo, 2 )
	API_FUNC("redis_auth",do_auth, 2 )
	API_FUNC("redis_lastsave",do_lastsave, 1 )
	API_FUNC("redis_bgsave",do_bgsave, 1 )
	API_FUNC("redis_rename",do_rename, 3 )
	API_FUNC("redis_renamenx",do_renamenx, 3 )
	API_FUNC("redis_dump", do_dump, 2 )
	API_FUNC("redis_restore", do_restore, 4 )
	API_FUNC("redis_getrange", do_getrange, 4 )
	API_FUNC("redis_setrange", do_setrange, 4 )
	API_FUNC_MAXMIN("redis_set", do_set, 6, 3 )
	API_FUNC("redis_getset",do_getset, 3 )
	API_FUNC("redis_strlen",do_strlen, 2)
	API_FUNC("redis_append",do_append, 3 )
	API_FUNC("redis_randomkey",do_randomkey, 1 )
	API_FUNC("redis_ping", do_ping, 1 )
	API_FUNC("redis_clientList", do_clientList, 2 )
	API_FUNC("redis_clientGetName", do_clientGetName, 1)
        API_FUNC("redis_clientSetName", do_clientSetName, 2)
        API_FUNC("redis_clientPause", do_clientPause, 2)
	API_FUNC("redis_clientKillAddr", do_clientKillAddr, 2)
	API_FUNC("redis_clientKillId", do_clientKillId, 2)
	API_FUNC("redis_clientKillType", do_clientKillType, 2)
	API_FUNC("redis_flushdb",do_flushdb, 1)
	API_FUNC("redis_flushall",do_flushall, 1)
	API_FUNC("redis_dbsize",do_dbsize, 1)
	API_FUNC_MAXMIN("redis_bitcount",do_bitcount, 4, 2)
	API_FUNC("redis_bitop",do_bitop, 4)
	API_FUNC_MAXMIN("redis_sort",do_sort, 4, 3)
	API_FUNC_MAXMIN("redis_sortStore",do_sortStore, 4, 3)
	API_FUNC_MAXMIN("redis_sortLimit",do_sortLimit, 6, 5)
	API_FUNC_MAXMIN("redis_sortLimitStore",do_sortLimitStore, 6, 5)
	API_FUNC_MAXMIN("redis_zadd", do_zadd, 4, 3)
	API_FUNC("redis_zcard",	do_zcard, 2)
	API_FUNC("redis_zcount", do_zcount, 4)
	API_FUNC("redis_zrem", do_zrem, 3)
	API_FUNC("redis_zrange", do_zrange, 5)
	API_FUNC("redis_zrevrange", do_zrevrange, 5)
	API_FUNC_MAXMIN("redis_zunionstore", do_zunionstore, 5, 3)
	API_FUNC_MAXMIN("redis_zinterstore", do_zinterstore, 5, 3)
	API_FUNC("redis_zrank",	do_zrank, 3)
	API_FUNC("redis_zrangeWithScores",do_zrangeWithScores, 5)
	API_FUNC("redis_zrevrangeWithScores",do_zrevrangeWithScores, 5)
	API_FUNC("redis_zrevrank",do_zrevrank, 3)
	API_FUNC("redis_zscore",do_zscore, 3)
	API_FUNC("redis_zincrby",do_zincrby, 4)
        API_FUNC("redis_zlexcount",do_zlexcount, 4)
        API_FUNC("redis_zrangebylex",do_zrangebylex, 5)
        API_FUNC("redis_zrangebyscore",do_zrangebyscore, 5)
        API_FUNC("redis_zrevrangebyscore",do_zrevrangebyscore, 5)
        API_FUNC("redis_zremrangebylex",do_zremrangebylex, 4)
        API_FUNC("redis_zremrangebyrank",do_zremrangebyrank, 4)
        API_FUNC("redis_zremrangebyscore",do_zremrangebyscore, 4)
	API_FUNC_MAXMIN("redis_zscan", do_zscan, 5, 4 )
	API_FUNC_MAXMIN("redis_script", do_script, 4, 2 )
	API_FUNC("redis_evalsha", do_evalsha, 5 )
	API_FUNC("redis_eval", do_eval, 5 )
	API_FUNC("redis_multi", do_multi, 1 )
	API_FUNC("redis_exec", do_exec, 2 )
	API_FUNC("redis_watch", do_watch, 2 )
	API_FUNC("redis_unwatch", do_unwatch, 1 )
	API_FUNC("redis_discard", do_discard, 1 )
	API_FUNC_MAXMIN("redis_info", do_info, 3, 2 )
	API_FUNC_MAXMIN("redis_slowlog", do_slowlog, 4, 2 )
	API_FUNC("redis_configSet", do_configSet, 3 )
	API_FUNC("redis_configGet", do_configGet, 3 )
	API_FUNC("redis_configResetStat", do_configResetStat, 1 )
	API_FUNC_MAXMIN("redis_pubsub", do_pubsub, 4, 2)
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, redis, "")
