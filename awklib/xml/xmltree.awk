#------------------------------------------------------------------
# xmltree --- DOM-like content tree processing (read only)
#
# Author: Manuel Collado, http://lml.ls.fi.upm.es/~mcollado
# License: Public domain
# Updated: December, 2007
#
# RESPONSIBILITIES
# - Construct a tree representation of the input document
# - Search the tree for selected items
# - Pretty print selected nodes
# Restrictions:
# - Only one input document
# - Don't modify the stored document
#
# INTERFACE SUMMARY
# - Prefix for user seeable items:  Xml
# - Prefix for internal only items: _Xml_
#
# Functions that print nodes and/or subtrees:
#   XmlPrintElementStart( nodeindex )  prints '<tag attr="value" ...>'
#   XmlPrintElementEnd( nodeindex )  prints '</tag>'
#   XmlPrintNodeText( nodeindex )  prints _XML_[nodeindex, "text"]
#   XmlPrintNodeTree( nodeindex )  prints the whole subtree rooted at the node
#
# Functions that query the XML tree:
#   XmlGetNodes( rootnode, path, nodeset )
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
#   XmlGetValue( rootnode, path )
#    - returns the concatenation of text values inside the selected nodes
#       (for element nodes, just child and text nodes, but not attributes)
#------------------------------------------------------------------
#
# INTERNAL STRUCTURES
#
# The whole XML tree is stored in a single _XML_ array.
# - Each node has an integer index, starting from 1
# - The root element node has index 1
# - The XML document root is a special node with index 0
#
# A node can be:
# - an element
# - an attribute
# - a text content fragment
#
# Each node has a label:
# - element:   tagname
# - attribute: "@"attrname
# - text:      "#text"
#
# The path from some root node to a descendant node is denoted as
#   /label/label/...
#
# For each node, the following items are stored:
#   _XML_[nodeindex]   is the node's label
#   _XML_[nodeindex, "attrs"]   is the number of attributes
#   _XML_[nodeindex, "attr", n]   is the node index of the n-th attribute
#   _XML_[nodeindex, "children"]   is the number of children nodes
#   _XML_[nodeindex, "child", n]   is the node index of the n-th child
#   _XML_[nodeindex, "parent"]   is the node index of the parent node
#   _XML_[nodeindex, "text"]  is the text content of a CDATA/attribute node
#   _XML_["last"]   is the highest assigned node index, so far
#
# Internal variables
#   _Xml_data:  collected character data
#   _Xml_node:  temporary node index
#   _Xml_i:  loop control variable
#   _Xml_CurrentNode:  current node index
#   _Xml_Level:  nesting level of the current node
#   _Xml_OpenStack[]:  array of currently open elements
#------------------------------------------------------------------
#
# TODO:
# - Suppress local arguments in public functions
# - Use local arguments instead of global temporary variables
#   in private _Xml_ functions
# - Check global temporary variables usage
# - Avoid global temporary variables. Instead, use auxiliary
#   local functions with local variables
# - Make _Xml_Path visible
# - Use nodeindex+k instead of _XML_[nodeindex, "attr", k]  (?)
#------------------------------------------------------------------

# load the XML extension
@load xml

# use the xmlwrite library
@include xmlwrite


#------------------------------------------------------------------
#     Error checking and reporting functions:
#------------------------------------------------------------------

# XMLgawk error reporting needs some redesign.
# Interim code: uses both ERRNO and XMLERROR to generate consistent messages
function XmlCheckError () {
   if (XMLERROR) {
      printf("\n%s:%d:%d:(%d) %s\n", FILENAME, XMLROW, XMLCOL, XMLLEN, XMLERROR)
   } else if (ERRNO) {
      printf("\n%s\n", ERRNO)
      ERRNO = ""
   }
}

#------------------------------------------------------------------
# Internal: _Xml_trim: remove leading and trailing [[:space:]]
#    characters, and collapse repeated spaces into a single one
function _Xml_trim( string ) {
   sub(/^[[:space:]]+/, "", string)
   if (string) sub( /[[:space:]]+$/, "", string )
   if (string) gsub( /[[:space:]]+/, " ", string )
   return string
}


#------------------------------------------------------------------
#     Functions that print nodes and/or subtrees:
#------------------------------------------------------------------

# Internal: print unclosed '<tag attr="value" ...
function _Xml_PrintElementStart( node,       k, attr ) {
   xwstarttag( _XML_[node]  )
   for (k=1; k<=_XML_[node, "attrs"]; k++) {
      attr = _XML_[node, "attr", k]
      _Xml_PrintAttribute( attr )
   }
}

#------------------------------------------------------------------
# Internal: print ' name="value"'
function _Xml_PrintAttribute( node ) {
   xwattrib( substr(_XML_[node], 2), _XML_[node, "text"] )
}

#------------------------------------------------------------------
# Internal: print just the text ( with '<', '>','&' quoted )
function _Xml_PrintNodeText( node ) {
   xwtext( _XML_[node, "text"] )
}


#------------------------------------------------------------------
# XmlPrintElementStart( nodeindex )  prints '<tag attr="value" ...>'
#------------------------------------------------------------------
function XmlPrintElementStart( node ) {
   _Xml_PrintElementStart( node )
}

#------------------------------------------------------------------
# XmlPrintElementText( nodeindex )  prints the node's text, indented
#------------------------------------------------------------------
function XmlPrintNodeText( node ) {
   _Xml_PrintNodeText( node )
}

#------------------------------------------------------------------
# XmlPrintElementEnd( nodeindex )  prints '</tag>'
#------------------------------------------------------------------
function XmlPrintElementEnd( node ) {
   xwendtag( _XML_[node] )
}

#------------------------------------------------------------------
# XmlPrintNodeTree( nodeindex )  prints the whole subtree rooted at the node
#------------------------------------------------------------------
function XmlPrintNodeTree( node,         k ) {
   if (_XML_[node] == "#text") {                  # text node
      XmlPrintNodeText( node )
   } else if (substr(_XML_[node],1,1)=="@") {     # attribute node
      _Xml_PrintAttribute( node )
   } else {                                      # element
      XmlPrintElementStart( node )
      for (k=1; k<=_XML_[node, "children"]; k++) {
         XmlPrintNodeTree( _XML_[node, "child", k] )
      }
      XmlPrintElementEnd( node )
   }
}


#------------------------------------------------------------------
#     Auxiliary functions to query the XML tree:
#------------------------------------------------------------------

# Internal: adapt the pattern to the whole string
function _Xml_FixPathPattern( pattern ) {
   if (substr(pattern,1,1)=="^") {               # xs_trim leading ^
      pattern = substr( pattern, 2 )
   }
   if (substr(pattern,length(pattern),1)=="$") { # xs_trim trailing $
      pattern = substr( pattern, 1, length(pattern)-1 )
   }
#   if (substr(pattern,1,1)!="/") {               # force leading /
#      pattern = "/" pattern
#   }
   if (substr(pattern,length(pattern),1)=="/") { # xs_trim trailing /
      pattern = substr( pattern, 1, length(pattern)-1 )
   }
   return "^" pattern "$"                        # force full match
}

#------------------------------------------------------------------
# Internal: compute the path from one node to a descendant
function _Xml_Path( rootnode, childnode,     parent, path ) {
   if (childnode <= 0) {
      return ""
   } else if ((parent = _XML_[childnode, "parent"]) == rootnode) {
      return "/" _XML_[childnode]
   } else {
      path = _Xml_Path( rootnode, parent )
      if (path) {
         return path "/" _XML_[childnode]
      } else {
         return ""
      }
   }
}

#------------------------------------------------------------------
# Internal: retrieve the nodeset that matches a path
function _Xml_Select( from, pathpattern, nodeset,     k, path ) {
   for (k=from+1; path=_Xml_Path(from, k); k++) {
      if (match(path, pathpattern)) {
         nodeset[k] = k
      }
   }
}

#------------------------------------------------------------------
# Internal: check if the text content contains a text pattern
function _Xml_Check( from, pathpattern, valpattern,    k, path ) {
   if (pathpattern=="^$") {
      return valpattern=="" || match(Xml_CollectValue(from), valpattern)
   } else {
      for (k=from+1; path=_Xml_Path(from, k); k++) {
         if (match(path, pathpattern)) {
            if (valpattern=="" || match(Xml_CollectValue(k), valpattern)) {
               return 1
            }
         }
      }
   }
   return 0
}

#------------------------------------------------------------------
#  XmlGetNodes: get nodes that match the given criteria
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
#    - a null path-pattern keeps the selection
#------------------------------------------------------------------
function XmlGetNodes( rootnode, path, nodeset ) {
   # build the initial nodeset
   delete nodeset
   Xml_found = 0
   if (rootnode in _XML_) {
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
      Xml_selector[Xml_step] = _Xml_FixPathPattern( Xml_selseq[Xml_k] )
      Xml_j = index( Xml_selseq[Xml_k+1], "/?" )
      if (Xml_j) {
         Xml_condsel[Xml_step] = _Xml_FixPathPattern( substr( Xml_selseq[Xml_k+1], 1, Xml_j-1 ) )
         Xml_condval[Xml_step] = substr( Xml_selseq[Xml_k+1], Xml_j+2 )
      } else {
         Xml_condsel[Xml_step] = _Xml_FixPathPattern( Xml_selseq[Xml_k+1] )
      }
      Xml_step++
   }

   # Loop over the selection steps
   for (Xml_step=1; Xml_step in Xml_selector; Xml_step++) {

      if (Xml_selector[Xml_step]!="^$") {   # apply pre-selection path

         # Save previous selection
         delete Xml_nodeset
         for (_Xml_node in nodeset) {
            Xml_nodeset[_Xml_node] = _Xml_node
         }
         # Empty pre-selection
         delete nodeset
         Xml_found = 0

         # Loop over previous selection
         for (_Xml_node in Xml_nodeset) {
            _Xml_Select( _Xml_node, Xml_selector[Xml_step], nodeset )
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
         for (_Xml_node in Xml_nodeset) {
            if (_Xml_Check( _Xml_node, Xml_condsel[Xml_step], Xml_condval[Xml_step] )) {
               nodeset[_Xml_node] = _Xml_node+0    # force numeric, for later sorting
            }
         }
      }

   }

   return asort( nodeset )
}

#------------------------------------------------------------------
#  Xml_CollectValue: recursively collect the text content of a node
#------------------------------------------------------------------
function Xml_CollectValue( node, yetcollected,      k, child, value ) {
   value = ""
   if (!(node in yetcollected)) {
      yetcollected[node] = node
      if (_XML_[node, "children"]) {
         for (k=1; k<=_XML_[node, "children"]; k++) {
            value = value Xml_CollectValue( _XML_[node, "child", k], yetcollected )
         }
      } else {
         value = _XML_[node, "text"]
      }
   }
   return value
}

#------------------------------------------------------------------
#  Xml_CollectValue: collect the text content of a set of nodes
#
#   string = XmlGetValue( rootnode, path )
#    - returns the concatenation of text values inside the selected nodes
#      (for element nodes, just child and text nodes, but not attributes)
#------------------------------------------------------------------
function XmlGetValue( rootnode, path ) {
   Xml_n = XmlGetNodes( rootnode, path, Xml_nodeset2 )
   Xml_value = ""
   delete Xml_nodeset    # nodes collected so far
   for (Xml_j=1; Xml_j<=Xml_n; Xml_j++) {
      Xml_value = Xml_value Xml_CollectValue( Xml_nodeset2[Xml_j], Xml_nodeset )
   }
   return Xml_value
}


#------------------------------------------------------------------
#     Auxiliary functions that create the XML tree
#------------------------------------------------------------------

function Xml_NewChild( node,     child ) {       # Add a new child node
   child = ++_XML_["last"]
   _XML_[node, "child", ++_XML_[node, "children"]] = child
   _XML_[child, "parent"] = node
   return child
}

function Xml_NewAttribute( node, name, value,     attr ) {  # Add a new atribute node
   attr = ++_XML_["last"]
   _XML_[node, "attr", ++_XML_[node, "attrs"]] = attr
   _XML_[attr, "parent"] = node
   _XML_[attr] = "@" name
   _XML_[attr, "text"] = value
}


#------------------------------------------------------------------
#     Store the XML tree in the _XML_ array
#------------------------------------------------------------------

BEGIN {
   _XML_["last"] = 0
   _Xml_CurrentNode = 0
   _XML_[0] = ""
   XMLMODE = 1
}

# Check for previous errors (must be the first clause!)
ERRNO {
   XmlCheckError()
}

# Process input tokens
XMLEVENT {
   switch (XMLEVENT) {

   case "STARTELEM":
   case "ENDELEM":
      # Element tag: store the preceding text fragment
      _Xml_data = _Xml_trim( _Xml_data )
      if (_Xml_data) {
         _Xml_node = Xml_NewChild( _Xml_CurrentNode )
         _XML_[_Xml_node] = "#text"
         _XML_[_Xml_node, "text"] = _Xml_data
      }
      _Xml_data = ""
      if (XMLEVENT=="STARTELEM") {
         # Element start: store the new node as a child of the currente node
         _Xml_node = Xml_NewChild( _Xml_CurrentNode )
         _XML_[_Xml_node] = XMLNAME
         for (_Xml_i =1; _Xml_i <= NF; _Xml_i++) {
            Xml_NewAttribute( _Xml_node, $_Xml_i, XMLATTR[$_Xml_i] )
         }
         _Xml_CurrentNode = _Xml_node
         _Xml_OpenStack[++_Xml_Level] = _Xml_node
      } else {
         # Element end: pop the nested node stack
         _Xml_Level--
         _Xml_CurrentNode = _Xml_OpenStack[_Xml_Level]
      }
      break

   case "CHARDATA":
      # Character data: concatenate contiguous text fragments
      _Xml_data = _Xml_data $0
      break

   case "DECLARATION":
   case "STARTDOCT":
   case "ENDDOCT":
   case "PROCINST":
   case "STARTCDATA":
   case "ENDCDATA":
   case "COMMENT":
   case "UNPARSED":
   case "ENDDOCUMENT":
      # skip
      break

   default:
      # internal error
      ERRNO = "Unrecognized XML event <" XMLEVENT ">"
      XmlCheckError()
   }
}

END {
   # report error, if any
   XmlCheckError()
   # tree processing begins at END
   xwopen( "/dev/stdout" )  # default output
}

