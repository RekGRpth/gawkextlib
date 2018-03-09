# Convert a text file to HTML
#
# Blank lines delimit paragraphs. They generate <p> tags.
# Indented blocks means preformatted text. They generate <pre> tags. 

# Establish the text processing mode: <p>, <pre> or none
function setmode( newmode ) {
    if (newmode != mode) {
	    # ignore space at end of blocks
	    space = 0
	    # close the current mode, if not null
		if (mode) print "</" mode ">"
		# record the new mode
		mode = newmode
		# open the new mode, if not null
		if (mode) print "<" mode ">"
	}
}

# Print a line in the current mode
function printline( line ) {
    if (mode == "pre") {
	    for (;space>0;space--) print ""
	} else if (mode == "p") {
	    line = gensub(/`([^`]+)`/, "\\1", "g", line)
	}
	print line
}

BEGIN { # HTML preamble
    file = gensub(/^.*\//, "", 1, ARGV[1])
    print "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\""
    print "\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">"
    print "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
    print "<head>"
    print "<title>" file "</title>"
    print "<link href=\"../gawkextlib.css\" rel=\"StyleSheet\" type=\"text/css\" />"
    print "</head>"
    print "<body>"
}

# Escape HTML metacharacters
{
    gsub(/&/, "\\&amp;") # this must be the first
    gsub(/</, "\\&lt;")
    gsub(/>/, "\\&gt;")
}

# Header underlining lines
/^-+[[:space:]]*$/ || /^=+[[:space:]]*$/ {
    print "<br/>"
	print
	next
}

# Non-indented line
/^[^[:space:]]/ {
    setmode( "p" )
	print gensub(/`([^`]+)`/, "<code>\\1</code>", "g", $0)
	next
}

# Indented line
/[^[:space:]]/ {
    if (mode == "pre") {
	    for (;space>0;space--) print ""
	} else {
        setmode( "pre" )
	}
	print
	next
}

# Blank line
{
    if (mode == "pre") {
	    space++
	} else {
	    setmode( "" )
	}
	next
}

# HTML epilogue
END {
    setmode( "" )
    print "</body>"
    print "</html>"
}
