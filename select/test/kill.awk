BEGIN {
	pid = PROCINFO["pid"]
	rc = kill(pid, 0); print rc, ERRNO
	rc = kill(pid, "pipe"); print rc, ERRNO
	rc = kill(pid, "SigPipe"); print rc, ERRNO
	rc = kill(pid, -100); print rc, ERRNO
}
