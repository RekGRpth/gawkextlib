CC = gcc
CFLAGS = -W -Wall -pedantic -I. -I.. -O2 -g

all: xml.so dl.so filefuncs.so arrayparm.so readfile.so ordchr.so

clean:
	rm -f *.o
	rm -f *.sh.o
	rm -f *.so

xml_interface.sh.o: xml_interface.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -DDYNAMIC_LOADING -c -o xml_interface.sh.o xml_interface.c
xml_puller.sh.o: xml_puller.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o xml_puller.sh.o xml_puller.c
xml.so: xml_interface.sh.o xml_puller.sh.o
	$(CC) -shared -o xml.so  xml_interface.sh.o xml_puller.sh.o -lexpat


dl.sh.o: dl.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o dl.sh.o dl.c
dl.so: dl.sh.o
	$(CC) -shared -o dl.so dl.sh.o

filefuncs.sh.o: filefuncs.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o filefuncs.sh.o filefuncs.c
filefuncs.so: filefuncs.sh.o
	$(CC) -shared -o filefuncs.so filefuncs.sh.o

arrayparm.sh.o: arrayparm.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o arrayparm.sh.o arrayparm.c
arrayparm.so: arrayparm.sh.o
	$(CC) -shared -o arrayparm.so arrayparm.sh.o

readfile.sh.o: readfile.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o readfile.sh.o readfile.c
readfile.so: readfile.sh.o
	$(CC) -shared -o readfile.so readfile.sh.o

ordchr.sh.o: ordchr.c
	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o ordchr.sh.o ordchr.c
ordchr.so: ordchr.sh.o
	$(CC) -shared -o ordchr.so ordchr.sh.o



#.sh.o: .c
#	$(CC) -fpic $(CFLAGS) -DHAVE_CONFIG_H -I. -c -o .sh.o .c
#.so: .sh.o
#	$(CC) -shared -o .so .sh.o


