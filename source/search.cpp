/*
** Module   :SEARCH.CPP
** Abstract :Search engines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  12/11/1997   	Adopted from C SNIPPETS
*/

#include <string.h>

#include <_search.h>
#include <_ctype.h>
#include <version.h>

#define LARGE 32767

//-----------------------------------------
// Common BMH-search class
//-----------------------------------------

BMHBase::BMHBase()
{
    pat = 0;
}

BMHBase::~BMHBase()
{
    delete pat;
}

void BMHBase::subst(char* pat, char* src, PLine l)
{
    if(!l)
        return;

    l->set(pat ? pat:(char*)"");
}

//-----------------------------------------
// Case-sensitive search
//-----------------------------------------

int BMHSearch::init(char *pattern,int)
{
    if(!pattern)
        return -1;

	int i, lastpatchar;

	if (pat)
    {
		delete pat;
        pat = 0;
    }

	patlen = strlen(pattern);

    if(!patlen)
        return -1;

	pat = new char[patlen];

	for (i = 0; i < patlen; i++)
        pat[i] = pattern[i];

	for (i = 0; i <= 256; ++i)
		skip[i] = patlen;

	for (i = 0; i < patlen; ++i)
		skip[pat[i]] = patlen - i - 1;

	lastpatchar = pat[patlen - 1];

	skip[lastpatchar] = LARGE;
	skip2 = patlen;

	for (i = 0; i < patlen - 1; ++i)
    {
		if (pat[i] == lastpatchar)
			skip2 = patlen - i - 1;
    }

    return 0;
}

char *BMHSearch::search(char *string, int& match_len)
{
	int i, j;
	char *s;
    int stringlen = strlen(string);

    match_len = 0;

    i = patlen - 1 - stringlen;

	if (i >= 0)
        return 0;

	string += stringlen;
	for (;;)
    {
		while ((i += skip[(unsigned) string[i]]) < 0)
			;
		if (i < (LARGE - stringlen))
            return 0;
		i -= LARGE;
		j = patlen - 1;
		s = (char *) string + (i - j);
		while (--j >= 0 && s[j] == pat[j])
			;

        if (j < 0)
        {
    	    match_len = patlen;
            return s;
        }

		if ((i += skip2) >= 0)
            return 0;
    }
}

//-----------------------------------------
// Case-insensitive search
//-----------------------------------------

int BMHISearch::init(char *pattern, int)
{
	int i, lastpatchar;

	if (pat)
    {
		delete pat;
        pat = 0;
    }

	patlen = strlen(pattern);

    if(!patlen)
        return -1;

    pat = new char[patlen];

	for (i = 0; i < patlen; i++)
        pat[i] = __to_upper(pattern[i]);

	for (i = 0; i <= 256; ++i)
		skip[i] = patlen;

	for (i = 0; i < patlen - 1; ++i)
    {
		skip[pat[i]] = patlen - i - 1;
		skip[__to_lower(pat[i])] = patlen - i - 1;
    }

	lastpatchar = pat[patlen - 1];
	skip[lastpatchar] = LARGE;
    skip[__to_lower((char)lastpatchar)] = LARGE;

	skip2 = patlen;
	for (i = 0; i < patlen - 1; ++i)
    {
        if (pat[i] == lastpatchar)
			skip2 = patlen - i - 1;
    }

    return 0;
}

char *BMHISearch::search(char *string, int& match_len)
{
	int i, j;
	char *s;
	int stringlen = strlen(string);

    match_len = 0;

    i = patlen - 1 - stringlen;
	if (i >= 0)
        return 0;

	string += stringlen;
	for (;;)
    {
		while ((i += skip[(unsigned) string[i]]) < 0)
			;
		if (i < (LARGE - stringlen))
            return 0;
		i -= LARGE;
		j = patlen - 1;
		s = (char *) string + (i - j);
		while (--j >= 0 && __to_upper(s[j]) == pat[j])
			;
        if (j < 0)
        {
    	    match_len = patlen;
            return s;
        }
		if ((i += skip2) >= 0)
            return 0;
    }
}

//-----------------------------------------
// Regular Expression Search
//-----------------------------------------

RXSearch::RXSearch()
{
    flags = 0;
    memset(&reg, 0, sizeof(reg));
}

RXSearch::~RXSearch()
{
    regfree(&reg);
}

int RXSearch::init(char *pattern, int ignore_case)
{
    regfree(&reg);
    memset(&reg, 0, sizeof(reg));

    return regcomp(&reg, pattern, REG_EXTENDED | (ignore_case ? REG_ICASE:0));
}

char* RXSearch::search(char *string, int& match_len)
{
    if(!string)
        return 0;

    memset(match, 0, sizeof(match));

    int rc = regexec(&reg, string, __REG_SUBEXP_MAX, match, flags);

    if(rc)
        return 0;

    match_len = match[0].rm_eo - match[0].rm_so;

    return &string[match[0].rm_so];
}

void RXSearch::middle(int i)
{
    flags = (i) ? REG_NOTBOL:0;
}

void RXSearch::subst(char* pat, char* src, PLine l)
{
    if(!l)
        return;

    if(!pat || !src)
    {
        SearchEngine::subst(pat, src, l);
        return;
    }

    l->set("");

    while(*pat)
    {
        if(pat[0] == '$' && pat[1] >= '0' && pat[1] <= '9')
        {
            int i = pat[1] - '0';
            int len = match[i].rm_eo - match[i].rm_so;
            char* s = &src[match[i].rm_so];

            for(i = 0; i < len; i++)
                l->ins_char(l->len(), *s++);

            pat += 2;
        }
        else
        {
            if(pat[0] == '$' && pat[1] == '$')
                pat++;

            l->ins_char(l->len(), *pat++);
        }
    }
}

void RXSearch::get_match(int ord, int& beg, int& end)
{
	if(ord < __REG_SUBEXP_MAX)
	{
		beg = match[ord].rm_so;
		end = match[ord].rm_eo;
	}
	else
	{
		beg = 0; end = 0;
	}
}

