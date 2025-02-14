# XHTML to manpage converter
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Updated: February 2025

#--------------------------- EXPERIMENTAL, PROOF OF CONCEPT ONLY

# Supported XHTML tags:
# - <title>
# - <meta>     name=section|date|source|manual
# - <h2>       section name NAME|SYNOPSIS|...
# - <h3>       subsection
# - <p>        paragraph
# - <dl>       definition list
# - <dt>       definition term
# - <dd>       definition paragraph
# - <ol>       numbered list (*** TODO ***)
# - <ul>       bulleted list 
# - <li>       list item
# - <br>       line break
# - <b>        boldface
# - <i>        italics
# - <code>     monospace

@include "xmlsimple"

BEGIN {
    XMLTRIM = -1
    XMLCHARSET = "US-ASCII"
#    XMLCHARSET = "UTF-8"
}

#----------------------------------- ensure a newline
function br() {
    if (filled) {
        print "" > outfile
        filled = 0
    }
}

#----------------------------------- print text on the same line
function write( text ) {
    printf("%s", text) > outfile
    filled = 1
}

#----------------------------------- print text on the next line
function write_next( text ) {
    br()
    write(text)
}

#----------------------------------- print text alone on the next line
function write_line( text ) {
    br()
    write(text)
    br()
}

#----------------------------------- ignore unwanted stuff

ignore {
   if (EE && ignore==XMLPATH) {
       ignore = ""
   }
   next
}
SE && ("class" in XMLATTR) && XMLATTR["class"] !~ /man/ {
    ignore = XMLPATH
    next
}

#----------------------------------- text fragments

copyflag && (SE || EE) && CHARDATA {
    gsub("\\\\", "\\\\", CHARDATA)
    write(CHARDATA)
}

#----------------------------------- paragraph spacing

# SE {
    # level++
# }
# (SE=="p" || SE=="dd") && prev[level]==SE {
    # write_line(".sp")
# }
# EE {
    # prev[level] = EE
    # delete prev[level+1]
    # level--
# }
SE=="p" {
    write_line(".PP")
}

#----------------------------------- <head> elements

EE=="title" {
    meta["title"] = CHARDATA
    k = index(CHARDATA, ",")
    meta["title1"] = k ? substr(CHARDATA, 1, k-1) : CHARDATA
    next
}

SE=="meta" {
    meta[XMLATTR["name"]] = XMLATTR["content"]
    next
}

#----------------------------------- block elements

SE=="body" {
    outfile = FILENAME
#    sub("\\.in\\.xhtml$", "", outfile)
    sub("\\..*$", "", outfile)
    manpage = outfile
    outfile = outfile "." meta["section"]
    write_next(sprintf(".TH %s %s \"%s\" \"%s\" \"%s\"", manpage, meta["section"], meta["date"], meta["source"], meta["manual"]))
    next
}

SE=="h2" {
    write_next(".SH ")
    copyflag = 1
    next
}
EE=="h2" {
    br()
    if (CHARDATA=="NAME") {
        nameflag = 1
    }
    next
}

SE=="h3" {
    write_next(".SS ")
    next
}
EE=="h3" {
    br()
    next
}

SE ~ /h[4-6]/ {
    write_line(".PP")
    write("\\fB")
    next
}
EE ~ /h[4-6]/ {
    write("\\fP")
    br()
    next
}

SE=="pre" {
    write_line(".EX")
    old_trim = XMLTRIM
    XMLTRIM = 0
    next
}
EE=="pre" {
    write_line(".EE")
    XMLTRIM = old_trim
    next
}

SE=="dt" {
    write_next(".TP\n\\fB")
    next
}
EE=="dt" {
    write("\\fP")
    br()
    next
}

SE=="br" {
    write_line(".br")
    next
}

SE=="li" {
    write_line(".IP \\(bu 3")
}

#----------------------------------- nested lists

SE=="ol" || SE=="ul" || SE=="dl" {
    if (listlevel++) write_line(".RS")
}
EE=="ol" || EE=="ul" || EE=="dl" {
    if (--listlevel) write_line(".RE")
}

#----------------------------------- NAME section

SE=="p" && nameflag {
    write(tolower(meta["title"]) " \\- ")
    nameflag = 0
}

#----------------------------------- inline elements

SE=="i" {
    write("\\fI")
    next
}
EE=="i" {
    write("\\fP")
    next
}

SE=="b" {
    write("\\fB")
    next
}
EE=="b" {
    write("\\fP")
    next
}

SE=="code" {
    write("\\f(CW")
    next
}
EE=="code" {
    write("\\fP")
    next
}
