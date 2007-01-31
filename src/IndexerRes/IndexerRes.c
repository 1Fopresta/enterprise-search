#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "../common/langdetect.h"
#include "../common/langToNr.h"
#include "../IndexerRes/IndexerRes.h"
#include "../parser/html_parser.h"



void wordsReset() {

	pagewords.nr = 0;
	pagewords.nextPosition = 0;	
	pagewords.revIndexnr = 0;
	pagewords.nrOfOutLinks = 0;
}

void linkadd(char word[]) {

		int i;

                struct updateFormat updatePost;
		static char lasturl[201] = {""};

		//�ker oversikten over antall utg�ende linker
		++pagewords.nrOfOutLinks;

		//if (global_source_url_havpri) {
		//	printf("%s\n",word);
		//}
		if (!globalIndexerLotConfig.collectUrls) {
			//skal ikke lagge til urler
		}
		else if (strcmp(lasturl,word) == 0) {
			//gider ikke legge til like url. Men hva hvis dette er p� en ny side. Denne burde bli resettet
		}
		else if ((strchr(word,'?') != NULL) && (global_curentUrlIsDynamic)) {
                        //printf("NO add %s\n",word);
                }
		else if (gyldig_url(word)) {

			
			//printf("ADD %s\n",word);
                      
			strncpy((char *)updatePost.url,word,sizeof(updatePost.url));
                        strncpy((char *)updatePost.linktext,"",sizeof(updatePost.linktext));
                        updatePost.DocID_from = global_curentDocID;

			if (globalIndexerLotConfig.urlfilter == NULL) {
                        	addNewUrl(&global_addNewUrlha_pri1,&updatePost,"_pri1",subname);
			}
			else {
				//tester filterer om vi skal legge til en url
				i=0;
				while( (globalIndexerLotConfig.urlfilter[i] != NULL) ) {
	                                //printf("\t\t%i\tis ttl \"%s\" in url \"%s\"\n", i, globalIndexerLotConfig.urlfilter[i],word);
					if (url_isttl(word,globalIndexerLotConfig.urlfilter[i])) {
						//printf("added url\n");
						addNewUrl(&global_addNewUrlha_pri1,&updatePost,"_pri1",subname);
						break;
					}
					++i;
                                }				
			}
			/*

			if (url_havpri1(word) || global_source_url_havpri) {
				//printf("hav pri %s\n",word);
                        	addNewUrl(&global_addNewUrlha_pri1,&updatePost,"_pri1",subname);
			}

			temp: skrur av crawling av ikke pri sider
			else if (url_havpri2(word)) {
				//printf("source_url_havpri %s\n",word);
                                addNewUrl(&global_addNewUrlha_pri2,&updatePost,"_pri2",subname);
			}
			else {
				//printf("normal %s\n",word);
                        	addNewUrl(&global_addNewUrlha,&updatePost,"",subname);

			}
			*/

                }
		else {
			//printf("NO ADD %s\n",word);
		}

		strncpy(lasturl,word,sizeof(lasturl));

}


void wordsAdd(char word[],enum parsed_unit_flag puf) {
int i;
int wordlLength;
int wordTypeadd;
			if (pagewords.nr > maxWordForPage){
				#ifdef DEBUG
					printf("mor then maxWordForPage words\n");
				#endif
			}
			else {

				switch (puf)
                        	{
                            		case puf_none: 
						//printf(" +p"); 
						wordTypeadd=1000;break;
                            		case puf_title: 
						//printf(" +title"); 
						wordTypeadd=100; break;
                            		case puf_h1: 
						//printf(" +h1"); 
						wordTypeadd=500; break;
                            		case puf_h2: 
						//printf(" +h2"); 
						wordTypeadd=500; break;
                            		case puf_h3: 
						//printf(" +h3"); 
						wordTypeadd=500; break;
                            		case puf_h4: 
						//printf(" +h4"); 
						wordTypeadd=500; break;
                            		case puf_h5: 
						//printf(" +h5"); 
						wordTypeadd=500; break;
                            		case puf_h6: 
						//printf(" +h6"); 
						wordTypeadd=500; break;
					default:
						printf(" no catsh\n"); break;
                        	}

				wordlLength = strlen(word);

				//gj�r om til sm� bokstaver
				for(i=0;i<wordlLength;i++) {
					word[i] = (char)btolower(word[i]);
				}


				#ifdef DEBUG_ADULT
					strcpy(pagewords.words[pagewords.nr].word,word);
				#endif
				pagewords.words[pagewords.nr].WordID =  crc32boitho(word);
				pagewords.words[pagewords.nr].position = (pagewords.nextPosition + wordTypeadd);
				// m� ha en index posisjon her. Slik at vi kan finne ord f�r og etter. Posisjon er koded
				pagewords.words[pagewords.nr].unsortetIndexPosition = pagewords.nr;

				//printf("nextPosition %i, wordTypeadd %i, position %i\n",pagewords.nextPosition,wordTypeadd,pagewords.words[pagewords.nr].position);

				++pagewords.nextPosition;

				//printf("%s : %lu\n",word,pagewords.words[pagewords.nr]);

				++pagewords.nr;		
			}
}

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf , void *largs)
{


	struct pagewordsFormat *pagewords = (struct pagewordsFormat *)largs;

	#ifdef DEBUG
    		printf("\t%s (%i) ", word, pos);
	#endif
    switch (pu)
        {
            case pu_word: 

			wordsAdd(word,puf);

			#ifdef DEBUG
	    			switch (puf)
        			{
        			    	case puf_none: printf(" none"); break;
        			    	case puf_title: printf(" +title"); break;
        		    		case puf_h1: printf(" +h1"); break;
        		    		case puf_h2: printf(" +h2"); break;
        		    		case puf_h3: printf(" +h3"); break;
        		    		case puf_h4: printf(" +h4"); break;
        		    		case puf_h5: printf(" +h5"); break;
        		    		case puf_h6: printf(" +h6"); break;
		        	}

				printf("[word] is now %s ",word); 
			#endif


		break;
            case pu_linkword: 
			#ifdef DEBUG
				printf("[linkword]"); 
			#endif
		break;
            case pu_link:

			
			linkadd(word);
			#ifdef DEBUG 
				printf("[link]"); 
			#endif
		break;
            case pu_baselink:
			#ifdef DEBUG 
				printf("[baselink]");
			#endif 
		break;
            case pu_meta_keywords: 
			#ifdef DEBUG
				printf("[meta keywords]"); 
			#endif
			break;
            case pu_meta_description:
			#ifdef DEBUG 
				printf("[meta description]"); 
			#endif
			break;
            case pu_meta_author:
			#ifdef DEBUG 
				printf("[meta author]");
			#endif 
			break;
            default: printf("[...]");
        }

	#ifdef DEBUG
    		printf("\n");
	#endif

}


int compare_elements_words (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct wordsFormat*)p1)->WordID < ((struct wordsFormat*)p2)->WordID)
                return -1;
        else
                return ((struct wordsFormat*)p1)->WordID > ((struct wordsFormat*)p2)->WordID;

}

int compare_elements_nr (const void *p1, const void *p2) {


        if (((struct revIndexFomat*)p1)->nr < ((struct revIndexFomat*)p2)->nr)
                return -1;
        else
                return ((struct revIndexFomat*)p1)->nr > ((struct revIndexFomat*)p2)->nr;

}



static int cmp1_crc32(const void *p, const void *q)
{
	printf("%lu %lu\n",*(const unsigned long *) p,*(const unsigned long *) q);

   return *(const unsigned long *) p - *(const unsigned long *) q;
}

void pagewordsSortOnOccurrence(){
	qsort(&pagewords.revIndex, pagewords.revIndexnr , sizeof(struct revIndexFomat), compare_elements_nr);

}
void wordsMakeRevIndex(struct adultFormat *adult,int *adultWeight, unsigned char *langnr) {

	int i,y,adultpos,adultFraserpos;
	unsigned long oldcrc32;
	int oldRevIndexnr;

	//kopierer over til den word arrayn som skal v�re sortert
	for(i=0;i<pagewords.nr;i++) {
		//pagewords.words_sorted[i].WordID = pagewords.words[i].WordID;
		//pagewords.words_sorted[i].position = pagewords.words[i].position;
		pagewords.words_sorted[i] = pagewords.words[i];
		#ifdef DEBUG_ADULT
			strcpy(pagewords.words_sorted[i].word,pagewords.words[i].word);
		#endif
	}

	//sorter ordene
	qsort(&pagewords.words_sorted, pagewords.nr , sizeof(struct wordsFormat), compare_elements_words);

	//finner spr�k
	char lang[4];
	langdetectDetect(pagewords.words_sorted,pagewords.nr,lang);

	*langnr = getLangNr(lang);

	oldcrc32 = 0;	
	adultpos = 0;
	adultFraserpos = 0;
	(*adultWeight) = 0;
	pagewords.revIndexnr = 0;
	for(i=0;i<pagewords.nr;i++) {
		
		//printf("oo: %lu %i\n",pagewords.words_sorted[i].WordID,pagewords.words_sorted[i].position);

		//printf("t%i\n",pagewords.revIndexnr);

		if ((pagewords.words_sorted[i].WordID != oldcrc32) || (oldcrc32 == 0)) {
			// nytt ord, skal lages nytt
			#ifdef DEBUG_ADULT
				printf("new word. Word \"%s\"\n",pagewords.words_sorted[i].word);
			#endif
			#ifdef DEBUG
				printf("new word. WordID \"%u\"\n",pagewords.words_sorted[i].WordID);
			#endif

			pagewords.revIndex[pagewords.revIndexnr].WordID = pagewords.words_sorted[i].WordID;
			pagewords.revIndex[pagewords.revIndexnr].nr = 0;
			pagewords.revIndex[pagewords.revIndexnr].hits[pagewords.revIndex[pagewords.revIndexnr].nr] = pagewords.words_sorted[i].position;

			#ifdef DEBUG_ADULT			
				strcpy(pagewords.revIndex[pagewords.revIndexnr].word,pagewords.words_sorted[i].word);
				//printf("word %lu %s\n",pagewords.revIndex[pagewords.revIndexnr].WordID,pagewords.words_sorted[i].word);
			#endif

			///////////////////////////
			// adultWords


			while ((pagewords.revIndex[pagewords.revIndexnr].WordID 
				> (*adult).AdultWords[adultpos].crc32) && (adultpos < (*adult).adultWordnr)) {
				#ifdef DEBUG
					printf("testing for %lu %s\n",(*adult).AdultWords[adultpos].crc32,(*adult).AdultWords[adultpos].word);
				#endif
				++adultpos;
			}
			//printf("ll: adultpos %i < adultWordnr %i\n",adultpos,(*adult).adultWordnr);
			if ((*adult).AdultWords[adultpos].crc32 == pagewords.revIndex[pagewords.revIndexnr].WordID) {
				#if DEBUG_ADULT
					printf("adult hit \"%s\" %i\n",(*adult).AdultWords[adultpos].word,(*adult).AdultWords[adultpos].weight);
				#endif
				(*adultWeight) += (*adult).AdultWords[adultpos].weight;
				//printf("adultWeight1 %u\n",(*adultWeight));
				//printf("weight %i\n",(*adult).AdultWords[adultpos].weight);

				//exit(1);
			}
			///////////////////////////
			//adult fraser

			while ((pagewords.revIndex[pagewords.revIndexnr].WordID > (*adult).adultFraser[adultFraserpos].crc32) && (adultFraserpos < (*adult).adultWordFrasernr)) {
				#ifdef DEBUG
					//printf("s %lu %s\n",(*adult).adultFraser[adultFraserpos].crc32,(*adult).adultFraser[adultFraserpos].word);
				#endif
				++adultFraserpos;
			}
			if ((*adult).adultFraser[adultFraserpos].crc32 == pagewords.revIndex[pagewords.revIndexnr].WordID) {
				
				//har hitt. Lopper gjenom word2'ene p� jakt etter ord nr to
				#ifdef DEBUG_ADULT
					printf("frase hit %s\n",(*adult).adultFraser[adultFraserpos].word);
					printf("word pos %i\n",pagewords.words_sorted[i].unsortetIndexPosition);
					printf("bb nex word is \"%s\"\n",pagewords.words[(pagewords.words_sorted[i].unsortetIndexPosition +1)].word);
					printf("x words to try %i, pos %i\n",(*adult).adultFraser[adultFraserpos].adultWordCount,
								adultFraserpos);
				#endif

				for(y=0;y<(*adult).adultFraser[adultFraserpos].adultWordCount;y++) {

					//if ((*adult).adultFraser[adultFraserpos].adultWord[y].crc32 == pagewords.words[pagewords.words_sorted[i].position +1].WordID) {
					if ((*adult).adultFraser[adultFraserpos].adultWord[y].crc32 == pagewords.words[(pagewords.words_sorted[i].unsortetIndexPosition +1)].WordID) {
						#ifdef DEBUG_ADULT
							printf("frase hit \"%s %s\", weight %i\n",pagewords.words_sorted[i].word,(*adult).adultFraser[adultFraserpos].adultWord[y].word,(*adult).adultFraser[adultFraserpos].adultWord[y].weight);
						#endif
						//�ker adult verdien med vekt
						(*adultWeight) += (*adult).adultFraser[adultFraserpos].adultWord[y].weight;
						//printf("adultWeight2 %u\n",(*adultWeight));
						break;
					}
				}

			}

			///////////////////////////

			oldRevIndexnr = pagewords.revIndexnr;
			//har f�t et til ord i revindex
			++pagewords.revIndex[pagewords.revIndexnr].nr;

			//printf("b%i\n\n",pagewords.revIndexnr);
			++pagewords.revIndexnr;
		}
		else {
			#ifdef DEBUG
				printf("word seen befor. Adding. WordID \"%u\"\n",pagewords.words_sorted[i].WordID);
			#endif
			// ord er set f�r, skal legges til det tidligere
			// m� passe p� at vi ikke overskriver med flere hits en MaxsHits
			if (pagewords.revIndex[oldRevIndexnr].nr < MaxsHitsInIndex) {
				pagewords.revIndex[oldRevIndexnr].hits[pagewords.revIndex[oldRevIndexnr].nr] = pagewords.words_sorted[i].position;
				++pagewords.revIndex[oldRevIndexnr].nr;
			}
			else {
				#ifdef DEBUG
				printf("to many hits for WordID \"%u\"\n",pagewords.words_sorted[i].WordID);
				#endif
			}
			//break;

		}

		oldcrc32 = pagewords.words_sorted[i].WordID;

	
	}

}


int compare_elements_adultWord (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct adultWordFormat*)p1)->crc32 < ((struct adultWordFormat*)p2)->crc32)
                return -1;
        else
                return ((struct adultWordFormat*)p1)->crc32 > ((struct adultWordFormat*)p2)->crc32;

}

int compare_elements_AdultFraser (const void *p1, const void *p2) {

//        struct iindexFormat *t1 = (struct iindexFormat*)p1;
//        struct iindexFormat *t2 = (struct iindexFormat*)p2;

        if (((struct adultWordFraserFormat*)p1)->crc32 < ((struct adultWordFraserFormat*)p2)->crc32)
                return -1;
        else
                return ((struct adultWordFraserFormat*)p1)->crc32 > ((struct adultWordFraserFormat*)p2)->crc32;

}



void adultLoad (struct adultFormat *adult) {

	FILE *FH;
	char buff[128];
	int i,y,x;
	char *cpoint;
	char word1[128];
	char word2[128];	
	int weight;
	unsigned long crc32tmp;

	//AdultWordsFile
	if ((FH = bfopen(AdultWordsVektetFile,"r")) == NULL) {
                        perror(AdultWordsVektetFile);
                        exit(1);
	}

	i=0;
	while ((fgets(buff,sizeof(buff),FH) != NULL) && (i < maxAdultWords)) {
		//fjerner \n en p� slutteten
		buff[strlen(buff) -1] = '\0';

		if ((buff[0] == '#') || (buff[0] == '\0')) {
			//printf("bad line or comment: %s\n",buff);
			continue;
		}

		//gj�r om til lite case
		for(x=0;x<strlen(buff);x++) {
			buff[x] = btolower(buff[x]);
		}

		//finner space, som er det som skiller
                cpoint = strchr(buff,' ');
		if (cpoint != NULL) {

			strncpy((*adult).AdultWords[i].word,buff,cpoint - buff);
			//vil ikke ha men spacen. G�r et hakk vidre
			++cpoint;
			(*adult).AdultWords[i].weight = atoi(cpoint);
	
			(*adult).AdultWords[i].crc32 = crc32boitho((*adult).AdultWords[i].word);

		}		

		//(*adult).AdultWords[i].word[strlen((*adult).AdultWords[i].word) -1] = '\0';

		//printf("%i: -%s- %lu %i\n",i,(*adult).AdultWords[i].word,(*adult).AdultWords[i].crc32,(*adult).AdultWords[i].weight);
		++i;
	}
	if (maxAdultWords == i) {
		printf("Adult list is larger then maxAdultWords (%i)\n",maxAdultWords);
		exit(1);
	}

	(*adult).adultWordnr = i;

	fclose(FH);

	qsort((*adult).AdultWords, i , sizeof(struct adultWordFormat), compare_elements_adultWord);	

//debug: vis alle ordene, sortert
//	for(y=0;y<i;y++) {
//		printf("%i: -%s- %lu %i\n",y,(*adult).AdultWords[y].word,(*adult).AdultWords[y].crc32,(*adult).AdultWords[y].weight);
//	}

	for(i=0;i<maxAdultWords;i++) {
		(*adult).adultFraser[i].adultWordCount = 0;
	}


	//AdultFraserFile
	if ((FH = bfopen(AdultFraserVektetFile,"r")) == NULL) {
                        perror(AdultFraserVektetFile);
                        exit(1);
	}

	i=-1;
	while ((fgets(buff,sizeof(buff) -1,FH) != NULL) && (i < maxAdultWords)) {
                //gj�r om til lite case
                for(x=0;x<strlen(buff);x++) {
                        buff[x] = btolower(buff[x]);
                }

		//printf("buff %s\n",buff);
		if ((x=sscanf(buff,"%s %s %i\n",word1,word2,&weight))!=3) {
			
			printf("bad AdultFraserVektetFile format: \"%s\" . x: %i\n",buff, x);

		}
		else {

			//printf("%i: %s, %s, %i\n",i,word1,word2,weight);
	
			//finner crc32 verdeien for f�rste ord
			crc32tmp = crc32boitho(word1);

			//hvsi dette er f�rste s� her vi ikke noen forige � legge den til i, s� vi m� opprette ny
			//hvsi dette derimot har samme word1 som forige s� legger vi det til
			if ((i!=-1) && (crc32tmp == (*adult).adultFraser[i].crc32)) {
				//printf("nr to\n");
			}
			else {
				++i;
			}
		

			strcpy((*adult).adultFraser[i].word,word1);
			(*adult).adultFraser[i].crc32 = crc32boitho(word1);		

			(*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].weight = weight;
			strcpy((*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].word,word2);
			(*adult).adultFraser[i].adultWord[(*adult).adultFraser[i].adultWordCount].crc32 = crc32boitho(word2);

			if ((*adult).adultFraser[i].adultWordCount < MaxAdultWordCount -1) {
				++(*adult).adultFraser[i].adultWordCount;
			}
			else {
				printf("MaxAdultWordCount %i for %s\n",MaxAdultWordCount,buff);
				exit(1);
			}

			



		}
	}
	fclose(FH);

	(*adult).adultWordFrasernr = i;
	qsort((*adult).adultFraser, (*adult).adultWordFrasernr , sizeof(struct adultWordFraserFormat), compare_elements_AdultFraser);

/*
	for(i=0;i<(*adult).adultWordFrasernr;i++) {
		printf("%i, -%s-, nr %i\n",i,(*adult).adultFraser[i].word,(*adult).adultFraser[i].adultWordCount);

		for(y=0;y<(*adult).adultFraser[i].adultWordCount;y++) {
			printf("\t %i: %s-%s: %i\n",y,(*adult).adultFraser[i].word,(*adult).adultFraser[i].adultWord[y].word,(*adult).adultFraser[i].adultWord[y].weight);
		}
		

	}
*/

}

void revindexFilesOpenNET(FILE *revindexFilesHa[]) {
	int i;

	for(i=0;i<NrOfDataDirectorys;i++) {
		if ((revindexFilesHa[i] = tmpfile()) == NULL) {
			perror("tmpfile");
			exit(1);
		}
	}
}
/**************************************************************************************
Sender revindex filene til en server som kj�rer boithold
***************************************************************************************/
void revindexFilesSendNET(FILE *revindexFilesHa[],int lotNr) {
	int i;
	char dest[64];

	for(i=0;i<NrOfDataDirectorys;i++) {

		sprintf(dest,"revindex/Main/%i.txt",i);

		rSendFileByOpenHandler(revindexFilesHa[i],dest,lotNr,"a",subname);
	}
}
/**************************************************************************************
Skriver reversert index til disk
***************************************************************************************/
void revindexFilesAppendWords(FILE *revindexFilesHa[],unsigned int DocID,unsigned char *langnr) {

	int i,y;
	int bucket;

	/*	
	//skriver f�rst en hedder til alle filene
	for(i=0;i<NrOfDataDirectorys;i++) {
		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[i]);
		// skriver 3 tegn av spr�ket. Er det vi bruker
		//fwrite(&lang,sizeof(char),3,revindexFilesHa[i]);
		//temp, lattlige spr�kproblemer her :(
		fprintf(revindexFilesHa[i],"aa ");
	}
	*/

	for(i=0;i<pagewords.revIndexnr;i++) {


		bucket = pagewords.revIndex[i].WordID % NrOfDataDirectorys;

		#ifdef DEBUG
			printf("WordID %lu forekomster %i (+1). bucket %i\n",pagewords.revIndex[i].WordID,pagewords.revIndex[i].nr,bucket);
		#endif

		//++pagewords.revIndex[i].nr;
		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[bucket]);
		//fprintf(revindexFilesHa[bucket],"aa ");
		fwrite(langnr,sizeof(char),1,revindexFilesHa[bucket]);
		//printf("lang1 %i%\n",(int)*langnr);

		fwrite(&pagewords.revIndex[i].WordID,sizeof(unsigned long),1,revindexFilesHa[bucket]);	
		fwrite(&pagewords.revIndex[i].nr,sizeof(unsigned long),1,revindexFilesHa[bucket]);

		for(y=0;y<pagewords.revIndex[i].nr;y++) {
			//printf("\thits %i\n",pagewords.revIndex[i].hits[y]);
			fwrite(&pagewords.revIndex[i].hits[y],sizeof(unsigned short),1,revindexFilesHa[bucket]);
		}

	}	

	/*
	//skriver record terminator
	for(i=0;i<NrOfDataDirectorys;i++) {
		//ToDo: her bruker vi \n for linefeed, men bruker \cJ i perl. P� andre platformer en *nix vil det f�re til problmer
		//	erstatt \n med tegnet for linefeed
		fprintf(revindexFilesHa[i],"**\n");
	}
	*/
}

void copyRepToDi(struct DocumentIndexFormat *DocumentIndexPost,struct ReposetoryHeaderFormat *ReposetoryHeader) {
			strcpy((*DocumentIndexPost).Url,(*ReposetoryHeader).url);
			
			strcpy((*DocumentIndexPost).Dokumenttype,(*ReposetoryHeader).content_type);

			(*DocumentIndexPost).IPAddress 		= (*ReposetoryHeader).IPAddress;
			(*DocumentIndexPost).response 		= (*ReposetoryHeader).response;
			(*DocumentIndexPost).htmlSize 		= (*ReposetoryHeader).htmlSize;
			(*DocumentIndexPost).imageSize 		= (*ReposetoryHeader).imageSize;
			(*DocumentIndexPost).userID 		= (*ReposetoryHeader).userID;
			(*DocumentIndexPost).clientVersion 	= (*ReposetoryHeader).clientVersion;
			(*DocumentIndexPost).CrawleDato 	= (*ReposetoryHeader).time;

			(*DocumentIndexPost).ResourcePointer 	= 0;
			(*DocumentIndexPost).ResourceSize 	= 0;

}

void html_parser_timout( int signo )
{
    if ( signo == SIGALRM ) {
        printf("got alarm\n");
        alarm_got_raised = 1;
    }
 

}


void handelPage(char lotServer[], unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char HtmlBuffer[],int HtmlBufferLength,char imagebuffebuffer[],FILE *revindexFilesHa[],struct DocumentIndexFormat *DocumentIndexPost, int DocID,int httpResponsCodes[], struct adultFormat *adult, unsigned char *langnr) {

		int AdultWeight;

		if ((*ReposetoryHeader).response < nrOfHttpResponsCodes) {
			++httpResponsCodes[(*ReposetoryHeader).response];
		}

		char *title, *body;

		//printf("%lu %s\n",(*ReposetoryHeader).DocID, (*ReposetoryHeader).url);

//		if (((*ReposetoryHeader).response >= 200) && ((*ReposetoryHeader).response <= 299)) {

		

			
				//if ((*ReposetoryHeader).DocID == 363093661) {
				//	FILE *FH;
				//	FH = fopen("/tmp/IndexerLotdump.html","wb");
				//	fwrite(HtmlBuffer,HtmlBufferSize,1,FH);
				//	fclose(FH);
				//	exit(1);
				//}
		
				
				//begynner p� en ny side
				wordsReset();

				if (strcmp((*ReposetoryHeader).content_type,"htm") == 0) {

					//setter opp en alarm slik at run_html_parser blir avbrut hvis den henger seg 
					alarm_got_raised = 0;
					signal(SIGALRM, html_parser_timout);
					
					alarm( 5 );
					//parser htmlen
					//run_html_parser( (*ReposetoryHeader).url, HtmlBuffer, HtmlBufferLength, fn );

					html_parser_run((*ReposetoryHeader).url, HtmlBuffer, HtmlBufferLength,&title,&body,fn,NULL);

					alarm( 0);
					if(alarm_got_raised) {
						printf("run_html_parser did time out. At DocID %lu\n",(*ReposetoryHeader).DocID);
					}
					else {

						wordsMakeRevIndex(adult,&AdultWeight,langnr);

						if (AdultWeight > 255) {
							(*DocumentIndexPost).AdultWeight = 255;
						}
						else {
							(*DocumentIndexPost).AdultWeight = AdultWeight;
						}

						revindexFilesAppendWords(revindexFilesHa,(*ReposetoryHeader).DocID,langnr);
					}
				
				}
				else if (strcmp((*ReposetoryHeader).content_type,"btx") == 0) {
					//ToDo: definer Boitho TeXt formatet
				}
				else {
					printf("unknown content_type \"%s\"\n",(*ReposetoryHeader).content_type);
				}


//			}

//		}
//		else { //not in 200->299 range
//			//radress m� vel ikke nulles her, like g�r � ha den pekene til strukturen i reposetoryet 
//			HtmlBufferSize = 0;
//		}

			
		free(title);
		free(body);

}





