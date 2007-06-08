#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "debug.h"
#include "bstr.h"


int find_domain_path_len (char url[]) {


        char *cpnt;

        //s�ker oss til / og kapper
        if ((cpnt = strchr(url + 7,'/')) == NULL) {
                //printf("bad url\n");
                return 0;
        }
        else {
                #ifdef DEBUG
                printf("aa %s, len %i\n",cpnt,strlen(cpnt));
                #endif

                return strlen(cpnt);
        }
}


int isWikiUrl(char url[]) {

	if (strstr(url,"/wiki/") != NULL) {
		#ifdef DEBUG
		printf("is wiki: \"%s\"\n",url);
		#endif
		return 1;
	}
	else {
		return 0;
	}

}

int strchrcount(char s[], char c) {

	char *cpnt = s;
	int count = 0;

	while ((cpnt = strchr(cpnt,c)) != NULL) {
		++cpnt;
		++count;
	}

	return count;
}

int url_normalization (char url[], int urlsize) {

	char domain[200];
	char *cpnt;


	//pr�ver � finne domener uten trailing slash
        //kutter av http:// (de 7 f�rste tegnene)
        strscpy(domain,url + 7,sizeof(domain));

        //s�ker oss til / , hvis vi ikke har har vi nokk en domene skrevet som www.boitho.com.
	//vi �nsker da og legge p� / for og f� www.boitho.com/
	// kan ogs� v�re www.boitho.com?query=chat. S� hvis det er ? i den kan vi ikke bare apende en /
        if ( ((cpnt = strchr(domain,'/')) == NULL) && ((cpnt = strchr(domain,'?')) == NULL)) {
                //printf("bad url\n");
                //return 0;
		//printf("no trailing slash. Url \"%s\"\n",url);
		strlcat(url,"/",urlsize);
		//printf("no trailing slash. revriten to Url \"%s\"\n",url);
		
        }

}

int gyldig_url(char word[]) {
                static char lasturl[201] = {""};
                int lengt;
                char *cpnt;
                char *filetype;
		int querylength;
		struct updateFormat updatePost;
		char domain[128];
		int subdomains;
		int domainNrOfLines;

                lengt = strlen(word);

		if (!find_domain(word,domain,sizeof(domain)) ) {
			debug("gyldig_url: can't find domain. Url \"%s\"\n",word);
			return 0;
		}

		//teller subdomener
		subdomains = strchrcount(domain,'.');
		

		domainNrOfLines = strchrcount(domain,'-');

		//printf("url: \"%s\", subdomains = %i, domainNrOfLines = %i\n",word,subdomains,domainNrOfLines);
		
                //finner siste bokstav
                if (word[lengt -1] == '/') {
                        filetype = &word[lengt -1];
                }
                else {
                        if ((filetype = strrchr(word,'.')) == NULL){
                                filetype = NULL;
                        }
                }
                        //printf("filetype %s\n",filetype);

		//finner lengde p� query. Alts� det som er etter ?
		querylength = 0;
		if ((cpnt = strstr(word,"?")) != NULL) {
			querylength = strlen(cpnt +1); //+1 da vi ikke skal ha med ?
		}
		//printf("url \"%s\", querylength %i\n",word,querylength);


                //temp: for n� tar vi bare korte urler
                if (lengt > 150) {
			debug("To long. Was %i bytes",lengt);
			return 0;
                }
		else if (querylength > 21) {
			debug("To long query. Was %i bytes",querylength);
			return 0;
		}
		else if (subdomains == 1) {
			//if not will we index boitho.com and www.boitho.com. Now are we only takint he www.boitho"
			debug("To few subdomains. Most have one. Url: \"%s\"\n",word);
			return 0;
		}
		else if (domainNrOfLines >= 3) {
			debug("To many lines in domain. Was %i lines. Url: \"%s\"\n",domainNrOfLines,word);
			return 0;
		}
		else if (word[lengt -1] == '\n') {
			debug("url has \\n as last char");
			return 0;
		}
                else if (filetype == NULL) {
                        debug("bad url. filetype is NULL");
			return 0;
                }
                else if (
                        !(
                        (strcmp(filetype,"/") == 0)
                        || (strstr(filetype,"?") != NULL)
                        || (strcmp(filetype,".cfm") == 0)
                        || (strcmp(filetype,".htm") == 0)
                        || (strcmp(filetype,".html") == 0)
                        || (strcmp(filetype,".shtml") == 0)
                        || (strcmp(filetype,".shtm") == 0)
                        || (strcmp(filetype,".php") == 0)
                        || (strcmp(filetype,".asp") == 0)
                        || (strcmp(filetype,".stm") == 0)
                        || (strcmp(filetype,".ece") == 0)
                        || (strcmp(filetype,".aspx") == 0)
                        || (strcmp(filetype,".do") == 0)
                        || (strcmp(filetype,".jsp") == 0)
			|| isWikiUrl(word)
                        )
                ) {
                        //tar bar de filendelsene vi vil ha
                        debug("bad ending \"%s\"",filetype);
			return 0;
                }
                else if (strchr(word,'#') != NULL) {
                        //printf("has #: %s\n",word);
			return 0;
                }
                else if (strchr(word,'@') != NULL) {
                        //printf("has @: %s\n",word);
			return 0;
                }
                else if (strchr(word,'%') != NULL) {
                        //printf("has %: %s\n",word);
			return 0;
                }
                else if (strcmp(word,lasturl) == 0) {
                        debug("last and ne the same %s == %s",word,lasturl);
			return 0;
                }
		else if (strstr(word,"..") != NULL) {
			debug("hav ..");
			return 0;
		}
		else if (strstr(word,"/./") != NULL) {
			debug("hav /./");
			return 0;
		}
		else if (strstr((word + 7),"http:/") != NULL) {
			//for � hindre http://go.kvasir.sol.no/fs/http:/www.nettfinn.no/
			debug("hav http:/");
			return 0;
		}
		else if (strstr((word + 7),"http://") != NULL) {
			debug("hav multiple http://");
			return 0;
		}
		else if (strstr((word + 7),"https://") != NULL) {
			debug("hav https://");
			return 0;
		}
		else if (strstr((word + 7),"ftp://") != NULL) {
			debug("hav ftp://");
			return 0;
		}
		else if (strstr((word + 7),"//") != NULL) {
			debug("hav //");
			return 0;
		}
		else if (strstr(word,"/default") != NULL) {
			debug("hav /default",word);
			return 0;
		}
		else if (strstr(word,"/index") != NULL) {
			debug("hav /index",word);
			return 0;
		}
		else if (strstr(word,"/www1") != NULL) {
			debug("hav /www1",word);
			return 0;
		}
		else if (strstr(word,"/www2") != NULL) {
			debug("hav /www2",word);
			return 0;
		}
		else if (strstr(word,"/www3") != NULL) {
			debug("hav /www3",word);
			return 0;
		}
		else if (strstr(word,"/www4") != NULL) {
			debug("hav /index",word);
			return 0;
		}
		else if (strstr(word,"mailto:") != NULL) {
			debug("hav mailto:",word);
			return 0;
		}
		else if (strstr(word,"//:") != NULL) {
			debug("hav //:",word);
			return 0;
		}
                else if (lengt > sizeof(updatePost.url)) {
                        debug("url to long at %i char",strlen(word));
			return 0;
                }
                else {
                        //printf("link: %s\n",word);

			return 1;
		}

		printf("bug: did go to end in gyldig_url(). Isnt suposet to happend\n");
		exit(1);
}


int url_havpri1(char word[]) {

	if (
		(strstr((word + 7),".no/") != NULL)
		|| (strstr((word + 7),".gov/") != NULL)
		|| (strstr((word + 7),".mil/") != NULL)
		|| (strstr((word + 7),".edu/") != NULL)
		//|| (strstr((word + 7),".org/") != NULL)
		) {
                //printf("hav .pri/\n");
        	return 1;
        }
	else {
		return 0;
	}
}

int url_havpri2(char word[]) {

	if (
		(strstr((word + 7),".se/") != NULL)
		|| (strstr((word + 7),".dk/") != NULL)
		|| (strstr((word + 7),".uo.uk/") != NULL)
		|| (strstr((word + 7),".com/") != NULL)
		) {
                //printf("hav .pri/\n");
        	return 1;
        }
	else {
		return 0;
	}
}

int isOkTttl(char word[]) {

//Barry
//> 2) Initially we can start with .com, .net, .org., .gov, . mil, .edu,.biz.,
//> .info, .us as well as 
//> .uk, .eu, .au and .nz from your existing index


	if (
		(strstr((word + 7),".com/") != NULL)
		|| (strstr((word + 7),".net/") != NULL)
		|| (strstr((word + 7),".org/") != NULL)
		|| (strstr((word + 7),".gov/") != NULL)
		|| (strstr((word + 7),".mil/") != NULL)
		|| (strstr((word + 7),".edu/") != NULL)
		|| (strstr((word + 7),".biz/") != NULL)
		|| (strstr((word + 7),".info/") != NULL)
		|| (strstr((word + 7),".us/") != NULL)
		|| (strstr((word + 7),".uk/") != NULL)
		|| (strstr((word + 7),".eu/") != NULL)
		|| (strstr((word + 7),".au/") != NULL)
		|| (strstr((word + 7),".nz/") != NULL)
		|| (strstr((word + 7),".fm/") != NULL)  // brukes av mange netradioer, som di.fm
		) {
                //printf("hav .pri/\n");
        	return 1;
        }
	else {
		return 0;
	}

}

int url_isttl(char word[],char ttl[]) {
    char ttlfull[6];

    snprintf(ttlfull,sizeof(ttlfull),".%s/",ttl);

    if (strstr((word + 7),ttlfull) == NULL) {
            return 0;
    }
    else {
            return 1;
    }
}

int find_domain_no_subname2 (const char url[],char domain[], int sizeofdomain) {

 char **Data;
  int Count, TokCount;
	int i;

	if (!find_domain_no_www(url,domain,sizeofdomain)) {
		return 0;
	}

  	TokCount = split(domain, ".", &Data);

	#ifdef DEBUG

		printf("url %s\n",url);

		printf("TokCount %i\n",TokCount);
  		Count = 0;
  		while( (Data[Count] != NULL) )
    			printf("\t\t%d\t\"%s\"\n", Count, Data[Count++]);
  		printf("\n");

		printf("nr %i\n",TokCount);


	#endif

	//--TokCount;
	domain[0] = '\0';

	if (TokCount < 2) {
		printf("bad domain url: \"%s\", nr %i\n",url, TokCount);

		return 0;
	}

/*
	if (strlen(Data[TokCount -2]) == 2) {

		for (i=3;i>0;i--) {

			printf("oo i %i (%i)\n",i,TokCount -i);
			printf("oo i %i (%i), \"%s\"\n",i,TokCount -i,Data[TokCount -i]);
			strlcat(domain,Data[TokCount -i],sizeofdomain);
			strlcat(domain,".",sizeofdomain);

		}

	}	
	else {
*/

		for (i=2;i>0;i--) {
			//printf("i %i (%i), \"%s\"\n",i,TokCount -i,Data[TokCount -i]);
			strlcat(domain,Data[TokCount -i],sizeofdomain);
			strlcat(domain,".",sizeofdomain);

		}
		
//	}


	domain[strlen(domain) -1] = '\0';
	//printf("Count %i\n",Count);

	#ifdef DEBUG
	printf("domain: \"%s\"\n",domain);
	#endif
  FreeSplitList(Data);

  return 1;

}
//fjerner ogs� www. i begyndelsen av en url, slik at det blir lettere � samenligne www.vg.no == vg.no
int find_domain_no_subname (const char url[],char domain[], int sizeofdomain) {
//ToDo har lagt til sizeofdomain, for � ung� buffer owerflow

	#define maxsubdomains 10

        char *cpnt;
        char buff[64];
        char darray[10][64];
        int i,y;
        //kutter av http:// (de 7 f�rste tegnene)
        strscpy(domain,url + 7,sizeofdomain -1);




        //s�ker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                printf("bad url. Dont have any / after protokoll, or domain (or subdomain) is to long Url \"%s\"\n",url);
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';

                strscpy(buff,domain,sizeof(buff));
                i = 0;
                while (((cpnt = strrchr(buff,'.'))) != NULL && (i<maxsubdomains)) {
                        //++cpnt;
                        //printf("el %s\n",cpnt);
                        strscpy(darray[i],cpnt,sizeof(darray[i]));
                        cpnt[0] = '\0';
                        ++i;
                }
                strscpy(darray[i],buff,sizeof(darray[i]));

                if (strlen(darray[1]) > 3) {
                        //printf("%s > 3\n",darray[1]);
                        domain[0] = '\0';

                        snprintf(domain,sizeofdomain,"%s%s",darray[1],darray[0]);

                        if (strncmp(domain,"www",3) == 0) {
                                strcpy(domain,domain +3);
                        }

                        //fjerner . p� begyndelsen
                        strcpy(domain,domain +1);
                }
                else {
                        //returnerer domanet slik vi fant det lengere opp
                        if (strncmp(domain,"www.",4) == 0) {
                                strscpy(domain,domain +4,sizeofdomain);
                        }

                }

		//18 mai 2007. Forsikrer os at vi altid slutter p� \0
		//underlig at det er n�dvendig. Men ser ut til at det kan oppst� feil som gir oss domene uten \0
		domain[sizeofdomain] = '\0';

                return 1;
        }
}



//fjerner ogs� www. i begyndelsen av en url, slik at det blir lettere � samenligne www.vg.no == vg.no
int find_domain_no_www (char url[],char domain[], int sizeofdomain) {

        char *cpnt;


        //kutter av http:// (de 7 f�rste tegnene)
        strscpy(domain,url + 7,sizeofdomain);

	if (strncmp(domain,"www.",4) == 0) {
		strscpy(domain,domain +4,sizeofdomain);
	}

        //s�ker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                //printf("bad url\n");
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';
                return 1;
        }
}


int find_TLD(char domain[], char TLD[],int TLDsize) {

      char *cpoint;

      if ((cpoint = strrchr(domain,'.')) == NULL) {
                printf("bad domain\n");
                return 0;
        }
      //h<E5>pper over . i domene. Skal ha "com", ikke ".com"
      ++cpoint;

      strscpy(TLD,cpoint,TLDsize);
      return 1;
}



int find_domain (char url[],char domain[],int domainsize) {

        char *cpnt;


        //kutter av http:// (de 7 f�rste tegnene)
        strscpy(domain,url + 7,domainsize);

        //s�ker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                //printf("bad url\n");
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';
                return 1;
        }
}

