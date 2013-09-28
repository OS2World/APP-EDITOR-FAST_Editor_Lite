/*
** Module   :COLLECT.CPP
** Abstract :Class Collection and Sorted Collection methods
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  13/03/1994     Updated
**      Sun  23/02/1997     Updated
**      Wed  05/03/1997     Some routines moved to the .H file
**                      	to achieve maximum performance
*/

#include <string.h>
#include <collect.h>
#include <version.h>

Collection::Collection(unsigned aCount, unsigned aDelta)
{
    dwCount = aCount;
    dwLast  = 0;
    dwDelta = aDelta;
    ppData  = new Ptr[dwCount];
}

Collection::~Collection()
{
    RemoveAll();
    delete ppData;
}

void Collection::Add(Ptr newitem)
{
    if (dwLast < dwCount)
        ppData[dwLast++] = newitem;
	else
	{
        Ptr * tmp = new Ptr[dwCount + dwDelta];
        memmove(tmp, ppData, sizeof(Ptr) * dwCount);
        dwCount += dwDelta;
        delete ppData;
        ppData = tmp;
        ppData[dwLast++] = newitem;
	}
}
void Collection::At(Ptr p, unsigned pos)
{
    if(dwLast < dwCount)
    {
        if(pos > dwLast)
            pos = dwLast;
        if(pos == dwLast)
            ppData[dwLast++] = p;
        else
        {
            memmove(&ppData[pos+1], &ppData[pos], sizeof(Ptr)*(dwLast - pos));
            ppData[pos] = p;
            dwLast++;
        }
    }
    else
    {
        Ptr* tmp = new Ptr[dwCount+dwDelta];
        if(pos)
            memmove(tmp, ppData, sizeof(Ptr) * pos);
        if(pos < dwLast)
            memmove(&tmp[pos+1], &ppData[pos], sizeof(Ptr) * (dwLast - pos));
        tmp[pos] = p;
        dwCount += dwDelta;
        dwLast++;
        delete ppData;
        ppData = tmp;
    }
}

Ptr Collection::Remove(unsigned item)
{
    Ptr tmp = Get(item);
    if(tmp)
    {
        memmove(&ppData[item], &ppData[item+1], sizeof(Ptr) * (dwCount-item-1));
        dwLast--;
    }
    return tmp;
}

void Collection::ForEach(ForEachFunc func)
{
    for(int i = 0; i < dwLast; i++)
        func(ppData[i]);
}

void Collection::RemoveAll()
{
    for(int i = 0; i < dwLast; i++)
        Free(ppData[i]);
    dwLast = 0;
}

void Collection::Free(Ptr p)
{
    delete p;
}

void Collection::move_items(PCollection src, unsigned from)
{
    if((src->dwLast + dwLast) >= dwCount)
    {
        Ptr* tmp = new Ptr[dwCount + src->dwLast];
        memcpy(tmp, ppData, dwLast * sizeof(Ptr));
        delete ppData;
        ppData = tmp;
        dwCount += src->dwLast;
    }

    //Make free space
    memmove(&ppData[from + src->dwLast],
            &ppData[from],
            (dwLast - from) * sizeof(Ptr));
    memcpy(&ppData[from], src->ppData, src->dwLast * sizeof(Ptr));
    dwLast += src->dwLast;
    src->dwLast = 0;
}

void Collection::remove_items(PCollection dest, unsigned from, unsigned count)
{
    dest->RemoveAll();
    if((from + count) > dwLast)
    {
        if(from > dwLast)
            return;
        count = dwLast - from;
    }


    if(dest->dwCount < count)
    {
        delete dest->ppData;
        dest->dwCount = count;
        dest->ppData = new Ptr[count];
    }

    dest->dwLast = count;
    memcpy( dest->ppData,
            &ppData[from],
            count * sizeof(Ptr));

    if((dwLast-from-count) > 0)
        memmove(&ppData[from],
                &ppData[from+count],
                sizeof(Ptr) * (dwLast-from-count));
    dwLast -= count;
}

void SortedCollection::Add(Ptr p)
{
    if(!dwLast)
    {
        Collection::Add(p);
        return;
    }
    unsigned pos = Look(p);
    int rc = 0;

    if(pos < dwLast)
        rc = Compare(p, Get(pos));
    else
    {
        Collection::Add(p);
        return;
    }
    if(( !rc && bDuplicates) || rc)
        At(p, pos);
}

unsigned SortedCollection::Look(Ptr p)
{
    int l, m, h;

    l = m = 0;
    h = dwLast-1;

    while(l <= h)
    {
        m = (l+h) >> 1;
        int rc = Compare(p, Get(m));
        if(!rc)
            return m;
        if(rc < 0)
            h = m-1;
        if(rc > 0)
            l = m+1;
    }
    return l;
}

int SortedCollection::Find(Ptr p, unsigned* pndx)
{
	unsigned index = Look(p);

	if(index < Count() && !Compare(p, Get(index)))
	{
		if(pndx)
			*pndx = index;

		return 1;
	}

	return 0;
}

