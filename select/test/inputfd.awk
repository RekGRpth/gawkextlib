BEGIN {
	print (input_fd("") >= 0)
	cmd = "echo hello; echo goodbye"
	print (cmd | getline x)
	print x

	# test error checking
	print (input_fd(cmd) >= 0)
	print (input_fd(cmd, "|<<") >= 0)

	print (input_fd(cmd, "|<") >= 0)
	print (cmd | getline x)
	print x
	print close(cmd)
}
