# Compose the web page of an individual extension
# and populate the web directory with the required files
#
# Input: The web page template
# + webdir = the web directory of the extension: <root>/_web/<extension>
# + inputdir = the source directory with doc files
# + webtoc = the webTOC file specification file
#
# Global variables:
# - mode: text processing mode = HTML tag (<ul>, <p>, <pre>, ...)
# - name: dir name of the extension (short name), i.e. .../name/
# - package: package name, usually "gawk-name"
# - summary: one-line description like "xxx extension for gawk"
# - version: version number, usually x.x.x
# - license: license name or acronym, usually "GPLv3+"
# - description: multi-line one paragraph description
# - license_file: document text of the license, usually "COPYING"

# External command: copy file to web
function copy_file( aux_file ) {
    system("cp " aux_file " " webdir)
}

# External command: convert file to web
function convert_file( doc_file ) {
    system("_web/makeweb-any.sh " doc_file " " webdir)
}

# Include text from file
function include_text( txt_file ) {
    while ((getline line < txt_file) > 0) {
        if (line ~ /^[^[:space:]]/) {        # non indented, <p> mode
            set_mode("p")
        } else if (line ~ /[^[:space:]]/) {  # indented, <pre> mode
            for (; space>0; space--) print ""
            set_mode("pre")
        } else if (mode=="pre") {            # blank line in <pre> mode
            space++
        } else {                             # blank line
            set_mode("")
        }
        if (!space) print line
    }
}

# Parse the configure.ac file
function parse_configure( config_ac,          line, f ) {
    while ((getline line < config_ac) > 0) {
        if (line ~ /^AC_INIT/) {
            split(line, f, /,[[:space:]]*/)
            version = f[2]
            package = gensub(/[[:space:]]*\)[[:space:]]*/, "", 1, f[4])
            break
        }
    }
}

# Parse the package.spec.in file
function parse_spec( spec_in,            line ) {
    while ((getline line < spec_in) > 0) {
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
        }
    }
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
            print "<li>" link_external(f[2], f[1]) "</li>"
        } else if (line ~ "->") {               # convert and link file
            split(line, f, /[[:space:]]*->[[:space:]]*/)
            convert_file(inputdir "/" f[2])
            fn = base_name(f[2])
            set_mode("ul")
            print "<li>" link(fn ".html", f[1]) " [" link(fn ".pdf", "PDF version") "]</li>"
        } else if (line !~ /^[[:space:]]*$/) {  # section header
            set_mode("")
            print "<h2>" line "</h2>"
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
    if (file !~ ".3am") {
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
        if (mode) print "</" mode ">"
        # record the new mode
        mode = newmode
        # open the new mode, if not null
        if (mode) print "<" mode ">"
    }
}

# Start processing the page template
BEGIN {
    parse_configure(inputdir "/configure.ac")
    parse_spec(inputdir "/packaging/" package ".spec.in")
    name = gensub(/.*\//, "", 1, webdir)  # last component of the webdir path
}

# Package name as title
/@TITLE@/ {
    if (! package) {
        package = "gawk-" name
    }
    print gensub(/@TITLE@/, package, 1, $0)  # package name
    next
}

# Top level header
/@HEADER@/ {
    getline header < webtoc        # fisrt line of the webTOC file
    if (summary) {
        header = package ": " summary       # from the spec file
    }
    print gensub(/@HEADER@/, header, 1, $0)  # page header
    next
}

# Version number from the spec file
/@VERSION@/ {
    if (version) {
        print gensub(/@VERSION@/, version, 1, $0)  # package version
    }
    next
}

# License given from the spec file
/@LICENSE@/ {
    if (license) {
        print gensub(/@LICENSE@/, link( license_file, license), 1, $0)  # package license
    }
    next
}

# Brief description from the spec file
/@SUMMARY@/ {
    if (description) {
        print gensub(/@SUMMARY@/, description, 1, $0)  # package description
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
    print
}
