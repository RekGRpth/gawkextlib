# Add gawkextlib.css stylesheet to HTML input
#
/<\/head/ {
    print "<link href=\"../gawkextlib.css\" rel=\"StyleSheet\" type=\"text/css\" />"
}

{ print }
