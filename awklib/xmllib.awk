# start of the little library
#
# $Id: xmllib.awk,v 1.1 2005/03/09 18:27:53 jkahrs Exp $
# Modified by M. Collado 2004/06/18
#
# prefix for user seeable items:  Xml
# prefix for internal only items: Xml_
#
# Xml_INDENT is set by xmlgrep.awk


############################################################
#
#     Initial settings
#
############################################################

BEGIN {
    XMLMODE = 1
    XMLCHARSET = "ISO-8859-1"
}


############################################################
#
#     String library functions
#
############################################################

# remove leading and trailing [[:space:]] characters
function trim(str)
{
    sub(/^[[:space:]]+/, "", str)
    if (str) sub(/[[:space:]]+$/, "", str)
    return str
}

# quote function for character data
#  escape & and <
#  the name is a historical accident
function quoteamp(str)
{
    gsub(/&/, "\\&amp;", str) # this must be the first
    gsub(/</, "\\&lt;", str)
    return str
}

# quote function for attribute values
#  escape every character, which can make problems
#  in attribute value strings
#  we have no information, whether attribute values
#  were enclosed in single or double quotes
function quotequote(str)
{
    str = quoteamp(str)
    gsub(/"/, "\\&quot;", str)
    gsub(/'/, "\\&apos;", str)
    return str
}

# return the last element from a string 
# splitrx (default "/") delimited path s (default PATH)
function XmlPathTail(s, splitrx,   nf, a)
{
   if (s == "") s = PATH
   if (splitrx == "") splitrx = "/"
   nf = split(s, a, splitrx)
   return a[nf]
}

# return the pretty formatted current startelement
#  top(== XmlPathTail) from PATH
function XmlStartelement(   s, i, len)
{
    s = "<" Xml_estack[Xml_sp]
    len = length(PATH) + 2
    for (i = 1; i <= Xml_astack[Xml_sp]; i++) {
        s = s " " substr(Xml_astack[Xml_sp, i], len)
        s = s "=\"" quotequote(ATTR[Xml_astack[Xml_sp, i]]) "\""
        # Xml_INDENT set by xmlgrep.awk
        if (Xml_INDENT && i > 2) {
            s = s "\n" Xml_INDENT Xml_INDENT
        }
    }
    s = s ">"
    return s
}

# return the pretty formatted current endelement
#  top(== XmlPathTail) from PATH
function XmlEndelement()
{
    return "</" Xml_estack[Xml_sp] ">"
}

# print the stored attributes for a given absolute path
#  example: XmlTracAattr("/root/e1/e2")
# a regexp is not allowed for path
# a little helper for debugging purposes
function XmlTraceAttr(path,   i)
{
    path = ( path ? path : PATH ) "@"
    for (i in ATTR) {
        if (index(i, path) == 1) printf("ATTR[%s]=\"%s\"\n", i, ATTR[i])
    }
}


############################################################
#
#     Clear flags/variables from previous record
#
############################################################

SE { # clear the startelem flag
    SE = CDATA = ""
   }

EE { # clear the endelem flag
    EE = CDATA = ""
    # delete attributes of current node
    for (Xml_i = Xml_astack[Xml_sp]; Xml_i > 0; --Xml_i) {
         delete ATTR[Xml_astack[Xml_sp, Xml_i]]
         delete Xml_astack[Xml_sp, Xml_i]
    }
    Xml_astack[Xml_sp] = 0 # XXX delete Xml_astack[Xml_sp] ??
    Xml_estack[Xml_sp] = ""
    Xml_sp--
    # pop last from path
    sub(/\/[^/]+$/, "", PATH)
}

PI { PI = CDATA = "" }

CM { CM = "" }

SD { SD = "" }

ED { ED = "" }

UP { UP = "" }


############################################################
#
#     Set flags/varibles for current record
#
############################################################

# special processing for <![CDATA[ ... ]]>, deliver the collected
# character data in the variable XmlCDATA and with tag in $0
XMLSTARTCDATA {
    Xml_ctemp = ""
    Xml_CDATAMODE = 1
    $0 = "<![CDATA["
}

Xml_CDATAMODE && XMLCHARDATA { Xml_ctemp = Xml_ctemp $0 }

XMLENDCDATA {
    XmlCDATA = Xml_ctemp; Xml_ctemp = ""
    $0 = "]]>"
    Xml_CDATAMODE = 0
    # include the real character data also in CDATA
    Xml_temp = Xml_temp XmlCDATA
}
# collect character data into one trimed string
!Xml_CDATAMODE && XMLCHARDATA  { Xml_temp = Xml_temp $0 }
# collect more into CDATA if we find a comment:
# aaa<!-- -->bbb gives CDATA="aaabbb"
# finish collection if we see an element
XMLSTARTELEM || XMLENDELEM {
    CDATA = trim(Xml_temp); Xml_temp = ""
}

# maintain a parse stack and make short varnames available
#  also maintain a stack of attribute names to speed EE processing
#  use the internal Xml_astack and Xml_sp variables
XMLSTARTELEM { # push token
    PATH = PATH "/" XMLSTARTELEM
    # push the full qualified attribute names onto Xml_astack
    Xml_sp++
    for (Xml_i in XMLATTRPOS) {
        Xml_temp = PATH "@" XMLATTRPOS[Xml_i]
        ATTR[Xml_temp] = XMLATTR[XMLATTRPOS[Xml_i]]
        Xml_astack[Xml_sp, Xml_i] = Xml_temp
        Xml_astack[Xml_sp]++
    }
    Xml_temp = ""
    SE = Xml_estack[Xml_sp] = XMLSTARTELEM
    $0 = XmlStartelement()
}

XMLENDELEM { # set the EE flag for later stack pop
    EE = XMLENDELEM
    $0 = XmlEndelement()
}

XMLCOMMENT {
    XMLCOMMENT = $0
    CM = trim($0)
    $0 = "<!--" $0 "-->"
}

XMLPROCINST {
    PI = XMLPROCINST
    $0 = "<?" XMLPROCINST ($0 ? " " $0 : "") "?>"
}

# very first procinst
XMLVERSION || XMLENCODING {
    PI = "xml"
    $0 = "<?xml" \
         (XMLVERSION  ? " version=\""  XMLVERSION  "\"" : "")\
         (XMLENCODING ? " encoding=\"" XMLENCODING "\"" : "")\
         "?>"
}

XMLSTARTDOCT {
    SD = XMLSTARTDOCT
    $0 = "<!DOCTYPE " XMLSTARTDOCT
    if (XMLDOCTPUBID) {
        $0 = $0 " PUBLIC " XMLDOCTPUBID
        if (XMLDOCTSYSID) {
            $0 = $0 " " XMLDOCTSYSID
        }
    } else if (XMLDOCTSYSID) {
        $0 = $0 " SYSTEM " XMLDOCTSYSID
    }
}

XMLENDDOCT {
    ED = XMLENDDOCT
    $0 = ">"
}

XMLUNPARSED {
    UP = XMLUNPARSED
}

# end of the little library

# vim: set filetype=awk :
