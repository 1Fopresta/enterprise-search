/*
Rutinner for � jobbe p� en Boitho DocumentIndex fil.

ToDo: trenger en "close" prosedyre for filhandlerene.
*/
//#include "define.h"
#include "DocumentIndex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//skrur av filcashn da vi deiver � segge feiler med den
//#define DI_FILE_CASHE

static int openDocumentIndex = -1;
#ifdef DI_FILE_CASHE
                FILE *DocumentIndexHA;
#endif


#define CurrentDocumentIndexVersion 4


/*
finner riktif fil og S�ker seg frem til riktig adresse, slik at man bare kan lese/skrive
*/
FILE *GetFileHandler (unsigned int DocID,char type,char subname[], char *diname) {

	#ifndef DI_FILE_CASHE
		FILE *DocumentIndexHA;
	#endif
	int LotNr;

	char FileName[128];
	char FilePath[128];
	//unsigned int adress = -1;
	//off_t adress = -1;
	int adress = -1;
	
	//finner lot for denne DocIDen
	LotNr = rLotForDOCid(DocID);

	//printf("%u-%i\n",DocID,LotNr);

	//hvis filen ikke er open �pner vi den
	//segfeiler en skjelden gang
	#ifdef DI_FILE_CASHE
	if (LotNr != openDocumentIndex) {
	#else
	if(1) {
	#endif		
		#ifdef DI_FILE_CASHE
		//hvis vi har en open fil lukkes denne
		if (openDocumentIndex != -1) {
			//segfeiler her for searchkernel
			//18,okt segefeiler her igjen ????
			printf("open file for lot %i\n",openDocumentIndex);
			fclose(DocumentIndexHA);
		}
		#endif
				
		
	

		GetFilPathForLot(FilePath,LotNr,subname);
		
		strncpy(FileName,FilePath,128);
		strncat(FileName,diname == NULL ? "DocumentIndex" : diname,128);

		//debug:viser hvpathen til loten
		//printf("path: %s\n",FileName);

		//pr�ver f�rst � �pne for lesing

		if (type == 'r') {
			//printf("opening file\n");
			//temp: setter filopning til r+ for � f� til � samarbeid melom DIRead og DIwrite
			//dette gj�r at s�k ikke funker p� web p� grun av rettighter :-(
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				printf("cant open file %s\n",FileName);
				perror(FileName);
			    return NULL;
			}
		}
		else if (type == 'w'){
			//printf("opening file\n");
			if ((DocumentIndexHA = fopen(FileName,"r+b")) == NULL) {
				//hvis det ikke g�r lager vi og �pne filen
				makePath(FilePath);
				if ((DocumentIndexHA = fopen(FileName,"w+b")) == NULL) {
					perror(FileName);
					//exit(1);
					return NULL;
				}
			}
		}

		openDocumentIndex = LotNr;
	}

	#ifdef BLACK_BOKS
		adress = (sizeof(struct DocumentIndexFormat) + sizeof(unsigned int))* (DocID - LotDocIDOfset(LotNr));
	#else
		adress = sizeof(struct DocumentIndexFormat) * (DocID - LotDocIDOfset(LotNr));
	#endif
	//printf("tell: %i\n",ftell(DocumentIndexHA));
	


	//s�ker til riktig post
	if (fseek(DocumentIndexHA,adress,0) != 0) {
		perror("Can't seek");
		exit(1);
	}

//	printf("tell: %i\n",ftell(DocumentIndexHA));
		
	//file = DocumentIndexHA;

//	printf("tell �pen: %i\n",ftell(DocumentIndexHA));
//	printf("fa �pnet %i\n",(int)DocumentIndexHA);

	return DocumentIndexHA;

}

//sjeker om det fins en DocumentIndex fro denne loten

int DIHaveIndex (int lotNr,char subname[]) {

	char FilePath[512];
	FILE *FH;

        GetFilPathForLot(FilePath,lotNr,subname);

        strncat(FilePath,"DocumentIndex",sizeof(FilePath));

	if ((FH = fopen(FilePath,"r")) == NULL) {
		return 0;
	}
	else {
		fclose(FH);
		return 1;
	}
}

/*
skriver en post til DocumentIndex
*/
void DIWrite (struct DocumentIndexFormat *DocumentIndexPost, unsigned int DocID,char subname[], char *diname) {


	FILE *file;

	//printf("ha uinalisert %i\n",(int)file);
	
	if ((file = GetFileHandler(DocID,'w',subname, diname)) != NULL) {

		//printf("fa mottat %i\n",(int)file);

		//printf("motatt tell: %i\n",ftell(file));
		//printf("aa url: %s\n",(*DocumentIndexPost).Url);
		#ifdef BLACK_BOKS
			unsigned int CurrentDocumentIndexVersionAsUInt = CurrentDocumentIndexVersion;

			if (fwrite(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,file) != 1) {
	                        perror("Can't write");
        	                exit(1);
                	}
		#endif
		//skriver posten
		if (fwrite(DocumentIndexPost,sizeof(struct DocumentIndexFormat),1,file) != 1) {
			perror("Can't write");
			exit(1);
		}
	}
	else {
		printf("Cant get fh\n");
	}

	//hvis vi ikke har p� DI_FILE_CASHE m� vi lokke filen
	#ifndef DI_FILE_CASHE
		fclose(file);
	#endif
}

/*
tar et lott nr inn og henter neste post

Den vil retunere 1 s� lenge det er data og lese. slik at man kan ha en lopp slik

while (DIRGetNext(LotNr,DocumentIndexPost)) {

        ..gj�r noe med ReposetoryData..

}
*/

int DIGetNext (struct DocumentIndexFormat *DocumentIndexPost, int LotNr,unsigned int *DocID,char subname[]) {


        static FILE *LotFileOpen;
        static int LotOpen = -1;
	static unsigned LastDocID;
	int n;

	char FileName[128];


        //tester om reposetoriet allerede er open, eller ikke
        if (LotOpen != LotNr) {
                //hvis den har v�rt open, lokker vi den. Hvi den er -1 er den ikke brukt enda, s� ingen vits � � lokke den da :-)
                if (LotOpen != -1) {
                        fclose(LotFileOpen);
                }
                GetFilPathForLot(FileName,LotNr,subname);
                strncat(FileName,"DocumentIndex",128);

                printf("Opending lot %s\n",FileName);

                if ( (LotFileOpen = fopen(FileName,"rb")) == NULL) {
                        perror(FileName);
                        exit(1);
                }
                LotOpen = LotNr;
		LastDocID = GetStartDocIFForLot(LotNr);
        }
	else {
		++LastDocID;
	}

	(*DocID) = LastDocID;

	

        //hvis det det er data igjen i filen leser vi den
        if (!feof(LotFileOpen)) {
         

        	//leser posten
		//mystisk at vi f�r "Can't reed DocumentIndexPost: Success", ved eof her,
		//i steden for at vi  f�r eof lenger opp
		#ifdef BLACK_BOKS
                        unsigned int CurrentDocumentIndexVersionAsUInt;
			if ((fread(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,LotFileOpen)) != 1) {
				perror("CurrentDocumentIndexVersionAsUInt");
				return 0;
			}
		#endif

        	if ((n=fread(DocumentIndexPost,sizeof(*DocumentIndexPost),1,LotFileOpen)) != 1) {
                	printf("Can't reed DocumentIndexPost. n: %i\n",n);
			perror("");
			//stnger ned filen
			fclose(LotFileOpen);

                	//exit(1);
			return 0;
        	}
		else {
			//printf("Url: %s\n",(*DocumentIndexPost).Url);
	       		return 1;
		}

        }
        else {
        //hvis vi er tom for data stenger vi filen, og retunerer en 0 som sier at vi er ferdig.
                printf("ferdig\n");
                fclose(LotFileOpen);
                return 0;
        }
}

/*
leser en post fra DocumentIndex
*/
int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]) {

	FILE *file;
	int forReturn;

	if ((file = GetFileHandler(DocID,'r',subname, NULL)) != NULL) {


		#ifdef BLACK_BOKS
                        unsigned int CurrentDocumentIndexVersionAsUInt;
                        if ((fread(&CurrentDocumentIndexVersionAsUInt,sizeof(unsigned int),1,file)) != 1) {
                                perror("CurrentDocumentIndexVersionAsUInt");
                                forReturn = 0;
                        }
                #endif


        	//lesr posten
        	if (fread(DocumentIndexPost,sizeof(*DocumentIndexPost),1,file) != 1) {
                	perror("Can't reed");

			//selv om vi ikke fikk lest fra filen m� vi lokke den, s� vi kan ikke kalle retun directe her
			forReturn =  0;
        	}
		else {
			forReturn  = 1;
		}
		
		//hvis vi ikke har p� DI_FILE_CASHE m� vi lokke filen
		#ifndef DI_FILE_CASHE
			fclose(file);
		#endif

                return forReturn;
		
        }
        else {
		printf("cant get GetFileHandler\n");
                return  0;
        }




}

//stanger ned filer
void DIClose(FILE *DocumentIndexHA) {
//	fclose(DocumentIndexHA);
}
