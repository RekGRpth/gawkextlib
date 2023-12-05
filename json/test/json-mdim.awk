@load "json"

BEGIN {	
	print json::from_json("[[1],[2,3,4]]", a)
	walk_array(a, "a")
}

function walk_array(arr, name,      i)
{
    for (i in arr) {
        if (isarray(arr[i]))
            walk_array(arr[i], (name "[" i "]"))
        else
            printf("%s[%s] = %s\n", name, i, arr[i])
    }
}
