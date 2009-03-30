#ifndef _BBDOCUMENT__H_
#define _BBDOCUMENT__H_

#ifdef BLACK_BOKS

#include <db.h>
#include "../ds/dcontainer.h"


//rutine for � inalisere.
int bbdocument_init(container **attrkeys);

/*
 bbdocument_exist()

 rutine for � sjekke om en dokument eksisterer. Hvis det ikke gj�r det bruke bbdocument_add f�r � legge det til.
 subname	Hvilken koleksjon dette er. Som grunregel har vi en koleksjon for hver kilde, og for hver bruker. For eks 
		f�r 24so s�k her vi en bruker som heter per. Han her kolekson 24soper_mail for mailen sin.
		Koleksons navnet m� bli gitt inn fra komandolinjen.

 documenturi

 lastmodified

*/
int bbdocument_exist(char subname[],char documenturi[],unsigned int lastmodified);

/*
 bbdocument_add()

 subname        Hvilken koleksjon dette er. Som grunregel har vi en koleksjon for hver kilde, og for hver bruker. For eks
                f�r 24so s�k her vi en bruker som heter per. Han her kolekson 24soper_mail for mailen sin.
                Koleksons navnet m� bli gitt inn fra komandolinjen. 

 documenturi

 documenttype	Hvilken type. Set til NULL f�r � la systemet gjette denne basert p� filnavn

 document	Selve dokumentet. dokument_size langt.

 dokument_size	St�relsen p� dokumentet i bytes.

 lastmodified

 acl
*/
//rutine for � legge til et dokument
int bbdocument_add(char subname[],char documenturi[],char documenttype[],char document[],const int dokument_size,unsigned int 
	lastmodified,char *acl_allow, char *acl_denied, const char title[],char doctype[], char *attributes, container *attrkeys);


struct hashtable; /* so everyone who include bbdocument.h does not have to include hashtable.h */

//int bbdocument_convert(char filetype[],char document[],const int dokument_size,char **documentfinishedbuf,int *documentfinishedbufsize
//	, const char titlefromadd[]);
int bbdocument_convert(
			char filetype[],
			char document[],
			const int dokument_size,
			char **documentfinishedbuf,
			int *documentfinishedbufsize, 
			const char titlefromadd[], 
			char *subname, 
			char *documenturi, 
			unsigned int lastmodified, 
			char *acl_allow, 
			char *acl_denied,
			struct hashtable **metahash
		);



int bbdocument_close (container *attrkeys);
int bbdocument_delete (char uri[], char subname[]);

int uriindex_add (char uri[], unsigned int DocID, unsigned int lastmodified, char subname[]);
int uriindex_get (char uri[], unsigned int *DocID, unsigned int *lastmodified, char subname[]);
int uriindex_close (DB **dbpp);
int uriindex_open(DB **dbpp, char subname[]);

unsigned int bbdocument_nrOfDocuments(char subname[]);
int bbdocument_deletecoll(char collection[]);

#endif
#endif
