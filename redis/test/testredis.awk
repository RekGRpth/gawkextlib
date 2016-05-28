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
  
  

  redis_flushdb(c)
  print redis_close(c)
  
}
