{
	printf("\"%s\": length: %d, mbs_length: %d\n",
		$0, length($0), mbs_length($0))

	l = mbs_split($0, a)
	printf("\"%s\": got %d elements\n", $0, l)
	for (i = 1; i <= l; i++)
		printf("\t%d %c\n", a[i], a[i])

	newstr = mbs_join(a)
	printf("\"%s\": got \"%s\" from mbs_join()\n", $0, newstr)

	printf("\"%s\": wcswidth: %d\n", $0, mbs_wcswidth($0))
}
