#------------------------------------------------------------------
# xmlcopy --- compose and print the current XML token
#
# Author: Manuel Collado, <m-collado@users.sourceforge.net>
# License: Public domain
# Updated: November 2014
#
# Prefix for user seeable items:  Xml
# Prefix for internal only items: _Xml_
#------------------------------------------------------------------

# load the XML extension
@load "xml"

#------------------------------------------------------------------
# Auxiliary functions to update attributes
# (avoid unintended inconsistencies between $0 and XMLATTR)
#------------------------------------------------------------------
function XmlSetAttribute(name, value) {
   if (XMLEVENT=="STARTELEM") {  # only start tags have attributes
      XMLATTR[name] = value
      if ($0 !~ "\\<" name "\\>") {  # new attribute, add to the list
         $0 = $0 " " name
      }
   }
}

function XmlIgnoreAttribute(name) {
   if (XMLEVENT=="STARTELEM") {  # only start tags have attributes
      # Let XmlCopy() ignore it: remove its name from the list
      sub("\\<" name "\\>", "", $0) 
   }
}

#------------------------------------------------------------------
# XmlToken - reconstruct the XML code of the current token
#------------------------------------------------------------------

function XmlToken(        token, n, str) {

   switch (XMLEVENT) {

   case "STARTELEM":
      token = "<" XMLNAME
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
#      if ("ENCODING" in XMLATTR && tolower(XMLATTR["ENCODING"]) != tolower(XMLCHARSET)) {
#         XmlWriteError("XmlCopy warning: possible encoding mismatch,"\
#                       " declared <" XMLATTR["ENCODING"] "> actual <" XMLCHARSET ">")
#      }
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
   
   return token
}

#------------------------------------------------------------------
# XmlCopy - write the XML code of the current token
#------------------------------------------------------------------
function XmlCopy() {
   printf( "%s", XmlToken() )
}

