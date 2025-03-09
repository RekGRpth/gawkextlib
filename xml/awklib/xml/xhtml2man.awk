# XHTML to manpage converter
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Updated: March 2025

#--------------------------- EXPERIMENTAL, FIRST VERSION

# Supported XHTML tags:
# - <title>
# - <meta>     name=section|date|source|manual
# - <h2>       section name NAME|SYNOPSIS|...
# - <h3>       subsection
# - <p>        paragraph
# - <dl>       definition list
# - <dt>       definition term
# - <dd>       definition paragraph
# - <ol>       numbered list
# - <ul>       bulleted list 
# - <li>       list item
# - <pre>      preformatted text
# - <br>       line break
# - <a>        link
# - <em>       italic
# - <i>        italic
# - <var>      italic
# - <b>        boldface
# - <strong>   boldface
# - <code>     monospace
# - <kbd>      monospace
# - <samp>     monospace
# - <tt>       monospace

@include "xmlsimple"

BEGIN {
    #--- Set the XML mode
    XMLTRIM = -1
    XMLCHARSET = "US-ASCII" # non-ASCII chars are problematic
#    XMLCHARSET = "ISO-8859-1"
#    XMLCHARSET = "WINDOWS-1252"
#    XMLCHARSET = "UTF-8"

    #--- Font variables
    # style numbers
    sital = 1
    sbold = 2
    smono = 3
    # style nesting counters
    # slevel [1/2/3]
    # style values
    vital = 1
    vbold = 2
    vmono = 4
    # style codes - font[vital+vbold+vmono]
    font[0] = "\\fR"        # initial, default font
    font[1] = "\\fI"        # italic
    font[2] = "\\fB"        # bold
    font[3] = "\\f(BI"      # italic + bold
    font[4] = "\\fR\\f[C]"  # monospace
    font[5] = "\\fI"        # monospace + italic ==> italic
    font[6] = "\\fB"        # monospace + bold ==> bold
    font[7] = "\\f(BI"      # monospace + italic + bold ==> italic + bold
    # enable style flag
    with_style = 1          # enable font changes, by default

    #--- Options
    if (OUTFILE) outfile = OUTFILE
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

#----------------------------------- font changes (may be nested)
function set_style( sty,    style ) {
    if (with_style) {
        if (sty>0) {
            slevel[sty]++
        } else {
            slevel[-sty]--
        }
        write(font_code())
    }
}
function font_code(         style) {
    style = (slevel[sital]>0)*vital + (slevel[sbold]>0)*vbold + (slevel[smono]>0)*vmono
    return font[style]
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
    # escape backslashes
    gsub("\\\\", "\\\\", CHARDATA)
    # first paragraph of the NAME section
    if (EE=="p" && nameflag) {
        sub("-", "\\-", CHARDATA)
    }
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
    if (outfile "" == "") {   # default output file  xxx.xhtml --> xxx.sec
        outfile = FILENAME
        sub("\\..*$", "", outfile)
        outfile = outfile "." meta["section"]
    }
    write_next(sprintf(".TH %s %s \"%s\" \"%s\" \"%s\"", meta["title"], meta["section"], meta["date"], meta["source"], meta["manual"]))
    body = 1
    next
}

EE ~ /h[2-6]/ {     # re-enable styles
    with_style = 1
    br()
    # special NAME section
    nameflag = (EE=="h2" && CHARDATA=="NAME")
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

SE ~ /h[2-6]/ {     # disable styles
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

SE=="br" {
    write_line(".br")
    next
}

SE~/^(em|i|var)$/ {         # italic
    set_style(sital)
    next
}
EE~/^(em|i|var)$/ {
    set_style(-sital)
    next
}

SE~/^(b|strong)$/ {         # bold
    set_style(sbold)
    next
}
EE~/^(b|strong)$/ {
    set_style(-sbold)
    next
}

SE~/^(code|kbd|samp|tt)$/ { # monospace
    set_style(smono)
    next
}
EE~/^(code|kbd|samp|tt)$/ {
    set_style(-smono)
    next
}

SE=="a" {
    write_line(".UR " XMLATTR["href"])
}
EE=="a" {
    write_next(".UE ")
}
