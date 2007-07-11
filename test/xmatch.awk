{
    r = match($0,/^ */,t);
    print "R=" r " S=" RSTART " L=" RLENGTH;
} 
