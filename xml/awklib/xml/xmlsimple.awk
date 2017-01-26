#------------------------------------------------------------------
# xmlsimple --- simpler interface to the gawkextlib XML extension
#
# Based on initial work done by Stefan Tramms in summer 2003 (xmllib)
#
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Updated: December 2014
#
# Prefix for user seeable items:  Xml    (no prefix for short names)
# Prefix for internal only items: _Xml_
#------------------------------------------------------------------
#
# RESPONSIBILITIES
# - Provide a single (short name) variable for each parsing event
# - Automatically report parsing errors
# - Collect in a single variable contiguous text parsed in several chunks
# - Keep a stack of attributes for the current element and its ancestors
# - Optionally, normalize space in text data
#
# INTERFACE SUMMARY
#
# Input variables (set by the library for each input token)
#   XD:  XML declaration (null/1 flag) + XMLATTR
#   PI:  Processing instruction (name)
#   CM:  XML comment (comment text)
#   SD:  Start of a DOCTYPE declaration (root element name) + XMLATTR 
#   ED:  End of a DOCTYPE declaration (null/1 flag)
#   UP:  Unparsed input token (null/1 flag, mostly DOCTYPE internal set items)
#   SE:  Start of element (name) + XMLATTR
#   EE:  End of element (name)
#   SC:  Start of a CDATA section (null/1 flag)
#   EC:  End of a CDATA section (null/1 flag)
#   TX:  Character data (text content)
#   EOI: End of input document (null/1 flag, only for XMLMODE=-1)
#
#   CHARDATA:  Collected character data (meaningful only at SE or EE)
#   ATTR:  Array[path@name] of attribute values for nested open elements
#   XmlDocCount:  Number of documents in a single XML stream     (** TODO **)
#
# Additional input variables (set by the parser itself, not by the library)
#   XMLPATH:  Path of nested elements (/name/name...)
#   XMLATTR:  Array[attrib_name] of attribute values for the current token
#
# Mode variables (set by the user)
#   XMLTRIM:  Normalize whitespace in character data (-1, 0, 1)
#   XMLINDENT:  Indentation step (-1, 0, n)     (** TODO **)
#
# Internal variables
#   _Xml_cdata:  collected character data
#   _Xml_x:  loop control or auxiliary variable
#   _Xml_str:  auxiliary variable
#   _Xml_endpath:  just closed element path (to clear ATTR after EE)
#   _Xml_CDATAMODE:  true inside CDATA sections
#   _Xml_internal_subset: true inside DOCTYPE with internal declarations
#   _Xml_CACHE:  array to store compiled xpath-like expressions
#   _Xml_GREP:  PATH of the current element to grep
#------------------------------------------------------------------

# load the XML extension and ensure error reporting
@include "xmlbase"
# allow reconstruction of the current XML token
@include "xmlcopy"


#------------------------------------------------------------------
#     Initial settings
#------------------------------------------------------------------

BEGIN {
    # trim CHARDATA by default (if not already disabled)
    XMLTRIM = XMLTRIM=="" ? 1 : (XMLTRIM == 0 ? 0 : 1)
    # XMLINDENT = 2   #** TODO **
}


#------------------------------------------------------------------
#     High-level facilities
#------------------------------------------------------------------

# Convert selection path to regular expression, and cache it for future uses
# Supported selectors:
# - /element...       Absolute path
# - element...        Relative path
# - *                 Match any element
# - //...             Match any descendant
function _Xml_Path_RE(path,         k, step, re) {
    if (path in _Xml_CACHE) {  # already converted, use cached stuff
        return _Xml_CACHE[path]
    } else {
        split(path, step, "/")
        re = ""
        for (k=1; k in step; k++) {
            if (step[k]=="") {
                re = re (k==1 ? "^" : "(|/.+)")
            } else if (step[k]=="*") {
                re = re "/[^/]+"
            } else {
                re = re "/" step[k]
            }
        }
        _Xml_CACHE[path] = re
        return re
    }
}

# Return the PATH to the parent node
function XmlParent(xpath) {
    return gensub(/\/[^/]*$/, "", "", path ? path : XMLPATH)
}

# Copy the current element to the standard output
function XmlGrep() {
    if (!_Xml_GREP && XMLEVENT=="STARTELEM") {
        _Xml_GREP = XMLPATH
        XmlCopy()
    }
}

# Match current XMLPATH against a XPath-like pattern
function XmlMatch(xpath) {
    return XMLPATH ~ _Xml_Path_RE(xpath) "$"
}

# Return the path prefix of the matched subpath
# - XmlMatch(XmlMatchScope(xpath) xpath) should succeed
function XmlMatchScope(xpath) {
    if (match(XMLPATH "/", _Xml_Path_RE(xpath) "/")) {
        return substr(XMLPATH, 1, RSTART+RLENGTH-2)
    } else {
        return ""
    }
}

# Look for the innermost element on the path with a given attribute and value
# - xpath:  XPath-like path expression
# - name:  attribute name
# - value:  optional attribute value or value pattern
# - mode:  optional flag to handle value as regular expression
function XmlMatchAttr(xpath, name, value, mode,            re, path, p) {
    re = _Xml_Path_RE(xpath) "$"
    # search from innermost scope outwards
    # substr(path,1,1)=="/" avoid infinite loop for corrupted XMLPATH
    for (path=XMLPATH; substr(path,1,1)=="/"; path=gensub(/\/[^/]*$/, "", "", path)) {
        if (path ~ re) {
            if ((p = (path "@" name)) in ATTR) {
                if (value) {
                    if (mode) {
                        if (ATTR[p] ~ value) {
                            return path
                        }
                    } else {
                        if (ATTR[p] == value) {
                            return path
                        }
                    }
                } else {
                    return path
                }
            }
        }
    }
    return ""
}


#------------------------------------------------------------------
#     Clear flags/variables from previous event
#------------------------------------------------------------------

SE {
    CHARDATA = ""
}

EE {
    CHARDATA = ""
    # delete attributes of just ended element
    for (_Xml_x in ATTR) {
        if ( substr(_Xml_x,1,index(_Xml_x, "@")-1) == _Xml_endpath ) delete ATTR[_Xml_x]
    }
}

EOI { # end-of-instance
    XmlDocCount++
}

{
    XD = PI = CM = SD = ED = UP = SE = EE = SC = EC = TX = EOI = ""
}


#------------------------------------------------------------------
#     Set flags/variables for current event
#------------------------------------------------------------------

# make collected character data available
function _Xml_ProvideCHARDATA() {
    CHARDATA = _Xml_cdata; _Xml_cdata = ""
    if (XMLTRIM > 0) {
        CHARDATA = _Xml_trim(CHARDATA)
    } else if (XMLTRIM < 0) {
        CHARDATA = _Xml_trim_1(CHARDATA)
    }
}

! XMLPATH {
    _Xml_cdata = ""
}

XMLEVENT {
    switch (XMLEVENT) {

    case "DECLARATION":
        XD = 1
        break;

    case "STARTDOCT":
        SD = XMLNAME
        break

    case "ENDDOCT":
        ED = 1
        break

    case "PROCINST":
        PI = XMLNAME
        break

    case "STARTELEM":
        SE = XMLNAME
        _Xml_ProvideCHARDATA()
        # add attributes of the current node
        for (_Xml_x = 1; _Xml_x <= NF; _Xml_x++) {
             ATTR[XMLPATH "@" $_Xml_x] = XMLATTR[$_Xml_x]
        }
        break

    case "ENDELEM":
        EE = XMLNAME
        _Xml_ProvideCHARDATA()
        _Xml_endpath = XMLPATH
        break

    case "CHARDATA":
        TX = 1
        # collect contiguous data fragments
        _Xml_cdata = _Xml_cdata $0
        break

    case "STARTCDATA":
        SC = 1
        _Xml_CDATAMODE = 1
        break

    case "ENDCDATA":
        EC = 1
        _Xml_CDATAMODE = 0
        break

    case "COMMENT":
        CM = $0
        break

    case "UNPARSED":
        UP = 1
        break

    case "ENDDOCUMENT":
        EOI = 1
        break

    default:
        XmlWriteError("xmlsimple: Unrecognized XML event <" XMLEVENT ">")
    }
   
}


#------------------------------------------------------------------
#     Copy curent token on demand
#------------------------------------------------------------------

_Xml_GREP {
    XmlCopy()
    if (EE && _Xml_GREP==XMLPATH) _Xml_GREP = ""
}
