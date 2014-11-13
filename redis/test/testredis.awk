BEGIN {
  MAXDB=15
  c=connectRedis()
  if(c==-1){
    print ERRNO, " There is a Redis server listening?"
    exit
  }
  print c
  for(i=1; i<=MAXDB; i++) {
     ret=select(c,i)
     if(keys(c,"*",AR)!=0) 
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
  print keys(c,"*",AR)
  print ping(c)
  print set(c,"foo","bar")
  print get(c,"foo")
  print echo(c,"foobared")
  print del(c,"foo")
  set(c,"keyY","valY")
  print exists(c,"keyY")
  set(c,"keyZ","valZ")
  delete(AR)
  AR[1]="keyY"
  AR[2]="keyZ"
  print del(c,AR)
  print exists(c,"keyZ")
  print incr(c,"key1") 
  print incr(c,"key1") 
  print del(c,"key1")
  print incrby(c,"key1",10)
  print incrbyfloat(c,"key1", 1.5) 
  print incrbyfloat(c,"key1", 1.5) 
  print decr(c,"keyXY")
  print decr(c,"keyXY")
  print decrby(c,"keyXY",10)
  AR[1]="keyXY"
  AR[2]="key1"
  print del(c,AR)
  set(c,"keyA","val1")
  set(c,"keyB","val2")
  set(c,"keyC","val3")
  set(c,"keyD","val4")
  set(c,"keyE","")
  AR[1]="keyA"
  AR[2]="keyB"
  AR[3]="keyZ" # this key no exists
  AR[4]="keyC"
  AR[5]="keyD"
  AR[6]="keyE"
  ret=mget(c,AR,K) # K is the array with results
  for(i=1; i<=length(K); i++) {
     if(!K[i]) {
       if(exists(c,AR[i])){ 
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
  print del(c,AR)
  print closeRedis(c)
}
