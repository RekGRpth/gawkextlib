# AFP_Bug1.awk - illustrate a problem with `gawk' (GNU Awk 3.0.3 on OS/2)
# Arthur Pool .. pool@commerce.uq.edu.au
# $Id: fnasgnm.awk,v 1.1 1998/03/23 18:52:15 jkahrs Exp $

# Assignment to a variable with the same name as a function from within
# that function causes an ABEND.
#
# Yes, I do realise that it's not a smart thing to do, but an error
# message would be a kinder response than a core dump (and would make
# debugging a whole lot easier).

{ShowMe()}

function ShowMe() {ShowMe = 1}
