#!/bin/sh

usage () {
   echo "
Usage: `basename $0` [-g <path to gawk>] [-l <path to gawkextlib>] [-C] [-s] <new extension directory name> <author name> <author email address>

	Configures a new directory for adding an extension.  This installs
	the boilerplate stuff so you can focus on writing code, documentation,
	and test cases.

	Options:
		-g	Specify path to your gawk installation, in case
			it's in a nonstandard place.
		-l	Specify path to your gawkextlib installation, in case
			it's in a nonstandard place.
		-C	Create C++ source with .cpp extension.
		-s	Print the spec.in file template to stdout and exit.
			This is useful only when trying to update an old
			spec.in file.
"
   exit 1
}

confargs=""
ext="c"
do_genspec=""
while getopts g:l:Cs flag ; do
   case $flag in
   g) confargs="$confargs --with-gawk=$OPTARG" ;;
   l) confargs="$confargs --with-gawkextlib=$OPTARG" ;;
   C) ext="cpp" AC_CPP='AC_PROG_CXX';;
   s) do_genspec=1 ;;
   *) usage ;;
   esac
done
shift `expr $OPTIND - 1`

[ $# -ne 3 ] && usage
name="$1"
author="$2"
email="$3"

genspec () {
   cat<<__EOF__
Name:             @PACKAGE@
Summary:          $name library for gawk
Version:          @VERSION@
Release:          1%{?dist}
License:          GPLv3+

URL:              https://sourceforge.net/projects/gawkextlib
Source:           %{url}/files/%{name}-%{version}.tar.gz

Requires:         gawk
BuildRequires:    gawk-devel
BuildRequires:    gawkextlib-devel
Requires(post):   info
Requires(preun):  info

# Make sure the API version is compatible with our source code:
BuildRequires:    gawk(abi) >= 1.1
BuildRequires:    gawk(abi) < 3.0

# At runtime, the ABI must be compatible with the compile-time version
%global gawk_api_version %(gawk 'BEGINFILE {if (ERRNO) nextfile} match(\$0, /#define gawk_api_(major|minor)_version[[:space:]]+([[:digit:]]+)/, f) {v[f[1]] = f[2]} END {print (v["major"] "." v["minor"])}' /usr/include/gawkapi.h)
Requires:         gawk(abi) >= %{gawk_api_version}
Requires:         gawk(abi) < %(echo %{gawk_api_version} | gawk -F. '{printf "%d.0\n", \$1+1}')

# This is the default as of Fedora 23:
%global _hardened_build 1

%description
%{name} provides the gawk $name extension module that provides
several useful functions.

# =============================================================================

%prep
%autosetup

%build
%configure
%make_build

%check
make check

%install
%make_install

# The */dir file is not necessary for info pages to work correctly...
rm -f %{buildroot}%{_infodir}/dir

# Install NLS language files:
%find_lang %{name}

# Always update the info pages:
%post
/sbin/install-info %{_infodir}/%{name}.info.gz %{_infodir}/dir || :

%preun
if [[ \$1 -eq 0 ]]; then
   /sbin/install-info --delete %{_infodir}/%{name}.info.gz %{_infodir}/dir || :
fi

%files -f %{name}.lang
%license COPYING
%doc NEWS
%doc test/*.awk
%{_infodir}/*.info*
%{_libdir}/gawk/$name.so
%{_mandir}/man3/*

# =============================================================================

%changelog
* `date "+%a %b %d %Y"` $author <$email> - @VERSION@-1
- Rebuilt for new release
__EOF__
}

if [ -n "$do_genspec" ]; then
   genspec
   exit 0
fi
if [ -e "$name" ]; then
   echo "Error: $name exists already"
   usage
fi

echo "$name" | awk 'END {exit (NF != 1) || (NR != 1)}' || {
   echo "Error: the extension name may not contain any embedded white space"
   usage
}

doit () {
   echo "
	Running: $@"
   eval "$@" || {
      echo "Error: command [$@] failed with exit status $?"
      exit 1
   }
}

doit mkdir $name
doit cd $name
doit "echo $author > AUTHORS"
doit ln -s ../shared/COPYING
echo "
	Initializing ChangeLog"
printf "%-18s %-21s <%s>\n\n\t* First version.\n" \
       "`date +%Y-%m-%d`" "$author" "$email" > ChangeLog || {
   echo "Error: cannot create ChangeLog"
   exit 1
}

echo "
	Initializing Makefile.am"
echo "# This variable insures that aclocal runs
# correctly after changing configure.ac
ACLOCAL_AMFLAGS = -I m4

include extension.makefile

pkgextension_LTLIBRARIES = $name.la

${name}_la_SOURCES	= $name.$ext
${name}_la_LIBADD	= \$(LTLIBINTL)
${name}_la_LDFLAGS	= \$(GAWKEXT_MODULE_FLAGS)

SUBDIRS = doc po packaging test

EXTRA_DIST = common.h unused.h" > Makefile.am || {
   echo "Error: cannot create Makefile.am"
   exit 1
}

doit "printf 'Version 1.0.0\n-------------\nFirst version.\n' > NEWS"
doit "echo This is the $name extension. > README"
doit ln -s ../shared/common.h
doit ln -s ../shared/unused.h

echo "
	Initializing configure.ac"
echo "dnl Process this file with autoconf to produce a configure script.

AC_INIT([Gawk $name Extension], 1.0.0, gawkextlib-users@lists.sourceforge.net, gawk-$name)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
$AC_CPP

AC_GAWK_EXTENSION

AC_CONFIG_HEADERS([config.h:configh.in])

AC_CONFIG_FILES(Makefile
	doc/Makefile
	packaging/Makefile
	packaging/gawk-$name.spec
	po/Makefile.in
	test/Makefile)
AC_OUTPUT" > configure.ac || {
   echo "Error: cannot create configure.ac"
   exit 1
}

doit "mkdir doc"
doit "echo dist_man_MANS = $name.3am > doc/Makefile.am"

echo "
	Initializing doc/$name.3am"
uname=`echo $name | awk '{print toupper($0)}'`
mandate=`date +"%b %d %Y"`
year=`date +%Y`
cat<<__EOF__>doc/$name.3am
.TH $uname 3am "$mandate" "Free Software Foundation" "GNU Awk Extension Modules"
.SH NAME
$name \- do something useful
.SH SYNOPSIS
.ft CW
@load "$name"
.sp
string = $name(11)
.ft R
.SH DESCRIPTION
The
.I $name
extension adds a function named
.BR $name()
as follows:
.TP
.B $name()
This function takes an integer argument and returns the result from calling
the
.IR $name (3)
C library function.  If the argument is not numeric, it will return an empty string.
... .SH NOTES
... .SH BUGS
.SH EXAMPLE
.ft CW
.nf
@load "$name"
\&...
printf "The mapped value of 11 is %s\en", $name(11)
.fi
.ft R
.SH "SEE ALSO"
.BR $name (3)
.SH AUTHOR
$author
.SH COPYING PERMISSIONS
Copyright \(co $year,
Free Software Foundation, Inc.
.PP
Permission is granted to make and distribute verbatim copies of
this manual page provided the copyright notice and this permission
notice are preserved on all copies.
.ig
Permission is granted to process this file through troff and print the
results, provided the printed document carries copying permission
notice identical to this one except for the removal of this paragraph
(this paragraph not being relevant to the printed manual page).
..
.PP
Permission is granted to copy and distribute modified versions of this
manual page under the conditions for verbatim copying, provided that
the entire resulting derived work is distributed under the terms of a
permission notice identical to this one.
.PP
Permission is granted to copy and distribute translations of this
manual page into another language, under the above conditions for
modified versions, except that this permission notice may be stated in
a translation approved by the Foundation.
__EOF__

echo "
	Initializing $name.$ext"
cat<<__EOF__>$name.$ext
/*
 * $name.$ext - Builtin functions to implement $name.
 */

/*
 * Copyright (C) $year the Free Software Foundation, Inc.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335, USA
 */

#include "common.h"

/*  do_$name --- call $name */

static awk_value_t *
do_$name(int nargs, awk_value_t *result API_FINFO_ARG)
{
	awk_value_t x;

#if gawk_api_major_version < 2
	if (do_lint && nargs > 1)
		lintwarn(ext_id, _("$name: called with too many arguments"));
#endif

	if (get_argument(0, AWK_NUMBER, & x)) {
		char str[256];
		snprintf(str, sizeof(str), "%g", x.num_value);
		return make_const_string(str, strlen(str), result);
	}
	if (do_lint) {
		if (nargs == 0)
			lintwarn(ext_id, _("$name: called with no arguments"));
		else
			lintwarn(ext_id, _("$name: called with inappropriate argument(s)"));
	}
	return make_null_string(result);
}

/*
 * The third argument to API_FUNC_MAXMIN is the maximum number of expected
 * arguments, and the fourth is the minimum number of required arguments.
 * If they are the same, you may use the API_FUNC macro instead, which takes
 * only three arguments. Note that in version 1 of the API, the third value
 * in the awk_ext_func_t struct called num_expected_args is actually the
 * maximum number of allowed arguments.
 */
static awk_ext_func_t func_table[] = {
	API_FUNC_MAXMIN("$name", do_$name, 1, 0)
};

static awk_bool_t
init_my_module(void)
{
  GAWKEXTLIB_COMMON_INIT
  return awk_true;
}

static awk_bool_t (*init_func)(void) = init_my_module;
static const char *ext_version = PACKAGE_STRING;

dl_load_func(func_table, $name, "")
__EOF__

doit ln -s ../shared/extension.makefile
doit mkdir m4
doit ln -s ../../shared/gawkext.m4 m4/gawkext.m4
doit mkdir packaging
doit ln -s ../../shared/packaging.makefile packaging/Makefile.am

echo "
	Initializing packaging/gawk-$name.spec.in"
genspec > packaging/gawk-$name.spec.in

doit mkdir po
doit ln -s ../../shared/Makevars po/Makevars
doit "echo $name.$ext > po/POTFILES.in"

doit mkdir test
doit ln -s ../../shared/test.makefile test/test.makefile

echo "
	Initializing test/Makefile.am"
cat<<__EOF__>test/Makefile.am
EXTRA_DIST = \\
	$name.awk \\
	$name.ok

# Get rid of core files when cleaning and generated .ok file
CLEANFILES = _* *_.png core core.* junk out1 out2 out3 test1 test2 seq *~

include test.makefile

# Message stuff is to make it a little easier to follow.
# Make the pass-fail last and dependent on others to avoid
# spurious errors if \`make -j' in effect.
check:	test-msg-start mytests test-msg-end
	@\$(MAKE) pass-fail || { \$(MAKE) diffout; exit 1; }

mytests: $name

test-msg-start:
	@echo "======== Starting $name tests ========"

test-msg-end:
	@echo "======== Done with $name tests ========"

${name}::
	@echo \$@
	@\$(AWK) -l $name -f \$(srcdir)/\$@.awk >_\$@ 2>&1 || echo EXIT CODE: \$\$? >>_\$@
	@-\$(CMP) \$(srcdir)/\$@.ok _\$@ && rm -f _\$@
__EOF__

doit "printf 'BEGIN {\n\tprint $name(7)\n}\n' > test/$name.awk"
doit "echo 7 > test/$name.ok"

doit autoreconf -i
doit ./configure $confargs
doit make
doit make check

# test language for initializing message translation stuff
lang=fr

doit cd po
doit msginit --no-translator -l $lang
doit "echo $lang > LINGUAS"
doit cd ..
doit make
doit make check
if type -p git > /dev/null; then
   doit git add .
   doit git add -f m4/gawkext.m4
else
   echo "Warning: I cannot find the git command, so I am skipping those commands."
fi

echo "
Congratulations!  A working sample extension has been created as a starting
point.  Please edit the files.  You will need to change these files among
others:

	$name.$ext
		Implement the extension.
	README
		Explain what this does.
	Makefile.am
		You may need to add more source files, add library dependencies
		to ${name}_la_LIBADD, and/or add files to EXTRA_DIST.
	configure.ac
		You may need to check for some functions, headers, libraries,
		paths, etc.
	doc/${name}.3am
		Document your extension.
	packaging/gawk-${name}.spec.in	
		Fix the Summary, build and package dependencies,
		description, list of files installed, and perhaps
		install scripts.
	test/Makefile.am
		Implement tests for this extension.
	
When you are done, please commit and push.  To remove everything done by this
script, please simply run these commands:

	rm -rf $name
	git rm -r $name

You can then start over with a clean slate."
