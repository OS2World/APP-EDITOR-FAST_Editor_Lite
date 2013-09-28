/*
** Module   :IDCOLL.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  15/06/2003 Created
**
*/

#include <idcoll.h>
#include <string.h>
#include <vio.h>
#include <line.h>
#include <dialog.h>
#include <version.h>

IDColl::IDColl():SortedCollection(128, 128), LastPos(0), WasMatch(0)
{
    bDuplicates = 0;
}

int IDColl::Compare(Ptr p1, Ptr p2)
{
    return strcmp((char*)p1, (char*)p2);
}

int IDColl::FillList(ListBox* lb, Line* ln, int len)
{
    if(!lb)
        return 0;

    lb->go_item(0);
    lb->RemoveAll();

     if(!ln || !len)
        return 0;

    char* p = 0;
    char* start = ln->str;

    Line tmp(ln, 0, len, 1);

    int pos = Look(tmp.str);

    p = (char*)Get(pos);

    if(!p || memcmp(p, start, len)) //Still no match
        return 0;

    if(pos > 0)
    {
        while(--pos)    //Scan back
        {
            p = (char*)Get(pos);

            if(!p || memcmp(p, start, len))
            {
                pos++;
                p = (char*)Get(pos);
                break;
            }
        }
    }

    while(p && !memcmp(p, start, len))
    {
        lb->add_at_end(p)->set_user_data(pos + 1);

        p = (char*)Get(++pos);
    }

    return lb->Count();
}

int IDColl::NextID(Line* ln, int len, int npos)
{
    if(!ln || !len)
        return 0;

    char* p = 0;
    char* start = ln->str;

    if(npos >= 0)
    {
        WasMatch = 1;
        LastPos = npos;
    }

    if(WasMatch)
    {
        p = (char*)Get(LastPos);

        if(p && !memcmp(p, start, len)) //Still match
        {
            if(npos < 0)
	            ++LastPos;

            p = (char*)Get(LastPos);

            if(p && !memcmp(p, start, len)) //Next one is acceptable
            {
                ln->set(p);
                return 1;
            }

            //Next one is not acceptable, end of list is reached, scan back

            while(--LastPos)
            {
                p = (char*)Get(LastPos);

                if(!p || memcmp(p, start, len))
                {
                    LastPos++;
                    p = (char*)Get(LastPos);
                    break;
                }
            }

            //return p;
            ln->set(p);
            return 1;
        }

        //No match, look for candidate

        WasMatch = 0;
    }

    Line tmp(ln, 0, len, 1);

    LastPos = Look(tmp.str);

    p = (char*)Get(LastPos);

    if(!p || memcmp(p, start, len)) //Still no match
        return 0;

    if(!LastPos)
    {
        ln->set(p);
        return 1;
    }

    while(--LastPos)    //Scan back
    {
        p = (char*)Get(LastPos);

        if(!p || memcmp(p, start, len))
        {
            LastPos++;
            p = (char*)Get(LastPos);
            break;
        }
    }

    WasMatch = 1;

    ln->set(p);
    return 1;
}

void IDColl::AddID(char* start, int len)
{
    unsigned pos;
    int rc;

    char* tmp = str_dup(start, len);

    pos = Look(tmp);

    if(pos < Count())
    {
        rc = Compare(tmp, Get(pos));

        if(rc)  //No match, add into list
            At(tmp, pos);
        else
            delete tmp;
    }
    else
        Add(tmp);
}

void IDColl::Refill(int mask)
{
    for(int i = 0; keywords[i].key; i++)
    {
        if(keywords[i].mask & mask)
            AddID(keywords[i].key, strlen(keywords[i].key));
    }
}

