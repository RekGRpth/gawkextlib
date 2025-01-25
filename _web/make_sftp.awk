# Process the <extension>/webTOC file and generate the sftp commands
# required to upload the files in the _web/<extension> directory.

#-----------------------------------
# Auxiliary functions
#-----------------------------------

# Remove leading and trailing space
function trim( str ) {
    sub(/^[[:space:]]+/, "", str)
    sub( /[[:space:]]+$/, "", str )
    return str
}

# Base name of a file
function base_name( file ) {
    sub(/.*\//, "", file)  # remove leading path
    return file
}

# Dir name of a file
function dir_name( file,          dir ) {
    dir = gensub(/\/[^\\]*$/, "", 1, file)  # remove last path component
    if (dir == file) {    # no leading path
        dir = "."
    }
    return dir
}

#-----------------------------------
# Process the webTOC file
#-----------------------------------

BEGINFILE {          # set local and remote directories
    print "cd /home/project-web/gawkextlib/htdocs"
    extdir = dir_name(FILENAME)   # extract the extension name
    print "lcd _web/" extdir
    print "mkdir " extdir
    print "cd " extdir
    if (extdir == ".") {
        print "put index.html"
    } else {
        print "put " extdir ".html"
    }
}

{                    # discard leading and trailing space
    $0 = trim($0)
}

/-->/ {              # copy file
    split($0, f, /[[:space:]]*-->[[:space:]]*/)
    file = base_name(f[2])
    print "put " file
    next
}

/\*->/ {             # external link
    next
}

/->/ {               # convert and link file
    split($0, f, /[[:space:]]*->[[:space:]]*/)
    file = base_name(f[2])
    if (file !~ "(.3am|.awk)$") {
        sub(/\.[^.]*$/, "", file)  # remove trailing extension
    }
    print "put " file ".html"
    print "put " file ".pdf"
}

END {                # terminate processing
    print "exit"
}
