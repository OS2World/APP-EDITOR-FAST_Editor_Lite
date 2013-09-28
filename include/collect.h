/*
** Module   :COLLECT.H
** Abstract :Class Collection and SortedCollection
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  13/03/1994 	Updated
**      Sat  03/05/1997 	Some methods inlined, to achieve maximum performance
*/

#ifndef  __COLLECT_H
#define  __COLLECT_H

class Collection;
typedef Collection* PCollection;
typedef Collection& RCollection;

typedef void * Ptr;

typedef void (*ForEachFunc)(Ptr);

class Collection
{
    protected:

        Ptr * ppData;
        unsigned     dwLast;
        unsigned     dwCount;
        unsigned     dwDelta;
        int      bDuplicates;

        void remove_items(PCollection dest, unsigned from, unsigned count);
        void move_items(PCollection src, unsigned from);

    public:

        //ÄÄÄÄÄ New

        Collection(unsigned aCount =1024, unsigned aDelta =1024);
        virtual ~Collection();
        Ptr Get(unsigned index) { return (index < dwLast) ? ppData[index]:0;}

        Ptr Remove(unsigned);

        virtual void Add(Ptr);
        virtual void At(Ptr, unsigned);
        virtual void Free(Ptr p);

        void  ForEach(ForEachFunc);
        unsigned Count()                {return dwLast;}
        void  RemoveAll();

};

class SortedCollection;
typedef SortedCollection* PSortedCollection;
typedef SortedCollection& RSortedCollection;

class SortedCollection:public Collection
{
    public:

        //ÄÄÄÄÄ New

        SortedCollection(unsigned aCount = 10, unsigned aDelta = 5):Collection(aCount, aDelta)
                        { bDuplicates = 1;}

        virtual int Compare(Ptr p1, Ptr p2)
                        {return *((int *)p1) - *((int *)p2);}

        unsigned Look(Ptr);		//Return index of the found key or place to insert new one

        int Find(Ptr, unsigned* = 0);	//Return 1 if key found, 0 otherwise

        //ÄÄÄÄÄ Replaced

        virtual void Add(Ptr);
};

#endif //__COLLECT_H
