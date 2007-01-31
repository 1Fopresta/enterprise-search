#include "../common/reposetory.h"
//#include "../common/DocumentIndex.h";

#include <stdlib.h>





main (int argc, char *argv[]) {


        int LotNr;
        char lotPath[255];

        struct ReposetoryHeaderFormat ReposetoryHeader;
        unsigned long int radress;
	unsigned int LastIndexTime;

        char htmlbuffer[524288];
        char imagebuffer[524288];
	struct DocumentIndexFormat DocumentIndexPost;

        if (argc < 2) {
                printf("Error ingen lotnr spesifisert.\n\nEksempel p� bruk for � lese lot 2:\n\trread 2\n");
                exit(1);
        }

        LotNr = atoi(argv[1]);

	LastIndexTime = GetLastIndexTimeForLot(LotNr);

        GetFilePathForIindex(&lotPath,LotNr);
        printf("Opning lot at: %s\n",lotPath);

    	//loppergjenom alle. Har oppdatert "rGetNext", ved � legge til file offset p� 0, uten � ha testet
        while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,imagebuffer,&radress,LastIndexTime,0)) {

                //printf("DocId: %i url: %s IP: %u\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.IPAddress);

		if (DIRead(&DocumentIndexPost,ReposetoryHeader.DocID)) {

			//hvis vi ikke har noen IP adresse s� legger vi den til � skriver den tilbake
			if (DocumentIndexPost.IPAddress == 0) {
				DocumentIndexPost.IPAddress = ReposetoryHeader.IPAddress;

				DIWrite(&DocumentIndexPost,ReposetoryHeader.DocID);
			}
		}
		else {
			printf("can't read DocumentIndex for DocID %u\n",ReposetoryHeader.DocID);
		}
		
                //sletter litt slik at vi ser om det blir noen feil i lesingen
                ReposetoryHeader.DocID = -1;

        }


}

