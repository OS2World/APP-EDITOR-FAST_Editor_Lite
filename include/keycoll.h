/*
** Module   :KEYCOLL.H
** Abstract :Key bindings container
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  27/03/1998     Created
*/

#include <collect.h>
#include <vio.h>
#include <common.h>
#include <dict.h>

#ifndef __KEYCOLL_H
#define __KEYCOLL_H

struct keydef_pair
{
    char key[KEY_NAME_LEN];
    int  prog_sz;
    char prog[1];

	static keydef_pair* copy(keydef_pair*);
};

class FunctionDictionary: public Dictionary
{
    public:
        FunctionDictionary():Dictionary(0,0,0) {}

        static void fill_functions(FunctionDictionary* p);
        static char* locate_name(void*);
};

int macro_len(char *macro);

class KeyDefCollection:public SortedCollection
{
        int compile_keydef(char *str, char *out);
        int decompile_keydef(char* macro, char* buff);

	public:
    	FunctionDictionary Func_DIC;

        KeyDefCollection():SortedCollection(),Func_DIC()
        		{
        			bDuplicates = 0;
        			FunctionDictionary::fill_functions(&Func_DIC);
        		}

        virtual int Compare(Ptr p1, Ptr p2);

        void InsKey(char *buf, int namelen);
        void AddAssignment(char *keyname, keydef_pair* def);

        void RemoveDef(char* key);
        keydef_pair* GetDef(char *key);
        keydef_pair* GetDef(unsigned ndx) { return (keydef_pair*)Get(ndx);}

        char* GetDefText(char* key);
};

#endif  /* __KEYCOLL_H */

