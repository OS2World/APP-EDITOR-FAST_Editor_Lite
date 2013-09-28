/*
** Module   :KEYCOLL.CPP
** Abstract :Keyboard definition collection
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  27/03/1998     Created
**
*/

#include <string.h>

#include <boxcoll.h>
#include <version.h>

keydef_pair* keydef_pair::copy(keydef_pair* src)
{
	if(!src)
		return 0;

    keydef_pair* def = (keydef_pair*)new char[sizeof(keydef_pair)+src->prog_sz + 1];

    memcpy(def, src, sizeof(keydef_pair)+src->prog_sz + 1);

    return def;
}

int KeyDefCollection::Compare(Ptr p1, Ptr p2)
{
    return __cstrcmp((char*)p1, (char*)p2);
}

void KeyDefCollection::InsKey(char *buf, int namelen)
{
    char *str;

    if(!buf || namelen >= KEY_NAME_LEN)
        return;

    str = buf+namelen;

    while(__issp(*str))
        str++;

    if(!*str)
        return;

    if(*str != ':' && *str != '=')
        return;

    str++;

    int nlen = compile_keydef(str, 0);

    if(nlen <= 0)
        return;

    keydef_pair* def = (keydef_pair*)new char[sizeof(keydef_pair)+nlen + 1];

    def->prog_sz    = nlen;
    def->prog[nlen] = 0;

    compile_keydef(str, def->prog);

    memcpy(def->key, buf, namelen);
    def->key[namelen] = 0;

	unsigned index;

	if(Find(def->key, &index))
    {
        //Replace existing keydef
        Free(Remove(index));
        At(def, index);
    }
    else
    	Add(def);
}

keydef_pair* KeyDefCollection::GetDef(char *key)
{
	unsigned index;

	if(Find(key, &index))
        return (keydef_pair*)Get(index);
    return 0;
}

char* KeyDefCollection::GetDefText(char* macro)
{
	if(!macro)
		return 0;

	int sz = decompile_keydef(macro, 0);
	char* prog = new char[sz + 16];

	decompile_keydef(macro, prog);

	return prog;
}


void KeyDefCollection::RemoveDef(char* key)
{
	unsigned index;

	if(Find(key, &index))
		Free(Remove(index));
}

void KeyDefCollection::AddAssignment(char *keyname, keydef_pair* def)
{
    if(!keyname || !def)
        return;

    strncpy(def->key, keyname, KEY_NAME_LEN);

	unsigned index;

	if(Find(def->key, &index))
    {
        Free(Remove(index));
        At(def, index);
    }
    else
        Add(def);
}

