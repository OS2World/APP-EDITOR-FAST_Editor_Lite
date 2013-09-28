/*
** Module   :PMCLIP.CPP
** Abstract :PM Clipboard interface
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  28/03/1998 	Created
*/

#include <string.h>

#include <pmproc.h>
#include <buffer.h>

extern HAB hab;

void Buffer::to_pm()
{
    if(!hab)
        return;

    char *pByte = 0;

    //Calculate size of buffer
    int sz = 0;
    int i;

    for(i = 0; i < Count(); i++)
    {
        sz += line(i)->size();
        sz += 2; //CRLF
    }
    sz++; //NULL terminator

    _inOpenClipbrd(hab);
    _inEmptyClipbrd(hab);

    if (!DosAllocSharedMem((PPVOID)&pByte, 0, sz,
        PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE | OBJ_GETTABLE))
    {
        char *ptr = pByte;

        for(i = 0; i < Count(); i++)
        {
            ptr += line(i)->export_buf(ptr);

//            if(i != (Count() - 1))
            {
            	*ptr++ = '\r';
            	*ptr++ = '\n';
            }
        }
        *ptr = 0;

        _inSetClipbrdData(hab, (ULONG) pByte, CF_TEXT, CFI_POINTER);
    }
    _inCloseClipbrd(hab);
}

void Buffer::from_pm()
{
    if(!hab)
        return;

    _inOpenClipbrd(hab);

    do
    {
        char *ClipData = 0;
        ULONG ulFormat = 0;

        _inQueryClipbrdFmtInfo(hab, CF_TEXT, &ulFormat);

	    if(ulFormat != CFI_POINTER)
            break;

        ClipData = (char *)_inQueryClipbrdData(hab, CF_TEXT);

        if(!ClipData)
        	break;

    	RemoveAll();

        char *str;
        char *ptr;
        char *tmp = new char[strlen(ClipData)+1];
        int sz = -1;

        set_column_block(1);

        strcpy(tmp, ClipData);

        for(ptr = str = tmp; *str;)
        {
            while(*str && *str != '\n')
                str++;

        	if(*str)
            {
                if(*(str - 1) == '\r')
                    *(str - 1) = 0;
                *str++ = 0;
                if(*str == '\r' || *str == '\n')
                    *str++ = 0;
            }

            PLine ln = new Line(ptr);
            //ln->touch();
            ln->detach();

            if(sz < 0)
                sz = ln->len();
            else
                if(sz != ln->len() || !sz)
                    set_column_block(0);

            Add(ln);
	        ptr = str;
        }
        delete tmp;
    }
    while(0);

    _inCloseClipbrd(hab);
    return;
}

//--------------------------------------------------------------
// Buffer->text and text->Buffer conversion
//--------------------------------------------------------------

char* Buffer::as_text()
{
    char *pByte = 0;

    //Calculate size of buffer
    int sz = 0;
    int i;

    for(i = 0; i < Count(); i++)
    {
        sz += line(i)->size();
        sz += 2; //CRLF
    }
    sz++; //NULL terminator

    if(!DosAllocMem((PPVOID)&pByte, sz, PAG_WRITE | PAG_COMMIT))
    {
        char *ptr = pByte;

        for(i = 0; i < Count(); i++)
        {
            ptr += line(i)->export_buf(ptr);

//            if(i != (Count() - 1))
            {
                *ptr++ = '\r';
                *ptr++ = '\n';
            }
        }
        *ptr = 0;
    }

    return pByte;
}

void Buffer::from_text(char *ClipData)
{
    if(!ClipData)
        return;

    char *str;
    char *ptr;
    char *tmp = new char[strlen(ClipData)+1];
    int sz = -1;

    RemoveAll();

    set_column_block(1);

    strcpy(tmp, ClipData);

    for(ptr = str = tmp; *str;)
    {
        while(*str && *str != '\n')
            str++;

        if(*str)
        {
            if(*(str - 1) == '\r')
                *(str - 1) = 0;
            *str++ = 0;
            if(*str == '\r' || *str == '\n')
                *str++ = 0;
        }

        PLine ln = new Line(ptr);
        //ln->touch();
        ln->detach();

        if(sz < 0)
            sz = ln->len();
        else
            if(sz != ln->len())
                set_column_block(0);

        Add(ln);
        ptr = str;
    }
    delete tmp;
}

