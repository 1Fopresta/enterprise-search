#ifndef _RE__H_
#define _RE__H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "define.h"

/*
        RE_COPYONCLOSE: lager en kopi av filen, og kopierer s� inn denne kopien n�r vi avslutter. Dette gj�r at vi kan jobbe p� v�r egen private, l�ste kopi, og
        s� kopiere den inn anatomisk, n�r vi er ferdige med den, og alt gikk bra. Lesing til orginalen er fortsatt tilat, men man kan ikke lge nye kopier f�r vi er ferdige.
*/
#define RE_COPYONCLOSE 0x1
#define RE_HAVE_4_BYTES_VERSION_PREFIX 0x2


struct reformat {
        void *mem;
        //char *mem;
        int fd;
        size_t maxsize;
        size_t structsize;
        int flags;
        int lotNr;
        char subname[maxSubnameLength];
        char mainfile[62];
        char tmpfile[62];
        int fd_tmp;
};


struct reformat *reopen(int lotNr, size_t structsize, char file[], char subname[], int flags);
void reclose(struct reformat *re);
void *reget(struct reformat *re, unsigned int DocID);
void *renget(struct reformat *re, size_t nr);

#define RE_DocumentIndex(re, DocID) ((struct DocumentIndexFormat *)reget(re, DocID))
#define REN_DocumentIndex(re, nr) ((struct DocumentIndexFormat *)renget(re, nr))


#endif // _RE__H_
