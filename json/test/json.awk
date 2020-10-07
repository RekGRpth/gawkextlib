BEGIN {
	split("a b c d", lets)
	print json::to_json(lets)
	print json::to_json(lets, 1)
	lets["foo"] = 42
	print json::to_json(lets)
	print json::to_json(lets, 1)

#	stat("test.awk", data)
#	print json::to_json(data)

	s = json::to_json(42)
	if (s)
		print s		# should not print anything

	t[1] = "foo"
	t[2] = 42
	t[3] = 42.1
	t[4] = -123
	t[5] = @/regexp/
	t[6]["a"] = "6a"
	t[6]["b"] = "6b"
	t[6]["c"][1] = "6c 1"
	t[6]["c"][2] = "6c 2"
	t[7]			# Null

	print (data1 = json::to_json(t))

	if (json::from_json(data1, new)) {
		if (! cmp_array(t, new))
			printf("json::from_json: first import failed\n") > "/dev/stderr"
		else
			print "first import ok"
	} else
		printf("1 json::from_json(\"%s\") failed: %s\n", data1, ERRNO) > "/dev/stderr"

	print ""
	print (data2 = json::to_json(t, 1))

	if (json::from_json(data2, new)) {
		if (! cmp_array(t, new))
			printf("json::from_json: first import failed\n") > "/dev/stderr"
		else
			print "second import ok"
	} else
		printf("2 json::from_json(\"%s\") failed: %s\n", data1, ERRNO) > "/dev/stderr"
}

function cmp_array(old, new,	i, c)
{
	if (length(old) != length(new))
		return 0;

	for (i in old) {
		if (isarray(old[i])) {
			if (isarray(new[i])) {
				if (! cmp_array(old[i], new[i]))
					return 0
				else
					continue
			}
			return 0
		} else if (isarray(new[i]))
			return 0
		else if (old[i] != new[i])
			return 0
		else
			continue
	}

	return 1
}
