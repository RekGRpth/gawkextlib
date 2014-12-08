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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */
#if defined(__GNUC__) && ((__GNUC__==4 && __GNUC_MINOR__>=2) || (__GNUC__>4))
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif

#include "common.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hiredis/hiredis.h>


#include <netinet/in.h>		/* Internet address structures */
#include <sys/socket.h>		/* socket interface functions  */
#include <netdb.h>		/* host to IP resolution       */

#include <sys/types.h>
#include <sys/stat.h>

#define TOPC   100 //Top Connection
#define INCRPIPE   1000 //the pipeline increments

char *mem_str(char **,const char *,int);
void  free_mem_str(char **,int);
enum format_type {
   INDEF, CONN, NUMBER, STRING, ARRAY, ST_AR
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
awk_value_t * tipoExec(int,awk_value_t *,const char *);
awk_value_t * tipoEvalsha(int,awk_value_t *,const char *);
awk_value_t * tipoZrangebylex(int,awk_value_t *,const char *);
awk_value_t * tipoConnect(int,awk_value_t *,const char *);
awk_value_t * tipoScript(int,awk_value_t *,const char *);
awk_value_t * tipoScard(int,awk_value_t *,const char *);
awk_value_t * tipoRandomkey(int,awk_value_t *,const char *);
awk_value_t * tipoHincrby(int,awk_value_t *,const char *);
awk_value_t * tipoSismember(int,awk_value_t *,const char *);
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
awk_value_t * tipoSscan(int,awk_value_t *,const char *);
awk_value_t * tipoPipeline(int,awk_value_t *,const char *);
awk_value_t * tipoGetReply(int,awk_value_t *,const char *);
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

awk_value_t * theReply(awk_value_t *);
char ** getArrayContent(awk_array_t, size_t, const char *, int *);
char ** getArrayContentCont(awk_array_t, size_t, const char *, int *,int);
int getArrayContentSecond(awk_array_t, int, char **);

int theReplyArrayS(awk_array_t);
int theReplyArray(awk_array_t, enum resultArray, size_t);
int theReplyArrayK1(awk_array_t, redisReply *);
int theReplyArray1(awk_array_t, enum resultArray, size_t);
int theReplyScan(awk_array_t,char *);

static  size_t pipel[TOPC][2];

static  redisContext *c[TOPC];
static  redisReply *reply;
static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;
static awk_bool_t init_redis(void);
static awk_bool_t (*init_func)(void) = init_redis;
static const char *ext_version = "redis extension: version 1.0";

static void
array_set(awk_array_t array, const char *sub, awk_value_t *value)
{
	awk_value_t idx;

	set_array_element(array,make_const_string(sub, strlen(sub), & idx),value);

}


int plugin_is_GPL_compatible;

static awk_value_t * do_disconnect(int nargs, awk_value_t *result) {
   int ret=1;   
   int ival;
   awk_value_t val;
   if(nargs==1) {
    if(!get_argument(0, AWK_NUMBER, & val)) {
     set_ERRNO(_("disconnect/closeRedis: argument 1 is present but not is a number"));
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
       set_ERRNO(_("disconnect/closeRedis: the argument does not correspond to a connection"));
       ret=-11;
     }
    }
    else {
     set_ERRNO(_("disconnect/closeRedis: argument out of range"));
     ret=-1;
    }
   }
   else {
     set_ERRNO(_("disconnect/closeRedis: need one argument"));
     ret=-1;
   }
   return make_number(ret, result);
}

static awk_value_t * do_hvals(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoKeys(nargs,result,"hvals");
   return p_value_t;
}

static awk_value_t * do_srandmember(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   if(nargs==2) {
     p_value_t=tipoScard(nargs,result,"srandmember");
   }
   else {
     p_value_t=tipoSrandmember(nargs,result,"srandmember");
   }
   return p_value_t;
}

static awk_value_t * do_hscan(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSscan(nargs,result,"hscan");
   return p_value_t;
}

static awk_value_t * do_zscan(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSscan(nargs,result,"zscan");
   return p_value_t;
}

static awk_value_t * do_sscan(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSscan(nargs,result,"sscan");
   return p_value_t;
}

static awk_value_t * do_scan(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScan(nargs,result,"scan");
   return p_value_t;
}

static awk_value_t * do_lpop(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"lpop");
   return p_value_t;
}

static awk_value_t * do_rpop(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"rpop");
   return p_value_t;
}

static awk_value_t * do_spop(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"spop");
   return p_value_t;
}

static awk_value_t * do_incrby(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"incrby");
   return p_value_t;
}

static awk_value_t * do_lindex(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"lindex");
   return p_value_t;
}

static awk_value_t * do_decrby(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"decrby");
   return p_value_t;
}

static awk_value_t * do_incrbyfloat(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"incrbyfloat");
   return p_value_t;
}

static awk_value_t * do_getbit(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"getbit");
   return p_value_t;
}

static awk_value_t * do_bitpos(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoBitpos(nargs,result,"bitpos");
   return p_value_t;
}

static awk_value_t * do_setbit(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSetbit(nargs,result,"setbit");
   return p_value_t;
}

static awk_value_t * do_pexpire(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"pexpire");
   return p_value_t;
}

static awk_value_t * do_expire(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoExpire(nargs,result,"expire");
   return p_value_t;
}

static awk_value_t * do_decr(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"decr");
   return p_value_t;
}

static awk_value_t * do_incr(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"incr");
   return p_value_t;
}

static awk_value_t * do_persist(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"persist");
   return p_value_t;
}

static awk_value_t * do_pttl(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"pttl");
   return p_value_t;
}

static awk_value_t * do_ttl(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"ttl");
   return p_value_t;
}

static awk_value_t * do_type(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"type");
   return p_value_t;
}

static awk_value_t * do_llen(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"llen");
   return p_value_t;
}
static awk_value_t * do_hlen(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"hlen");
   return p_value_t;
}

static awk_value_t * do_strlen(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"strlen");
   return p_value_t;
}

static awk_value_t * do_script(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScript(nargs,result,"script");
   return p_value_t;
}

static awk_value_t * do_bitop(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoBitop(nargs,result,"bitop");
   return p_value_t;
}

static awk_value_t * do_bitcount(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoBitcount(nargs,result,"bitcount");
   return p_value_t;

}
static awk_value_t * do_zcard(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"zcard");
   return p_value_t;
}

static awk_value_t * do_scard(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"scard");
   return p_value_t;
}

static awk_value_t * do_echo(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoScard(nargs,result,"echo");
   return p_value_t;
}

static awk_value_t * do_hkeys(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoKeys(nargs,result,"hkeys");
   return p_value_t;
}

static awk_value_t * do_zrevrank(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"zrevrank");
   return p_value_t;
}

static awk_value_t * do_zscore(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"zscore");
   return p_value_t;
}

static awk_value_t * do_zrank(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"zrank");
   return p_value_t;
}

static awk_value_t * do_smembers(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoKeys(nargs,result,"smembers");
   return p_value_t;
}

static awk_value_t * do_hget(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"hget");
   return p_value_t;
}

static awk_value_t * do_hexists(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"hexists");
   return p_value_t;
}

static awk_value_t * do_getset(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"getset");
   return p_value_t;
}

static awk_value_t * do_set(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSet(nargs,result,"set");
   return p_value_t;
}

static awk_value_t * do_publish(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"publish");
   return p_value_t;
}

static awk_value_t * do_append(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"append");
   return p_value_t;
}

static awk_value_t * do_rpoplpush(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"rpoplpush");
   return p_value_t;
}

static awk_value_t * do_rpushx(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"rpushx");
   return p_value_t;
}

static awk_value_t * do_lpushx(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"lpushx");
   return p_value_t;
}

static awk_value_t * do_sortStore(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSort(nargs,result,"sortStore");
   return p_value_t;
}

static awk_value_t * do_sortLimitStore(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSortLimit(nargs,result,"sortLimitStore");
   return p_value_t;
}

static awk_value_t * do_sortLimit(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSortLimit(nargs,result,"sortLimit");
   return p_value_t;
}

static awk_value_t * do_sort(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSort(nargs,result,"sort");
   return p_value_t;
}

static awk_value_t * do_sismember(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSismember(nargs,result,"sismember");
   return p_value_t;
}

static awk_value_t * do_lrange(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoZrange(nargs,result,"lrange");
   return p_value_t;
}

static awk_value_t * do_ltrim(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSetbit(nargs,result,"ltrim");
   return p_value_t;
}

static awk_value_t * do_zcount(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoSmove(nargs,result,"zcount");
   return p_value_t;
}

static awk_value_t * do_zrangeWithScores(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoZrange(nargs,result,"zrangeWithScores");
   return p_value_t;
}

static awk_value_t * do_zrevrangeWithScores(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoZrange(nargs,result,"zrevrangeWithScores");
   return p_value_t;
}

static awk_value_t * do_zrange(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoZrange(nargs,result,"zrange");
   return p_value_t;
}

static awk_value_t * do_zrevrange(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoZrange(nargs,result,"zrevrange");
   return p_value_t;
}

static awk_value_t * do_hmset(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoHmset(nargs,result,"hmset");
   return p_value_t;
}

static awk_value_t * do_hmget(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoHmget(nargs,result,"hmget");
   return p_value_t;
}

static awk_value_t * do_mset(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoMset(nargs,result,"mset");
   return p_value_t;
}

static awk_value_t * do_msetnx(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoMset(nargs,result,"msetnx");
   return p_value_t;
}

static awk_value_t * do_mget(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoMget(nargs,result,"mget");
   return p_value_t;
}

static awk_value_t * do_connectRedis(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoConnect(nargs,result,"connectRedis");
   return p_value_t;
}

static awk_value_t * do_keys(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoKeys(nargs,result,"keys");
   return p_value_t;
}

static awk_value_t * do_connect(int nargs, awk_value_t *result) {
  int ret;
  unsigned int i;
  char str[240];
  struct timeval timeout = { 1, 500000 }; // 1.5 seconds
  if(nargs!=0) {
     set_ERRNO(_("connection:  not require argument"));
     return make_number(-1, result);
  }
  for(i=0;i<TOPC;i++) {
    if(!c[i]) {
      break;
    }
  }
  if(i==TOPC) {
     set_ERRNO(_("connection: is not possible over the limit"));
     return make_number(-1, result);
  }
  c[i] = redisConnectWithTimeout((const char*)"127.0.0.1", 6379, timeout);
  if (c[i]->err) {
    sprintf(str,"connection: error %s\n", c[i]->errstr);
    set_ERRNO(_(str));
    c[i]=(redisContext *)NULL;
    ret=-1;
  }
  else {
    ret=i;
  }
  return make_number(ret, result);
}

void free_mem_str(char **p,int n) {
  int i;
  for(i=0;i<n;i++){
    free((void *)p[i]);
  }
  free((void *)p);
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
   char **sts;
   awk_value_t idx,val;
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
         make_const_string(reply->element[0]->str,reply->element[0]->len, & tmp));
      for(j=0; j < reply->element[1]->elements ;j++) {
        sprintf(str,"%d",j+2);
	array_set(array,str,
	make_const_string(reply->element[1]->element[j]->str,reply->element[1]->element[j]->len, & tmp));
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
         make_const_string(reply->element[0]->str,reply->element[0]->len, & tmp));
      }
    }
    return 1;
}

int theReplyArrayK1(awk_array_t array, redisReply *rep ){
    size_t j;
    char str[15],str1[15], stnull[1];
    awk_value_t tmp,value,ind;
    awk_array_t a_cookie;
    stnull[0]='\0';
    if(rep->elements==0) {
      return 0;
    }
    if(rep->elements>0) {
     for (j = 0; j < rep->elements; j++) {
         sprintf(str, "%d", j+1);
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
             array_set(array,str,make_const_string(stnull,strlen(stnull), & tmp));
	   }
	   else {
             array_set(array,str,
                make_const_string(rep->element[j]->str,rep->element[j]->len, & tmp));
	   }
         }
       }
    }
    return 1;
}

int theReplyArray(awk_array_t array, enum resultArray r,size_t incr){
    size_t j;
    char str[15],str1[15], stnull[1];
    awk_value_t tmp;
    stnull[0]='\0';
    if(reply->elements==0) {
      return 0;
    }
    if(reply->elements>0) {
     for (j = 0; j < reply->elements; j+=incr) {
       if(r==KEYNUMBER) {
         sprintf(str, "%d", j+1);
	 if(reply->element[j]->type==REDIS_REPLY_INTEGER) {
	   sprintf(str1, "%lld", reply->element[j]->integer);
           array_set(array,str,make_const_string(str1,strlen(str1),& tmp));
         }
	 if(reply->element[j]->type==REDIS_REPLY_STRING || reply->element[j]->type==REDIS_REPLY_STATUS) {
	   if(reply->element[j]->str==NULL) {
             array_set(array,str,make_const_string(stnull,strlen(stnull), & tmp));
	   }
	   else {
             array_set(array,str,
                make_const_string(reply->element[j]->str,reply->element[j]->len, & tmp));
	   }
         }
       }
       else {
        if(r==KEYSTRING) {
            array_set(array,reply->element[j]->str,
               make_const_string(reply->element[j+1]->str,reply->element[j+1]->len, & tmp));
	}
       }
     }
    }
    return 1;
}

int theReplyArray1(awk_array_t array, enum resultArray r,size_t incr){
    size_t j;
    char str[15], stnull[1];
    awk_value_t tmp;
    stnull[0]='\0';
    for (j = 0; j < reply->elements; j+=incr) {
       if(r==KEYNUMBER) {
         sprintf(str, "%d", j+1);
	 if(reply->element[j]->str==NULL) {
           array_set(array,str,make_const_string(stnull,strlen(stnull), & tmp));
	 }
	 else {
           array_set(array,str,
                make_const_string(reply->element[j]->str,reply->element[j]->len, & tmp));
         }
       }
       else {
        if(r==KEYSTRING) {
            array_set(array,reply->element[j]->str,
               make_const_string(reply->element[j+1]->str,reply->element[j+1]->len, & tmp));
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
    sprintf(str,"%d",j+1);
    array_set(array,str,
      make_const_string(reply->element[1]->element[j]->str,reply->element[1]->element[j]->len, & tmp));
   }
   return 1;
}

awk_value_t * theReply(awk_value_t *result) {
    awk_value_t *pstr;
    char stnull[1];
    stnull[0]='\0';
    pstr=NULL;
    if(reply->type==REDIS_REPLY_STATUS || reply->type==REDIS_REPLY_STRING){
       if(reply->type==REDIS_REPLY_STATUS){
	 if(strcmp(reply->str,"OK")==0) {
           pstr=make_number(1,result);
	 }
       }
       if(pstr==NULL) {
         if(reply->str==NULL) {
           pstr=make_string_malloc(stnull,strlen(stnull),result);
         }
	 else {
           pstr=make_string_malloc(reply->str,reply->len,result);
         }
       }
    }
    if(reply->type==REDIS_REPLY_ERROR){
      set_ERRNO(_(reply->str));
      pstr=make_number(-1, result);
    }
    if(reply->type==REDIS_REPLY_NIL){
      pstr=make_string_malloc("\0",0,result);
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
    if(!validate_conn(ival,str,"command",&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s",command);
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArrayK1(array_ou,reply)) {
          pstr=make_number(1, result);
        }
        else {
          pstr=make_number(0, result);
        }
      }
      else {
        pstr=theReply(result); 
      }
      if(reply->type==REDIS_REPLY_ERROR){
        set_ERRNO(_(reply->str));
        pstr=make_number(-1, result);
      }
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s",command);
      pipel[pconn][1]++;
      return make_number(1, result);
    }
  }
  else {
    sprintf(str,"%s need two arguments",command);
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
    if(!validate_conn(ival,str,"command",&pconn)) {
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
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArrayK1(array_ou,reply)) {
          pstr=make_number(1, result);
        }
        else {
          pstr=make_number(0, result);
        }
      }
      else {
        pstr=theReply(result); 
      }
      freeReplyObject(reply);
    }
    else {
      redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      pipel[pconn][1]++;
      return make_number(1, result);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s need five arguments",command);
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
  enum resultArray k=KEYNUMBER;
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
    if(!validate_conn(ival,str,"command",&pconn)) {
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
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
	  pstr=make_number(1, result);
        }
        else {
	  pstr=make_number(0, result);
        }
      }
      else {
	  pstr=theReply(result);
      }
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      return make_number(1, result);
    }
  }
  else {
    sprintf(str,"%s need five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSscan(int nargs,awk_value_t *result,const char *command) {
  int r,ival,ival2;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, array_param;
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
    if(!validate_conn(ival,str,"command",&pconn)) {
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
    if (reply->type == REDIS_REPLY_ARRAY) {
      if(theReplyArrayS(array)) {
         freeReplyObject(reply);
         return make_number(1, result);
      }
      else{
         freeReplyObject(reply);
         return make_number(0, result);
      }
    }
    if(reply->type==REDIS_REPLY_ERROR){
       set_ERRNO(_(reply->str));
       freeReplyObject(reply);
       return make_number(-1, result);
    }
  }
  else {
    sprintf(str,"%s need three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return make_number(1, result);
}

awk_value_t * tipoScan(int nargs,awk_value_t *result,const char *command) {
  int r,ival,ival1;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, array_param;
  awk_array_t array;
  enum format_type there[4];
  int pconn=-1;

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
    if(!validate_conn(ival,str,"command",&pconn)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    get_argument(1, AWK_NUMBER, & val1);
    ival1=val1.num_value;
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    if(nargs==4) {
      get_argument(3, AWK_STRING, & val2);
      if(pconn==-1) {
        reply = redisCommand(c[ival],"%s %d MATCH %s",command,ival1,val2.str_value.str);      
      }
      else {
        redisAppendCommand(c[pconn],"%s %d MATCH %s",command,ival1,val2.str_value.str);
        pipel[pconn][1]++;
        return make_number(1, result);
      }
    }
    else {
      if(pconn==-1) {
        reply = redisCommand(c[ival],"%s %d",command,ival1);
      }
      else {
        redisAppendCommand(c[ival],"%s %d",command,ival1);
        pipel[pconn][1]++;
        return make_number(1, result);
      }
    }
    if (reply->type == REDIS_REPLY_ARRAY) {
      if(theReplyArrayS(array)) {
         freeReplyObject(reply);
	 return make_number(1, result);
      }
      else {
         freeReplyObject(reply);
	 return make_number(0, result);
      }
    }
  }
  else {
    sprintf(str,"%s need three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return make_number(1, result);
}

static awk_value_t * do_getReply(int nargs, awk_value_t *result) {
   awk_value_t *p_value_t;
   p_value_t=tipoGetReply(nargs,result,"getReply");
   return p_value_t;
}

static awk_value_t * do_pipeline(int nargs, awk_value_t *result) {
   int ival,ret;
   char str[250];
   awk_value_t val;
   int pconn=-1;
   if(nargs==1) {
    if(!get_argument(0, AWK_NUMBER, & val)) {
      set_ERRNO(_("pipeline: argument 1 is not a format valid connection"));
      return make_number(-1, result);
    }
    ival=val.num_value;
    if(!validate_conn(ival,str,"pipeline",&pconn)){
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if(pipel[ival][0]) {
      set_ERRNO(_("pipeline: existes already a pipeline for this connection"));
      return make_number(-1, result);
    }
    pipel[ival][0]=1;
    ret=ival+INCRPIPE;
   }
   else {
     set_ERRNO(_("pipeline: need one argument"));
     ret=-1;
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
      pstr=theReply(result); 
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %d",command,ival1);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need need two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoRandomkey(int nargs,awk_value_t *result,const char *command) {
  int r,ival;
  struct command valid;
  char str[240];
  awk_value_t val, *pstr;
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
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s",command);
      pstr=theReply(result); 
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s",command);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need one argument",command);
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
      sprintf(str,"connection: error %s\n", c[i]->errstr);
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
  int r,ival,flush,kill,load,exists,count,earg;
  struct command valid;
  char str[240],**sts;
  awk_value_t val0,val,val1,array_param, *pstr;
  awk_array_t array_in,array_ou;
  enum resultArray k=KEYNUMBER;
  enum format_type there[4];
  earg=flush=kill=load=exists=0;
  sts=NULL;
  pstr=NULL;
  array_ou=0;
  int pconn=-1;
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
    else { // ERROR
      sprintf(str,"%s need a valid subcommand",command);
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
    }
    if(load){
      get_argument(2, AWK_STRING, & val1);
    }
    if(pconn==-1) {
     if(load) {
      reply = redisCommand(c[ival],"%s %s %s",command,"LOAD",val1.str_value.str);
     }
     if(flush) {
      reply = redisCommand(c[ival],"%s %s",command,"FLUSH");
     }
     if(kill) {
      reply = redisCommand(c[ival],"%s %s",command,"KILL");
     }
     if(exists) {
      reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      free_mem_str(sts,count);
     }
     if (reply->type == REDIS_REPLY_ARRAY) {
        if(!theReplyArray(array_ou,k,1)) {
          pstr=make_number(0, result);
        }
	else{
          pstr=make_number(1, result);
        }
     }
     else {
        pstr=theReply(result); 
     }
     freeReplyObject(reply);
    }
    else {
     if(load) {
      redisAppendCommand(c[pconn],"%s %s %s",command,"LOAD",val1.str_value.str);
     }
     if(flush) {
      redisAppendCommand(c[pconn],"%s %s",command,"FLUSH");
     }
     if(kill) {
      redisAppendCommand(c[pconn],"%s %s",command,"KILL");
     }
     if(exists) {
      redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      free_mem_str(sts,count);
     }
     pipel[pconn][1]++;
     pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need two, three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoScard(int nargs,awk_value_t *result,const char *command) {
  int r,ival;
  struct command valid;
  char str[240];
  awk_value_t val, *pstr;
  enum format_type there[2];
  int pconn=-1;

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
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s",command,val.str_value.str);
      pstr=theReply(result); 
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s",command,val.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBrpop(int nargs,awk_value_t *result,const char *command) {
   int count,r,ival;
   struct command valid;
   char str[240], **sts;
   awk_value_t val, val1, val2, array_param, *pstr;
   awk_array_t array_in, array_ou;
   enum format_type there[4];
   enum resultArray k=KEYNUMBER;
   int pconn=-1;   
   pstr=NULL;
   sts=NULL;
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
    }
    if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & val1);
    }
    get_argument(2, AWK_ARRAY, & array_param);
    array_ou = array_param.array_cookie;

    if(pconn==-1) {
      if(there[1]==STRING) {
        reply = redisCommand(c[ival],"%s %s %s",command,val1.str_value.str,val2.str_value.str);
      }
      else {
        reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      }
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(!theReplyArray(array_ou,k,1)) {
          pstr=make_number(0, result);
        }
	else{
          pstr=make_number(1, result);
        }
      }
      else {
        pstr=theReply(result); 
      }
      freeReplyObject(reply);
    }
    else {
      if(there[1]==STRING) {
        redisAppendCommand(c[pconn],"%s %s %s",command,val1.str_value.str,val2.str_value.str);
      }
      else {
        redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      }
      pipel[pconn][1]++;
      pstr=make_number(1, result);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s need four arguments",command);
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
   enum resultArray k=KEYNUMBER;
   int pconn=-1;   
   pstr=NULL;
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
    if(pconn==-1) {
      reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(!theReplyArray(array_ou,k,1)) {
          pstr=make_number(0, result);
        }
	else{
          pstr=make_number(1, result);
        }
      }
      else {
        pstr=theReply(result); 
      }
      freeReplyObject(reply);
    }
    else {
      redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      pipel[pconn][1]++;
      pstr=make_number(1, result);
    }
    free_mem_str(sts,count);
  }
  else {
    sprintf(str,"%s need three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoUnsubscribe(int nargs,awk_value_t *result,const char *command) {
  int count,r;
  char **sts;
  int ival;
  struct command valid;
  char str[240];
  awk_value_t val, array_param, name_channel, *pstr;
  awk_array_t array;
  enum format_type there[2];
  int pconn=-1;
  pstr=(awk_value_t *)NULL;
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
    if(nargs==1) {
      if(pconn==-1) {
        reply = redisCommand(c[ival],"%s",command);
        freeReplyObject(reply);
      }
      else {
         redisAppendCommand(c[pconn],"%s",command);
         pipel[pconn][1]++;
      }
      pstr=make_number(1, result);
      return pstr;
    }
    if(there[1]==STRING) {
      get_argument(1, AWK_STRING, & name_channel);
      if(pconn==-1) {
        reply = redisCommand(c[ival],"%s %s",command,name_channel.str_value.str);
        freeReplyObject(reply);
      }
      else {
        redisAppendCommand(c[pconn],"%s %s",command,name_channel.str_value.str);
        pipel[pconn][1]++;
      }
      pstr=make_number(1, result);
    }
    else {
      // the argument is an array
      get_argument(1, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,1,command,&count);
      if(pconn==-1) {
        reply = redisCommandArgv(c[ival],count,(const char**)sts,NULL);
        freeReplyObject(reply);
      }
      else {
        redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
        pipel[pconn][1]++;
      }
      free_mem_str(sts,count);
      pstr=make_number(1, result);
    }
  }
  else {
    sprintf(str,"%s need one or two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSubscribe(int nargs,awk_value_t *result,const char *command) {
  int r;
  char **sts;
  int count;
  int ival=0;
  struct command valid;
  char str[240];
  awk_value_t val, array_param, name_channel, *pstr;
  awk_array_t array;
  enum format_type there[3];
  int pconn=-1;
  pstr=(awk_value_t *)NULL;
  strcpy(valid.name,command); 
  valid.type[0]=CONN;
  if(nargs==1 && (strcmp(command,"unsubscribe")==0)) {
    valid.num=1;
    if(!validate(valid,str,&r,there)) {
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    reply = redisCommand(c[ival],"%s",command);
    freeReplyObject(reply);
    pstr=make_number(1, result);
    return pstr;
  }
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
      if(pconn==-1) {
         reply = redisCommand(c[ival],"%s %s",command,name_channel.str_value.str);
         pstr=theReply(result); 
         freeReplyObject(reply);
      }
      else {
         redisAppendCommand(c[pconn],"%s %s",command,name_channel.str_value.str);
         pipel[pconn][1]++;
         pstr=make_number(1, result);
      }
    }
    else {
      // is an array calling to CommandArgv
      get_argument(1, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,1,command,&count);
      if(pconn==-1) {
        reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
        pstr=theReply(result); 
        freeReplyObject(reply);
      }
      else {
        redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
        pipel[pconn][1]++;
        pstr=make_number(1, result);
      }
      free_mem_str(sts,count);
    }
  }
  else {
    sprintf(str,"%s need two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSadd(int nargs,awk_value_t *result,const char *command) {
  int r;
  char **sts;
  int count;
  int ival;
  struct command valid;
  char str[240];
  awk_value_t val, array_param, name_set, *pstr;
  awk_array_t array;
  enum format_type there[3];
  int pconn=-1;
  pstr=(awk_value_t *)NULL;
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
    get_argument(1, AWK_STRING, & name_set);
    if(there[2]==STRING) {
      get_argument(2, AWK_STRING, & val);
      if(pconn==-1) {
         reply = redisCommand(c[ival],"%s %s %s",command,name_set.str_value.str,val.str_value.str);
         pstr=theReply(result); 
         freeReplyObject(reply);
      }
      else {
         redisAppendCommand(c[pconn],"%s %s %s",command,name_set.str_value.str,val.str_value.str);
         pipel[pconn][1]++;
         pstr=make_number(1, result);
      }
    }
    else {
      // is an array calling to commandARGV
      get_argument(2, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,2,command,&count);
      mem_str(sts,name_set.str_value.str,1);
      if(pconn==-1) {
        reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
        pstr=theReply(result);
        freeReplyObject(reply);
      }
      else {
        redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
        pipel[pconn][1]++;
        pstr=make_number(1, result);
      }
      free_mem_str(sts,count);
    }
  }
  else {
    sprintf(str,"%s need three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGetrange(int nargs,awk_value_t *result,const char *command) {
  int ival,ival3,ival2,r;
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
    get_argument(2, AWK_NUMBER, & val2);
    ival2=val2.num_value;
    get_argument(3, AWK_NUMBER, & val3);
    ival3=val3.num_value;
    if(pconn==-1) {
       reply = redisCommand(c[ival],"%s %s %d %d",command,val1.str_value.str,ival2,ival3);
       pstr=theReply(result);
       freeReplyObject(reply);
     }
     else {
       redisAppendCommand(c[pconn],"%s %s %d %d",command,val1.str_value.str,ival2,ival3);
       pipel[pconn][1]++;
       pstr=make_number(1,result);
     }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZadd(int nargs,awk_value_t *result,const char *command) {
  int ival,r,count;
  struct command valid;
  char str[240], **sts;
  awk_value_t array_param, val, val1, val2, val3, *pstr;
  awk_array_t array;
  enum format_type there[4];
  int pconn=-1;
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
    sprintf(str,"%s need three or four arguments",command);
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
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    get_argument(2, AWK_ARRAY, & array_param);
    array = array_param.array_cookie;
    sts=getArrayContent(array,2,command,&count);
    mem_str(sts,val1.str_value.str,1);
    if(pconn==-1) {
      reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
    free_mem_str(sts,count);
  }
  return pstr;
}

awk_value_t * tipoSetrange(int nargs,awk_value_t *result,const char *command) {
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
      reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBitcount(int nargs,awk_value_t *result,const char *command) {
  int ival,ival2,ival3,r;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, *pstr;
  enum format_type there[4];
  ival2=ival3=0;
  int pconn=-1;
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
    if(nargs==4) {
      get_argument(2, AWK_STRING, & val2);
      ival2=val2.num_value;
      get_argument(3, AWK_NUMBER, & val3);
      ival3=val3.num_value;
    }
    if(pconn==-1) {
      if(nargs==4) {
        reply = redisCommand(c[ival],"%s %s %d %d",command,val1.str_value.str,ival2,ival3);
      }
      else {
        reply = redisCommand(c[ival],"%s %s",command,val1.str_value.str);
      }
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      if(nargs==4) {
        redisAppendCommand(c[pconn],"%s %s %d %d",command,val1.str_value.str,ival2,ival3);
      }
      else {
        redisAppendCommand(c[pconn],"%s %s",command,val1.str_value.str);
      }
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need two or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBitpos(int nargs,awk_value_t *result,const char *command) {
  int ival,ival2,ival3,ival4,r;
  struct command valid;
  char str[240];
  awk_value_t val, val1, val2, val3, val4, *pstr;
  enum format_type there[5];
  ival3=ival4=0;
  int pconn=-1;
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
    get_argument(2, AWK_NUMBER, & val2);
    ival2=val2.num_value;
    if(nargs==4){
      get_argument(3, AWK_NUMBER, & val3);
      ival3=val3.num_value;
    }
    if(nargs==5){
      get_argument(3, AWK_NUMBER, & val3);
      ival3=val3.num_value;
      get_argument(4, AWK_NUMBER, & val4);
      ival4=val4.num_value;
    }
    if(pconn==-1) {
      if(nargs==3){
       reply = redisCommand(c[ival],"%s %s %d",command,val1.str_value.str,ival2);   
      }
      if(nargs==4){
       reply = redisCommand(c[ival],"%s %s %d %d",command,val1.str_value.str,ival2,ival3);
      }
      if(nargs==5){
       reply = redisCommand(c[ival],"%s %s %d %d %d",command,val1.str_value.str,ival2,ival3,ival4);
      } 
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      if(nargs==3){
       redisAppendCommand(c[pconn],"%s %s %d",command,val1.str_value.str,ival2);
      }
      if(nargs==4){
       redisAppendCommand(c[pconn],"%s %s %d %d",command,val1.str_value.str,ival2,ival3);
      }
      if(nargs==5){
       redisAppendCommand(c[pconn],"%s %s %d %d %d",command,val1.str_value.str,ival2,ival3,ival4);
      }
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need three, four or five arguments",command);
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
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoBitop(int nargs,awk_value_t *result,const char *command) {
  size_t i;
  int count,ival,r;
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
  int pconn=-1;
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
    }
    if(there[3]==ARRAY) {
      get_argument(3, AWK_ARRAY, & array_param);
      array = array_param.array_cookie;
      sts=getArrayContent(array,3,command,&count);
      mem_str(sts,val1.str_value.str,1);
      mem_str(sts,val2.str_value.str,2);
    }
    if(bop[i]==NOT && there[3]==ARRAY) {
     sprintf(str,"%s Operator NOT, need only one source key",command);
     set_ERRNO(_(str));
     return make_number(-1, result);
    }
    if(pconn==-1) {
     if(there[3]==STRING){
       reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
       pstr=theReply(result);
       freeReplyObject(reply);
     }
     else {
       reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
       pstr=theReply(result);
       freeReplyObject(reply);
       free_mem_str(sts,count);
     }
    }
    else {
     if(there[3]==STRING){
       redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
     }
     else {
       redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
       free_mem_str(sts,count);
     }
     pipel[pconn][1]++;
     pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoLinsert(int nargs,awk_value_t *result,const char *command) {
  int ival,r;
  struct command valid;
  char str[240],cmd[]="linsert", where[10]="AFTER";
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
    if(strcmp(command,"linsertBefore")==0) {
      strcpy(where,"BEFORE");
    }
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s %s %s %s",cmd,val1.str_value.str,where,val2.str_value.str,val3.str_value.str);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s %s",cmd,val1.str_value.str,where,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSmove(int nargs,awk_value_t *result,const char *command) {
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
      reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZincrby(int nargs,awk_value_t *result,const char *command) {
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
      reply = redisCommand(c[ival],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoHincrby(int nargs,awk_value_t *result,const char *command) {
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
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %s",command,val1.str_value.str,val2.str_value.str,val3.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
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
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s %b",command,val1.str_value.str,val2.str_value.str,val3.str_value.str,val3.str_value.len);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
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
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s",command,val1.str_value.str,val2.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSet(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   int i;
   struct command valid;
   char str[240], **sts=NULL;
   awk_value_t val, mbr, mbr1, *pstr;
   enum format_type there[3];
   int pconn=-1;
   pstr=(awk_value_t *)NULL;
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
    if(nargs > 3) {
     sts=(char **)malloc((nargs)*sizeof(char *));
     mem_str(sts,command,0);
     mem_str(sts,val.str_value.str,1);
     mem_str(sts,mbr.str_value.str,2);
     for(i=3;i<nargs;i++) {
       get_argument(i, AWK_STRING, & mbr1);
       mem_str(sts,mbr1.str_value.str,i);
     }
    }
    if(pconn==-1){ 
      if(nargs==3) {
        reply = redisCommand(c[ival],"%s %s %s",command,val.str_value.str,mbr.str_value.str);
      }
      if(nargs>3) {
        reply = redisCommandArgv(c[ival],nargs,(const char **)sts,NULL);
      }
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      if(nargs==3) {
        redisAppendCommand(c[pconn],"%s %s %s",command,val.str_value.str,mbr.str_value.str);
      }
      if(nargs>3) {
        redisAppendCommandArgv(c[pconn],nargs,(const char **)sts,NULL);
      }
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s: arguments must be between three and six",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSismember(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   struct command valid;
   char str[240];
   awk_value_t val, mbr, *pstr;
   enum format_type there[3];
   int pconn=-1;
   pstr=(awk_value_t *)NULL;
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
    if(pconn==-1){ 
      reply = redisCommand(c[ival],"%s %s %s",command,val.str_value.str,mbr.str_value.str);
      pstr=theReply(result);
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s",command,val.str_value.str,mbr.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need three arguments",command);
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
   enum resultArray k=KEYNUMBER;
   pstr=NULL;
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
    if(pconn==-1 || pipel[pconn][1]<=0) {
      sprintf(str,"%s: No such reply, nothing to getReply\n",command);
      set_ERRNO(_(str));
      return make_number(-1, result);
    }
    if((ret=redisGetReply(c[pconn],(void **)&reply))==REDIS_OK) {
      pipel[pconn][1]--;
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
           pstr=make_number(1, result);
        }
	else {
           pstr=make_number(0, result);
	}
      }
      else {
        pstr=theReply(result); 
      }
      freeReplyObject(reply);
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
    sprintf(str,"%s need two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoGetMessage(int nargs,awk_value_t *result,const char *command) {
   int r,ival,ret;
   struct command valid;
   char str[240];
   awk_value_t val, array_param, *pstr=NULL;
   awk_array_t array;
   enum format_type there[2];
   enum resultArray k=KEYNUMBER;
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
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
          pstr=make_number(1, result);
        }
	else {
          pstr=make_number(0, result);
	}
      }
      else {
        pstr=theReply(result);
      }
      freeReplyObject(reply);
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
    sprintf(str,"%s need two arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoZrange(int nargs,awk_value_t *result,const char *command) {
   int r,ival,with=0;
   struct command valid;
   char str[240],cmd[30];
   awk_value_t val, val1, val2, val3, array_param, *pstr;
   awk_array_t array;
   enum format_type there[5];
   enum resultArray k=KEYNUMBER;
   pstr=NULL;
   int pconn=-1;
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
    if(pconn==-1) {
      if(with) {
        reply = redisCommand(c[ival],"%s %s %s %s %s",cmd,val1.str_value.str, val2.str_value.str,val3.str_value.str,"WITHSCORES");
      }
      else {
        reply = redisCommand(c[ival],"%s %s %s %s",cmd,val1.str_value.str, val2.str_value.str,val3.str_value.str);
      }
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
	  pstr=make_number(1, result);
        }
	else {
	  pstr=make_number(0, result);
	}
      }
      else {
        pstr=theReply(result);
      }
      freeReplyObject(reply);
    }
    else {
      if(with) {
        redisAppendCommand(c[pconn],"%s %s %s %s %s",cmd,val1.str_value.str, val2.str_value.str,val3.str_value.str,"WHITHSCORES");
      }
      else {
        redisAppendCommand(c[pconn],"%s %s %s %s",cmd,val1.str_value.str, val2.str_value.str,val3.str_value.str);
      }
      pipel[pconn][1]++;
      pstr=make_number(1, result);
    }
  }
  else {
    sprintf(str,"%s need five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr; 
}

awk_value_t * tipoZunionstore(int nargs,awk_value_t *result,const char *command) {
   int i,r,ival,count;
   size_t nkeys,nkeys_w;
   struct command valid;
   char str[240], st_nkeys[15], **sts, *pt;
   awk_value_t val, val1, array_param, array_param_w, *pstr;
   awk_array_t array, array_w;
   enum format_type there[5];
   int pconn=-1;
   pstr=NULL;
   sts=NULL;
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
    sprintf(st_nkeys,"%d",nkeys);
    mem_str(sts,st_nkeys,2); // passing a string nkeys
    if(pconn==-1) {
     reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
     pstr=theReply(result); 
     freeReplyObject(reply);
    }
    else {
     redisAppendCommandArgv(c[pconn],count,(const char **)sts,NULL);
     pipel[pconn][1]++;
     pstr=make_number(1, result);
    }
    for(i=0;i<count;i++){
      free(sts[i]);
    }
    free(sts);
  }
  else {
    sprintf(str,"%s need three, four or five arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSortLimit(int nargs,awk_value_t *result,const char *command) {
   int r,ival,ret=1;
   size_t i,j,store;
   struct command valid;
   char cmd[]="sort", str[240], *pch, *sts[15];
   awk_value_t val, val1, val2, val3, array_param, valstore, *pstr;
   awk_array_t array=NULL;
   enum format_type there[6];
   enum resultArray k=KEYNUMBER;
   int pconn=-1;
   j=store=0;
   pstr=NULL;
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
    if(nargs==6) {
      get_argument(5, AWK_STRING, & val3);
      mem_str(sts,cmd,0);
      mem_str(sts,val.str_value.str,1);
      mem_str(sts,"LIMIT",2);
      mem_str(sts,val1.str_value.str,3);
      mem_str(sts,val2.str_value.str,4);
      pch = strtok (val3.str_value.str," ");
      j=4;
      while (pch != NULL) {
        j++;
        mem_str(sts,pch,j);
	pch = strtok (NULL," ");
      }
      if(store) {
        mem_str(sts,"STORE",++j);
        mem_str(sts,valstore.str_value.str,++j);
      }
    }
    if(pconn==-1) {
      if(nargs==5 && !store) {
        reply = redisCommand(c[ival],"%s %s %s %s %s",cmd,val.str_value.str,
	                                      "LIMIT",val1.str_value.str,
	                                      val2.str_value.str
	);
      }
      if(nargs==5 && store) {
        reply = redisCommand(c[ival],"%s %s %s %s %s %s %s",cmd,
	                                      val.str_value.str,
	                                      "LIMIT",val1.str_value.str,
	                                      val2.str_value.str,
					      "STORE",valstore.str_value.str
	);
      }
      if(nargs==6) {
	reply = redisCommandArgv(c[ival],j+1,(const char **)sts,NULL);
      }
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
 	  ret=1;
        }
	else {
 	  ret=0;
	}
        freeReplyObject(reply);
      }
      else {
        pstr=theReply(result); 
      }
    }
    else {
      if(nargs==5 && !store) {
        redisAppendCommand(c[pconn],"%s %s %s %s %s",cmd,val.str_value.str,
	                                      "LIMIT",val1.str_value.str,
	                                      val2.str_value.str
	);
      }
      if(nargs==5 && store) {
        redisAppendCommand(c[pconn],"%s %s %s %s %s %s %s",cmd,
	                                      val.str_value.str,
	                                      "LIMIT",val1.str_value.str,
	                                      val2.str_value.str,
					      "STORE",valstore.str_value.str
	);
      }
      if(nargs==6) {
        redisAppendCommandArgv(c[pconn],j+1,(const char **)sts,NULL);
      }
      pipel[pconn][1]++;
    }
    if(nargs==6) {
      for(i=0;i<=j;i++){
        free(sts[i]);
      }
    }
  }
  else {
    sprintf(str,"%s need five or six arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  if(pstr) {
    return pstr;
  }
  return make_number(ret, result);
}

awk_value_t * tipoSort(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   size_t i,j,store;
   struct command valid;
   char str[240], *pch, *sts[15],cmd[]="sort";
   awk_value_t val, val1, array_param, valstore, *pstr;
   awk_array_t array=NULL;
   enum format_type there[4];
   enum resultArray k=KEYNUMBER;
   int pconn=-1;
   j=store=0;
   pstr=NULL;
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
    if(nargs==4) {
      get_argument(3, AWK_STRING, & val1);
      mem_str(sts,cmd,0);
      mem_str(sts,val.str_value.str,1);
      pch = strtok (val1.str_value.str," ");
      j=1;
      while (pch != NULL) {
        j++;
        mem_str(sts,pch,j);
	pch = strtok (NULL," ");
      }
      if(store) {
        mem_str(sts,"STORE",++j);
	mem_str(sts,valstore.str_value.str,++j);
      }
    }
    if(pconn==-1) {
      if(nargs==3 && !store) {
       reply = redisCommand(c[ival],"%s %s",cmd,val.str_value.str);
      }
      if(nargs==3 && store) {
        reply = redisCommand(c[ival],"%s %s %s %s",cmd,val.str_value.str,"STORE",valstore.str_value.str);
      }
      if(nargs==4) {
        reply = redisCommandArgv(c[ival],j+1,(const char **)sts,NULL);
      }
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(!theReplyArray(array,k,1)) {
          return make_number(0, result);
        }
      }
      else {
        pstr=theReply(result);
      }
      freeReplyObject(reply);
    }
    else {
      if(nargs==3 && !store) {
        redisAppendCommand(c[pconn],"%s %s",cmd,val.str_value.str);
      }
      if(nargs==3 && store) {
        redisAppendCommand(c[pconn],"%s %s %s %s",cmd,val.str_value.str,"STORE",valstore.str_value.str);
      }
      if(nargs==4) {
        redisAppendCommandArgv(c[pconn],j+1,(const char **)sts,NULL);
      }
      pipel[pconn][1]++;
    }
    if(nargs==4) {
      for(i=0;i<=j;i++){
        free(sts[i]);
      }
    }
  }
  else {
    sprintf(str,"%s need three or four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  if(pstr) {
    return pstr;
  }
  return make_number(1, result);
}

awk_value_t * tipoKeys(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   struct command valid;
   char str[240];
   awk_value_t val, array_param, *pstr;
   awk_array_t array;
   enum format_type there[3];
   enum resultArray k=KEYNUMBER;
   pstr=NULL;
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
    if(pconn==-1) {
      reply = redisCommand(c[ival],"%s %s",command,val.str_value.str);
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
	  pstr=make_number(1, result);
        }
	else {
	  pstr=make_number(0, result);
	}
      }
      else {
        pstr=theReply(result); 
      }
      freeReplyObject(reply);
    }
    else {
      redisAppendCommand(c[pconn],"%s %s",command,val.str_value.str);
      pipel[pconn][1]++;
      pstr=make_number(1,result);
    }
  }
  else {
    sprintf(str,"%s need three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoSrandmember(int nargs,awk_value_t *result,const char *command) {
   int r,ival;
   struct command valid;
   char str[240];
   awk_value_t val,val1, array_param;
   awk_array_t array;
   enum format_type there[4];
   enum resultArray k=KEYNUMBER;
   int pconn=-1;
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
      if (reply->type == REDIS_REPLY_ARRAY) {
        if(theReplyArray(array,k,1)) {
         freeReplyObject(reply);
        }
      }
    }
    else {
      redisAppendCommand(c[pconn],"%s %s %s",command,val.str_value.str,val1.str_value.str);
      pipel[pconn][1]++;
    }
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return make_number(1, result);
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
    there[i]=t;
  }
  return 1;
}

static awk_value_t * do_hgetall(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoKeys(nargs,result,"hgetall");
  return p_value_t;
}

awk_value_t * tipoMset(int nargs,awk_value_t *result,const char *command) {
  int r,ival,count;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, *pts;
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
    sts=getArrayContent(array,1,command,&count);
    if(pconn==-1) {
      reply = redisCommandArgv(c[ival],count,(const char **)sts,NULL);
      pts=theReply(result);
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
    sprintf(str,"%s need three arguments",command);
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
    if(!validate_conn(ival,str,"hmset",&pconn)) {
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
      pts=theReply(result);
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
    sprintf(str,"%s need three arguments",command);
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
  enum resultArray k=KEYNUMBER;
  pstr=NULL;
  int pconn=-1;
  if(nargs==4) {
    strcpy(valid.name,"command"); 
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
    if (reply->type == REDIS_REPLY_ARRAY) {
     if(theReplyArray(array_ou,k,1)) {
       pstr=make_number(1, result);
     }
     else {
       pstr=make_number(0, result);
     }
    }
    else {
       pstr=theReply(result);
    }
    freeReplyObject(reply);
  }
  else {
    sprintf(str,"%s need four arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

awk_value_t * tipoMget(int nargs,awk_value_t *result,const char *command) {
  int r,ival,count;
  struct command valid;
  char str[240], **sts;
  awk_value_t val, array_param, *pstr;
  awk_array_t array_in,array_ou;
  enum format_type there[3];
  enum resultArray k=KEYNUMBER;
  pstr=NULL;
  int pconn=-1;
  if(nargs==3) {
    strcpy(valid.name,"command"); 
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
      if(pconn==-1) {
       reply = redisCommand(c[ival],"%s %s",command,val.str_value.str);
      }
      else {
       redisAppendCommand(c[pconn],"%s %s",command,val.str_value.str);
       pipel[pconn][1]++;
       return make_number(1, result);
      }
    }
    else {
      get_argument(1, AWK_ARRAY, & array_param);
      array_in = array_param.array_cookie;
      sts=getArrayContent(array_in,1,command,&count);
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
    if (reply->type == REDIS_REPLY_ARRAY) {
     if(theReplyArray(array_ou,k,1)) {
       pstr=make_number(1, result);
     }
     else {
       pstr=make_number(0, result);
     }
    }
    else {
       pstr=theReply(result);
    }
    freeReplyObject(reply);
  }
  else {
    sprintf(str,"%s need three arguments",command);
    set_ERRNO(_(str));
    return make_number(-1, result);
  }
  return pstr;
}

static awk_value_t * do_select(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSelect(nargs,result,"select");
  return p_value_t;
}

static awk_value_t * do_randomkey(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"randomkey");
  return p_value_t;
}

static awk_value_t * do_dbsize(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"dbsize");
  return p_value_t;
}

static awk_value_t * do_ping(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"ping");
  return p_value_t;
}

static awk_value_t * do_multi(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"multi");
  return p_value_t;
}

static awk_value_t * do_discard(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"discard");
  return p_value_t;
}

static awk_value_t * do_unwatch(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"unwatch");
  return p_value_t;
}

static awk_value_t * do_exec(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoExec(nargs,result,"exec");
  return p_value_t;
}

static awk_value_t * do_flushdb(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRandomkey(nargs,result,"flushdb");
  return p_value_t;
}

static awk_value_t * do_auth(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoScard(nargs,result,"auth");
  return p_value_t;
}

static awk_value_t * do_exists(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoScard(nargs,result,"exists");
  return p_value_t;
}

static awk_value_t * do_get(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoScard(nargs,result,"get");
  return p_value_t;
}

static awk_value_t * do_sdiff(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSinter(nargs,result,"sdiff");
  return p_value_t;
}

static awk_value_t * do_sunion(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSinter(nargs,result,"sunion");
  return p_value_t;
}

static awk_value_t * do_sdiffstore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"sdiffstore");
  return p_value_t;
}

static awk_value_t * do_sunionstore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"sunionstore");
  return p_value_t;
}

static awk_value_t * do_getMessage(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoGetMessage(nargs,result,"getMessage");
  return p_value_t;
}

static awk_value_t * do_punsubscribe(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoUnsubscribe(nargs,result,"punsubscribe");
  return p_value_t;
}

static awk_value_t * do_unsubscribe(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoUnsubscribe(nargs,result,"unsubscribe");
  return p_value_t;
}

static awk_value_t * do_psubscribe(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSubscribe(nargs,result,"psubscribe");
  return p_value_t;
}

static awk_value_t * do_del(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSubscribe(nargs,result,"del");
  return p_value_t;
}

static awk_value_t * do_subscribe(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSubscribe(nargs,result,"subscribe");
  return p_value_t;
}

static awk_value_t * do_watch(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSubscribe(nargs,result,"watch");
  return p_value_t;
}

static awk_value_t * do_sinterstore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"sinterstore");
  return p_value_t;
}

static awk_value_t * do_sinter(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSinter(nargs,result,"sinter");
  return p_value_t;
}

static awk_value_t * do_hdel(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"hdel");
  return p_value_t;
}

static awk_value_t * do_hincrby(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoHincrby(nargs,result,"hincrby");
  return p_value_t;
}

static awk_value_t * do_brpoplpush(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoHincrby(nargs,result,"brpoplpush");
  return p_value_t;
}

static awk_value_t * do_zincrby(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZincrby(nargs,result,"zincrby");
  return p_value_t;
}

static awk_value_t * do_hincrbyfloat(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoHincrby(nargs,result,"hincrbyfloat");
  return p_value_t;
}

static awk_value_t * do_hset(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSmove(nargs,result,"hset");
  return p_value_t;
}

static awk_value_t * do_hsetnx(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSmove(nargs,result,"hsetnx");
  return p_value_t;
}

static awk_value_t * do_lrem(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSetrange(nargs,result,"lrem");
  return p_value_t;
}

static awk_value_t * do_blpop(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoBrpop(nargs,result,"blpop");
  return p_value_t;
}

static awk_value_t * do_brpop(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoBrpop(nargs,result,"brpop");
  return p_value_t;
}

static awk_value_t * do_lset(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSetrange(nargs,result,"lset");
  return p_value_t;
}

static awk_value_t * do_setrange(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSetrange(nargs,result,"setrange");
  return p_value_t;
}

static awk_value_t * do_rename(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSismember(nargs,result,"rename");
  return p_value_t;
}

static awk_value_t * do_renamenx(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSismember(nargs,result,"renamenx");
  return p_value_t;
}

static awk_value_t * do_restore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoRestore(nargs,result,"restore");
  return p_value_t;
}

static awk_value_t * do_dump(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoScard(nargs,result,"dump");
  return p_value_t;
}

static awk_value_t * do_move(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSismember(nargs,result,"move");
  return p_value_t;
}

static awk_value_t * do_getrange(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSetbit(nargs,result,"getrange");
  return p_value_t;
}

static awk_value_t * do_zremrangebyrank(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSetbit(nargs,result,"zremrangebyrank");
  return p_value_t;
}

static awk_value_t * do_linsertBefore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoLinsert(nargs,result,"linsertBefore");
  return p_value_t;
}

static awk_value_t * do_linsertAfter(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoLinsert(nargs,result,"linsertAfter");
  return p_value_t;
}

static awk_value_t * do_zremrangebylex(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSmove(nargs,result,"zremrangebylex");
  return p_value_t;
}

static awk_value_t * do_zremrangebyscore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSmove(nargs,result,"zremrangebyscore");
  return p_value_t;
}

static awk_value_t * do_zlexcount(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSmove(nargs,result,"zlexcount");
  return p_value_t;
}

static awk_value_t * do_zrangebyscore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZrangebylex(nargs,result,"zrangebyscore");
  return p_value_t;
}

static awk_value_t * do_zrevrangebyscore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZrangebylex(nargs,result,"zrevrangebyscore");
  return p_value_t;
}

static awk_value_t * do_zrangebylex(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZrangebylex(nargs,result,"zrangebylex");
  return p_value_t;
}

static awk_value_t * do_evalsha(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoEvalsha(nargs,result,"evalsha");
  return p_value_t;
}

static awk_value_t * do_evalRedis(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoEvalsha(nargs,result,"eval");
  return p_value_t;
}

static awk_value_t * do_smove(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSmove(nargs,result,"smove");
  return p_value_t;
}

static awk_value_t * do_srem(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"srem");
  return p_value_t;
}

static awk_value_t * do_rpush(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"rpush");
  return p_value_t;
}

static awk_value_t * do_lpush(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"lpush");
  return p_value_t;
}

static awk_value_t * do_zinterstore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZunionstore(nargs,result,"zinterstore");
  return p_value_t;
}

static awk_value_t * do_zunionstore(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZunionstore(nargs,result,"zunionstore");
  return p_value_t;
}

static awk_value_t * do_zadd(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoZadd(nargs,result,"zadd");
  return p_value_t;
}

static awk_value_t * do_zrem(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"zrem");
  return p_value_t;
}

static awk_value_t * do_sadd(int nargs, awk_value_t *result) {
  awk_value_t *p_value_t;
  p_value_t=tipoSadd(nargs,result,"sadd");
  return p_value_t;
}

static awk_bool_t
init_redis(void)
{
   GAWKEXTLIB_COMMON_INIT
   return awk_true;
}

static awk_ext_func_t func_table[] = {
	{ "redis_connect",do_connectRedis, 2 },
	{ "redis_scan",	do_scan, 4 },
	{ "redis_sadd",	do_sadd,3 },
	{ "redis_smembers",	do_smembers,3 },
	{ "redis_scard",	do_scard,2 },
	{ "redis_spop",	do_spop,2 },
	{ "redis_sinter",	do_sinter,3 },
	{ "redis_sinterstore",do_sinterstore,3 },
	{ "redis_sunion",	do_sunion,3 },
	{ "redis_sunionstore",do_sunionstore,3 },
	{ "redis_sdiffstore", do_sdiffstore,3 },
	{ "redis_sdiff",	do_sdiff,3 },
	{ "redis_sismember",	do_sismember,3 },
	{ "redis_smove",	do_smove,4 },
	{ "redis_srem",	do_srem,3 },
	{ "redis_srandmember",do_srandmember,4 },
	{ "redis_sscan",      do_sscan,5 },
	{ "redis_disconnect",	do_disconnect, 1 },
	{ "redis_close",	do_disconnect, 1 },
	{ "redis_keys",	do_keys, 3 },
	{ "redis_mget",       do_mget, 3 },
	{ "redis_mset",       do_mset, 2 },
	{ "redis_msetnx",     do_msetnx, 2 },
	{ "redis_hmget",      do_hmget, 4 },
	{ "redis_hgetall",    do_hgetall, 3 },
	{ "redis_hkeys",      do_hkeys, 3 },
	{ "redis_hexists",    do_hexists, 3 },
	{ "redis_hvals",      do_hvals, 3 },
	{ "redis_hmset",      do_hmset, 3 },
	{ "redis_hsetnx",     do_hsetnx, 4 },
	{ "redis_hset",       do_hset, 4 },
	{ "redis_hget",       do_hget, 3 },
	{ "redis_hdel",       do_hdel, 3 },
	{ "redis_hincrby",    do_hincrby, 4 },
	{ "redis_hincrbyfloat",do_hincrbyfloat, 4 },
	{ "redis_hlen",       do_hlen, 2 },
	{ "redis_hscan",      do_hscan,5 },
	{ "redis_publish",    do_publish, 3 },
	{ "redis_subscribe",  do_subscribe, 2 },
	{ "redis_psubscribe",  do_psubscribe, 2 },
	{ "redis_unsubscribe",do_unsubscribe, 2 },
	{ "redis_punsubscribe",do_punsubscribe, 2 },
	{ "redis_getMessage", do_getMessage, 2 },
	{ "redis_select",     do_select, 2 },
	{ "redis_get",        do_get, 2 },
	{ "redis_del",        do_del, 2 },
	{ "redis_exists",     do_exists, 2 },
	{ "redis_lrange",	do_lrange, 5 },
	{ "redis_llen",	do_llen, 2 },
	{ "redis_lindex",	do_lindex, 3 },
	{ "redis_lpop",	do_lpop, 2 },
	{ "redis_lpush",	do_lpush, 3 },
	{ "redis_lset",	do_lset, 4 },
	{ "redis_brpop",	do_brpop, 4 },
	{ "redis_blpop",	do_blpop, 4 },
	{ "redis_brpoplpush", do_brpoplpush, 4 },
	{ "redis_linsertBefore",do_linsertBefore, 4 },
	{ "redis_linsertAfter",do_linsertAfter, 4 },
	{ "redis_lrem",	do_lrem, 4 },
	{ "redis_ltrim",	do_ltrim, 4 },
	{ "redis_lpushx",	do_lpushx, 3 },
	{ "redis_rpush",	do_rpush, 3 },
	{ "redis_rpushx",	do_rpushx, 3 },
	{ "redis_rpop",	do_rpop, 2 },
	{ "redis_rpoplpush",	do_rpoplpush, 3 },
	{ "redis_pipeline",	do_pipeline, 1 },
	{ "redis_getReply",	do_getReply, 2 },
	{ "redis_type",	do_type, 2 },
	{ "redis_incr",	do_incr, 2 },
	{ "redis_incrbyfloat",do_incrbyfloat, 3 },
	{ "redis_pttl",	do_pttl, 2 },
	{ "redis_persist",	do_persist, 2 },
	{ "redis_pexpire",	do_pexpire, 3 },
	{ "redis_setbit",	do_setbit, 4 },
	{ "redis_getbit",	do_getbit, 3 },
	{ "redis_bitpos",	do_bitpos, 5 },
	{ "redis_incrby",	do_incrby, 3 },
	{ "redis_decrby",	do_decrby, 3 },
	{ "redis_decr",	do_decr, 2 },
	{ "redis_ttl",	do_ttl, 2 },
	{ "redis_expire",	do_expire, 3 },
	{ "redis_move",	do_move, 3 },
	{ "redis_echo",	do_echo, 3 },
	{ "redis_auth",	do_auth, 2 },
	{ "redis_rename",	do_rename, 3 },
	{ "redis_renamenx",	do_renamenx, 3 },
	{ "redis_dump",	do_dump, 2 },
	{ "redis_restore",	do_restore, 4 },
	{ "redis_getrange",	do_getrange, 4 },
	{ "redis_setrange",	do_setrange, 4 },
	{ "redis_set",	do_set, 6 },
	{ "redis_getset",	do_getset, 3 },
	{ "redis_strlen",	do_strlen, 2 },
	{ "redis_append",	do_append, 3 },
	{ "redis_randomkey",	do_randomkey, 1 },
	{ "redis_ping",	do_ping, 1 },
	{ "redis_flushdb",	do_flushdb, 1 },
	{ "redis_dbsize",	do_dbsize, 1 },
	{ "redis_bitcount",	do_bitcount, 4 },
	{ "redis_bitop",	do_bitop, 4 },
	{ "redis_sort",	do_sort, 4 },
	{ "redis_sortStore",	do_sortStore, 4 },
	{ "redis_sortLimit",	do_sortLimit, 6 },
	{ "redis_sortLimitStore",do_sortLimitStore, 6 },
	{ "redis_zadd",	do_zadd, 4 },
	{ "redis_zcard",	do_zcard, 2 },
	{ "redis_zcount",	do_zcount, 4 },
	{ "redis_zrem",	do_zrem, 3 },
	{ "redis_zrange",	do_zrange, 5 },
	{ "redis_zrevrange",	do_zrevrange, 5 },
	{ "redis_zunionstore",do_zunionstore, 5 },
	{ "redis_zinterstore",do_zinterstore, 5 },
	{ "redis_zrank",	do_zrank, 3 },
	{ "redis_zrangeWithScores",	do_zrangeWithScores, 5 },
	{ "redis_zrevrangeWithScores",	do_zrevrangeWithScores, 5 },
	{ "redis_zrevrank",	do_zrevrank, 3 },
	{ "redis_zscore",	do_zscore, 3 },
	{ "redis_zincrby",	do_zincrby, 4 },
        { "redis_zlexcount",  do_zlexcount, 4 },
        { "redis_zrangebylex",do_zrangebylex, 5 },
        { "redis_zrangebyscore",do_zrangebyscore, 5 },
        { "redis_zrevrangebyscore",do_zrevrangebyscore, 5 },
        { "redis_zremrangebylex",do_zremrangebylex, 4 },
        { "redis_zremrangebyrank",do_zremrangebyrank, 4 },
        { "redis_zremrangebyscore",do_zremrangebyscore, 4 },
	{ "redis_zscan",	do_zscan, 5 },
	{ "redis_script",	do_script, 4 },
	{ "redis_evalsha",	do_evalsha, 5 },
	{ "redis_evalRedis",	do_evalRedis, 5 },
	{ "redis_multi",	do_multi, 1 },
	{ "redis_exec",	do_exec, 2 },
	{ "redis_watch",	do_watch, 2 },
	{ "redis_unwatch",	do_unwatch, 1 },
	{ "redis_discard",	do_discard, 1 },
};



/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, redis, "")
