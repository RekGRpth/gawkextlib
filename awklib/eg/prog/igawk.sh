#! /bin/sh
# igawk   --- like gawk but do @include processing
# xmlgawk --- like igawk, but @include xmllib.awk and set XMLMODE=1
#
# Arnold Robbins, arnold@gnu.org, Public Domain
# July 1993
# extension by Stefan Tramm, August 2003

if [ "$1" = debug ]
then
    set -x
    shift
fi

# A literal newline, so that program text is formmatted correctly
n='
'

# Initialize variables to empty
program=
opts=

while [ $# -ne 0 ] # loop over arguments
do
    case $1 in
    --)     shift; break;;

    -W)     shift
            # The ${x?'message here'} construct prints a
            # diagnostic if $x is the null string
            set -- -W"${@?'missing operand'}"
            continue;;

    -[vF])  opts="$opts $1 '${2?'missing operand'}'"
            shift;;

    -[vF]*) opts="$opts '$1'" ;;

    -f)     program="$program$n@include ${2?'missing operand'}"
            shift;;

    -f*)    f=`expr "$1" : '-f\(.*\)'`
            program="$program$n@include $f";;

    -[W-]file=*)
            f=`expr "$1" : '-.file=\(.*\)'`
            program="$program$n@include $f";;

    -[W-]file)
            program="$program$n@include ${2?'missing operand'}"
            shift;;

    -[W-]source=*)
            t=`expr "$1" : '-.source=\(.*\)'`
            program="$program$n$t";;

    -[W-]source)
            program="$program$n${2?'missing operand'}"
            shift;;

    -[W-]version)
            echo igawk: version 2.1 1>&2
            gawk --version
            exit 0 ;;

    -[W-]*) opts="$opts '$1'" ;;

    *)      break;;
    esac
    shift
done

if [ -z "$program" ]
then
     program=${1?'missing program'}
     shift
fi

# and now include the xmllib.awk (if the scriptname is xmlgawk)
# enable the interval regexp by default (we cant break old
# code, because there is no -- ST)
case "$0" in
    *xmlgawk) program="@include xmllib.awk$n$program"
              opts="--re-interval $opts";;
esac

# At this point, `program' has the program.
expand_prog='

function pathto(file,    i, t, junk)
{
    if (index(file, "/") != 0)
        return file

    for (i = 1; i <= ndirs; i++) {
        t = (pathlist[i] "/" file)
        if ((getline junk < t) > 0) {
            # found it
            close(t)
            return t
        }
    }
    return ""
}
BEGIN {
    path = ENVIRON["AWKPATH"]
    ndirs = split(path, pathlist, ":")
    for (i = 1; i <= ndirs; i++) {
        if (pathlist[i] == "")
            pathlist[i] = "."
    }
    stackptr = 0
    input[stackptr] = ARGV[1] # ARGV[1] is first file

    for (; stackptr >= 0; stackptr--) {
        while ((getline < input[stackptr]) > 0) {
            if (tolower($1) != "@include") {
                print
                continue
            }
            fpath = pathto($2)
            if (fpath == "") {
                printf("igawk:%s:%d: cannot find %s\n",
                    input[stackptr], FNR, $2) > "/dev/stderr"
                continue
            }
            if (! (fpath in processed)) {
                processed[fpath] = input[stackptr]
                input[++stackptr] = fpath  # push onto stack
            } else
                print $2, "included in", input[stackptr],
                    "already included in",
                    processed[fpath] > "/dev/stderr"
        }
        close(input[stackptr])
    }
}'  # close quote ends `expand_prog' variable

processed_program=`gawk -- "$expand_prog" /dev/stdin <<EOF
$program
EOF
`
eval gawk $opts -- '"$processed_program"' '"$@"'
