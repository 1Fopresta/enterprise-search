#CC=	gcc
#LDFLAGS= -lm 
#CFLAGS=	-g -Wall -DTESTMAIN -std=c99 -O2

WANT_LOGGER=    true

OBJS=	spelling.o dmetaphone.o levenshtein.o
LIB=	libspelling.a
CFLAGS+=        -D_FILE_OFFSET_BITS=64

#PROG=	spelling

#all: $(PROG)

#$(PROG): $(OBJS)
#	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS) ../3pLibs/keyValueHash/hashtable.c ../3pLibs/keyValueHash/hashtable_itr.c ../common/ht.c ../ds/libds.a

#clean:
#	rm -f $(PROG) $(OBJS)

#dmp:
#	gcc -o dmetaphone dmetaphone.c -Wall -std=c99 -g -DDMP_TEST

include ../../mk/lib.mk
