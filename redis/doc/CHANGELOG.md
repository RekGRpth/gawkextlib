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

