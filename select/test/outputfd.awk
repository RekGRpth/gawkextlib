BEGIN {
	cmd = "echo hello; echo goodbye"
	print (cmd | getline x)
	print x
	print (cmd | getline x)
	print x

	# test error checking
	print (output_fd(cmd, "<>") >= 0)
	print close(cmd)

	cmd = "cksum"
	print close(cmd)
	print "see how output_fd actually launches the command"
	print "rc", (output_fd(cmd, "|>") >= 0)
	print close(cmd)
}
