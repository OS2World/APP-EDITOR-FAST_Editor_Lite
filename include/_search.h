/*
** Module   :_SEARCH.H
** Abstract :Search engine implementation
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  12/11/1997   	Adopted from C SNIPPETS
*/

#include <version.h>
#include <line.h>
#include <_regex.h>

#ifndef  __SEARCH_H
#define  __SEARCH_H

class SearchEngine
{
    public:
        virtual ~SearchEngine()								{}

        virtual int init(char *,int)  						{ return 0;}
        virtual char* search(char *, int& match_len) 		{ match_len = 0; return 0;}

        //Tell parser (not) to match BOL
        virtual void middle(int i)    						{}

        //Prepare substitution string
        virtual void subst(char* pat, char* src, PLine l)	{}
};

//-----------------------------------------------------------
// Boyer-Moore-Horspool pattern match
//-----------------------------------------------------------

class BMHBase: public SearchEngine
{
	protected:
        int patlen;
        int skip[257];
        int skip2;
        char *pat;
	public:

		BMHBase();
		~BMHBase();

        virtual void subst(char* pat, char* src, PLine l); //Prepare substitution string
};

class BMHSearch: public BMHBase
{
    public:

        virtual int init(char *pattern,int);
        virtual char* search(char *string, int& match_len);
};

//-----------------------------------------------------------
// Boyer-Moore-Horspool case insensitive pattern match
//-----------------------------------------------------------

class BMHISearch: public BMHBase
{
    public:

        virtual int init(char *pattern,int);
        virtual char* search(char *string, int& match_len);
};

//-----------------------------------------------------------
// Regular expression pattern match
//-----------------------------------------------------------

class RXSearch: public SearchEngine
{
	    regmatch_t match[__REG_SUBEXP_MAX];
        int flags;
        regex_t reg;

    public:

		RXSearch();
		virtual ~RXSearch();

        virtual int init(char *pattern,int);
        virtual char* search(char *string, int& match_len);
        virtual void middle(int i);
        virtual void subst(char* pat, char* src, PLine l);

		void get_match(int ord, int& beg, int& end);
};

#endif //__SEARCH_H

