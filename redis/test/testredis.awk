BEGIN {
  MAXDB=15
  c=redis_connect()
  if(c==-1){
    print ERRNO, " There is a Redis server listening?"
    exit
  }
  print c
  for(i=1; i<=MAXDB; i++) {
     ret=redis_select(c,i)
     if(redis_keys(c,"*",AR)!=0) 
       continue
     else {
       print ret 
       break
     }
  }
  if(i==MAXDB + 1) {
    print "It is recommended to use an empty db. If possible do FLUSHDB before passing the test."
    exit
  }
  print redis_keys(c,"*",AR)
  print redis_ping(c)
  print redis_set(c,"foo","bar")
  print redis_get(c,"foo")
  print redis_echo(c,"foobared")
  print redis_del(c,"foo")
  redis_set(c,"keyY","valY")
  print redis_exists(c,"keyY")
  redis_set(c,"keyZ","valZ")
  delete(AR)
  AR[1]="keyY"
  AR[2]="keyZ"
  print redis_del(c,AR)
  print redis_exists(c,"keyZ")
  print redis_incr(c,"key1") 
  print redis_incr(c,"key1") 
  print redis_del(c,"key1")
  print redis_incrby(c,"key1",10)
  print redis_incrbyfloat(c,"key1", 1.5) 
  print redis_incrbyfloat(c,"key1", 1.5) 
  print redis_decr(c,"keyXY")
  print redis_decr(c,"keyXY")
  print redis_decrby(c,"keyXY",10)
  AR[1]="keyXY"
  AR[2]="key1"
  print redis_del(c,AR)
  redis_set(c,"keyA","val1")
  redis_set(c,"keyB","val2")
  redis_set(c,"keyC","val3")
  redis_set(c,"keyD","val4")
  redis_set(c,"keyE","")
  AR[1]="keyA"
  AR[2]="keyB"
  AR[3]="keyZ" # this key no exists
  AR[4]="keyC"
  AR[5]="keyD"
  AR[6]="keyE"
  ret=redis_mget(c,AR,K) # K is the array with results
  for(i=1; i<=length(K); i++) {
     if(!K[i]) {
       if(redis_exists(c,AR[i])){ 
          print i": "AR[i]" ----> "null
       }
       else {
          print i": "AR[i]" ----> not exists"
       }
     }
     else {
         print i": "AR[i]" ----> ""\""K[i]"\""
     }
  }
  print redis_del(c,AR)
  redis_set(c,"x", "42")
  print redis_getset(c,"x","lol") # return "42", now the value of x is "lol"
  print redis_get(c,"x") # return "lol"
  redis_set(c,"x", "valx");
  redis_rename(c,"x","y");
  print redis_get(c,"y")
  print redis_get(c,"x")
  redis_set(c,"x", "42")
  redis_expire(c,"x", 2)
  system("sleep 3")    
  print redis_get(c,"x")
  redis_set(c,"user1","us1")
  redis_keys(c,"user*",AR)  # for matching all keys begining with "user"
  print (length(AR) >  0)
  delete(AR)
  redis_keys(c,"uuuser*",AR)  # for matching all keys begining with "user"
  print (length(AR) == 0)
  redis_set(c,"keyZ","valZ")
  print redis_type(c,"keyZ")
  redis_append(c,"keyZ","lol")
  print redis_get(c,"keyZ")
  print redis_getrange(c,"keyZ", 0, 2)
  print redis_getrange(c,"keyZ", -3, -1)
  redis_setrange(c,"keyZ",6," redis")
  print redis_get(c,"keyZ")
  redis_set(c,"key","value")
  print redis_strlen(c,"key")
  redis_set(c,"key", "s") # this is 0111 0011
  print redis_getbit(c,"key", 5) # 0
  print redis_getbit(c,"key", 6) # 1
  redis_set(c,"key", "*") # ord("*") = 42 = "0010 1010"
  print redis_setbit(c,"key", 5, 1) #  returns 0
  print redis_setbit(c,"key", 7, 1) #  returns 0
  print redis_get(c,"key") #  "/" = "0010 1111"
  redis_lpush(c,"thelist","bed")
  redis_lpush(c,"thelist","pet")
  redis_lpush(c,"thelist","key")
  redis_lpush(c,"thelist","art")
  ret=redis_sort(c,"thelist",AR,"alpha desc")
  for(i in AR){
     printf("%s,", AR[i])
  }
  print ""
  redis_del(c,"foo")
  redis_set(c,"foo","bar")
  redis_type(c,"foo")
  print redis_object(c,"idletime","foo")
  if(redis_object(c,"refcount","foo") > 0)
   print 1
  if (redis_object(c,"encoding","foo") > 0)
   print 1
  delete(AR)
  AR[1]="a"; AR[2]="b"; AR[3]="c"
  AR[4]="d"; AR[5]="e"; AR[6]="f"
  redis_del(c,"hll")
  print redis_pfadd(c,"hll",AR) 
  if(redis_pfcount(c,"hll")>0) 
   print 1
  BR[1]="a"; BR[2]="b"; BR[3]="c"; BR[4]="foo"
  redis_del(c,"hll2")
  redis_del(c,"hll3")
  print redis_pfadd(c,"hll2",BR)
  delete K
  K[1]="hll1"; K[2]="hll2"
  print  redis_pfmerge(c,"hll3",K)
  if(redis_pfcount(c,"hll3")>0)
   print "1"
  p=redis_pipeline(c)
  print redis_info(p,AR,"clients")
  print redis_getReplyInfo(p,AR)
  for(i in AR) {
    if(i=="connected_clients") {
      print "clients"
      break
    }
  }
  delete A
  redis_del(c,"Sicilia")
  A[1]="13.361389"
  A[2]="38.115556"
  A[3]="Palermo"
  A[4]="15.087269"
  A[5]="37.502669"
  A[6]="Catania"
  print redis_geoadd(c,"Sicilia",A)
  if(redis_geodist(c,"Sicilia","Palermo","Catania","km") < 250) {
     print "ok"
  }
  delete AR
  print redis_geopos(c,"Sicilia","Catania",AR)
  if(AR[1][1]~/^15/) {
     print "ok"
  }
  delete AR
  print redis_georadiusbymember(c,"Sicilia",AR,"Palermo",200,"km","desc",1)
  print AR[1]
  redis_sadd(c,"MySet","one")
  delete AR
  AR[1]="two";
  AR[2]="three";
  AR[3]="four";
  print redis_sadd(c,"MySet",AR)
  print redis_spop(c,"MySet",3,AR)
  print length(AR)
  delete RET
  print redis_psubscribe(c,"ib*",RET)  # returns 1
  print RET[1]
  print redis_punsubscribe(c)
  redis_del(c,"MyList")
  print redis_rpush(c,"MyList","hello1")
  delete A
  A[1]="hello2"
  A[2]="hello3"
  A[3]="hello4"
  print redis_rpush(c,"MyList",A)
  print redis_rpush(p,"MyList",A) # we have a pipe
  print redis_lpush(p,"MyList","hello0")
  print redis_getReplyMass(p)
  delete AR
  print redis_lrange(c,"MyList",AR,0,-1)
  print AR[1]
  print AR[8]

  delete AR
  AR[1]="nm"
  AR[2]="x20lp66"
  AR[3]="nombre"
  AR[4]="silvia"
  AR[5]="address"
  AR[6]="Rio Negro 23"
  print redis_hmset(c,"hashPerson",AR)
  print redis_hget(c,"hashPerson","nombre")
  print redis_hgetall(c,"hashPerson",H)
  print length(H)
  for(i in H){
    if(H[i] == "silvia")
      print "silvia"
  }
  print redis_hdel(c,"hashPerson","nombre")
  print redis_hexists(c,"hashPerson","address")
  print redis_hexists(c,"hashPerson","nombre")
  delete H
  redis_hgetall(c,"hashPerson",H)
  print length(H)
  print redis_hset(c,"hashPerson","nombre","silvia")
  print redis_hget(c,"hashPerson","nombre")

  delete H
  print redis_hvals(c,"hashPerson",H)
  print length(H)
  for(i in H){
    if(H[i] == "silvia")
      print "silvia"
  }
  delete H
  print redis_hkeys(c,"hashPerson",H)
  lhash = length(H)
  for(i in H) {
    if(H[i] == "address")
      print "address"
  }
  print (lhash == redis_hlen(c,"hashPerson"))

  num=0
  delete(AR)
  while(1){
    ret=redis_hscan(c,"hashPerson",num,AR)
    if(ret==0){
     break
    }
    n=length(AR)
    for(i=2;i<=n;i++) {
      if(AR[i] == "silvia")
        var = "silvia"
    }
    num=AR[1]  # AR[1] contains the cursor
    delete(AR)
  }
  for(i=2;i<=length(AR);i++) {
    if(AR[i] == "silvia")
        var = "silvia"
  }
  print var

  delete J
  delete T
  J[1]="adress"
  J[2]="nm"
  J[3]="city"
  J[4]="country"
  J[5]="address"
  J[3]="age"
  print redis_hmget(c,"hashPerson",J,T)
  print (length(J) != length(T))
  print (redis_hget(c,"hashPerson",J[5]) == T[5])

  print redis_hstrlen(c,"hashPerson","address")

  redis_del(c,"zmyset")
  print redis_zadd(c,"zmyset",1,"oneNumber")
  delete AR
  AR[1]="2"; AR[2]="two"; AR[3]="3"; AR[4]="three"; AR[5]="1"; AR[6]="one"
  print redis_zadd(c,"zmyset",AR)
  print redis_zincrby(c,"zmyset",3,"one") # returns 4

  delete RET
  print redis_zrange(c,"zmyset",RET,0,-1) # returns 1
  print length(RET)
  print (RET[4] == "one")
  delete RET
  print redis_zrangeWithScores(c,"zmyset",RET,0,-1) # returns 1
  print length(RET)
  print redis_zrank(c,"zmyset","three") 
  print redis_zrevrank(c,"zmyset","three") 
  delete RES
  print redis_zrevrange(c,"zmyset",RES,0,-1)  # returns 1
  print ("one" == RES[1] )


  print redis_configSet(c,"dir","/tmp")
  print redis_configGet(c,"*entries*",R2)
  print (length(R2) == 0)

  delete AR
  print redis_info(c,AR)
  print "maxmemory" in AR  || "used_memory" in AR

  print (redis_dbsize(c) > 0)

  redis_del(c,"thehash")
  delete AR
  ARG[1]="hset"
  ARG[2]="thehash"
  ARG[3]="field3"
  ARG[4]="value3"
  print redis_eval(c,"return redis.call(KEYS[1],ARGV[1],ARGV[2],ARGV[3])",1,ARG,R)
  print length(R)
  print redis_hget(c,"thehash","field3")

  cmd1=redis_script(c,"load","return {1,2,{7,'Hello World!',89}}")
  print redis_evalsha(c,cmd1,0,ARG,R)
  print length(R)
  print R[1]
  print R[2]
  print R[3][1],R[3][2],R[3][3]

  delete T
  print redis_clientList(c,T)
  print (length(T) > 0)

  print redis_clientSetName(c,"XvbT")
  print redis_clientGetName(c)

  redis_del(c,"zset")
  delete A
  A[1]="0"; A[2]="a"; A[3]="0"; A[4]="b"; A[5]="0"
  A[6]="c"; A[7]="0"; A[8]="d"; A[9]="0"; A[10]="e"
  A[11]="0"; A[12]="f"; A[13]="0"; A[14]="g"
  redis_zadd(c,"zset",A)
  print redis_zrangebylex(c,"zset",AR,"[aaa","(g") # returns 1
    # AR contains b,c,d,e,f
  print length(AR)
  print AR[2]
  print redis_zlexcount(c,"zset","-","+")  # return 7
  print redis_zlexcount(c,"zset","[b","(d")  # returns 2
  print redis_zremrangebylex(c,"zset","[b","(d") # returns 2
  print redis_zremrangebyrank(c,"zset",0,2) # returns 3
  
  redis_del(c,"zmyset")
  delete AR
  AR[1]="2"; AR[2]="two"; AR[3]="3"; AR[4]="three"; AR[5]="1"; AR[6]="one"
  redis_zadd(c,"zmyset",AR)

  print redis_zscore(c,"zmyset","three") # returns 3
  print redis_zscore(c,"zmyset","one") # returns 1

  print redis_zrem(c,"zmyset","three") # returns 1
  delete R
  R[1]="five"; R[2]="two"; R[3]="one"
  print redis_zrem(c,"zmyset",R) # returns 2
  print redis_zrangeWithScores(c,"zmyset",RES,0,-1)  # returns 0

  redis_del(c,"Sicilia")
  delete A
  A[1]="13.361389"
  A[2]="38.115556"
  A[3]="Palermo"
  A[4]="15.087269"
  A[5]="37.502669"
  A[6]="Catania"
  print redis_geoadd(c,"Sicilia",A)
  if(redis_geodist(c,"Sicilia","Palermo","Catania","km") < 250) {
     print "ok"
  }
  delete AR
  print redis_geopos(c,"Sicilia","Catania",AR)
  if(AR[1][1]~/^15/) {
     print "ok"
  }
  delete AR
  print redis_georadiusbymember(c,"Sicilia",AR,"Palermo",200,"km","desc",1)
  print AR[1]
  delete AR
  print redis_georadiusWDWC(c,"Sicilia",AR,"14",37.9,100,"km","asc")
  print AR[1][1]
  print redis_georadius(c,"Sicilia",AR,15,37,270,"km")
  print length(AR)

  redis_del(c,"list1")
  delete AR
  AR[1]="AA"
  AR[2]="BB"
  AR[3]="CC"
  AR[4]="BB"
  AR[5]="PP"
  AR[6]="YY"
  AR[7]="ZZ"
  redis_lpush(c,"list1",AR)
  redis_multi(c)
  redis_lrem(c,"list1",4,"BB") # count is 4 but removes only two (existing values)
  delete B
  redis_lrange(c,"list1",B,0,-1)
  redis_ltrim(c,"list1",1,3)
  print redis_llen(c,"list1") # return QUEUED
  delete R
  print redis_exec(c,R) # return 1
  print R[1] # return 2
  print R[3] # return 1
  print R[4] # return 3

  redis_del(c,"listb")
  redis_del(c,"listbb")
  redis_del(c,"listbbb")
  redis_lpush(c,"listb","hello")
  redis_lpush(c,"listb","Sussan")
  redis_lpush(c,"listb","nice")
  delete LIST
  LIST[1]="listbbb"; LIST[2]="listbb"; LIST[3]="listb"
  delete AR
  print redis_blpop(c,LIST,AR,10) # return is 1
  for(i in AR) {
    print AR[i]
  }
  delete AR
  print redis_brpop(c,LIST,AR,10) # return is 1
  for(i in AR) {
    print AR[i]
  }
  redis_rpush(c,"listb","new")
  print redis_brpoplpush(c,"listb","listbb",10) 
  print redis_rpoplpush(c,"listb","listbb") 
  delete AR
  redis_lrange(c,"listbb",AR,0,-1)
  print AR[1]
  print redis_linsertBefore(c,"listbb","new","Hi")
  print redis_linsertAfter(c,"listbb","Sussan","Great")
  delete AR
  redis_lrange(c,"listbb",AR,0,-1)
  print AR[2]

  redis_del(c,"myset1")
  delete A
  A[1]="55"; A[2]="c16"; A[3]="89"; A[4]="c15"
  redis_sadd(c,"myset1",A)
  redis_del(c,"myset2")
  redis_sadd(c,"myset2",89)
  redis_del(c,"myset3")
  delete A
  A[1]="9"; A[2]="c16"; A[3]="89"
  redis_sadd(c,"myset3",A)
  delete A
  A[1]="myset1"; A[2]="myset2"; A[3]="myset3"
  delete RE
  print redis_sinter(c,A,RE)
  print length(RE)
  print RE[1]
  delete RE
  print redis_sdiff(c,A,RE)
  print length(RE)
  print redis_sismember(c,"myset3","c16")
  print redis_scard(c,"myset3")
  delete AR
  print redis_smembers(c,"myset3",AR)
  for(i in AR){
    if("9" == AR[i])
      print "9"
  }
  delete B
  B[1]=9
  B[2]="89"
  print redis_srem(c,"myset3",B)
  print redis_srem(c,"myset3","c16")
  redis_sadd(c,"myset3",B)
  ret = redis_spop(c,"myset3")
  print (ret == 9 || ret == 89)
  redis_flushdb(c)
  print redis_close(c)
}
