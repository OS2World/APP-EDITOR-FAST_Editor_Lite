/*
** Module   :HISTORY.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Tue  26/11/2002 Created
**
*/

#include <history.h>
#include <string.h>

History::~History()
{
    RemoveAll();
}

PHistoryItem History::PutItem(char* pItem)
{
    if(!pItem || !*pItem)
        return 0;

	int mode = 0;

    //Search item and remove if it matches
    //passed one

    int i;

    for(i = 0; i < Count(); i++)
    {
        if(!GetItem(i))
            continue;

        if(!strcmp(GetItemText(i), pItem))
        {
        	mode = GetItem(i)->IsFixed();
            Free(Remove(i));
            break;
        }
    }

    if(dwLast == dwCount)	//History is full
    {
    	//Locate first not marked as fixed

    	for(i = Count() - 1; i >= 0; i--)
    	{
			if(!GetItem(i)->IsFixed())
				break;
    	}

    	if(i >= 0 && !GetItem(i)->IsFixed())
        	Free(Remove(i));
        else
        	return 0;
    }

	PHistoryItem p = new HistoryItem(pItem, mode);

    At(p, 0);

	return p;
}

void History::FillList(ListBox* pBox)
{
    if(!pBox)
        return;

    pBox->RemoveAll();

    for(int i = 0; i < Count(); i++)
    {
        PListBoxItem p = pBox->add_at_end(GetItemText(i));
		p->set_mark(GetItem(i)->IsFixed() ? '\xFE':' ');
    }
}

PHistoryItem History::GetItem(int item)
{
    return (PHistoryItem)Get(item);
}

