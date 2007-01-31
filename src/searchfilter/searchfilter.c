#include "../common/define.h"

void findDomain(char url[], char domain[]);


int filterDocumentIndex(struct DocumentIndexFormat *DocumentIndex) {

	char domain[65]; //domane kan maks v�re p� 64 bokstaver, + \0
	char *cpoint;
	int hyphenCount;

	printf("url: %s\n",(*DocumentIndex).Url);

	findDomain((*DocumentIndex).Url,domain);

	//teller antal forekomster av -
	hyphenCount = 0;
	cpoint = domain;
	while(cpoint = strchr(cpoint,'-')) {
		//g�r vidre
		++cpoint;
		++hyphenCount;
	}	

	printf("domain: %s\nhyphenCount %i\n",domain,hyphenCount);

}

/*******************************************************
Finner doemene for en url. Denne m� bare brukes p� velformede urler, 
som begynner p� http://
********************************************************/
void findDomain(char url[], char domain[]) {

	char *cpoint;

	// +7 fjerner http://, som er de f�rste 7 bokstavene i urlen
	strcpy(domain,url +7);
	printf("domain: %s\n",domain);
	//begynner med � fjerne http://
	cpoint = strchr(domain,'/');
	//vil ikke ha med / p� slutten
	

	domain[cpoint - domain] = '\0';


}
