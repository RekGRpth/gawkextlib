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
# - <i>        italic
# - <code>     monospace

@include "xmlsimple"

BEGIN {
    #--- Set the XML mode
    XMLTRIM = -1
    XMLCHARSET = "US-ASCII"
#    XMLCHARSET = "UTF-8"

    #--- Set font codes
    sital = 1
    sbold = 2
    smono = 4
    style = 0              # 1=italic + 2=bold + 4=monospace
    font[0] = "\\fR"       # initial, default font
    font[1] = "\\fI"       # italic
    font[2] = "\\fB"       # bold
    font[3] = "\\f(BI"     # italic + bold
    font[4] = "\\fR\\f[C]" # monospace
    font[5] = "\\fI"       # monospace + italic ==> italic
    font[6] = "\\fB"       # monospace + bold ==> bold
    font[7] = "\\f(BI"     # monospace + italic + bold ==> italic + bold
    with_style = 1         # enable font changes, by default
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

#----------------------------------- font changes
function set_style( sty ) {
    if (with_style) {
        style += sty
        write(font[style])
    }
}

#----------------------------------- ignore unwanted stuff

skip {
    if (SE) skip++
    if (EE) skip--
    next
}
SE && ("class" in XMLATTR) && XMLATTR["class"] !~ /man/ || SE=="h1" {
    skip = 1
    next
}

#----------------------------------- text fragments

body && (SE || EE) && CHARDATA {
    gsub("\\\\", "\\\\", CHARDATA)
    write(CHARDATA)
}

#----------------------------------- paragraph spacing

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
    sub("\\..*$", "", outfile)
    outfile = outfile "." meta["section"]
    write_next(sprintf(".TH %s %s \"%s\" \"%s\" \"%s\"", meta["title"], meta["section"], meta["date"], meta["source"], meta["manual"]))
    body = 1
    next
}

EE ~ /h[2-6]/ {     # reset bold, re-enable styles
    with_style = 1
    set_style(-sbold)
    br()
    next
}

SE=="h2" {
    write_next(".SH ")
}

SE=="h3" {
    write_next(".SS ")
}

SE ~ /h[4-6]/ {
    write_line(".PP")
}

SE ~ /h[2-6]/ {     # set bold, disable styles
    set_style(sbold)
    with_style = 0
}

SE=="pre" {
#    write_line(".EX")
    write_line(".IP\n.nf\n")
    set_style(smono)
    old_trim = XMLTRIM
    XMLTRIM = 0
    next
}
EE=="pre" {
#    write_line(".EE")
    set_style(-smono)
    write_line("\n.fi")
    XMLTRIM = old_trim
    next
}

SE=="dt" {
    write_next(".TP\n")
    style += sbold
    write(font[style])
    next
}
EE=="dt" {
    style -= sbold
    write(font[style])
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

#----------------------------------- inline elements

SE=="i" {
    set_style(sital)
    next
}
EE=="i" {
    set_style(-sital)
    next
}

SE=="b" {
    set_style(sbold)
    next
}
EE=="b" {
    set_style(-sbold)
    next
}

SE=="code" {
    set_style(smono)
    next
}
EE=="code" {
    set_style(-smono)
    next
}
