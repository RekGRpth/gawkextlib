# xmlcopy - copy the current XML token
# Author: M. Collado
# (based on code from xmllib by S. Tramms)
# Public domain
# Oct. 2007
#
# prefix for user seeable items:  Xml
# prefix for internal only items: _Xml_

# load the XML extension
@load xml


#------------------------------------------------------------------
#     XmlCopy
#------------------------------------------------------------------

function XmlCopy(        token, n, str) {

   switch (XMLEVENT) {

   case "STARTELEM":
      token = "<" XMLNAME
      split( $0, set, " " )
      for (n = 1; n <= NF; n++) {
         str = XMLATTR[$n]
         gsub(/&/, "\\&amp;", str) # this must be the first
         gsub(/</, "\\&lt;", str)
         gsub(/"/, "\\&quot;", str)
         gsub(/'/, "\\&apos;", str)
         token = token " " $n "=\"" str "\""
      }
      token = token ">"
      break

   case "ENDELEM":
      token = "</" XMLNAME ">"
      break

   case "CHARDATA":
      token = $0
      if (!_Xml_CDATAMODE) {
         gsub(/&/, "\\&amp;", token) # this must be the first
         gsub(/</, "\\&lt;", token)
      }
      break

   case "COMMENT":
      token = "<!--" $0 "-->"
      break

   case "PROCINST":
      token = "<?" XMLNAME ($0 ? " " $0 : "") "?>"
      break

   case "DECLARATION":
      token = "<?xml" \
         ("VERSION" in XMLATTR  ? " version=\""  XMLATTR["VERSION"]  "\"" : "")\
         ("ENCODING" in XMLATTR ? " encoding=\"" XMLATTR["ENCODING"] "\"" : "")\
         ("STANDALONE" in XMLATTR ? " standalone=\"" XMLATTR["STANDALONE"] "\"" : "")\
         "?>"
      break

   case "STARTDOCT":
      token = "<!DOCTYPE " XMLNAME
      if ("PUBLIC" in XMLATTR) {
         token = token " PUBLIC \"" XMLATTR["PUBLIC"] "\""
         if ("SYSTEM" in XMLATTR) {
            token = token " \"" XMLATTR["SYSTEM"] "\""
         }
      } else if ("SYSTEM" in XMLATTR) {
         token = token " SYSTEM \"" XMLATTR["SYSTEM"] "\""
      }
      if ("INTERNAL_SUBSET" in XMLATTR) {
         token = token " ["
         _Xml_internal_subset = 1
      }
      break

   case "ENDDOCT":
      if (_Xml_internal_subset) {
         token = "]>"
         _Xml_internal_subset = ""
      } else {
         token = ">"
      }
      break

   case "STARTCDATA":
      _Xml_CDATAMODE = 1
      token = "<![CDATA["
      break

   case "ENDCDATA":
      _Xml_CDATAMODE = 0
      token = "]]>"
      break

   case "UNPARSED":
      token = $0
      break

   default:
      token = ""
      break
   }

   printf( "%s", token )
}

