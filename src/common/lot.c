#include "lot.h"
#include "define.h"
#include "bstr.h"
#include "bfileutil.h"
#include "boithohome.h"

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

//hvis det er mac os :
//#include <sys/param.h> // for statfs 
//#include <sys/mount.h> // for statfs 
#include <sys/vfs.h> // for statfs

//formater p� cashe over opne filhontarere
struct OpenFilesFormat {
        short int LotNr;
        FILE *FILEHANDLER;
};


//array med mapper som kan brukes til � lagre i
struct StorageDirectorysFormat dataDirectorys[NrOfDataDirectorys];

//holder om vi har inalisert. Slik at MakeMapListMap() bare bli kalt 1 gang, nemlig f�rste gangen vi buker denne rutinen.
static int MapListInitialised = 0;

static LotFilesInalisert = 0;

//array med opne filhonterere
struct OpenFilesFormat OpenFiles[MaxOpenFiles];

#ifndef DEFLOT
//LotForDOCid returnerer hvilken lot DOCid skal i
int rLotForDOCid (unsigned int DocID) {
	int lot;
	
	lot	= (int)((DocID / NrofDocIDsInLot) +1);

	/*
	//gjode litt om her s� vi ikke trenger � bruke ceil. Da det er trekt
	//testen nedenfor sjekker at vi fortsatt f�r samme resultat
	if (lot != ceil((DocID / NrofDocIDsInLot) +1)) {
		printf("lot proble\n");
		exit(1);
	}
	*/
	return lot;
}

/*
Antall DocIDer som er f�r denne lotten. Dette er viktig � vite da da indekser 
bruker DocID for adresering, og lotene er p� en m�te som en stabel med kort, man 
m� vite hvor n�verende kort bgynner
*/
int LotDocIDOfset (int LotNr) {

	return ((NrofDocIDsInLot * LotNr) - NrofDocIDsInLot);
}

#endif
int HasSufficientSpace(char FilePath[], int needSpace) {


	struct statfs buf;
	long fssize;

	if (statfs(FilePath,&buf) != 0) {
		perror(FilePath);
		return 0;
	}	
	fssize = (long)((long)buf.f_bavail * ((long)buf.f_bsize / 1024));

	//printf("f_bavail %li,f_bsize %li, free %li\n",(long)buf.f_bavail,(long)buf.f_bsize,fssize);


	if ((needSpace * 1024) > fssize) {
		return 0;
	}
	else {
		return 1;
	}

}

int lotHasSufficientSpace(int lot, int needSpace,char subname[]) {

	char FilePath[512];
	int i;

	GetFilPathForLot(FilePath,lot,subname);
	//m� lage, slik at vi er 100% sikker p� at vi har noe � teste mot

	#ifdef DEBUG
		printf("will hav to make lot path sow we can test for space\n");
	#endif
	makePath(FilePath);
	
	i=HasSufficientSpace(FilePath,needSpace);

	return i;
}


FILE *lotOpenFileNoCashe(unsigned int DocID,char resource[],char type[], char lock,char subname[]) {

	return lotOpenFileNoCasheByLotNr(rLotForDOCid(DocID),resource,type,lock,subname);
}
FILE *lotOpenFileNoCasheByLotNr(int LotNr,char resource[],char type[], char lock,char subname[]) {


	FILE *FILEHANDLER;
	int i;
	char FilePath[PATH_MAX]; 	//var 128
	char File [PATH_MAX];	//var 128

	#ifdef DEBUG
		printf("subname: \"%s\", resource %s\n",subname,resource);
	#endif

                 GetFilPathForLot(FilePath,LotNr,subname);
                 strcpy(File,FilePath);
                 strncat(File,resource,PATH_MAX); //var 128

		#ifdef DEBUG
                	printf("lotOpenFileNoCasheByLotNr: opening file \"%s\" for %s\n",File,type);
		#endif

		if (strcmp(type,">>") == 0) {
			//emulating perl's >>. If the file eksist is is opene for reading and writing.
			//if not it is createt and openf for writing and reading
			if ( (FILEHANDLER = (FILE *)fopen64(File,"r+")) == NULL ) {
                        	makePath(FilePath);

				if ( (FILEHANDLER = (FILE *)fopen64(File,"r+")) == NULL ) {

                        		if ( (FILEHANDLER = (FILE *)fopen64(File,"w+")) == NULL ) {
                        		        perror(File);
                        		        //exit(0);
                        		        return NULL;
                        		}
				}
                	}
		}
		//hvis dette er lesing s� hjelper det ikke og pr�ve � opprette path. Filen vil fortsatt ikke finnes
		else if (strcmp(type,"rb") == 0) {
			if ( (FILEHANDLER = (FILE *)fopen64(File,type)) == NULL ) {
				#ifdef DEBUG
				perror(File);
				#endif
				return NULL;
			}
		}
		else {
                //temp: Bytte ut FilePath med filnavnet
                if ( (FILEHANDLER = (FILE *)fopen64(File,type)) == NULL ) {
                        makePath(FilePath);

			//hvorf�r har vi type "File" her ???, det verste er at det ser ut til � fungere ogs�
                        //if ( (FILEHANDLER = (FILE *)fopen64(File,"File")) == NULL ) {
                        if ( (FILEHANDLER = (FILE *)fopen64(File,type)) == NULL ) {
                                perror(File);
                                //exit(0);
				return NULL;
                        }
                }
		}

            	#ifdef DEBUG
                        printf("lotOpenFile: tryint to obtain lock \"%c\"\n",lock);
                #endif
                //honterer l�sning
                if (lock == 'e') {
			//skal vi ha flock64() her ?
                        flock(fileno(FILEHANDLER),LOCK_EX);
                }
                else if (lock == 's') {
                        flock(fileno(FILEHANDLER),LOCK_SH);
                }
		#ifdef DEBUG
                        printf("lotOpenFile: lock obtained\n");
                #endif
 
		#ifdef DEBUG
			printf("lotOpenFileNoCasheByLotNr: finished\n");
		#endif
                return FILEHANDLER;

}
//gir andre tilgan til lot filer. Casher opne filhandlere
FILE *lotOpenFile(unsigned int DocID,char resource[],char type[], char lock,char subname[]) {

        int LotNr;
        int i;
        char FilePath[128];
        char File [128];

	if (!LotFilesInalisert) {
		for(i=0; i < MaxOpenFiles; i++) {
			OpenFiles[i].LotNr = -1;
		}

		LotFilesInalisert = 1;
	}

        File[0] = '\0';

        //finner i hvilken lot vi skal lese fra
        LotNr = rLotForDOCid(DocID);

	//printf("LotNr: %i, DocID: %i\n",LotNr,DocID);

        //begynner med � s�ke cashen. Lopper til vi enten er ferdig, eller til vi har funne �nskede i cashen
	i = 0;
        while ((i < MaxOpenFiles) && (OpenFiles[i].LotNr != LotNr)) {
                i++;
        }
        //temp: skrur av s�king her med i=0
        //type er ogs� lagt til uten at det tar hensyn til det i cashe arrayen
        i = 0;



        //hvis vi fant i casehn returnerer vi den
        if (OpenFiles[i].LotNr == LotNr) {
                return OpenFiles[i].FILEHANDLER;
        }
        //hvis ikke opner vi og returnerer
        else {

		//hvis dette er en open filhonterer, m� vi lukke den
		if (OpenFiles[i].LotNr != -1) {
			fclose(OpenFiles[i].FILEHANDLER);
			OpenFiles[i].LotNr = -1;
			
		}
	

                 GetFilPathForLot(FilePath,LotNr,subname);
                 strcpy(File,FilePath);
                 strncat(File,resource,128);

		#ifdef DEBUG
                	printf("lotOpenFile: opening file \"%s\" for %s\n",File,type);
		#endif

                //temp: Bytte ut FilePath med filnavnet
                if ( (OpenFiles[i].FILEHANDLER = fopen(File,type)) == NULL ) {
                        makePath(FilePath);

                        if ( (OpenFiles[i].FILEHANDLER = fopen(File,"a+b")) == NULL ) {
                                perror(File);
                                exit(0);
                        }
                }

		OpenFiles[i].LotNr = LotNr;

		#ifdef DEBUG
			printf("lotOpenFile: tryint to obtain lock \"%c\"\n",lock);
		#endif
		//honterer l�sning
		if (lock == 'e') {
			flock(fileno(OpenFiles[i].FILEHANDLER),LOCK_EX);
		}
		else if (lock == 's') {
			flock(fileno(OpenFiles[i].FILEHANDLER),LOCK_SH);
		}
		#ifdef DEBUG
			printf("lotOpenFile: lock obtained\n");
		#endif


                return OpenFiles[i].FILEHANDLER;

        }
	
}

//stenger ned filer (og frigj�r l�ser)
void lotCloseFiles() {
	int i;

	printf("lotCloseFiles\n");
	if (LotFilesInalisert) {
		for(i=0; i < MaxOpenFiles; i++) {
                	if (OpenFiles[i].LotNr != -1) {
   				fclose(OpenFiles[i].FILEHANDLER);
			}
		}
	}
}

//fjerner \n p� slutten av strenger

FILE *openMaplist() {

	FILE *MAPLIST;
	char *cptr;

	//sjekker f�rst om vi har en env variabel kalt "BOITHOMAPLIST". Hvis vi har det s� bruker vi den filen
	//gj�r det slik slik at vi kan ha lokal maplist, p� hver bbs, man fortsat ha resten likt p� alle, og p� read onlu nfs.
	if ((cptr = getenv("BOITHOMAPLIST")) != NULL) {
		if ( (MAPLIST = fopen(cptr,"r")) == NULL) {
			perror(cptr);
			exit(1);
		}
	}
        //leser liten over mapper vi kan bruke.
        else if ( (MAPLIST = bfopen("config/maplist.conf","r")) == NULL) {
                perror(bfile("config/maplist.conf"));
                exit(1);
        }

	return MAPLIST;
}

/*
Laster mappene som ligger i maplist.conf og lager en oversikt over de.
*/

void MakeMapListMap () {

	FILE *MAPLIST;
	char *line;
	char buff[1024];
	short int i;

	MAPLIST = openMaplist();

	/*
	//lager mappe oversikt
	i = 0;
	while ((feof(MAPLIST) == 0) && (NrOfDataDirectorys > i)) {
		line = fgets(buff,sizeof(buff),MAPLIST);
		//line = gets(MAPLIST);
		chomp(line);
		printf("aa: -%s-\n",line);
		//leger til linjen i oversikten over mapper
		sprintf(dataDirectorys[i].Name,"%s",line);

		i++;
	}
	*/
	//bedre metode som ogs� f�r med seg linje 64
	i = 0;
	while((fgets(buff,sizeof(buff),MAPLIST) != NULL) && (NrOfDataDirectorys > i)) {
		chomp(buff);
		//printf("line -%s-\n",buff);
		sprintf(dataDirectorys[i].Name,"%s",buff);

		++i;
	}
}



// gir hav som er f�rste DocID i en lot
int GetStartDocIFForLot (int LotNr) {
	return ((LotNr * NrofDocIDsInLot) - NrofDocIDsInLot);
}

void GetFilePathForIDictionary (char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]) {

	//hvis vi ikke har inlisert mapplisten enda gj�r vi det.
	if (!MapListInitialised) {
		//printf("aaa");
		MakeMapListMap();
		MapListInitialised = 1;
	}

	//printf("dataDirectorys: %s\n",dataDirectorys[IndexNr].Name);
	if (strcmp(subname,"www") == 0) {
		sprintf(FilePath,"%s/iindex/%s/dictionary/%s/",dataDirectorys[IndexNr].Name,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);
	}
	else {
		sprintf(FilePath,"%s/iindex/%s/%s/dictionary/%s/",dataDirectorys[IndexNr].Name,subname,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);

	}
}
void GetFilePathForIindex (char *FilePath, char *FileName,int IndexNr,char Type[], char lang[],char subname[]) {

	//hvis vi ikke har inlisert mapplisten enda gj�r vi det.
	if (!MapListInitialised) {
		//printf("aaa");
		MakeMapListMap();
		MapListInitialised = 1;
	}

	//printf("dataDirectorys: %s\n",dataDirectorys[IndexNr].Name);

	if (strcmp(subname,"www") == 0) {
		sprintf(FilePath,"%s/iindex/%s/index/%s/",dataDirectorys[IndexNr].Name,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);
	}
	else {
		sprintf(FilePath,"%s/iindex/%s/%s/index/%s/",dataDirectorys[IndexNr].Name,subname,Type,lang);

		sprintf(FileName,"%s%i.txt",FilePath,IndexNr);
	}
}

void GetFilPathForLotFile(char *FilePath,char lotfile[],int LotNr,char subname[]) {

	GetFilPathForLot(FilePath,LotNr,subname);

	strcat(FilePath,lotfile);
}

/*
Gir oss pats for en lot. 
*/

void GetFilPathForLot(char *FilePath,int LotNr,char subname[]) {
	//char FilePath[64];
	int subdir;

	
	//hvis vi ikke har inlisert mapplisten enda gj�r vi det.
	if (!MapListInitialised) {
		MakeMapListMap();
		MapListInitialised = 1;
	}

	//subdir = fmod(LotNr,64);
	//subdir = LotNr % 64;

	//sprintf(FilePath,"%s/%i/%i/",dataDirectorys[subdir].Name,subdir,LotNr);

	if (strcmp(subname,"www") == 0) {
		sprintf(FilePath,"%s/%i/",dataDirectorys[LotNr % 64].Name,LotNr);
	}
	else {
		sprintf(FilePath,"%s/%i/%s/",dataDirectorys[LotNr % 64].Name,LotNr,subname);
	}
	//printf("%s\n",FilePath);
	//printf("dataDirectorys: %s\n",dataDirectorys[subdir].Name);

	

}

/*
Finner path for en lot fra docid
*/
void GetFilPathForLotByDocID(char *FilePath,int DocID,char subname[]) {

	int lot;
	lot = rLotForDOCid(DocID);
	
	GetFilPathForLot(FilePath,lot,subname);
	
}


//gir ful path for et bilde fra DocID
void GetFilPathForThumbnaleByDocID(char *FileName,int DocID,char subname[]) {

	int LotNr;
        int ImageBucket;

        ImageBucket = fmod(DocID,512);

        //finner path
        LotNr = rLotForDOCid(DocID);
        GetFilPathForLot(FileName,LotNr,subname);


        sprintf(FileName,"%simages/%i/%i.jpg",FileName,ImageBucket,DocID);

	
}


void makePath (char path[]) {
	DIR *dp;

	if ((dp = opendir(path)) == NULL) {
		bmkdir_p(path,0755);
	}
	else {
		#ifdef DEBUG
			printf("dir exsist. Wont make\n");
		#endif

		closedir(dp);
	}

/*
        int i;
        char temp[128];
        int tempnr;
        char partdir[128];
        char command[128];

        tempnr = 0;
        partdir[0] = '\0';

        //printf("gf: -%s-\n",path);
	
	sprintf(command,"mkdir -p %s",path);

	system(command);

	//printf("%s\n",command);
*/
//temp: fjerner dette gamle skrullet.
//det nye ovenfor er utestet
/*
        for (i=0; path[i] != '\0'; i++) {

                if (path[i] == '/') {
                        temp[tempnr] = '\0';

                        strncat(partdir,temp,tempnr);
                        strncat(partdir,"/",1);

                        printf("%s\n",command);

                        sprintf(command,"mkdir %s",partdir);

                        system(command);
                        tempnr = 0;
                }
                else {
                        temp[tempnr] = path[i];
                        tempnr++;
                }

        }
*/

}

