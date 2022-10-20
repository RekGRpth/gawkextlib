@load "json"

BEGIN {
	a[1] = mkbool(42)
	a[2] = 42
	a[3] = "hello, world"
	a[4]["x"] = "x"
	a[4]["y"] = "y"
	a[5] = mkbool(! 42)

	out = json::to_json(a, 1)
	print out

	status = json::from_json(out, back)
	for (i in back) {
		switch (typeof(back[i])) {
		case "array":
			for (j in back[i]) {
				printf("b[%d][%s] = %s\n", i, j, back[i][j])
			}
			break
		case "number":
			printf("b[%d] = %g\n", i, back[i])
			break
		case "number|bool":
			printf("b[%d] = %s (%g)\n", i, back[i] ? "true" : "false", back[i])
			break
		case "string":
		default:
			printf("b[%d] = %s\n", i, back[i])
			break
		}
	}
}
