/*
** Module   :DICT.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  09/04/1998 Created
**
*/

#include <collect.h>

#ifndef  __DICT_H
#define __DICT_H

struct Pair
{
    char *key;
};

typedef Pair* PPair;

class Dictionary: public SortedCollection
{
        int bCase;

    public:
        Dictionary(int Case, PPair pList, int size);
        virtual ~Dictionary();

        virtual int Compare(Ptr p1, Ptr p2);
        virtual void Free(Ptr p);

        PPair IsIn(char *key, int Case);
        PPair IsIn(char *key, int len, int Case);
};

#endif  /*__DICT_H*/

