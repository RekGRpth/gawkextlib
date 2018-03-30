### 1.7.8 (2018-03-29)

**Adding**:

* redis_pubsub function

**Fixes**:

* Improvement theReplyArray function to create an associative array when necessary.
* Minor corrections: completing the use of the function make_null_string.
* Minor corrections: the code of the tipoSubscribe function is now cleaner.

### 1.7.5 (2018-02-14)

**Adding**:

* redis_configResetStat function.

**Fixes**:

* Minor corrections: using now the make_null_string and make_const_user_input functions.

### 1.7.3 (2017-12-20)

**Adding**:

* redis_hstrlen function.

**Fixes**:

* Functions: redis_echo, redis_punsubscribe.

**Changes**:

* eval function: now the function `evalRedis` is `eval`

### 1.7.0 (2017-12-15)

**Upgrade**:

* Added support gawk API versions 2.

### 1.6.0 (2017-06-14)

**Adding**:

* Client functions: redis_clientList, redis_clientSetName, redis_clientGetName, redis_clientPause, redis_clientKillId, redis_clientKillAddr, redis_clientKillType.    
* Server functions: redis_lastsave, redis_bgsave, redis_configGet, redis_configSet, redis_slowlog.    

### 1.4.0 (2016-05-31)

**Fixes**:

* Added new count argument in `redis_spop` function, now available in a later version of redis server.    
* In `redis_getReply` function, the second argument is left optional.     
* Added new argument in `redis_subscribe` and `redis_psubsubscribe` functions.     

**Adding**:

* Geospatial functions: redis_geoadd, redis_geohash, redis_geopos, redis_geodist, redis_georadius, redis_georadiusbymember (they are identical to those found on the server).    
* Geospatial functions (these functions are specific to this client), redis_georadiusWC, redis_georadiusWDWC, redis_georadiusbymember, redis_georadiusbymemberWD, redis_georadiusbymemberWC, redis_georadiusbymemberWDWC.  
* `redis_getReplyInfo` function.
* `redis_getReplyMass` function: this feature facilitates `one liner script`

**BREAKING CHANGES**:

* Refactor `processREPLY`, fixed bug in redis_getReply with buffered functions (pipelined).
* Refactor, using now,  `redisCommandArgv` and `redisAppendCommandArgv` (hiredis library) getting better design and removing code.

