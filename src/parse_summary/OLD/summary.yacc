%{
// (C) Copyright Boitho 2005, Magnus Gal�en (magnusga@idi.ntnu.no)

/******************************************
changelog:
Runarb 13. des 2005
I steden for � bruke globale verider for title, body, metakeyw, metadesc og *current_buffer her jeg 
laget en "struct bufferformat" med disse i. generate_summary oppreter n� en buffers av denne typen 
og sender den med yyparse.

Hva den heter i yyparse defineres med "#define YYPARSE_PARAM buffers"

Dette f�r � gj�re honteringen av buffere threadsafe. Uten dette vil man ofte f� segfeil i buffer_exit n�r man 
"kj�rerfree( b.data );"

*******************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//navn p� en parameter  yyparse skal ta inn http://dinosaur.compilertools.net/bison/bison_7.html#SEC65
#define YYPARSE_PARAM buffers

#include "summary_common.h"
#include "summary.h"

#define INIT	0
#define	TITLE	1
#define	BODY	2


char	section;
char	*href_ptr=NULL, *name_ptr=NULL, *content_ptr=NULL;
int	href_attr=1, name_attr=2, content_attr=4;

typedef struct
{
    char	*data;
    int		pos, maxsize;
    char	overflow;
} buffer;

// pruker ikke globala variabler foo dette. Sender i steden med en sturft av typen "struct bufferformat"
//buffer		title, body, metakeyw, metadesc;
//buffer		*current_buffer;

struct bufferformat {
	buffer 		title;
	buffer 		body;
	buffer	 	metakeyw;
	buffer 		metadesc;
	buffer          *current_buffer;	
};

char* translate(char *s);
void print_with_escapes(char *c,struct bufferformat *buffers);


%}

%token WORD TAG_START TAG_STOPP TAG_ENDTAG_STOPP ENDTAG_START ENDTAG_STOPP ATTR EQUALS TEXTFIELD

%%
doc	:
	| doc block
	;
block	: tag
	| text
	;
tag	: starttag
	| endtag
	| startendtag
	;
starttag	: TAG_START ATTR attrlist TAG_STOPP
	    {
		if (section==INIT && !strcasecmp("title",(char*)$2)) section = TITLE;
		else if ((section==INIT||section==TITLE) && !strcasecmp("body",(char*)$2))
		    {
			section = BODY;
			//current_buffer = &buffers.body;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }
		else if (!strcasecmp("meta",(char*)$2) && (($3 & name_attr)>0) && (($3 & content_attr)>0))
		    {
			if (!strcasecmp("keywords",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metakeyw;
				print_with_escapes( translate(content_ptr),(struct bufferformat *) buffers );
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
			else if (!strcasecmp("description",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metadesc;
				print_with_escapes( translate(content_ptr) , (struct bufferformat *) buffers);
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
		    }
	    }
	;
endtag	: ENDTAG_START ATTR ENDTAG_STOPP
	    {
		if (section==TITLE && !strcasecmp("title",(char*)$2))
		    {
			section = BODY;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }
	    }
	;
startendtag	: TAG_START ATTR attrlist TAG_ENDTAG_STOPP
	    {
		if (!strcasecmp("meta",(char*)$2) && (($3 & name_attr)>0) && (($3 & content_attr)>0))
		    {
			if (!strcasecmp("keywords",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metakeyw;
				print_with_escapes( translate(content_ptr), (struct bufferformat *) buffers );
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
			else if (!strcasecmp("description",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metadesc;
				print_with_escapes( translate(content_ptr),(struct bufferformat *) buffers);
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
		    }
	    }
	;
attrlist :
	    { $$ = 0; }
	| attrlist attr
	    { $$ = $1 | $2; }
	;
attr	: ATTR EQUALS TEXTFIELD
	    {
		if (!strcasecmp("href",(char*)$1))
		    {
			href_ptr = (char*)$3;
			$$ = href_attr;
			href_ptr++;
			href_ptr[strlen(href_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("name",(char*)$1))
		    {
			name_ptr = (char*)$3;
			$$ = name_attr;
			name_ptr++;
			name_ptr[strlen(name_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("content",(char*)$1))
		    {
			content_ptr = (char*)$3;
			$$ = content_attr;
			content_ptr++;
			content_ptr[strlen(content_ptr)-1] = '\0';
		    }
		else
		    $$ = 0;
	    }
	| ATTR EQUALS ATTR
	    {
		if (!strcasecmp("href",(char*)$1))
		    {
			href_ptr = (char*)$3;
			$$ = href_attr;
		    }
		else if (!strcasecmp("name",(char*)$1))
		    {
			name_ptr = (char*)$3;
			$$ = name_attr;
		    }
		else if (!strcasecmp("content",(char*)$1))
		    {
			content_ptr = (char*)$3;
			$$ = content_attr;
		    }
		else
		    $$ = 0;
	    }
	| ATTR
	    { $$ = 0; }
	| TEXTFIELD
	    { $$ = 0; }
	;
text	: WORD
	    {
		if (section==INIT)
		    {
			section = BODY;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }

		// Translate escapes first, to ensure no illegal escapes,
		// then retranslate back to escapes:
		print_with_escapes( translate((char*)$1),(struct bufferformat *) buffers );
	    }
	;
%%

extern FILE *yyin;
extern int linenr;



struct trans_tab
{
    char	*escape, translation;
};

struct trans_tab tt[130] = {
    {"#178",'�'},{"#179",'�'},{"#185",'�'},{"#192",'�'},{"#193",'�'},{"#194",'�'},{"#195",'�'},{"#196",'�'},
    {"#197",'�'},{"#198",'�'},{"#199",'�'},{"#200",'�'},{"#201",'�'},{"#202",'�'},{"#203",'�'},{"#204",'�'},
    {"#205",'�'},{"#206",'�'},{"#207",'�'},{"#208",'�'},{"#209",'�'},{"#210",'�'},{"#211",'�'},{"#212",'�'},
    {"#213",'�'},{"#214",'�'},{"#216",'�'},{"#217",'�'},{"#218",'�'},{"#219",'�'},{"#220",'�'},{"#221",'�'},
    {"#222",'�'},{"#223",'�'},{"#224",'�'},{"#225",'�'},{"#226",'�'},{"#227",'�'},{"#228",'�'},{"#229",'�'},
    {"#230",'�'},{"#231",'�'},{"#232",'�'},{"#233",'�'},{"#234",'�'},{"#235",'�'},{"#236",'�'},{"#237",'�'},
    {"#238",'�'},{"#239",'�'},{"#240",'�'},{"#241",'�'},{"#242",'�'},{"#243",'�'},{"#244",'�'},{"#245",'�'},
    {"#246",'�'},{"#248",'�'},{"#249",'�'},{"#250",'�'},{"#251",'�'},{"#252",'�'},{"#253",'�'},{"#254",'�'},
    {"#255",'�'},{"AElig",'�'},{"Aacute",'�'},{"Acirc",'�'},{"Agrave",'�'},{"Aring",'�'},{"Atilde",'�'},{"Auml",'�'},
    {"Ccedil",'�'},{"ETH",'�'},{"Eacute",'�'},{"Ecirc",'�'},{"Egrave",'�'},{"Euml",'�'},{"Iacute",'�'},{"Icirc",'�'},
    {"Igrave",'�'},{"Iuml",'�'},{"Ntilde",'�'},{"Oacute",'�'},{"Ocirc",'�'},{"Ograve",'�'},{"Oslash",'�'},{"Otilde",'�'},
    {"Ouml",'�'},{"THORN",'�'},{"Uacute",'�'},{"Ucirc",'�'},{"Ugrave",'�'},{"Uuml",'�'},{"Yacute",'�'},{"aacute",'�'},
    {"acirc",'�'},{"aelig",'�'},{"agrave",'�'},{"aring",'�'},{"atilde",'�'},{"auml",'�'},{"ccedil",'�'},{"eacute",'�'},
    {"ecirc",'�'},{"egrave",'�'},{"eth",'�'},{"euml",'�'},{"iacute",'�'},{"icirc",'�'},{"igrave",'�'},{"iuml",'�'},
    {"ntilde",'�'},{"oacute",'�'},{"ocirc",'�'},{"ograve",'�'},{"oslash",'�'},{"otilde",'�'},{"ouml",'�'},{"sup1",'�'},
    {"sup2",'�'},{"sup3",'�'},{"szlig",'�'},{"thorn",'�'},{"uacute",'�'},{"ucirc",'�'},{"ugrave",'�'},{"uuml",'�'},
    {"yacute",'�'},{"yuml",'�'}};


int compare(const void *a, const void *b)
{
    return strncmp( (char*)a, ((struct trans_tab*)b)->escape, strlen(((struct trans_tab*)b)->escape) );
}

// Translate escapes in string:
char* translate(char *s)
{
    char	*d = (char*)malloc(strlen(s)+1);
    int		i, j, k;
    char	replace;

    for (i=0, j=0; s[j]!='\0';)
	switch (s[j])
	    {
		case '&':
		    replace = 0;

		    if (s[j+1]!='\0')
			{
			    struct trans_tab	*code = (struct trans_tab*)bsearch(&(s[j+1]),tt,130,sizeof(struct trans_tab),compare);

			    if (code!=NULL)
				{
				    replace = 1;
				    d[i++] = code->translation;
				    j+= strlen(code->escape)+1;
				    if (s[j]==';') j++;
				}
			}

		    if (!replace)
			{
			    d[i++] = '&';
			    j++;
			}

		    break;
		default:
		    d[i++] = s[j++];
	    }

end:
    d[i] = '\0';
    return d;
}


struct html_esc
{
    char	c, *esc;
};

struct html_esc he[65] = {
    {'�',"sup2"},{'�',"sup3"},{'�',"sup1"},{'�',"Agrave"},{'�',"Aacute"},{'�',"Acirc"},{'�',"Atilde"},{'�',"Auml"},
    {'�',"Aring"},{'�',"AElig"},{'�',"Ccedil"},{'�',"Egrave"},{'�',"Eacute"},{'�',"Ecirc"},{'�',"Euml"},{'�',"Igrave"},
    {'�',"Iacute"},{'�',"Icirc"},{'�',"Iuml"},{'�',"ETH"},{'�',"Ntilde"},{'�',"Ograve"},{'�',"Oacute"},{'�',"Ocirc"},
    {'�',"Otilde"},{'�',"Ouml"},{'�',"Oslash"},{'�',"Ugrave"},{'�',"Uacute"},{'�',"Ucirc"},{'�',"Uuml"},{'�',"Yacute"},
    {'�',"THORN"},{'�',"szlig"},{'�',"agrave"},{'�',"aacute"},{'�',"acirc"},{'�',"atilde"},{'�',"auml"},{'�',"aring"},
    {'�',"aelig"},{'�',"ccedil"},{'�',"egrave"},{'�',"eacute"},{'�',"ecirc"},{'�',"euml"},{'�',"igrave"},{'�',"iacute"},
    {'�',"icirc"},{'�',"iuml"},{'�',"eth"},{'�',"ntilde"},{'�',"ograve"},{'�',"oacute"},{'�',"ocirc"},{'�',"otilde"},
    {'�',"ouml"},{'�',"oslash"},{'�',"ugrave"},{'�',"uacute"},{'�',"ucirc"},{'�',"uuml"},{'�',"yacute"},{'�',"thorn"},
    {'�',"yuml"}};


int esc_compare(const void *a, const void *b)
{
    if (*((char*)a) < ((struct html_esc*)b)->c) return -1;
    if (*((char*)a) > ((struct html_esc*)b)->c) return +1;
    return 0;
}

void print2buffer(struct bufferformat *buffers,const char *fmt, ...)
{
    if (((struct bufferformat *) buffers)->current_buffer->overflow) return;

    va_list	ap;

    va_start(ap, fmt);
    int	len_printed = vsnprintf(&(((struct bufferformat *) buffers)->current_buffer->data[((struct bufferformat *) buffers)->current_buffer->pos]), ((struct bufferformat *) buffers)->current_buffer->maxsize - ((struct bufferformat *) buffers)->current_buffer->pos - 1, fmt, ap);

    ((struct bufferformat *) buffers)->current_buffer->pos+= len_printed;

//    if (section==TITLE) title_size+= len_printed;
//    else body_size+= len_printed;

    if (((struct bufferformat *) buffers)->current_buffer->pos >= ((struct bufferformat *) buffers)->current_buffer->maxsize - 1) ((struct bufferformat *) buffers)->current_buffer->overflow = 1;
}


void print_with_escapes(char *c,struct bufferformat *buffers)
{
    int		i;

    if (((struct bufferformat *) buffers)->current_buffer->pos > 0) print2buffer((struct bufferformat *) buffers," ");

    for (i=0; c[i]!='\0'; i++)
	if ((unsigned char)c[i]<128)
	    print2buffer((struct bufferformat *) buffers,"%c", c[i]);
	else
	    {
		struct html_esc	*p = (struct html_esc*)
		bsearch( (const void*)(((char*)&(c[i]))), he, 65, sizeof(struct html_esc), esc_compare);

		if (p==NULL)
		    print2buffer((struct bufferformat *) buffers,"%c", c[i]);
		else
		    print2buffer((struct bufferformat *) buffers,"&%s;", p->esc);
	    }

    free( c );
}

buffer buffer_init( int _maxsize )
{
    buffer	b;

    b.overflow = 0;
    b.pos = 0;
    b.maxsize = _maxsize;
    b.data = (char*)malloc(b.maxsize);

    return b;
}

char* buffer_exit( buffer b )
{
    char	*output;

    output = (char*)malloc(b.pos+1);
    memcpy( output, &(b.data[0]), b.pos );
    output[b.pos] = '\0';
    free( b.data );

    return output;
}

void generate_summary( char text[], int text_size, char **output_title, char **output_body, char **output_metakeywords, char **output_metadescription )
{
    // Set global variables for lexer:
    custom_input = text;
    custom_pos = 0;
    custom_size = text_size;

    // Set variables for yacc-er:
    section = INIT;

    // We accept output summaries of a size up to double the original textsize.
    // (Although on almost all occations the size will shrink).
//    maxsize = text_size*2;

    // Allocate output_buffer:
//    output_buffer = (char*)malloc(maxsize);
//    title_size = body_size = pos = 0;
//    overflow = 0;	// To prevent corrupt inputfiles to overflow output_buffer.

    // Fields 'title', 'meta keywords' and 'meta description', will only keep first 10240 bytes,
    // field body will only keep up to double original textsize (should be enough for all ordinary documents).

    struct bufferformat buffers;

    buffers.title = buffer_init( 10240 );
    buffers.body = buffer_init( text_size*2 );
    buffers.metakeyw = buffer_init( 10240 );
    buffers.metadesc = buffer_init( 10240 );

    buffers.current_buffer = &buffers.title;

    // Run parser:
    do
	{
	    yyparse((void*)&buffers);
	}
    while (custom_pos<custom_size);

    (*output_title) = buffer_exit( buffers.title );
    (*output_body) = buffer_exit( buffers.body );
    (*output_metakeywords) = buffer_exit( buffers.metakeyw );
    (*output_metadescription) = buffer_exit( buffers.metadesc );
}


yyerror( char *s )
{
    //fprintf( stderr, "parse_error %s on line %i\n", s, linenr );
}
