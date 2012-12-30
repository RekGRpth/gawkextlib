#------------------------------------------------------------------
# xmlbase --- basic facilities
#
# Author: Manuel Collado, http://lml.ls.fi.upm.es/~mcollado
# License: Public domain
# Updated: November 2012
#
# prefix for user seeable items:  Xml
# prefix for internal only items: _Xml_
#------------------------------------------------------------------

# load the XML extension
@load "xml"

#------------------------------------------------------------------
#     Default settings (the less restrictive ones)
#     Can be overrriden by explicitly setting them
#------------------------------------------------------------------

BEGIN {
   XMLMODE = XMLMODE=="" ? -1 : XMLMODE  # Use streaming by default
   # Set UTF-8 as default encoding, to avoid non-representable chars
   XMLCHARSET = XMLCHARSET=="" ? "UTF-8" : XMLCHARSET
}

#------------------------------------------------------------------
#     Error checking and reporting facilities:
#------------------------------------------------------------------

# Format and write an error message to the standard error stream
function XmlWriteError(message) {
   printf("\n%s:%d:%d:(%d) %s\n", FILENAME, XMLROW, XMLCOL, XMLLEN, message) > "/dev/stderr"
}

# Check for error and write a message
# gawkextlib XML error reporting needs some redesign.
# Interim code: uses both ERRNO and XMLERROR to generate consistent messages
function XmlCheckError() {
   if (XMLERROR) {
      XmlWriteError(XMLERROR)
   } else if (ERRNO) {
      printf("\n%s\n", ERRNO) > "/dev/stderr"
      ERRNO = ""
   }
}

ENDFILE {
   # report error, if any
   XmlCheckError()
}

#------------------------------------------------------------------
#     Escaping and whitespace trimming functions:
#------------------------------------------------------------------

# Encode metacharacters '<', '>' and '&' as XML predefined entities:
function XmlEscape(str) {
   gsub(/&/, "\\&amp;", str) # this must be the first
   gsub(/</, "\\&lt;", str)
   gsub(/>/, "\\&gt;", str)
   return str
}

# Encode '<', '>', '&' and quotes as XML predefined entities:
function XmlEscapeQuote(str) {
    str = XmlEscape(str)
    gsub(/\"/, "\\&quot;", str)
    gsub(/'/, "\\&apos;", str)
    return str
}

# Internal: remove leading and trailing [[:space:]] characters,
#    and collapse contiguous spaces into a single one
function _Xml_trim(str) {
   sub(/^[[:space:]]+/, "", str)
   if (str) sub( /[[:space:]]+$/, "", str )
   if (str) gsub( /[[:space:]]+/, " ", str )
   return str
}

