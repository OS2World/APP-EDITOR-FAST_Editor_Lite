/*
** Module   :LAUNCH.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  02/05/2004	Created
**
*/

#define INCL_DOS
#include <os2.h>
#include <string.h>

#include <buffer.h>
#include <_search.h>
#include <version.h>

#include <fio.h>
#include <stdio.h>

#define STDIN     0
#define STDOUT    1
#define STDERR    2

//-----------------------------------------
// Filter program output
//-----------------------------------------

static Buffer* filter_pipe(int iPipe, char* expr)
{
    APIRET rc;
    ULONG ulBytesRead = 0;
    int offset = 0;
    ULONG ulReadSz = 32;	//Initial size, it will be extended if necessary

    char* pBuff = new char[ulReadSz+1];
    Buffer* pOut = new Buffer;
	int match_len = 0;

	if(!expr || !expr[0])
		expr = ".+";

    RXSearch flt;

    if(flt.init(expr, 0) < 0)	//Case sensitive search
    {
    	//Problems with expression
    	return 0;
    }

    for(;;)
    {
        ulBytesRead = 0;

        rc = DosRead(iPipe, pBuff + offset, ulReadSz - offset, &ulBytesRead);

        if(rc || !ulBytesRead)
        	break;

		ulBytesRead += offset;
		pBuff[ulBytesRead] = 0;

		//Split buffer into single strings and pass through filter

		char* ptr;
		char* str;

		offset = 0;

    	for(ptr = str = pBuff; (str - pBuff) < ulBytesRead; )
    	{
	        while(*str && *str != '\n')
    	    {
        	    if(str[0] == '\r' && str[1] != '\n')
            	{
                	str++;
	                break;
    	        }
        	    str++;
	        }

	        if(*str)
    	    {
        	    if((str > pBuff) && str[-1] == '\r')
            	{
	                str[-1] = 0;
    	        }

	            if(str[0] == '\n')
		            *str++ = 0;
        	}
        	else
        	{
        		offset = (str - ptr);

        		if(ptr == pBuff)
				{
					//Buffer is too small for string, expand it

					ulReadSz *= 2;

					char* new_buf = new char[ulReadSz + 1];

					memcpy(new_buf, ptr, offset);
					delete pBuff;

					pBuff = new_buf;
				}
				else
        			memmove(pBuff, ptr, offset);
        		break;
        	}

        	if(flt.search(ptr, match_len))
        	{
        		PLine ln = new Line(ptr);

        		ln->detach();

	        	pOut->Add(ln);
	        }

        	ptr = str;
	    }
    }

    if(offset)
    {
		if(flt.search(pBuff, match_len))
			pOut->Add(new Line(pBuff));
    }

    delete pBuff;

    return pOut;
}

//-----------------------------------------
// Execute program
//-----------------------------------------

static int run(char* command)
{
	int rc = 0;
	char* parm = 0;

	RESULTCODES rcl = {0};

	if(!command || !command[0])
		return 2;	//ERROR_FILE_NOT_FOUND

	int len = strlen(command);

	parm = new char[len + 2];

	parm[len + 1] = 0;

	strcpy(parm, command);

	do
	{
		char* str;
		char* beg = parm;

		while(*beg && __issp(*beg))
			beg++;

		if(!*beg)
		{
			rc = 2;
			break;
		}

		str = beg;

		if(*str == '"')
		{
			while(*str && *str != '"')
				str++;
		}
		else
		{
			while(*str && !__issp(*str))
				str++;
		}
		*str = 0;

		rc = DosExecPgm(0, 0, EXEC_ASYNCRESULT, (PSZ)parm, 0, &rcl, (PSZ)parm);
	}
	while(0);

	delete parm;

	return rc;
}

//-----------------------------------------
// Make handle non-inheritable for child process
//-----------------------------------------

static void set_inherit(HFILE hFile, int i)
{
	ULONG state = 0;

	DosQueryFHState(hFile, &state);
	DosSetFHState(hFile, (state & ~OPEN_FLAGS_NOINHERIT) | (i ? 0:OPEN_FLAGS_NOINHERIT));
}

//-----------------------------------------
// Regirect given pipe into specified stream (stdout/stderr)
//-----------------------------------------

static int redirect_and_run(HFILE local, HFILE remote, int stream, char* command)
{
	int rc = 0;
	HFILE save_pipe = (HFILE)-1;

	set_inherit(local, 0);
	set_inherit(stream, 1);

	do
	{
		rc = DosDupHandle(stream, &save_pipe);

		if(rc)
			break;

		set_inherit(save_pipe, 0);

		HFILE hTmp = stream;

		rc = DosDupHandle(remote, &hTmp);

		if(rc)
			break;

		rc = DosClose(remote);

		if(rc)
			break;

		rc = run(command);

		if(rc)
			break;
	}
	while(0);

	if(save_pipe != -1)
	{
		int rc = 0;
		HFILE hTmp = stream;

		rc = DosDupHandle(save_pipe, &hTmp);
		rc = DosClose(save_pipe);
	}

	return rc;
}

//-----------------------------------------
// Launch external app, redirect and filer its output
//-----------------------------------------

Buffer* launcher(char* command, char* expr, int type, int* prc)
{
	int rc = 0;

	if(!command || !command[0])
		return 0;

	HFILE hPipe[2] = {(HFILE)-1, (HFILE)-1};

	Buffer* buf = 0;

	do
	{
		rc = DosCreatePipe(&hPipe[0], &hPipe[1], 4096);

		if(rc)
			break;

//		noinherit(hPipe[0]);
//		noinherit(hPipe[1]);

		rc = redirect_and_run(hPipe[0], hPipe[1], (type) ? STDERR:STDOUT, command);

		if(rc)
			break;

		buf = filter_pipe(hPipe[0], expr);
	}
	while(0);

	DosClose(hPipe[0]);
	DosClose(hPipe[1]);

	if(prc)
		*prc = rc;

	return buf;
}
