#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/re.h"
#include "../common/boithohome.h"
#include "../common/logs.h"
#include "../common/time.h"

#include "../common/iindex.h"
#include "../common/gc.h"
#include "../bbdocument/bbdocument.h"




struct gcaoptFormat {
	unsigned int MaxAgeDiflastSeen;
	int dryRun;
	FILE *log;
	FILE *logSummary;
	int keept;
	int gced;
	int lastSeenHack;
};

int
gcdecide(int LotNr, char *subname, struct gcaoptFormat *gcaopt)
{
	int i;
	
	struct reformat *re;

	#ifdef BLACK_BOKS
		time_t newest_document;
	#endif
	FILE *DOCINDEXFH;




	//�pner dokument indeks f�r � teste at vi har en, hvis ikke kan vi bare avslutte.
        if ( (DOCINDEXFH = lotOpenFileNoCasheByLotNr(LotNr,"DocumentIndex","rb", 's',subname)) == NULL) {
		#ifdef DEBUG
                	printf("lot dont have a DocumentIndex file\n");
		#endif

                return 0;
        }
	fclose(DOCINDEXFH);

	blog(gcaopt->log,1,"Runing gc for collection \"%s\", lot nr %i",subname,LotNr);

	if((re = reopen(LotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_COPYONCLOSE|RE_HAVE_4_BYTES_VERSION_PREFIX)) == NULL) {
		perror("can't reopen()");
		exit(1);
	}


	#ifdef BLACK_BOKS
		newest_document = 0;	
	#endif

	//finner nyeste dokument 
	for (i=0;i<NrofDocIDsInLot;i++) {

		#ifdef BLACK_BOKS
			if ((REN_DocumentIndex(re, i)->lastSeen != 0) && (newest_document < REN_DocumentIndex(re, i)->lastSeen)) {
        	                newest_document = REN_DocumentIndex(re, i)->lastSeen;
				//printf("newest_document: i: %i, url \"%s\", time %s\n",i,REN_DocumentIndex(re, i)->Url, ctime_s(&REN_DocumentIndex(re, i)->lastSeen));

                	}
		#endif

	}
	#ifdef BLACK_BOKS
		blog(gcaopt->log,1,"Newest document: %s",ctime_s(&newest_document));
	#endif



	//g�r gjenom alle p� jakt etter de som kan slettes
	for (i=0;i<NrofDocIDsInLot;i++) {

	
		if (DIS_isDeleted(REN_DocumentIndex(re, i))) {
			continue;
		}

		#ifdef DEBUG
			#ifdef BLACK_BOKS
				printf("dokument \"%s\", lastSeen: %s",
					REN_DocumentIndex(re, i)->Url,
					ctime_s(&REN_DocumentIndex(re, i)->lastSeen));
			#endif
		#endif

		#ifdef BLACK_BOKS
		if (	( (gcaopt->lastSeenHack == 1) && (REN_DocumentIndex(re, i)->lastSeen == 0) )
			|| ( (REN_DocumentIndex(re, i)->lastSeen != 0) && (newest_document > (REN_DocumentIndex(re, i)->lastSeen + gcaopt->MaxAgeDiflastSeen)) )
		) {


			//sletter
			DIS_delete(REN_DocumentIndex(re, i));

			//sletter dokumentet i bb spesefike ting.
			bbdocument_delete (REN_DocumentIndex(re, i)->Url, subname);

			blog(gcaopt->log,1,"dokument \"%s\" can be deleted. Last seen: %s, DocID %u",REN_DocumentIndex(re, i)->Url,ctime_s(&REN_DocumentIndex(re, i)->lastSeen),LotDocIDOfset(LotNr) +i);
			++gcaopt->gced;
		
		} 
		else {
			++gcaopt->keept;
		}
		#endif
	}

	//markerer hva vi kan slette.
	gc_reduce(re, LotNr, subname);

	//vasker iindex
        struct IndekserOptFormat IndekserOpt;
        IndekserOpt.optMustBeNewerThen = 0;
        IndekserOpt.optAllowDuplicates = 0;
        IndekserOpt.optValidDocIDs = NULL;
        IndekserOpt.sequenceMode =1;
        IndekserOpt.garbareCollection = 1;

	for (i=0;i<64;i++) {
		Indekser(LotNr,"Main",i,subname,&IndekserOpt);
	}
	for (i=0;i<64;i++) {
		Indekser(LotNr,"acl_allow",i,subname,&IndekserOpt);
	}
	for (i=0;i<64;i++) {
		Indekser(LotNr,"acl_denied",i,subname,&IndekserOpt);
	}

	//siden vi n� har lagt til alle andringer fra rev index kan vi n� slettet gced filen ogs�
	Indekser_deleteGcedFile(LotNr, subname);

	reclose(re);

	return 0;
}


FILE *lockcoll(char subname[]) {

	char lockfile[512];
	FILE *LOCK;

        //oppretter var mappen hvis den ikke finnes. Dette slik at vi slipper og gj�re dette under instalsjonen
        bmkdir_p(bfile("var/"),0755);

        sprintf(lockfile,"var/boitho-collections-%s.lock",subname);

        printf("locking lock \"%s\"\n",lockfile);

        if ((LOCK = bfopen(lockfile,"w+")) == NULL) {
                perror(lockfile);
                return NULL;
        }

        //geting the lock. 
        if (flock(fileno(LOCK),LOCK_EX) != 0) {
                fclose(LOCK);
                return NULL;
        }


	return LOCK;
        

}

void gc_coll(char subname[], struct gcaoptFormat *gcaopt) {

	int LotNr, i;
	unsigned int DocIDcount = 0;
	FILE *LOCK;

	gcaopt->keept = 0;
	gcaopt->gced = 0;

	if ((LOCK = lockcoll(subname)) == NULL) {
		fprintf(stderr,"Can't lock lockfile!\n");
		exit(-1);
	}

	for(LotNr=1;LotNr<maxLots;LotNr++) {
		gcdecide(LotNr,subname, gcaopt);
	}

	/***************************/
	//merger indexene
        //skal lage for alle b�ttene
        for (i=0;i<NrOfDataDirectorys;i++) {
        	printf("gc_coll: bucket: %i\n",i);
		mergei(i,0,0,"Main","aa",subname,&DocIDcount);
        }

        for (i=0;i<NrOfDataDirectorys;i++) {
        	printf("gc_coll: bucket: %i\n",i);
		mergei(i,0,0,"acl_allow","aa",subname,&DocIDcount);
	}

        for (i=0;i<NrOfDataDirectorys;i++) {
        	printf("gc_coll: bucket: %i\n",i);
		mergei(i,0,0,"acl_denied","aa",subname,&DocIDcount);
	}

        printf("DocIDcount: %i (/64)\n",DocIDcount);

	/***************************/
	blog(gcaopt->log,1,"gc'ed \"%s\". Keept %i, gced %i",subname,gcaopt->keept,gcaopt->gced);
	blog(gcaopt->logSummary,1,"gc'ed \"%s\". Keept %i, gced %i",subname,gcaopt->keept,gcaopt->gced);

	fclose(LOCK);
}

int
main(int argc, char **argv)
{

	int LotNr, i;
	char *subname;
	struct gcaoptFormat gcaopt;

	gcaopt.MaxAgeDiflastSeen  = 86400;
	gcaopt.dryRun  = 0;
	gcaopt.log = NULL;
	gcaopt.logSummary = NULL;
	gcaopt.lastSeenHack = 0;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"t:dls"))!=-1) {
                switch (c) {
                        case 't':
                                gcaopt.MaxAgeDiflastSeen  = atou(optarg);
                                break;
			case 'd':
				gcaopt.dryRun = 1;
				break;
			case 'l':
				gcaopt.log = fopen(bfile("logs/gc"),"ab");
				gcaopt.logSummary = fopen(bfile("logs/gcSummary"),"ab");
				break;
			case 's':
				gcaopt.lastSeenHack = 1;
				break;
                        default:
                                exit(1);
                }

        }
        --optind;



	DIR *ll;


	#ifndef BLACK_BOKS
		fprintf("dette fungerer bare med black boks for n�\n");
		exit(1);
	#endif

	if ((argc -optind) == 3) {
		LotNr = atoi(argv[2 +optind]);

		fprintf(stderr, "St�test ikke for n�. Mangler subname og lock file\n");
		exit(-1);

		gcaopt.keept = 0;
		gcaopt.gced = 0;

		gcdecide(LotNr,subname, &gcaopt);

		blog(gcaopt.log,1,"gc'ed \"%s\". Keept %i, gced %i",subname,gcaopt.keept,gcaopt.gced);
		blog(gcaopt.logSummary,1,"gc'ed \"%s\". Keept %i, gced %i",subname,gcaopt.keept,gcaopt.gced);

	}
	else if ((argc -optind) == 2) {
		subname = argv[1 +optind];

		gc_coll(subname, &gcaopt);


	}
	else if ((argc -optind) == 1) {

		ll = listAllColl_start();

		while((subname = listAllColl_next(ll)) != NULL) {
			printf("indexing collection \"%s\"\n",subname);

			gc_coll(subname, &gcaopt);

		}

		listAllColl_close(ll);

	}
	else {
		errx(1, "Usage: ./gcrepo subname [ lotnr ]");
	}



	return 0;
}

