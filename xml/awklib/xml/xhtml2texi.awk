# XHTML to texinfo converter
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Updated: March 2025

#--------------------------- EXPERIMENTAL, FIRST VERSION

# Mode parameters:
# - toplevel = (chapter/section/...)  <h1> headings
# - numberlevel (0-4)  max. <hn> numbered headings (*)
# - nodelevel (0-4)  max. <hn> that generates a node (*)
# (*) effective max. numbered or node heading is "subsubsection"

# Supported XHTML tags (* = not yet implemented):
# - <h1>       section
# - <h2>       subsection
# - <h3>       subsubsection
# - <p>        paragraph
# - <dl>       definition list
# - <dt>       definition term
# - <dd>       definition paragraph
# - <ol>       numbered list
# - <ul>       bulleted list 
# - <li>       list item
# - <pre>      preformatted text
# - <dfn>      definition or usage - generate index entry
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
#
# Partially supported:
# - <table>    table
# - <col>      column specification
# - <tr>       table row
# - <th>       table header
# - <td>       table cell

@include "xmlsimple"

BEGIN {
    #--- Set the XML mode
    XMLTRIM = -1
    XMLCHARSET = "US-ASCII" # non-ASCII chars are problematic
#    XMLCHARSET = "ISO-8859-1"
#    XMLCHARSET = "UTF-8"

    #--- Process the file twice
    ARGV[2] = ARGV[1]
    ARGC = 3

    #--- Texi section names
    lastlevel = 5
    heading[1] = "chapter"
    heading[2] = "section"
    heading[3] = "subsection"
    heading[4] = "subsubsection"
    heading[5] = "subsubheading"
    unnumbered[1] = "unnumbered"
    unnumbered[2] = "unnumberedsec"
    unnumbered[3] = "unnumberedsubsec"
    unnumbered[4] = "unnumberedsubsubsec"
    unnumbered[5] = "subsubheading"
    nomenuheading = "subsubheading"

    #--- Inline text styles
    sopen["br"] = "@*"
    sopen["i"] = "@emph{"
    sclose["i"] = "}"
    sopen["b"] = "@strong{"
    sclose["b"] = "}"
    sopen["code"] = "@code{"
    sclose["code"] = "}"

    #--- Options
    if (OUTFILE) outfile = OUTFILE

    #--- Mode parameters
    if (!TOPLEVEL) TOPLEVEL = "section" # "chapter" "section"  # default TOPLEVEL
    hoffset = -1
    for (k=1; k in heading && hoffset<0; k++) {
        if (heading[k]==TOPLEVEL) hoffset = k-1
    }
    if (hoffset < 0) {
        error("Invalid TOPLEVEL '" TOPLEVEL "'")
    }
    if (!NUMBERLEVEL) NUMBERLEVEL = 2
    if ((NUMBERLEVEL+0) "" != NUMBERLEVEL || NUMBERLEVEL < 0 || NUMBERLEVEL-hoffset > 4) {
        error("Invalid NUMBERLEVEL '" NUMBERLEVEL "'")
    }
    if (!NODELEVEL) NODELEVEL = 2
    if ((NODELEVEL+0) "" != NODELEVEL || NODELEVEL < 0 || NODELEVEL-hoffset > 4) {
        error("Invalid NODELEVEL '" NODELEVEL "'")
    }
    if (!MENULEVEL) MENULEVEL = 2
    if ((MENULEVEL+0) "" != MENULEVEL || MENULEVEL < 0 || MENULEVEL-hoffset > 4) {
        error("Invalid MENULEVEL '" MENULEVEL "'")
    }
    
    #--- Ensure a toplevel node
    currentnode[1] = 0
    nodelevel[0] = 1
}

BEGINFILE {
    if (outfile "" == "") {   # default output file  xxx.xhtml --> xxx.texi
        outfile = FILENAME
        sub("\\..*$", "", outfile)
        outfile = outfile ".texi"
    }
    nodecount = 0
}

#----------------------------------- error message
function error(message) {
    printf("** html2texi: %s\n", message)
    exit
}

#----------------------------------- escape the @{} metacharacters
function escape( text ) {
    gsub("@", "@@", text)  # must be first
    gsub("{", "@{", text)
    gsub("}", "@}", text)
    return text
}

#----------------------------------- ensure a newline
function br() {
    if (filled) {
        write("\n")
        filled = 0
    }
    write_index()
}

#----------------------------------- insert a blank line
function blank_line() {
    if (!blank_line_flag) {
        br()
        write("\n")
        filled = 0
        blank_line_flag = 1
    }
}

#----------------------------------- print text on the same line
function write( text ) {
    printf("%s", text) > outfile
    filled = 1
    blank_line_flag = 0
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

#----------------------------------- print pending index references
function write_index() {
    while (index_count) write_line("@cindex " escape(index_term[index_count--]))
}

#----------------------------------- title of the node
function nodetitle( nodeindex ) {
    if (nodeindex in title) {
        return title[nodeindex]
    } else {
        return node[nodeindex]
    }
}

#----------------------------------- generate a subheaders menu of the given header
function genmenu( nodeindex,        level, found, ni ) {
    level = nodelevel[nodeindex]
    found = 0
    for (ni=nodeindex+1; ni in node && nodelevel[ni]>level; ni++) {
        if (!found) {
            blank_line()
            write_line("@menu")
            found = 1
        }
        if (nodelevel[ni] == level + 1) {
            write_line("* " nodetitle(ni) "::")
        }
    }
    if (found) {
        write_line("@end menu")
    }
}

#----------------------------------- texi heading name for a <hn> level
function headingname( headinglevel,              effectivelevel ) {
    effectivelevel = headinglevel + hoffset
    if (!(effectivelevel in heading)) effectivelevel = lastlevel
    if (headinglevel > MENULEVEL) {
        return nomenuheading
    } else if (headinglevel > NUMBERLEVEL) {
        return unnumbered[effectivelevel]
    } else {
        return heading[effectivelevel]
    }
}

#----------------------------------- ignore unwanted stuff

skip {
    if (SE) skip++
    if (EE) skip--
    next
}
SE && ("class" in XMLATTR) && XMLATTR["class"] !~ /texi/ {
    skip = 1
    next
}

#----------------------------------- first pass: collect the node structure

FNR==NR && SE~/^h[1-6]$/ {  # start of section title
    nodecount++
    nodelevel[nodecount] = substr(SE, 2) + 0
    if ("title" in XMLATTR) title[nodecount] = XMLATTR["title"]
    sec_title = ""
    get_title = 1
    next
}

FNR==NR && (EE~/^h[1-6]$/) {  # end of section title
    node[nodecount] = sec_title escape(CHARDATA)
    get_title = 0
    next
}

FNR==NR {  # collect fragments of section title
    if (get_title) {
         sec_title = sec_title escape(CHARDATA)
         if (SE in sopen) sec_title = sec_title sopen[SE]
         if (EE in sclose) sec_title = sec_title sclose[EE]
    }
    next
}

#----------------------------------- text fragments

body && (SE || EE) && CHARDATA {
    if (pre_flag) {
        write(CHARDATA)
    } else {
        write(escape(CHARDATA))
    }
}

#----------------------------------- contiguous paragraph spacing

SE {
    level++
}
(SE=="p" || SE=="dd") && prev[level]==SE {
    blank_line()
}
EE {
    prev[level] = EE
    delete prev[level+1]
    level--
}

#----------------------------------- block elements

SE=="body" {
    body = 1
    next
}

SE~/^h[1-6]$/ {
    #--- compute nesting
    headlevel = nodelevel[++nodecount]
    currentnode[headlevel] = nodecount
    hasmenu[headlevel] = 0
    #--- generate menu for the containing level
    if (headlevel > 1 && headlevel <= NODELEVEL  && !hasmenu[headlevel-1]) {
        genmenu(currentnode[headlevel-1])
        hasmenu[headlevel-1] = 1
    }
    #--- generate node
    if (headlevel <= NODELEVEL) {
        write_line("@node " nodetitle(nodecount))
    }
    if (nodecount in title) {
        write_line("@cindex " nodetitle(nodecount))
    }
    #--- generate heading
    write_line("@" headingname(headlevel) " " node[nodecount])
    #--- <hn> contents already processed
    skip = 1
    next
}

SE=="dl" {
    blank_line()
    write_line("@table @asis")
    next
}
EE=="dl" {
    write_next("@end table")
    blank_line()
    next
}

SE=="dt" {
    write_next("@item @strong{")
    next
}
EE=="dt" {
    write("}")
    br()
    next
}

SE=="ol" {
    blank_line()
    write_line("@enumerate")
    next
}
EE=="ol" {
    write_line("@end enumerate")
    next
}

SE=="ul" {
    blank_line()
    write_line("@itemize")
    next
}
EE=="ul" {
    write_line("@end itemize")
    next
}

SE=="li" {
    write_line("@item")
    next
}

SE=="pre" {
    blank_line()
    write_line("@example")
    old_trim = XMLTRIM
    XMLTRIM = 0
    next
}
EE=="pre" {
    XMLTRIM = old_trim
    write_line("@end example")
    blank_line()
    next
}

#----------------------------------- inline elements

SE=="br" {
    write("@*")
    next
}

SE~/^(em|i|var)$/ {         # italic
    write("@emph{")
    next
}

SE~/^(b|strong)$/ {         # bold
    write("@strong{")
    next
}

SE~/^(code|kbd|samp|tt)$/ { # monospace
    write("@code{")
    next
}

EE~/^(em|i|var|b|strong|code|kbd|samp|tt)$/ {
    write("}")
    next
}

SE=="dfn" {
    entry = XMLATTR["title"]
    next
}
EE=="dfn" {
    index_count++
    if (entry) {
        index_term[index_count] = entry
    } else {
        index_term[index_count] = CHARDATA
    }
    next
}

SE=="a" {
    reftarget = XMLATTR["href"]
    reftype = reftarget ~ /^[a-z]+:\/\// ? "@uref" : "@ref"  # URL or node
    write_next( reftype "{" reftarget ", ")
}
EE=="a" {
    write("}")
}

#----------------------------------- tables

SE=="table" {
    write_next("@multitable")
    cell_count = 0
    next
}
EE=="table" {
    write_line("@end multitable")
    next
}

SE=="col" {
    if (!cell_count++) {
        write(" @columnfractions")
    }
    write(" " (XMLATTR["width"] / 100))
    next
}

SE=="tr" {
    cell_count = 0
    next
}
SE=="th" {
    if (cell_count++) {
        write(" @tab ")
    } else {
        write_next("@headitem ")
    }
    next
}
SE=="td" {
    if (cell_count++) {
        write(" @tab ")
    } else {
        write_next("@item ")
    }
    next
}
