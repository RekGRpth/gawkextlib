# Descrip.MMS -- Makefile for building GNU awk on VMS.
#
# usage:
#  $ MMS /Description=[.vms]Descrip.MMS gawk
#	possibly add `/Macro=("GNUC=1")' to compile with GNU C,
#	or add `/Macro=("GNUC=1","DO_GNUC_SETUP=1")' to compile with GNU C
#	on a system where GCC is not installed as a defined command,
#	or add `/Macro=("VAXC=1")' to compile with VAX C,
#	or add `/Macro=("VAXC=1","CC=cc/VAXC")' to compile with VAX C on
#	a system which has DEC C installed as the default compiler.
#
# gawk.exe :
#	This is the default target.  DEC C has become the default compiler.
#
# pgawk.exe :
#	An alternate version which generates some profiling feedback for
#	the awk programs it executes.  Included with `make all'.
#
# awkgram.c :
#	If you don't have bison but do have VMS POSIX or DEC/Shell,
#	change the PARSER and PASERINIT macros to use yacc.  If you don't
#	have either yacc or bison, you'll have to make sure that the
#	distributed version of "awkgram.c" has its modification date later
#	than the date of "awkgram.y", so that MMS won't try to build that
#	target.  If you use bison and it is already defined system-wide,
#	comment out the PARSERINIT definition.
#
# install.help :
#	You can make the target 'install.help' to load the VMS help text
#	into a help library.  Modify the HELPLIB macro if you don't want
#	to put entry into the regular VMS library.  (If you use an alternate
#	help library, it must already exist; this target won't create it.)
#
# gawk.dvi :
#	If you have TeX, you can make the target 'gawk.dvi' to process
#	_The_GAWK_Manual_ from gawk.texi.  You'll need to use a device
#	specific post-processor on gawk.dvi in order to get printable data.
#	The full output is approximately 325 pages.
#

# location of various source files, relative to the 'main' directory
VMSDIR	= [.vms]
DOCDIR	= [.doc]
MAKEFILE = $(VMSDIR)Descrip.MMS

# debugging &c		!'ccflags' is an escape to allow external compile flags
#CCFLAGS = /noOpt/Debug

# a comma separated list of macros to define
CDEFS	= "GAWK","HAVE_CONFIG_H"

.ifdef GNUC
# assumes VAX
CC	= gcc
CFLAGS	= /Incl=([],$(VMSDIR))/Obj=[]/Def=($(CDEFS)) $(CCFLAGS)
LIBS	= gnu_cc:[000000]gcclib.olb/Library,sys$library:vaxcrtl.olb/Library
.ifdef DO_GNUC_SETUP
# in case GCC command verb needs to be manually defined
.first
	set command gnu_cc:[000000]gcc
.endif	!DO_GNUC_SETUP
.else	!!GNUC
.ifdef VAXC
# always VAX; versions of VAX C older than V3.2 won't work
CC	= cc
CFLAGS	= /Incl=[]/Obj=[]/Opt=noInline/Def=($(CDEFS)) $(CCFLAGS)
LIBS	= sys$share:vaxcrtl.exe/Shareable
.else	!!VAXC
# neither GNUC nor VAXC, assume DECC (same for either VAX or Alpha)
CC	= cc/DECC/Prefix=All
CFLAGS	= /Incl=[]/Obj=[]/Def=($(CDEFS)) $(CCFLAGS)
LIBS	=	# DECC$SHR instead of VAXCRTL, no special link option needed
.endif	!VAXC
.endif	!GNUC


PARSER	= bison
PARSERINIT = set command gnu_bison:[000000]bison
#PARSER	= yacc
#PARSERINIT = yacc := posix/run/path=posix """/bin/yacc"
#PARSERINIT = yacc := $shell$exe:yacc

# this is used for optional target 'install.help'
HELPLIB = sys$help:helplib.hlb
#HELPLIB = sys$help:local.hlb

#
########  nothing below this line should need to be changed  ########
#

ECHO = write sys$output
NOOP = continue

# object files
GAWKOBJ =  eval.obj,profile.obj
PGAWKOBJ =  eval_p.obj,profile_p.obj
AWKOBJ1 =  array.obj,awkgram.obj,builtin.obj,dfa.obj,ext.obj,\
	field.obj,floatcomp.obj,gawkmisc.obj,getopt.obj,getopt1.obj,io.obj
AWKOBJ2 = main.obj,msg.obj,node.obj,random.obj,re.obj,\
	regex.obj,replace.obj,version.obj
AWKOBJS = $(AWKOBJ1),$(AWKOBJ2)

# VMSOBJS
#	VMS specific stuff
VMSCODE = vms_misc.obj,vms_popen.obj,vms_fwrite.obj,vms_args.obj,\
	vms_gawk.obj,vms_cli.obj
VMSCMD	= gawk_cmd.obj			# built from .cld file
VMSOBJS = $(VMSCODE),$(VMSCMD)

# primary source files
AWKSRC = array.c,builtin.c,dfa.c,eval.c,eval_p.c,ext.c,field.c,\
	floatcomp.c,gawkmisc.c,getopt.c,getopt1.c,io.c,main.c,\
	msg.c,node.c,profile.c,profile_p.c,random.c,re.c,regcomp.c,\
	regex.c,regex_internal.c,regexec.c,replace.c,version.c

ALLSRC = $(AWKSRC),awkgram.y,awk.h,custom.h,dfa.h,getopt.h,\
	gettext.h,mbsupport.h,protos.h,random.h

VMSSRC = $(VMSDIR)gawkmisc.vms,$(VMSDIR)vms_misc.c,$(VMSDIR)vms_popen.c,\
	$(VMSDIR)vms_fwrite.c,$(VMSDIR)vms_args.c,$(VMSDIR)vms_gawk.c,\
	$(VMSDIR)vms_cli.c
VMSHDRS = $(VMSDIR)redirect.h,$(VMSDIR)vms.h,$(VMSDIR)fcntl.h,\
	$(VMSDIR)varargs.h,$(VMSDIR)unixlib.h
VMSOTHR = $(VMSDIR)descrip.mms,$(VMSDIR)vmsbuild.com,$(VMSDIR)version.com,\
	$(VMSDIR)gawk.hlp

DOCS= $(DOCDIR)gawk.1,$(DOCDIR)gawk.texi,$(DOCDIR)texinfo.tex

# Release of gawk
REL=3.1
PATCHLVL=5

# generic target
all : gawk,pgawk
	$(NOOP)

# dummy target to allow building "gawk" in addition to explicit "gawk.exe"
gawk : gawk.exe
	$(ECHO) " GAWK "
pgawk : pgawk.exe
	$(ECHO) " PGAWK "

# rules to build gawk
gawk.exe : $(GAWKOBJ) $(AWKOBJS) $(VMSOBJS) gawk.opt
	$(LINK) $(LINKFLAGS) gawk.opt/options

# rules to build pgawk
pgawk.exe : $(PGAWKOBJ) $(AWKOBJS) $(VMSOBJS) pgawk.opt
	$(LINK) $(LINKFLAGS) pgawk.opt/options

gawk.opt : $(MAKEFILE)			# create linker options file
	open/write opt gawk.opt		! ~ 'cat <<close >gawk.opt'
	write opt "! GAWK -- GNU awk"
      @ write opt "$(GAWKOBJ)"
      @ write opt "$(AWKOBJ1)"
      @ write opt "$(AWKOBJ2)"
      @ write opt "$(VMSOBJS)"
      @ write opt "psect_attr=environ,noshr	!extern [noshare] char **"
      @ write opt "stack=48	!preallocate more pages (default is 20)"
      @ write opt "iosegment=128	!ditto (default is 32)"
	write opt "$(LIBS)"
	write opt "identification=""V$(REL).$(PATCHLVL)"""
	close opt

pgawk.opt : $(MAKEFILE)			# create linker options file
	open/write opt pgawk.opt
	write opt "! PGAWK -- GNU awk w/ run-time profiling"
      @ write opt "$(PGAWKOBJ)"
      @ write opt "$(AWKOBJ1)"
      @ write opt "$(AWKOBJ2)"
      @ write opt "$(VMSOBJS)"
      @ write opt "psect_attr=environ,noshr	!extern [noshare] char **"
      @ write opt "stack=48	!preallocate more pages (default is 20)"
      @ write opt "iosegment=128	!ditto (default is 32)"
	write opt "$(LIBS)"
	write opt "identification=""V$(REL).$(PATCHLVL)"""
	close opt

vms_misc.obj	: $(VMSDIR)vms_misc.c
vms_popen.obj	: $(VMSDIR)vms_popen.c
vms_fwrite.obj	: $(VMSDIR)vms_fwrite.c
vms_args.obj	: $(VMSDIR)vms_args.c
vms_gawk.obj	: $(VMSDIR)vms_gawk.c
vms_cli.obj	: $(VMSDIR)vms_cli.c
$(VMSCODE)	: awk.h config.h $(VMSDIR)redirect.h $(VMSDIR)vms.h

gawkmisc.obj	: gawkmisc.c $(VMSDIR)gawkmisc.vms

$(AWKOBJS)	: awk.h gettext.h mbsupport.h regex.h dfa.h \
		  config.h $(VMSDIR)redirect.h
$(GAWKOBJ)	: awk.h config.h $(VMSDIR)redirect.h
$(PGAWKOBJ)	: awk.h config.h $(VMSDIR)redirect.h
random.obj	: random.h
builtin.obj	: random.h
awkgram.obj	: awkgram.c awk.h
dfa.obj	: dfa.c dfa.h
regex.obj : regex.c regcomp.c regex_internal.c regexec.c regex.h regex_internal.h

# bison or yacc required
awkgram.c	: awkgram.y	# foo.y :: yacc => y[_]tab.c, bison => foo_tab.c
     @- if f$search("ytab.c")	.nes."" then  delete ytab.c;*	 !POSIX yacc
     @- if f$search("y_tab.c")	.nes."" then  delete y_tab.c;*	 !DEC/Shell yacc
     @- if f$search("awk_tab.c").nes."" then  delete awk_tab.c;* !bison
      - $(PARSERINIT)
	$(PARSER) $(YFLAGS) $<
     @- if f$search("ytab.c")	.nes."" then  rename/new_vers ytab.c  $@
     @- if f$search("y_tab.c")	.nes."" then  rename/new_vers y_tab.c $@
     @- if f$search("awk_tab.c").nes."" then  rename/new_vers awk_tab.c $@

config.h	: $(VMSDIR)vms-conf.h
	copy $< $@

$(VMSCMD)	: $(VMSDIR)gawk.cld
	set command $(CLDFLAGS)/object=$@ $<

# special target for loading the help text into a VMS help library
install.help	: $(VMS)gawk.hlp
	library/help $(HELPLIB) $< /log

# miscellaneous other targets
tidy :
      - if f$search("*.*;-1").nes."" then  purge
      - if f$search("[.*]*.*;-1").nes."" then  purge [.*]

clean :
      - delete *.obj;*,gawk.opt;*,pgawk.opt;*

spotless : clean tidy
      - if f$search("config.h").nes."" then  rename config.h config.h-old/New
      - if f$search("gawk.exe").nes."" then  delete gawk.exe;*
      - if f$search("pgawk.exe").nes."" then  delete pgawk.exe;*
      - if f$search("gawk.dvi").nes."" then  delete gawk.dvi;*
      - if f$search("[.doc]texindex.exe").nes."" then  delete [.doc]texindex.exe;*

#
# Note: this only works if you kept a copy of [.support]texindex.c
# from a gawk 2.x distribution and put it into [.doc]texindex.c.
# Also, depending on the fonts available with the version of TeX
# you have, you might need to edit [.doc]texinfo.tex and change
# the reference to "lcircle10" to be "circle10".
#
# build gawk.dvi from within the 'doc' subdirectory
#
gawk.dvi : [.doc]texindex.exe [.doc]gawk.texi
      @ set default [.doc]
      @ write sys$output " Warnings from TeX are expected during the first pass"
	TeX gawk.texi
	mcr []texindex gawk.cp gawk.fn gawk.ky gawk.pg gawk.tp gawk.vr
      @ write sys$output " Second pass"
	TeX gawk.texi
	mcr []texindex gawk.cp gawk.fn gawk.ky gawk.pg gawk.tp gawk.vr
      @ write sys$output " Third (final) pass"
	TeX gawk.texi
     -@ purge
     -@ delete gawk.lis;,.aux;,gawk.%%;,.cps;,.fns;,.kys;,.pgs;,.toc;,.tps;,.vrs;
      @ rename/new_vers gawk.dvi [-]*.*
      @ set default [-]

# Note: [.doc]texindex.c is not included with the gawk 3.x distribution.
# Expect lots of "implicitly declared function" diagnostics from DEC C.
#
[.doc]texindex.exe : [.doc]texindex.c
      @ set default [.doc]
	$(CC) /noOpt/noList/Define=("lines=tlines") texindex.c
      @ open/Write opt texindex.opt
      @ write opt "texindex.obj"
      @ write opt "$(LIBS)"
      @ close opt
	$(LINK) /noMap/Exe=texindex.exe texindex.opt/Options
     -@ delete texindex.obj;*,texindex.opt;*
      @ set default [-]

#eof
