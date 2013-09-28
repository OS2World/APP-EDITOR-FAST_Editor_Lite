/*
** Module   :DICT.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  09/04/1998 Created
**
*/

#include <malloc.h>
#include <string.h>

#include <dict.h>
#include <_ctype.h>
#include <version.h>

#ifdef __EMX__
#include <alloca.h>
#endif

Dictionary::Dictionary(int Case, PPair pList, int size):SortedCollection()
{
    bCase = Case;
    bDuplicates = 0;

    for(int i = 0; i < size; i++)
        Add(&pList[i]);
}

Dictionary::~Dictionary()
{
    RemoveAll();
}

void Dictionary::Free(Ptr)
{
}

int Dictionary::Compare(Ptr p1, Ptr p2)
{
    return (bCase) ? strcmp(PPair(p1)->key, PPair(p2)->key) :
                  __cstrcmp(PPair(p1)->key, PPair(p2)->key);
}

PPair Dictionary::IsIn(char *key, int Case)
{
    Pair p;
    p.key = key;

    int save_case = bCase;
    bCase = Case;

    unsigned index;

    if(Find(&p, &index))
    {
        bCase = save_case;
        return PPair(Get(index));
    }

	bCase = save_case;
    return 0;
}

PPair Dictionary::IsIn(char *key, int len, int Case)
{
    char *buf;

    buf = (char*) alloca(len+1);

    memcpy(buf, key, len);
    buf[len] = 0;
    return IsIn(buf, Case);
}

