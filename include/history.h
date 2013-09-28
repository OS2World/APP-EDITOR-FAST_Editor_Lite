/*
** Module   :HISTORY.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Tue  26/11/2002 Created
**
*/

#include <dialog.h>

#ifndef __HISTORY_H
#define __HISTORY_H

class HistoryItem
{
		int flags;
		char* data;
	public:

		HistoryItem(char* p, int perm = 0)	{ data = str_dup(p); flags = (perm) ? 1:0;}
		~HistoryItem()		{ delete data;}

		char* GetText()		{ return data;}
		int IsFixed()		{ return flags;}
		void FlipFixed()	{ flags = 1 - flags;}
};

typedef HistoryItem* PHistoryItem;

class History:public Collection
{
    public:
        History(int sz):Collection(sz, 1){}
        ~History();

        PHistoryItem PutItem(char*);
        void FillList(ListBox*);
        PHistoryItem GetItem(int);
        char* GetItemText(int ndx)	{ return GetItem(ndx) ? GetItem(ndx)->GetText():0; }

        virtual void Free(Ptr p)	{ delete (PHistoryItem)p; }
};

#endif  /*__HISTORY_H*/

