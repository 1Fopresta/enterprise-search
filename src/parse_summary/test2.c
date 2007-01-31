
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "highlight.h"

//char	text[] = "<html><title>Test</title><body>Dette er en test for � finne ut om dette fungerer eller ei.</body></html>";

query_array query_array_init( int n )
{
    int			i;
    query_array		qa;

    qa.n = n;
    qa.query = (string_array*)malloc(sizeof(string_array[qa.n]));
//    for (i=0; i<qa.n; i++)
//	qa.query[i] = (string_array)malloc(sizeof(string_array));

    return qa;
}

void query_array_destroy( query_array qa )
{
    int			i;

//    for (i=0; i<qa.n; i++)
//	free( qa.query[i] );

    free( qa.query );
}

string_array string_array_init( int n )
{
    string_array	sa;

    sa.n = n;
    sa.s = (char**)malloc(sizeof(char*[sa.n]));

    return sa;
}

void string_array_destroy( string_array sa )
{
    free( sa.s );
}


int main( int argc, char *argv[] )
{
/*
    {
	char	ct[256];
	int	i,a;

	for (i=0; i<256; i++) ct[i] = 0;
	for (i='A'; i<='Z'; i++) ct[i] = 1;
	for (i='a'; i<='z'; i++) ct[i] = 1;
	for (i='0'; i<='9'; i++) ct[i] = 1;
	ct['_'] = 1;
	ct[39] = 1;	// '
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;
	ct['�'] = 1;

	for (a=0; a<16; a++)
	{
	for (i=0; i<16; i++) printf("%i, ", ct[a*16+i]);
	printf("\n");
	}
    }
*/
    if (argc<2)
	{
//	    printf( "Usage: %s <filename> <filename>\n\n", argv[0] );
	    printf( "Usage: %s <filename>\n\n", argv[0] );
	    exit(-1);
	}

    FILE	*file = fopen( argv[1], "r" );

    if (!file)
	{
	    fprintf( stderr, "Could not open %s.\n", argv[1] );
	    return -1;
	}

    // Get filesize:
    struct stat	fileinfo;
    fstat( fileno( file ), &fileinfo );

//    printf("Reading %i bytes...\n", fileinfo.st_size);

    int		size = fileinfo.st_size;
    char	*buf = (char*)malloc(sizeof(char)*size);

    int	i;
    for (i=0; i<size;)
	{
	    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
//	    printf("%i...\n", i);
	}

/*
    for (i=0; i<fileinfo.st_size; i++)
	printf("%c", buf[i]);
    printf("\n");
*/

    char	*title, *body, *metakeyw, *metadesc;
/*
struct
{
    int			n;
    char		**s;
} typedef string_array;

struct
{
    int			n;
    string_array	**query;
} typedef query_array;
*/
//    printf("|%s|\n\n\n", buf);

    query_array		qa = query_array_init(3);

    qa.query[0] = string_array_init(1);
    qa.query[1] = string_array_init(1);
    qa.query[2] = string_array_init(1);

    qa.query[0].s[0] = (char*)strdup("ntnu");
    qa.query[1].s[0] = (char*)strdup("epost");
    qa.query[2].s[0] = (char*)strdup("hostmaster");

    generate_highlighting( qa, buf, size, &body );

    free( qa.query[0].s[0] );
    free( qa.query[1].s[0] );
    free( qa.query[2].s[0] );
    string_array_destroy( qa.query[0] );
    string_array_destroy( qa.query[1] );
    string_array_destroy( qa.query[2] );
    query_array_destroy( qa );

//    generate_summary( buf, size, &title, &body, &metakeyw, &metadesc );

//    struct summary	*S = generate_summary( text, sizeof(text) );
/*
    printf("Title: { %s }\n", title);
    printf("Body: { %s }\n", body);
    printf("Meta Keywords: { %s }\n", metakeyw);
    printf("Meta Description: { %s }\n", metadesc);
*/
//    printf("%s\n", body);

    printf("\n\n%s\n", body);

    free(buf);
//    free(title);
//    free(body);
//    free(metakeyw);
//    free(metadesc);

    fclose(file);

    /**********/
/*
    file = fopen( argv[2], "r" );

    if (!file)
	{
	    fprintf( stderr, "Could not open %s.\n", argv[2] );
	    return -1;
	}

    // Get filesize:
    fstat( fileno( file ), &fileinfo );

    printf("Reading %i bytes...\n", fileinfo.st_size);

    size = fileinfo.st_size;
    buf = (char*)malloc(sizeof(char)*size);

    for (i=0; i<size;)
	{
	    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
	    printf("%i...\n", i);
	}

//    for (i=0; i<fileinfo.st_size; i++)
//	printf("%c", buf[i]);
//    printf("\n");

    generate_summary( buf, size, &title, &body );

//    struct summary	*S = generate_summary( text, sizeof(text) );

    printf("Title: { %s }\n", title);
    printf("Body: { %s }\n", body);
*/
}
