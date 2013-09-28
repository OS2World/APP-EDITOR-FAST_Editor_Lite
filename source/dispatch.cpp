/*
** Module   :DISPATCH.CPP
** Abstract :Keyboard events dispatcher
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  26/03/1998     Created
*/

#include <string.h>

#include <boxcoll.h>
#include <keynames.h>
#include <version.h>

#define INCL_DOS
#include <os2.h>

//-----------------------------------------
// Dispatcher
//-----------------------------------------

void EditBoxCollection::Dispatcher(KeyInfo& k, int iMode)
{
    lock();
    track_beg();

    if((k.skey & 0x00FF) == kbEnter && !(k.skey & (shAlt | shCtrl | shShift)))
            k.skey &= ~shIsCtrl;

    keydef_pair* pre_key  = (iMode) ? keys.GetDef("kbBeforeKey"):0;
    keydef_pair* post_key = (iMode) ? keys.GetDef("kbAfterKey"):0;

    if(pre_key)
    {
    	pre_key = keydef_pair::copy(pre_key);	//create a copy of the def
        play_macro(pre_key->prog);
        track_recording(pre_key, 0);
    }

    if(k.skey & shIsCtrl)
    {
        keydef_pair* prog = keys.GetDef(k.KeyName);

        if(prog)
        {
        	prog = keydef_pair::copy(prog);	//create a copy of the def
            play_macro(prog->prog);
            track_recording(prog, 0);
		}
    }
    else
    {
        track_recording(0, (char)k.key);
        usual_key(k);
        doFlashBracket(*this, 0);
    }

    if(post_key)
    {
    	post_key = keydef_pair::copy(post_key);	//create a copy of the def
        play_macro(post_key->prog);
        track_recording(post_key, 0);
    }

    track_end();
    unlock();
}

void EditBoxCollection::play_macro(char *macro)
{
    if(!macro)
        return;

    KeyInfo k;

    while(*macro)
    {
        if(*macro == FUNC_ESCAPE)
        {
            macro++;

            if(PEditProc(macro))
            {
            	PEditProc proc;
            	memcpy(&proc, macro, sizeof(PEditProc));
            	macro += sizeof(PEditProc);
            	macro += proc(*this, macro);
            }
            else
            	break;
        }
        else
        {
            k.key = *macro++;
            usual_key(k);
            doFlashBracket(*this, 0);
        }
    }
}

void EditBoxCollection::track_recording(keydef_pair* macro, char chr)
{
    if(!recording)
    {
    	delete macro;
        return;
    }

    macro_rec *tmp = new macro_rec(macro, chr, 0);

    if(!head)
        head = tmp;

    if(tail)
        tail->next = tmp;

    tail = tmp;
}

keydef_pair* EditBoxCollection::track_recording_done()
{
    int len = 0;

    macro_rec *tmp;

    for(tmp = head; tmp; tmp = tmp->next)
    {
        if(tmp->macro)
            len += tmp->macro->prog_sz;
        else
            len++;
    }

	keydef_pair* out = (keydef_pair*)new char[sizeof(keydef_pair)+ len + 1];

    char *ptr = out->prog;
    out->prog_sz = len;

    for(tmp = head; tmp; tmp = tmp->next)
    {
        if(tmp->macro)
        {
        	memcpy(ptr, tmp->macro->prog, tmp->macro->prog_sz);
        	ptr += tmp->macro->prog_sz;
        }
        else
            *ptr++ = tmp->chr;
    }
    *ptr = 0;

    while(head)
    {
        tmp = head->next;
        delete head;
        head = tmp;
    }
    head = tail = 0;
    recording = 0;

    return out;
}

void EditBoxCollection::start_recording()
{
    recording = 1;
}

