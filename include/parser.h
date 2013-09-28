/*
** Module   :PARSER.H
** Abstract :Syntax highlighting parsers
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Tue  11/03/1997   	Created
*/

#include <common.h>
#include <_ctype.h>
#include <dict.h>

#ifndef  __PARSER_H
#define  __PARSER_H

#define ST_INITIAL      0
#define ST_QUOTE1       1
#define ST_QUOTE2       2
#define ST_COMMENT      3
#define ST_COMMENT2     4
#define ST_STR_PHP1     5
#define ST_STR_PHP2     6
#define ST_COMMENT_PHP  7
#define ST_PHP_START    ST_QUOTE2
#define ST_PHP_END      ST_COMMENT_PHP

//-----------------------------------------
// Basic Parser class
//-----------------------------------------

struct RegRecord;

class Parser
{
        static char def_tbl[256];
        static int tbl_ready;
        static Dictionary* pKeywords;
        static Dictionary* pParsers;

    protected:
        int preserve_case;
        int uses_bracket;
        unsigned mask;
        char *start;
        char *cvt_tbl;
        char *old_tok;

    public:
        int tok_len;
        int color;
        int state;
        unsigned flags;

        char *tok;

        Parser();

        virtual void reset(char *tok, int initial_state);

        virtual int next_token();
        int is_kwd();
        int process_brackets()  { return uses_bracket;}
        int offset()            { return tok-start;}
        void SetXlat(char *t)   { cvt_tbl = t;}
        char* prev_token()      { return old_tok;}
        unsigned get_mask()     { return mask;}

        // Parser collection API

        static void RegParser(RegRecord*);      //Used by automated registration
        static Parser* GenParser(int i);        //Generate parser by HI_* key
        static Parser* GenParser(char*);        //Generate parser by name
        static int     Count();                 //Number of registered parsers
        static char*   Name (int iNdx, int iHint = 0);  //Name by ordinal
        static char*   NameByKey(int key, int iHint = 0);   //Name by HI_* key
        static int     Index(char* key);        //HI_* key by name
        static int     Type(int);               //HI_* key by ordinal
        static int     GuessType(char *name);
};

//-----------------------------------------
// Individual registrator
//-----------------------------------------

typedef Parser* (*PFactory)();

struct RegRecord
{
    char* cName;
    char* cHName;
    int   iType;
    PFactory pGen;
};

template <class T> class RegParser:public RegRecord
{
    public:

        static Parser* Gen()    { return new T;}

        RegParser(char* n, char* h, int t)
        {
            cName  = n;
            cHName = h;
            iType  = t;
            pGen  =  Gen;

            Parser::RegParser(this);
        }
};

//-----------------------------------------
// Classes for particular types of syntax
//-----------------------------------------

class Parser_CPP:public Parser
{
    public:
        Parser_CPP() { preserve_case = 1; uses_bracket = 1; mask = MASK_CPP;}
        virtual int next_token();
};

class Parser_REXX:public Parser
{
    public:
        Parser_REXX() { preserve_case = 0; mask = MASK_REXX;}
        virtual int next_token();
};

class Parser_PAS:public Parser
{
    public:
        Parser_PAS() { preserve_case = 0; mask = MASK_PAS;}
        virtual int next_token();
};

class Parser_ASM:public Parser
{
    public:
        Parser_ASM() { preserve_case = 0; mask = MASK_ASM;}
        virtual int next_token();
};

class Parser_MAKE:public Parser
{
    public:
        Parser_MAKE() { preserve_case = 0; mask = MASK_MAKE;}
        virtual int next_token();
};

class Parser_TEX:public Parser
{
    public:
        Parser_TEX() { preserve_case = 1; mask = MASK_TEX;}
        virtual int next_token();
};

class Parser_PL:public Parser
{
    public:
        Parser_PL() { preserve_case = 1; uses_bracket = 1; mask = MASK_PL;}
        virtual int next_token();
};

class Parser_HTML:public Parser
{
    public:
        Parser_HTML() { preserve_case = 0; uses_bracket = 1; mask = MASK_HTML;}
        virtual int next_token();
};

class Parser_MAIL:public Parser
{
        char *line_start;
    public:
        Parser_MAIL():line_start(0) { preserve_case = 0; mask = MASK_MAIL;}
        virtual int next_token();
        virtual void reset(char *tok, int initial_state);
};

#endif //__PARSER_H

