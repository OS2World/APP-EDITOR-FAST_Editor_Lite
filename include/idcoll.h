/*
** Module   :IDCOLL.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  15/06/2003 Created
**
*/

#include <collect.h>

#ifndef __IDCOLL_H
#define __IDCOLL_H

class Line;

//-----------------------------------------
// Id collection
//-----------------------------------------

class ListBox;

class IDColl: public SortedCollection
{
        int LastPos;
        int WasMatch;
    public:

        IDColl();

        virtual int Compare(Ptr p1, Ptr p2);

        int NextID(Line* ln, int minlen, int npos = -1);

        void AddID(char* start, int len);
        void Refill(int);

        int FillList(ListBox*, Line* ln, int minlen);
};

#endif  /*__IDCOLL_H*/

