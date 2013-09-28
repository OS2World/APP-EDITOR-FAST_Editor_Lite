/*
** Module   :PIPE.H
** Abstract :Named pipe server
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  05/11/2001 Created
**
*/

#ifndef __PIPE_H
#define __PIPE_H

#define MAX_DATA        16384

class NamedPipe
{
        unsigned long hPipe;
        char* cData;

    public:

        NamedPipe();
        NamedPipe(char* name);
        ~NamedPipe();

        int IsValid()       	{ return (hPipe == (unsigned long)-1) ? 0:1;}

        char* ReadMsg();

        void Open(char*);
        void Close();
};

#endif  /*__PIPE_H*/

