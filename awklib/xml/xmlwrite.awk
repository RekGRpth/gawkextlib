#------------------------------------------------------------------
# xmlwrite --- writes a XML document serially, piece by piece
#              (takes care of some subtleties of the XML stardard)
#
# Author: Manuel Collado, http://lml.ls.fi.upm.es/~mcollado
# License: Public domain
# Updated: December, 2007
#
# RESPONSIBILITIES
# - Generate and write meaningful markup fragments
# - Help in achieving well-formedness
# - Indent markup as requested
# - Allow rewriting the last printed token (** TODO **)
# - Reformat/preserve whitespace as requested (** TODO **)
# - Support multiple simultaneous output documents (** TODO **)
# - Support encoding conversion on output (** TODO **)
#
# RESTRICTIONS (for this draft version)
# - No encoding conversion on output. The user must ensure that
#   the declared or assumed encoding of the output document matches
#   the internal encoding of the awk strings.
# - Only one output document at a time. Before switching to a
#   new output document the current one must be finished.
#
# INTERFACE SUMMARY
# - Prefix for user seeable items:  xw
# - Prefix for internal only items: _xw
#
# Document handling:
#   xwopen( filename, options )
#   xwclose( filename )
#   xwto( filename )
#
# Declarations and processing instructions:
#   xwdeclaration( version, encoding, standalone )
#   xwstartdoct( root, pubid, sysid )
#   xwenddoct( )
#   xwprocinst( name, string )
#
# Comments:
#   xwcomment( comment )
#
# Elements:
#   xwstarttag( name )
#   xwattrib( name, value )
#   xwendtag( name )
#
# Character data:
#   xwtext( string )
#
# CDATA sections
#   xwstartcdata( )
#   xwendcdata( )
#
# Raw markup
#   xwunparsed( string )
#
# Additional convenience functions
#   xwcopy() (** TODO **)
#   xwrewrite() (** TODO **)
#   xwcdata( string )
#   xwdoctype( root, pubid, sysid, declarations )
#   xwstyle( type, uri )
#
# Internal variables
#   _xwtype: type of the last item:
#      <<   - <tag>
#      >>   - </tag>
#      <!   - <!DOCTYPE
#      !>   - end DOCTYPE
#      <>   - <?..?>, <!--..-->
#      ##   - text, <[CDATA[, ]]>, unparsed
#      null - begin/end of document
#   _xwname: tag name of the last item
#   _xwlevel: nesting level of the current item
#   _xwmargin: array[level] of indent margins
#   _xwindent: indent step (-1 = no indent at all = no newlines)
#   _xwfile: output file name
#   _xwritten: output file has data
#   _xwquot: preferred quote ( " or ' )
#   _xwincdata: true if inside a CDATA section
#   _xwindoctype: true if inside a DOCTYPE declaration
#   _xwinternal: true if DOCTYPE has internal declarations
#   _xwQUOTE: named entities for quotes
#------------------------------------------------------------------

#------------------------------------------------------------------
#  Initial settings
#------------------------------------------------------------------
BEGIN {
   # internal: names of entities for quote characters
   _xwQUOTE["\""] = "&quot;"
   _xwQUOTE["'"] = "&apos;"
   # default output = stdout with default options
   xwopen( "/dev/stdout" )
}

#------------------------------------------------------------------
#  internal functions
#------------------------------------------------------------------

#   _xwput: write the given textual item (apply indenting rules)
function _xwput( type, name, string ) {
##printf( "--_xwput %s %s %s %s\n", type, name, string, _xwfile )
   # fix unfinished start tag
   if (_xwtype == "<<") printf( ">") > _xwfile
   # compute level of the new item
   if (_xwtype == "<<") {       # old open tag, indent
      _xwlevel++
      _xwmargin[_xwlevel] = _xwmargin[_xwlevel-1] + _xwindent
   }
   if (type == ">>") {           # new close tag, outdent
         _xwlevel--
      }
   # indent if appropriate
   if (_xwdoindent( _xwtype, type )) {
      printf( "\n%*s", _xwmargin[_xwlevel], "" ) > _xwfile
   }
   # write the new item
#   if (type == "<<") sub( />$/, "", string ) # keep starttag open
   printf( "%s", string ) > _xwfile
   # record the item class
   _xwtype = type
   _xwname = name
   _xwritten = 1
}

#------------------------------------------------------------------
#   _xwdoindent: decide to apply or not the indenting rules
function _xwdoindent( old, new ) {
   if ( _xwindent < 0 ||
       old ~ /<<|##/ && new == "##" ||
       old == "##" && new ~ />>|##/ ||
       old == "<!" && new == "!>" ||
       ! old) {
      return 0
   } else {
      return 1
   }
}

#------------------------------------------------------------------
#  _xwchrefs: encode characters in the given range as references
function _xwchrefs( string, from, to,    c ) {
   for (c=from; c<=to; c++) {
      gsub( sprintf("\\%o", c), "\\" _xwchref(c), string )
      #                          ^^
      # NOTE: escape leading & in replacement expression !
   }
   return string
}

#------------------------------------------------------------------
#  _xwchref: character reference for a given codepoint '&#nnn;'
function _xwchref( code ) {
   return sprintf("&#%d;", code)
}

#------------------------------------------------------------------
#  xs_escape: encode metacharacters '<' and '&' in character data,
#     escape also '>' in "]]>" (see XML-1.0 sect. 2.4).
#     CHANGE: escape all '>' (not only in "]]>")
function _xwescape( string ) {
   gsub( /&/, "\\&amp;", string )  # this must be the first
   gsub( /</, "\\&lt;", string )
#   gsub( /]]>/, "]]\\&gt;", string )
   gsub( />/, "\\&gt;", string )
   return string
}

#------------------------------------------------------------------
#  _xwquote: for attribute values - escape '<', '&' and quotes,
#     encode control characters as references (see XML-1.0 sect. 3.3.3),
#     optionally encode non-ASCII characters as references, and
#     put quotes around the resulting value
function _xwquote( string ) {
   string = _xwescape( string )
   gsub( _xwquot, "\\" _xwQUOTE[_xwquot], string )
   #               ^^
   # NOTE: escape leading & in replacement expression !
   if (string ~ /[:cntrl:]/) {
      string = _xwchrefs( string, 0, 31 )
   }
   return _xwquot string _xwquot
}

#------------------------------------------------------------------
#  xs_attrib: generate string ' name="value"', with a leading space
function _xwattrib( name, value ) {
   return " " name "=" _xwquote(value)
}

#------------------------------------------------------------------
#  _xwoption: get an option value from an array of options
#     or a default value if missing
function _xwoption( options, name, defval ) {
   if (! name in options) {
      return defval
   } else {
      return options[name] ? options[name] : defval
   }
}

#------------------------------------------------------------------
#  xwopen: initialize output to the given file
#    options is an array of named options:
#    options["INDENT"] = indent step (-1 = no indent), default = 3
#    options["QUOTE"] = preferred quote, default = (")
#  Options not yet supported:
#    options["ENCODING"] = output encoding, default = UTF-8
#    options["PRESERVE"] = regexp: preserve space for these elements
#    options["WRAP"] = max line length, default = 0 = no wrap
#    options["COMPACTEMPTY"] = flag: empty elements as <tag .../>
#    options["ESCAPEGT"] = flag: escape ">" in character data
#------------------------------------------------------------------
function xwopen( filename, options ) {
   xwclose()
   _xwfile = filename
   _xwindent = _xwoption( options, "INDENT", 3)
   _xwlevel = 0
   _xwquot = options["QUOTE"]
   _xwquot = (quote == "'") ? quote : "\""    # default quote = "
   _xwmargin[_xwlevel] = 0
   delete _xwmargin
   _xwtype = ""
   _xwname = ""
   _xwincdata = ""
   _xwindoctype = ""
   _xwinternal = ""
   _xwritten = 0
}

#------------------------------------------------------------------
#  xwclose: end document
#------------------------------------------------------------------
function xwclose() {
   if (_xwfile && _xwritten) {
      if (_xwdoindent( _xwtype, "")) {
         printf( "\n" ) > _xwfile
      }
      close( _xwfile )
   }
   _xwfile = ""
}

#------------------------------------------------------------------
#  xwdecl: XML declaration
#------------------------------------------------------------------
function xwdecl( version, encoding, standalone ) {
   version = version ? version : "1.0"
   version = _xwattrib( "version", version )
   if (encoding) version = version _xwattrib( "encoding", encoding )
   if (standalone) version = version _xwattrib( "standalone", standalone )
   xwprocinst( "xml", version )
}

#------------------------------------------------------------------
#  xwstartdoctype: DOCTYPE declaration start
#------------------------------------------------------------------
function xwstartdoctype( root, pubid, sysid ) {
   has_internals = has_internals ? " [": ""
   if (pubid) {
       root = "<!DOCTYPE " root " PUBLIC \"" pubid "\" \"" sysid "\""
   } else if (sysid) {
       root = "<!DOCTYPE " root " SYSTEM \"" sysid "\""
   } else {  # well formed!, even if root alone (see XML-1.0 sect. 2.8)
       root = "<!DOCTYPE " root
   }
   _xwput( "<!", "!", root )
   _xwindoctype = 1
   _xwinternal = 0
}

#------------------------------------------------------------------
#  xwenddoctype: DOCTYPE declaration end
#------------------------------------------------------------------
function xwenddoctype( ) {
   if (_xwinternal) {
      _xwput( "!>", "!", "]>" )
   } else {
      _xwput( "!>", "!", ">" )
   }
   _xwindoctype = 0
   _xwinternal = 0
}

#------------------------------------------------------------------
#  xwdoctype: DOCTYPE declaration
#------------------------------------------------------------------
function xwdoctype( root, pubid, sysid, declarations ) {
   xwstartdoctype( root, pubid, sysid )
   if (declarations) {
      if (declarations) {
         xwunparsed( declarations )
      }
   }
   xwenddoctype()
}

#------------------------------------------------------------------
#  xwprocinst: processing instruction
#------------------------------------------------------------------
function xwprocinst( name, string ) {
   sub( /^[^ ]/, " &", string )  # force a space before the string
   _xwput( "<>", "?", "<?" name string "?>" )
}

#------------------------------------------------------------------
#  xwstyle: xml-stylesheet processing instruction
#------------------------------------------------------------------
function xwstyle( type, uri ) {
   xwprocinst( "xsl-stylesheet", _xwattrib("type", "text/" type) _xwattrib("href", uri) )
}

#------------------------------------------------------------------
#  xwcomment: XML comment
#------------------------------------------------------------------
function xwcomment( comment ) {
   _xwput( "<>", "--", "<!--" comment "-->" )
}

#------------------------------------------------------------------
#  xwstarttag: element start tag
#------------------------------------------------------------------
function xwstarttag( name ) {
   _xwput( "<<", name, "<" name )
}

#------------------------------------------------------------------
#  xwattrib: add attribute to the start tag
#------------------------------------------------------------------
function xwattrib( name, value ) {
   printf( "%s", _xwattrib(name, value) ) > _xwfile
}

#------------------------------------------------------------------
#  xwendtag: element end tag
#------------------------------------------------------------------
function xwendtag( name ) {
   if (_xwtype == "<<" && _xwname == name) {
      # empty element tag, collapse
      printf( "/>" ) > _xwfile
      _xwtype = ">>"
   } else{
      _xwput( ">>", name, "</" name ">" )
   }
}

#------------------------------------------------------------------
#  xwtext: character data
#------------------------------------------------------------------
function xwtext( string ) {
   if (_xwincdata) {
      _xwput( "##", "", string )
   } else {
      _xwput( "##", "", _xwescape( string ) )
   }
}


#------------------------------------------------------------------
#  xwstartcdata: CDATA section start tag
#------------------------------------------------------------------
function xwstartcdata( ) {
   _xwput( "##", "", "<![CDATA[" )
   _xwincdata = 1
}

#------------------------------------------------------------------
#  xwendcdata: CDATA section end tag
#------------------------------------------------------------------
function xwendcdata( ) {
   _xwput( "##", "", "]]>" )
   _xwincdata = 0
}

#------------------------------------------------------------------
#  xwcdata: CDATA section
#------------------------------------------------------------------
function xwcdata( string ) {
   xwstartcdata()
   _xwput( "##", "", string )
   xwendcdata()
}

#------------------------------------------------------------------
#  xwunparsed: raw markup
#------------------------------------------------------------------
function xwunparsed( string ) {
   if (_xwindoctype && !_xwinternal) {
      string = " [" string
      _xwinternal = 1
   }
   _xwput( "##", "", string )
}

#------------------------------------------------------------------
#  xwcopy: copy the current token (useful only while processing
#  XML input with the XML extension of gawk)
#------------------------------------------------------------------
function xwcopy( ) {
   switch (XMLEVENT) {
   case "DECLARATION":
      xwdecl( XMLATTR["VERSION"], XMLATTR["ENCODING"], XMLATTR["STANDALONE"] )
      break
   case "STARTDOCT":
      xwstartdoctype( XMLNAME, XMLATTR["PUBLIC"], XMLATTR["SYSTEM"] )
      break
   case "ENDDOCT":
      xwenddoctype( )
      break
   case "PROCINST":
      xwprocinst( XMLNAME, $0 )
      break
   case "STARTELEM":
      xwstarttag( XMLNAME )
      for (k=1; k<=NF; k++) {
         xwattrib( $k, XMLATTR[$k] )
      }
      break
   case "ENDELEM":
      xwendtag( XMLNAME )
      break
   case "CHARDATA":
      xwtext( $0 )
      break
   case "STARTCDATA":
      xwstartcdata( )
      break
   case "ENDCDATA":
      xwendcdata( )
      break
   case "COMMENT":
      xwcomment( $0 )
      break
   case "UNPARSED":
      xwunparsed( $0 )
      break
   case "ENDDOCUMENT":
      xwclose()
      break
   }
}

#------------------------------------------------------------------
#  Automatic closing
#------------------------------------------------------------------
END {
   xwclose()   # really ?
}
