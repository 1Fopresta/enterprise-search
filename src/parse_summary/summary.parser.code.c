
#include "summary.parser.c"

// --- fra flex:
typedef void* yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE summary_scan_bytes (const char *bytes,int len ,yyscan_t yyscanner );
// ---

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

void print2buffer(buffer *b, const char *fmt, ...)
{
    if (b->overflow) return;

    va_list	ap;

    va_start(ap, fmt);
    int	len_printed = vsnprintf((char*)(&(b->data[b->pos])), b->maxsize - b->pos - 1, fmt, ap);

    b->pos+= len_printed;

    if (b->pos >= b->maxsize - 1) b->overflow = 1;
}


void print_with_escapes(char *c, buffer *b)
{
    int		i;

//    if (b->pos > 0) print2buffer(b, " ");

    for (i=0; c[i]!='\0'; i++)
	{
	    if ((unsigned char)c[i]<128)
		print2buffer(b, "%c", c[i]);
	    else
		{
		    struct html_esc	*p = (struct html_esc*)
		    bsearch( (const void*)(((char*)&(c[i]))), he, 65, sizeof(struct html_esc), esc_compare);

		    if (p==NULL)
			print2buffer(b, "%c", c[i]);
		    else
			print2buffer(b, "&%s;", p->esc);
		}
	}

    free( c );
}

void print_raw(char *c, buffer *b)
{
//    if (b->pos > 0) print2buffer(b, " ");

    print2buffer(b, "%s", c);
}


buffer buffer_init( int _maxsize )
{
    buffer	b;

    b.overflow = 0;
    b.pos = 0;
    b.maxsize = _maxsize -1;	    // In case we need that trailing zero ;)
    b.data = (unsigned char*)malloc(b.maxsize +1);

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
    // Variables for lexical analyzer:
    struct _sp_yy_extra	*se = (struct _sp_yy_extra*)malloc(sizeof(struct _sp_yy_extra));

    se->top = -1;
    se->stringtop = -1;

    // Set variables for lalr(1)-parser:

    // Fields 'title', 'meta keywords' and 'meta description', will only keep first 10240 bytes,
    // field body will only keep up to double original textsize (should be enough for all ordinary documents).

    struct parseExtra pE;

    pE.section = INIT;

    pE.newspan = 1;
    pE.newdiv = 1;
    pE.newhead = 0;
    pE.endhead = 0;
    pE.inspan = 0;
    pE.indiv = 0;
    pE.inhead = 0;

    pE.title = buffer_init( 10240 );
    pE.body = buffer_init( text_size*2 );
    pE.metakeyw = buffer_init( 10240 );
    pE.metadesc = buffer_init( 10240 );

    void	*pParser = summaryParseAlloc(malloc);
    int		yv;

    // Run parser:
    yyscan_t	scanner;

    summarylex_init( &scanner );
    summaryset_extra( se, scanner );

    YY_BUFFER_STATE	bs = summary_scan_bytes( text, text_size, scanner );

    se->token.space = 0;
    while ((yv = summarylex(scanner)) != 0)
	{
	    summaryParse(pParser, yv, se->token, &pE);
	    if (yv==WORD || yv==ESC)
		se->token.space = 0;
	}

    summaryParse(pParser, 0, se->token, &pE);
    summaryParseFree(pParser, free);

    summary_delete_buffer( bs, scanner );
    summarylex_destroy( scanner );

    if (pE.inspan)
        print_raw( "</span>", &pE.body );
    if (pE.inhead)
        print_raw( "\n</h2>", &pE.body );
    if (pE.indiv)
        print_raw( "\n</div>", &pE.body );

//    pE.rawbody.data[pE.rawbody.pos] = '\0';

//    print_with_escapes( pE.rawbody.data, &(pE.body) );		// pr_w_esc kj�rer free automagisk!!

    (*output_title) = buffer_exit( pE.title );
    (*output_body) = buffer_exit( pE.body );
    (*output_metakeywords) = buffer_exit( pE.metakeyw );
    (*output_metadescription) = buffer_exit( pE.metadesc );

    free( se );
}
