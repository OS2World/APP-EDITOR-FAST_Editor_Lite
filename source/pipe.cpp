/*
** Module   :PIPE.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  05/11/2001 Created
**
*/

#define INCL_DOS
#include <os2.h>

#include <string.h>
#include <stdio.h>
#include <pipe.h>

#define TRACE_ENABLED       1
#include <version.h>

NamedPipe::NamedPipe():hPipe((unsigned long)-1)
{
    cData = new char[MAX_DATA];
}

NamedPipe::NamedPipe(char* name):hPipe((unsigned long)-1)
{
    cData = new char[MAX_DATA];
    Open(name);
}

NamedPipe::~NamedPipe()
{
    Close();
    delete cData;
}

void NamedPipe::Open(char* name)
{
    if(IsValid())
        Close();

    int rc = DosCreateNPipe((PSZ)name, &hPipe,
                            NP_NOINHERIT | NP_ACCESS_DUPLEX,
                            NP_WAIT | NP_TYPE_BYTE | NP_READMODE_BYTE | 1, /* one instance allowed */
                            4096, 4096, 512);

    if(rc)
        hPipe = (unsigned long)-1;
}

char* NamedPipe::ReadMsg()
{
    if(!IsValid())
        return 0;

    int rc = DosConnectNPipe(hPipe);

    if(rc)
        return 0;

    ULONG ulSz = 0;
    int iSz    = 0;
    cData[0]   = 0;

    for(;;)
    {
        char c;

        rc = DosRead(hPipe, &cData[iSz], (MAX_DATA - iSz), &ulSz);

        if(rc)
            break;

        if(iSz < MAX_DATA)
        {
            iSz += ulSz;
            break;
        }
    }

    DosDisConnectNPipe(hPipe);

    if(!iSz)
        return 0;

    cData[iSz] = 0;

    char* p = &cData[iSz - 1];

    while(p > cData)
    {
        if(*p == '\r' || *p == '\n' || *p == ' ')
        {
            *p-- = 0;
            continue;
        }
        break;
    }

    iSz = strlen(cData);

    if(!iSz)
        return 0;

    //Create a copy of data
    char* res = new char[iSz + 1];

    strcpy(res, cData);

    return res;
}

void NamedPipe::Close()
{
    if(!IsValid())
        return;

    DosClose(hPipe);

    hPipe = (unsigned long)-1;
}

