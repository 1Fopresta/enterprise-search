/*
 *	(C) Searchdaimon 2004-2008, Written by Magnus Gal�en (mg@searchdaimon.com)
 *
 *	Example for "query_parser".
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "query_parser.h"
#include "stemmer.h"


void test_expand( char *tekst );
void testit( char *tekst );

int main( int argc, char *argv[] )
{
    if (argc>1)
	{
	    testit( argv[1] );
//	    test_expand( argv[1] );
	}

    return 0;
}

void test_expand( char *tekst )
{
    printf("Loading thesaurus..."); fflush(stdout);

    // Initialiser thesaurus med ouput-filene fra 'build_thesaurus_*':
    thesaurus		*T = thesaurus_init("../../data/thesaurus.text", "../../data/thesaurus.id");

    printf("done\n");

    query_array		qa;

    // Parse query:
    get_query( tekst, strlen(tekst), &qa);

    // Kj�r stemming p� query:
    thesaurus_expand_query(T, &qa);

    // Print query med innebygd print-funksjon:
    char	buf[1024];
    sprint_expanded_query(buf, 1023, &qa);
    printf("\nExpanded query: %s\n\n", buf);

    // --- Eksempel p� iterering av expanded query:

    int		i, j, k;

    for (i=0; i<qa.n; i++)
	{
	    printf(" %c:", qa.query[i].operand);

	    // Brukerens query:
	    if (qa.query[i].n > 1 || qa.query[i].operand == QUERY_PHRASE) printf("\"");

	    for (j=0; j<qa.query[i].n; j++)
		{
		    if (j>0) printf(" ");
		    printf("%s", qa.query[i].s[j]);
		}

	    if (qa.query[i].n > 1 || qa.query[i].operand == QUERY_PHRASE) printf("\"");

	    // Expanded query (fra stemming og synonymer):
	    if (qa.query[i].alt != NULL)
		{
		    printf("(");
		    for (j=0; j<qa.query[i].alt_n; j++)
			{
			    if (j>0) printf("|");
			    if (qa.query[i].alt[j].n > 1) printf("\"");

			    for (k=0; k<qa.query[i].alt[j].n; k++)
				{
				    if (k>0) printf(" ");
				    printf("%s", qa.query[i].alt[j].s[k]);
				}

			    if (qa.query[i].alt[j].n > 1) printf("\"");
			}
		    printf(")");
		}
	}

    printf("\n");

    // ---

    destroy_query(&qa);

    thesaurus_destroy(T);
}




void testit( char *tekst )
{
//    char		*tekst = "-Jeg -s�ker etter- \"Magnus Gal�en\", Lars Monrad-Krohn og -\"Lille-Jon";
//    char		*tekst = "+hvor- \" \"  -e\"r - d'u\"\"?";
//    char		*tekst = "boitho date:\"this week\"";
    query_array		*qa = malloc(sizeof(query_array));
    query_array		qa2;

//    tekst = "espen �xnes ��� ��L� a��dlapAW��A�� æøå æ�øa�B�";

    get_query( tekst, strlen(tekst), qa);
    copy_query( &qa2, qa );

    char	buf[1024];

    sprint_query(buf, 1023, qa);
    printf("%s\n", buf);

    sprint_query(buf, 1023, &qa2);
    printf("%s\n", buf);

    sprint_query_array(buf, 1023, qa);
    printf("%s\n", buf);

    destroy_query( qa );
    free(qa);

    destroy_query( &qa2 );

/*
    int			i,j;
    printf("\nquery %s\n",tekst);

    for (i=0; i<qa.n; i++)
	{
	    printf("(%c)", qa.query[i].operand );

		for (j=0; j<qa.query[i].n; j++)
		{
			printf(" %s", qa.query[i].s[j]);
		}

	    printf("\n");
	}

    printf("\n");
*/
/*
    query_array		qa_html;
    copy_htmlescaped_query( &qa_html, &qa );

    for (i=0; i<qa_html.n; i++)
	{
	    printf("(%c)", qa_html.query[i].operand );

		for (j=0; j<qa_html.query[i].n; j++)
		{
			printf(" [%s]", qa_html.query[i].s[j]);
		}

	    printf("\n");
	}

    destroy_query( &qa_html );
*/
}

