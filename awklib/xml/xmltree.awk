# XML tree management library
#
# Author: Manuel Collado - http://lml.fi.upm.es/~mcollado
# Created: 2004-09-03
# Revised: 2006-02-06
#
# prefix for user seeable items:  Xml
# prefix for internal only items: Xml_
#

@load xml


############################################################
#
# TO DO:
# - Change XML_ -> Xml_    (?)
# - Suppress local arguments in public functions
# - Use local arguments instead of global temporary variables
#   in private Xml_ functions
# - Check global temporary variables usage
# - Avoid global temporary variables. Instead, use auxiliary
#   local functions with local variables
# - Make Xml_Path visible
# - Use nodeindex+k instead of XML_[nodeindex, "attr", k]  (?)

############################################################
#
# This library uses an internal XML_ array to emulate a storage pool.
# The whole XML tree is thus stored in a single XML_ array.
#
# Each node has an integer index, starting from 1.
# The root element node has index 1.
# The XML document root is a special node with index 0.
#
# A node can be:
# - an element
# - an attribute
# - a text content fragment (CDATA)
#
# Each node has a label:  - element:   tagname
#                         - attribute: "@"attrname
#                         - CDATA:     "#text"
#
# The path from some root node to a descendant node is denoted as
#   /label/label/...
#
# For each node, the following items are stored:
#
#   XML_[nodeindex]   is the node's label
#   XML_[nodeindex, "attrs"]   is the number of attributes
#   XML_[nodeindex, "attr", n]   is the node index of the n-th attribute
#   XML_[nodeindex, "children"]   is the number of children nodes
#   XML_[nodeindex, "child", n]   is the node index of the n-th child
#   XML_[nodeindex, "parent"]   is the node index of the parent node
#   XML_[nodeindex, "text"]  is the text content of a CDATA/attribute node
#   XML_["last"]   is the highest assigned node index, so far
#
# Functions that print nodes and/or subtrees:
#
#   XmlPrintElementStart( nodeindex )  prints '<tag attr="value" ...>'
#   XmlPrintElementEnd( nodeindex )  prints '</tag>'
#   XmlPrintNodeText( nodeindex )  prints XML_[nodeindex, "text"]
#   XmlPrintNodeTree( nodeindex )  prints the whole subtree rooted at the node
#
# Functions that query the XML tree:
#
#   n = XmlGetNodes( rootnode, path, nodeset )
#    - returns the number of nodes in the subtree rooted at 'rootnode'
#       and selected by the given 'path', and stores their node indexes
#       in the 'nodeset' array
#    - 'path' is a string "selector!condition!selector!condition..."
#       - 'selector' is a path-pattern (regular expression) used to preselect
#          the resulting nodeset
#       - 'condition' is a path-pattern, optionally followed
#          by '/?'value-pattern (also a regular expression), used to
#          filter the preselected nodeset by testing for existence of
#          nodes/values inside the preselected nodes.
#    - the global selection is made in a series of steps (selector!condition pair).
#       At each step a new nodeset is generated from the previous one by selecting
#       all the nodes that match the 'selector' and that also meet the 'condition'
#
#   string = XmlGetValue( rootnode, path )
#    - returns the concatenation of text values inside the selected nodes
#       (for element nodes, just child and CDATA nodes, but not attributes)


############################################################
#
#     Basic string functions:
#
############################################################

# remove leading and trailing [[:space:]] characters
function trim(str)
{
    sub(/^[[:space:]]+/, "", str)
    if (str) sub(/[[:space:]]+$/, "", str)
    return str
}

# Escape metacharacters in text data
function quote_text( str )
{
    gsub(/&/, "\\&amp;", str) # this must be the first
    gsub(/</, "\\&lt;", str)
    gsub(/>/, "\\&gt;", str)
    return str
}

# Escape metacharacters and quotes in attribute values
function quote_attrib(str)
{
    str = quote_text(str)
    gsub(/"/, "\\&quot;", str)
    gsub(/'/, "\\&apos;", str)
    return str
}


############################################################
#
#     Functions that print nodes and/or subtrees:
#
############################################################

# print unclosed '<tag attr="value" ...
function Xml_PrintElementStart( node, indent,      k, attr ) {
   printf( "%*s<%s", indent, "", XML_[node] )
   for (k=1; k<=XML_[node, "attrs"]; k++) {
      attr = XML_[node, "attr", k]
      Xml_PrintAttribute( attr )
   }
}

# print ' name="value"'
function Xml_PrintAttribute( node ) {
   printf( " %s=\"%s\"", substr(XML_[node], 2), quote_attrib( XML_[node, "text"] ) )
}

# print just the text ( with <,& quoted )
function Xml_PrintNodeText( node ) {
   printf( "%s", quote_text( XML_[node, "text"] ) )
}

# XmlPrintElementStart( nodeindex )  prints '<tag attr="value" ...>'
function XmlPrintElementStart( node, indent ) {
   Xml_PrintElementStart( node, indent )
   print ">"
}

# XmlPrintElementText( nodeindex )  prints the node's text, indented
function XmlPrintNodeText( node, indent ) {
   printf( "%*s", indent, "" )
   Xml_PrintNodeText( node )
}

# XmlPrintElementEnd( nodeindex )  prints '</tag>'
function XmlPrintElementEnd( node, indent ) {
   printf( "%*s", indent, "" )
   print "</" XML_[node] ">"
}

# XmlPrintNodeTree( nodeindex )  prints the whole subtree rooted at the node
function XmlPrintNodeTree( node, step, indent,        k ) {
   if (XML_[node] == "#text") {                  # CDATA node
      XmlPrintNodeText( node, indent )
   } else if (substr(XML_[node],1,1)=="@") {     # attribute node
      printf( "%*s", indent, "" )
      Xml_PrintAttribute( node )
      print ""
   } else if (!XML_[node, "children"]) {         # empty element
      Xml_PrintElementStart( node, indent )
      print "/>"
   } else if (XML_[node, "children"]==1 && XML_[(k=XML_[node, "child", 1])]=="#text") { # element with single text
      Xml_PrintElementStart( node, indent )
      printf( ">" )
      Xml_PrintNodeText( k )
      XmlPrintElementEnd( node )
   } else {                                      # composed element
      XmlPrintElementStart( node, indent )
      for (k=1; k<=XML_[node, "children"]; k++) {
         XmlPrintNodeTree( XML_[node, "child", k], step, indent+step )
      }
      XmlPrintElementEnd( node, indent )
   }
}


############################################################
#
#     Functions that query the XML tree:
#
############################################################

function Xml_FixPathPattern( pattern ) {
   if (substr(pattern,1,1)=="^") {               # trimm leading ^
      pattern = substr( pattern, 2 )
   }
   if (substr(pattern,length(pattern),1)=="$") { # trimm trailing $
      pattern = substr( pattern, 1, length(pattern)-1 )
   }
#   if (substr(pattern,1,1)!="/") {               # force leading /
#      pattern = "/" pattern
#   }
   if (substr(pattern,length(pattern),1)=="/") { # trimm trailing /
      pattern = substr( pattern, 1, length(pattern)-1 )
   }
   return "^" pattern "$"                        # force full match
}

function Xml_Path( rootnode, childnode,     parent, path ) {
   if (childnode <= 0) {
      return ""
   } else if ((parent = XML_[childnode, "parent"]) == rootnode) {
      return "/" XML_[childnode]
   } else {
      path = Xml_Path( rootnode, parent )
      if (path) {
         return path "/" XML_[childnode]
      } else {
         return ""
      }
   }
}

function Xml_Select( from, pathpattern, nodeset,     k, path ) {
   for (k=from+1; path=Xml_Path(from, k); k++) {
      if (match(path, pathpattern)) {
         nodeset[k] = k
      }
   }
}

function Xml_Check( from, pathpattern, valpattern,    k, path ) {
   if (pathpattern=="^$") {
      return valpattern=="" || match(Xml_CollectValue(from), valpattern)
   } else {
      for (k=from+1; path=Xml_Path(from, k); k++) {
         if (match(path, pathpattern)) {
            if (valpattern=="" || match(Xml_CollectValue(k), valpattern)) {
               return 1
            }
         }
      }
   }
   return 0
}

#   n = XmlGetNodes( rootnode, path, nodeset )
#    - returns the number of nodes in the subtree rooted at 'rootnode'
#       and selected by the given 'path', and stores their node indexes
#       in the 'nodeset' array
#    - 'path' is a string "selector!condition!selector!condition..."
#       - 'selector' is a path-pattern (regular expression) used to preselect
#          the resulting nodeset
#       - 'condition' is a path-pattern, optionally followed
#          by '/?'value-pattern (also a regular expression), used to
#          filter the preselected nodeset by testing for existence of
#          nodes/values inside the preselected nodes.
#    - the global selection is made in a series of steps (selector!condition pair).
#       At each step a new nodeset is generated from the previous one by selecting
#       all the nodes that match the 'selector' and that also meet the 'condition'
#    - a null path-pattern keeps the selection
function XmlGetNodes( rootnode, path, nodeset ) {
#print root "=>" path
   # build the initial nodeset
   delete nodeset
   Xml_found = 0
   if (rootnode in XML_) {
      nodeset[rootnode] = rootnode
      Xml_found = 1
   } else {
      return 0
   }

   # Decompose the path in steps
   split( path, Xml_selseq, "!" )
   delete Xml_selector
   delete Xml_condsel
   delete Xml_condval
   Xml_step=1
   for (Xml_k=1; Xml_k in Xml_selseq; Xml_k+=2) {
      Xml_selector[Xml_step] = Xml_FixPathPattern( Xml_selseq[Xml_k] )
      Xml_j = index( Xml_selseq[Xml_k+1], "/?" )
      if (Xml_j) {
         Xml_condsel[Xml_step] = Xml_FixPathPattern( substr( Xml_selseq[Xml_k+1], 1, Xml_j-1 ) )
         Xml_condval[Xml_step] = substr( Xml_selseq[Xml_k+1], Xml_j+2 )
      } else {
         Xml_condsel[Xml_step] = Xml_FixPathPattern( Xml_selseq[Xml_k+1] )
      }
      Xml_step++
   }

   # Loop over the selection steps
   for (Xml_step=1; Xml_step in Xml_selector; Xml_step++) {

      if (Xml_selector[Xml_step]!="^$") {   # apply pre-selection path

         # Save previous selection
         delete Xml_nodeset
         for (Xml_node in nodeset) {
            Xml_nodeset[Xml_node] = Xml_node
         }
         # Empty pre-selection
         delete nodeset
         Xml_found = 0

         # Loop over previous selection
         for (Xml_node in Xml_nodeset) {
            Xml_Select( Xml_node, Xml_selector[Xml_step], nodeset )
         }
      }

      if (Xml_condsel[Xml_step]!="^$" || Xml_condval[Xml_step]) {   # apply condition
         # Save pre-selection
         delete Xml_nodeset
         for (Xml_k in nodeset) Xml_nodeset[Xml_k] = Xml_k

         # Empty next selection
         delete nodeset
         Xml_found = 0

         # Loop over pre-selection
         for (Xml_node in Xml_nodeset) {
            if (Xml_Check( Xml_node, Xml_condsel[Xml_step], Xml_condval[Xml_step] )) {
               nodeset[Xml_node] = Xml_node+0    # force numeric, for later sorting
            }
         }
      }

   }

   return asort( nodeset )
}

function Xml_CollectValue( node, yetcollected,      k, child, value ) {
   value = ""
   if (!(node in yetcollected)) {
      yetcollected[node] = node
      if (XML_[node, "children"]) {
         for (k=1; k<=XML_[node, "children"]; k++) {
            value = value Xml_CollectValue( XML_[node, "child", k], yetcollected )
         }
      } else {
         value = XML_[node, "text"]
      }
   }
   return value
}

#   string = XmlGetValue( rootnode, path )
#    - returns the concatenation of text values inside the selected nodes
#       (for element nodes, just child and CDATA nodes, but not attributes)
function XmlGetValue( rootnode, path ) {
   Xml_n = XmlGetNodes( rootnode, path, Xml_nodeset2 )
   Xml_value = ""
   delete Xml_nodeset    # nodes collected so far
   for (Xml_j=1; Xml_j<=Xml_n; Xml_j++) {
      Xml_value = Xml_value Xml_CollectValue( Xml_nodeset2[Xml_j], Xml_nodeset )
   }
   return Xml_value
}


############################################################
#
#     Auxiliary functions that create the XML tree
#
############################################################

function Xml_NewChild( node,     child ) {       # Add a new child node
#print "Xml_NewChild(" node ")"
   child = ++XML_["last"]
   XML_[node, "child", ++XML_[node, "children"]] = child
   XML_[child, "parent"] = node
   return child
}

function Xml_NewAttribute( node, name, value,     attr ) {  # Add a new atribute node
#print "Xml_NewAttribute(" node ", " name ", " value ")"
   attr = ++XML_["last"]
   XML_[node, "attr", ++XML_[node, "attrs"]] = attr
   XML_[attr, "parent"] = node
   XML_[attr] = "@" name
   XML_[attr, "text"] = value
}


############################################################
#
#     Store the XML tree in the XML_ array
#
############################################################

BEGIN {
   XML_["last"] = 0
   Xml_CurrentNode = 0
   XML_[0] = ""
   XMLMODE = 1
}

# Character data: concatenate contiguous text fragments
XMLCHARDATA {
   Xml_data = Xml_data $0
}

# Element tag: store the preceding text fragment
XMLSTARTELEM || XMLENDELEM {
   Xml_data = trim( Xml_data )
   if (Xml_data) {
      Xml_node = Xml_NewChild( Xml_CurrentNode )
      XML_[Xml_node] = "#text"
      XML_[Xml_node, "text"] = Xml_data
   }
   Xml_data = ""
}

# Element start: store the new node as a child of the currente node
XMLSTARTELEM {
   Xml_node = Xml_NewChild( Xml_CurrentNode )
   XML_[Xml_node] = XMLSTARTELEM
   for (Xml_i =1; Xml_i <= NF; Xml_i++) {
      Xml_NewAttribute( Xml_node, $Xml_i, XMLATTR[$Xml_i] )
   }
   Xml_CurrentNode = Xml_node
   Xml_OpenStack[++Xml_Level] = Xml_node
}

# Element end: pop the nested node stack
XMLENDELEM {
   Xml_Level--
   Xml_CurrentNode = Xml_OpenStack[Xml_Level]
}

# Report error, if any
END {
   if (XMLERROR) {
      printf("\n%s:%d:%d:(%d) %s\n", FILENAME, XMLROW, XMLCOL, XMLLEN, ERRNO)
      exit  # Really ?
   }
}

