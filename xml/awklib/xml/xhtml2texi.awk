# XHTML to texinfo converter
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Updated: February 2025

#--------------------------- EXPERIMENTAL, PROOF OF CONCEPT ONLY

# Mode parameters:
# - toplevel = (chapter/section/...)  <h1> headings
# - numberlevel (0-4)  max. <hn> numbered headings (*)
# - nodelevel (0-4)  max. <hn> that generates a node (*)
# (*) effective max. numbered or node heading is "subsubsection"

# Supported XHTML tags:
# - <title>
# - <meta>     name=section|date|source|manual
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
# - <br>       line break
# - <b>        boldface
# - <i>        italics
# - <code>     monospace
# - <dfn>      definition or usage - generate index entry
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
    XMLCHARSET = "US-ASCII"
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
    
    #--- Ensure a toplevel node
    currentnode[1] = 0
    nodelevel[0] = 1
}

BEGINFILE {
    outfile = FILENAME
    sub("\\..*$", "", outfile)
    outfile = outfile ".texi"
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

#----------------------------------- instert a blank line
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
    # if (header_flag) {
        # header_line = header_line text
    # } else {
        printf("%s", text) > outfile
    # }
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
    if (headinglevel > NUMBERLEVEL) {
        return unnumbered[effectivelevel]
    } else {
        return heading[effectivelevel]
    }
}

# {
# print "<" XMLEVENT "><" XMLNAME ">" > outfile
# }

#----------------------------------- ignore unwanted stuff

ignore {
   if (EE && ignore==XMLPATH) {
       ignore = ""
   }
   next
}
SE && ("class" in XMLATTR) && XMLATTR["class"] !~ /texi/ {
    ignore = XMLPATH
    next
}

#----------------------------------- first pass: collect the node structure

FNR==NR {
    if (SE~/^h[1-6]$/) {
        nodecount++
        nodelevel[nodecount] = substr(SE, 2) + 0
        if ("title" in XMLATTR) title[nodecount] = XMLATTR["title"]
    }
    if (EE~/^h[1-6]$/) {
        node[nodecount] = escape(CHARDATA)
    }
    next
}

#----------------------------------- text fragments

copyflag && (SE || EE) && CHARDATA {
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

#----------------------------------- man NAME section

SE=="p" && nameflag {
    write(tolower(meta["title"]) " - ")
    nameflag = 0
}

#----------------------------------- <head> elements

EE=="title" {
    meta["title"] = CHARDATA
    next
}

SE=="meta" {
    meta[XMLATTR["name"]] = XMLATTR["content"]
    next
}

#----------------------------------- block elements

SE=="body" {
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
    #--- generate heading
    write_line("@" headingname(headlevel) " " node[nodecount])
    #--- disable CHARDATA automatic output
    copyflag = 0
    next
}
EE=="h2" && CHARDATA=="NAME" {
    #--- include the manpage name in the next paragraph
    nameflag = 1
}
EE~/^h[1-6]$/ {
    #--- enable CHARDATA automatic output
    copyflag = 1
    next
}

# SE=="h1" {
    # write_next("@node " nodetitle(nodecount))
    # br()
    # write_next("@section " node[nodecount])
    # br()
    # header_flag = 1
    # header_line = ""
    # copyflag = 1
    # next
# }
# EE=="h1" {
    # header_flag = 0
    # next
# }

# SE=="h2" {
    # write_next("@node " nodetitle(nodecount))
    # br()
    # write_next("@subsection " node[nodecount])
    # br()
    # header_flag = 1
    # header_line = ""
    # copyflag = 1
    # next
# }
# EE=="h2" {
    # header_flag = 0
    # next
# }

# SE=="h3" {
    # write_next("@node " nodetitle(nodecount))
    # br()
    # write_next("@subsubsection " node[nodecount])
    # br()
    # header_flag = 1
    # header_line = ""
    # next
# }
# EE=="h3" {
    # header_flag = 0
    # next
# }

# SE=="h6" {
    # header_flag = 1
    # header_line = ""
    # next
# }
# EE=="h6" {
    # header_flag = 0
    # write_next("@subsubheading " header_line)
    # br()
    # next
# }

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
#    pre_flag = 1
    next
}
EE=="pre" {
#    pre_flag = 0
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

SE=="i" {
    write("@emph{")
    next
}
EE=="i" {
    write("}")
    next
}

SE=="b" {
    write("@strong{")
    next
}
EE=="b" {
    write("}")
    next
}

SE=="code" {
    write("@code{")
    next
}
EE=="code" {
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
