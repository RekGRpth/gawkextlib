BEGIN {
	print select(readfds, writefds, exceptfds, 0)
}
