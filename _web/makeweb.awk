# Compose the web page of an individual extension
# and populate the web directory with the required files
#
# Input: The web page template
# + webdir = the web directory of the extension: <root>/_web/<extension>
# + inputdir = the source directory with doc files
# + webtoc = the webTOC file specification file
#
# Global variables:
# - webpage: the top page of the extension web <webdir>/<extension>.html
# - mode: text processing mode = HTML tag (<ul>, <p>, <pre>, ...)
# - name: dir name of the extension (short name), i.e. .../name/
# - package: package name, usually "gawk-name"
# - summary: one-line description like "xxx extension for gawk"
# - version: version number, usually x.x.x
# - license: license name or acronym, usually "GPLv3+"
# - description: multi-line one paragraph description
# - license_file: document text of the license, usually "COPYING"
# - require_gawk: required gawk version
# - require_api: required gawk API version
# - require_gawkextlib: requires gawkextlib
# - require_extra: required extra packages
# - errormessage: array or messsage templates
# - errmsg: multi-line error summary
# - linkre: URL matcher regex

# Error processing file
function filerror( code, file ) {
    if (name != ".") {   # ignore errors for the home page
        errmsg = errmsg sprintf(errormessage[code], file) "\n"
    }
}

# Output to webpage file
function webprint( line ) {
    print line > webpage
}

# External command: copy file to web
function copy_file( aux_file ,            rc ) {
    rc = system("cp " aux_file " " webdir)
    if (rc) filerror("copy", aux_file)
    return rc
}

# External command: convert file to web
function convert_file( doc_file ,            rc ) {
    rc = system("_web/makeweb-any.sh " doc_file " " webdir)
    sub(/\.md$/, "[.md]", doc_file)      # may be extra .md
    if (rc==2) filerror("notfound", doc_file)
    else if (rc) filerror("convert", doc_file)
    return rc
}

# Include text from file
function include_text( txt_file,      copy ) {
    copy = -1
    while (copy && (getline line < txt_file) > 0) {
        copy = 1
        if (line ~ /^[^[:space:]]/) {        # non indented, <p> mode
            set_mode("p")
        } else if (line ~ /[^[:space:]]/) {  # indented, <pre> mode
            for (; space>0; space--) webprint("")
            set_mode("pre")
        } else if (mode=="pre") {            # blank line in <pre> mode
            space++
        } else if (mode=="") {               # blank line in "" mode
            copy = 0
        } else {                             # blank line
            set_mode("")
        }
        if (!space) {
            gsub(/&/, "\\&amp;", line) # this must be the first
            gsub(/</, "\\&lt;", line)
            gsub(/>/, "\\&gt;", line)
            gsub(linkre, "<a href=\"&\">&</a>", line)  # URLs as links
            if (mode != "pre" && line ~ /^[[:space:]]*[-=]+[[:space:]]*$/) {
                # do not reflow
                line = "<br />" line "<br />"
            }
            webprint(line)
        }
    }
    if (copy==-1) filerror("empty", txt_file)
    return copy
}

# Parse the configure.ac file
function parse_configure( config_ac,          line, f, done ) {
    done = 0
    while ((getline line < config_ac) > 0) {
        done = 1
        if (line ~ /^AC_INIT/) {
            split(line, f, /,[[:space:]]*/)
            version = f[2]
            package = gensub(/[[:space:]]*\)[[:space:]]*/, "", 1, f[4])
            break
        }
    }
    if (!done) filerror("empty", config_ac)
    return done
}

# Parse the package.spec.in file
function parse_spec( spec_in,            line, fd, done ) {
    done = -1
    while ((getline line < spec_in) > 0) {
        done = 0
        line = trim(line)  # discard leading and trailing space
        if (line ~ /^Summary:/) {
            summary = gensub(/Summary:[[:space:]]*/, "", 1, line)
        } else if (line ~ /^License:/) {
            license = gensub(/License:[[:space:]]*/, "", 1, line)
        } else if (line ~ /^%description/) {
            while ((getline line < spec_in) > 0) {
                if (line ~ /^[[:space:]]*$/) break
                gsub(/%\{name\}/, package, line)
                description = description line "\n"
            }
        } else if (line ~ /^%license/) {
            license_file = gensub(/%license[[:space:]]*/, "", 1, line)
            copy_file(inputdir "/" license_file)
            license_file = base_name(license_file)
        } else if (line ~ /^BuildRequires:/) {
            split( line, fd)
            if (fd[2]=="gawk-devel" && fd[3]==">=") {
                require_gawk = fd[4]
            } else if (fd[2]=="gawk(abi)" && fd[3]==">=") {
                require_api = fd[4]
            } else if (fd[2]=="gawkextlib-devel") {
                require_gawkextlib = "gawkextlib"
            } else if (fd[2]~"-devel$" && fd[2]!~"gawk") {
                require_extra = require_extra ", " substr(fd[2], 1, length(fd[2])-6)
            }
        }
    }
    if (done) filerror("empty", spec_in)
    return done
}

# Process the webTOC file (from second line)
function process_webTOC( tocfile,           line, f, fn ) {
    # First webTOC line already processed
    while ((getline line < tocfile) > 0) {
        line = trim(line)  # discard leading and trailing space
        if (line ~ "<-") {                      # insert text from file
            set_mode("")
            split(line, f, /[[:space:]]*<-[[:space:]]*/)
            include_text( inputdir "/" f[2] )
        } else if (line ~ "-->") {              # copy file
            split(line, f, /[[:space:]]*-->[[:space:]]*/)
            copy_file(inputdir "/" f[2])
        } else if (line ~ "\\*->") {            # link URL
            split(line, f, /[[:space:]]*\*->[[:space:]]*/)
            set_mode("ul")
            webprint("<li>" link_external(f[2], f[1]) "</li>")
        } else if (line ~ "->") {               # convert and link file
            split(line, f, /[[:space:]]*->[[:space:]]*/)
            convert_file(inputdir "/" f[2])
            fn = base_name(f[2])
            set_mode("ul")
            webprint("<li>" link(fn ".html", f[1]) " [" link(fn ".pdf", "PDF version") "]</li>")
        } else if (line !~ /^[[:space:]]*$/) {  # section header
            set_mode("")
            webprint("<h2>" line "</h2>")
        }                                       # blank line, ignored
    }
    set_mode("")
}

# Remove leading and trailing space
function trim( str ) {
    sub(/^[[:space:]]+/, "", str)
    sub( /[[:space:]]+$/, "", str )
    return str
}

# Base name of a file
function base_name( file ) {
    sub(/.*\//, "", file)  # remove leading path
    if (file !~ "\\.(1|3am|awk)") {
        sub(/\.[^.]*$/, "", file)  # remove trailing extension
    }
    return file
}

# HTML link tag
function link(url, text) {
    return sprintf("<a href=\"%s\">%s</a>", url, text)
}
function link_external(url, text) {
    return sprintf("<a target=\"_bank\" href=\"%s\">%s</a>", url, text)
}

# Set the text processing mode: <p>, <pre>, <ul> or none
function set_mode( newmode ) {
    if (newmode != mode) {
        # ignore space at end of <pre> blocks
        space = 0
        # close the current mode, if not null
        if (mode) webprint("</" mode ">")
        # record the new mode
        mode = newmode
        # open the new mode, if not null
        if (mode) webprint("<" mode ">")
    }
}

# Start processing the page template
BEGIN {
    errormessage["copy"] = "Error copying %s to the web directory"
    errormessage["empty"] = "File %s missing or empty"
    errormessage["notfound"] = "File %s not found"
    errormessage["convert"] = "Error converting %s to HTML/PDF"
    name = gensub(/.*\//, "", 1, webdir)  # last component of the webdir path
    webpage = webdir "/" name ".html"

    linkre = @/https?:\/\/[^[:space:]]*/  # naive http URL matcher

    parse_configure(inputdir "/configure.ac")
    parse_spec(inputdir "/packaging/" package ".spec.in")
}

# Package name as title
/@TITLE@/ {
    if (! package) {
        package = "gawk-" name
    }
    webprint(gensub(/@TITLE@/, package, 1, $0))  # package name
    next
}

# Top level header
/@HEADER@/ {
    getline header < webtoc        # fisrt line of the webTOC file
    if (!header) {
        if (summary) {
            header = package ": " summary             # from the spec file
        } else {
            header = package " extension of GNU awk"  # default header
        }
    }
    webprint(gensub(/@HEADER@/, header, 1, $0))  # page header
    next
}

# Version number from the spec file
/@VERSION@/ {
    if (version) {
        webprint(gensub(/@VERSION@/, version, 1, $0))  # package version
    }
    next
}

# License given from the spec file
/@LICENSE@/ {
    if (license) {
        webprint(gensub(/@LICENSE@/, link( license_file, license), 1, $0))  # package license
    }
    next
}

# Requirements from the spec file
/@REQUIRE@/ {
    if (require_gawk) require = ", gawk " require_gawk "+"
    if (require_api) require = require ", gawk API " require_api "+"
    if (require_gawkextlib) require = require ", " require_gawkextlib
    if (require_extra) require = require require_extra
    if (require) {
        webprint(gensub(/@REQUIRE@/, substr(require, 3), 1, $0))  # requirements
    }
    next
}

# Brief description from the spec file
/@SUMMARY@/ {
    if (description) {
        webprint(gensub(/@SUMMARY@/, description, 1, $0))  # package description
    }
    next
}

# Page body
/@BODY@/ {
    process_webTOC(webtoc)
    next
}

# Literal content
{
    webprint($0)
}

# Print error(s), if any
END {
    if (errmsg) {
        print "=============================="
        print "makeweb: *** error summary ***"
        print "=============================="
        print errmsg
    }
}
