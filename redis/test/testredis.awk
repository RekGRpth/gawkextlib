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
  redis_flushdb(c)
  print redis_close(c)
}
