/*
** Module   :FIO.CPP
** Abstract :File I/O and related routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  11/04/1997   	Extracted from EDITOR.CPP
*/

#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#ifdef __EMX__
extern "C" {
#include <os2emx.h>
}
#else
#include <os2.h>
#endif
#include <fio.h>
#include <string.h>
#include <stdlib.h>
#include <_ctype.h>
#include <_search.h>
#include <version.h>

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

int _lopen(char *name, int mode)
{
    HFILE  hFile      = 0L;
    ULONG  ulAction   = 0;
    ULONG  ulOpenMode = 0;
    ULONG  ulFlags    = OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE;
    APIRET rc         = NO_ERROR;

    switch(mode)
    {
        case OP_READ:
            ulOpenMode = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
            ulFlags   |= OPEN_ACCESS_READONLY;
            break;

        case OP_WRITE:
            ulOpenMode = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS;
            ulFlags   |= OPEN_ACCESS_WRITEONLY;
            break;

        case OP_PIPE:
            ulOpenMode = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
            ulFlags   |= OPEN_ACCESS_WRITEONLY;
            break;
    }

    if(DosOpen((PCSZ)name, &hFile, &ulAction, 0L, FILE_ARCHIVED | FILE_NORMAL,
               ulOpenMode, ulFlags, 0L))
    {
        return -1;
    }

    return hFile;
}

int _lread(int handle, void * buff, int len)
{
    ULONG  ulBytesRead    = 0;
    APIRET rc;

    rc = DosRead(handle, buff, len, &ulBytesRead);

    return (rc) ? 0 : ulBytesRead;
}

int _lwrite(int handle, void * buff, int len)
{
    ULONG  ulWrote = 0;
    APIRET rc      = NO_ERROR;

    rc = DosWrite(handle, buff, len, &ulWrote);

    return (rc) ? 0 : ulWrote;
}

int _lclose(int handle)
{
    return DosClose(handle);
}

int _lsize(int handle)
{
    FILESTATUS3 stInfo = {0};
    APIRET      rc     = NO_ERROR;

    rc = DosQueryFileInfo(handle, FIL_STANDARD, &stInfo, sizeof(FILESTATUS3));

    return (rc) ? 0:stInfo.cbFile;
}

int _file_exists(char *name)
{
    APIRET      rc;
    FILESTATUS3 stInfo = {{0}};

    rc = DosQueryPathInfo((PCH)name, FIL_STANDARD, &stInfo, sizeof(FILESTATUS3));

    return (rc) ? 0:1;
}

int match_name(char *name, char *mask)
{
    char* pName = 0;

    pName = strrchr(name,'\\');

    if(pName)
        pName++;
    else
        pName = name;

    RXSearch flt;

    if(flt.init(mask, 0) < 0)
    	return 0;

    int match_len = 0;

    if(flt.search(pName, match_len))
    	return 1;

    return 0;
}

static char *_dayNames[]=
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

int curr_date_str(char *date)
{
    DATETIME DateTime = {0};
    APIRET   rc       = 0;
    char* ptr;
    int p[3];
    int i;

    *date = 0;

    rc = DosGetDateTime(&DateTime);

    if(rc)
        return rc;

    ptr = _dayNames[DateTime.weekday];
    while(*ptr)
        *date++ = *ptr++;

    *date++ = ' ';
    *date++ = ' ';

    switch(iDateFmt)
    {
        case 1:
            p[0] = DateTime.day;
            p[1] = DateTime.month;
            p[2] = DateTime.year;
            break;

        case 2:
            p[0] = DateTime.year;
            p[1] = DateTime.month;
            p[2] = DateTime.day;
            break;

        default:
            p[0] = DateTime.month;
            p[1] = DateTime.day;
            p[2] = DateTime.year;
            break;
    }
    for(i = 0; i < 3; i++)
    {
        if(p[i] < 32)
        {
            *date++ = char(p[i] / 10 + '0');
            *date++ = char(p[i] % 10 + '0');
        }
        else
        {
            *date++ = char((p[i] / 1000)       + '0');
            *date++ = char((p[i] % 1000) / 100 + '0');
            *date++ = char((p[i] % 100)  / 10  + '0');
            *date++ = char((p[i] % 10)         + '0');
        }
        if(i < 2)
            *date++ = char(cDateSep);
    }
    *date = 0;

    return 0;
}

void make_short_name(char *name, char *buff)
{
    if(strrchr(name, '\\'))
        name = strrchr(name, '\\') + 1;

    while(*name)
        *buff++ = __to_upper(*name++);

    *buff = 0;
}

char * _ld_file(char *name)
{
    int in = _lopen(name, OP_READ);

    char *orig_file = 0;

    unsigned flen = 0;

    if(in >= 0)
        flen = _lsize(in);

    //orig_file = new char[flen + 1];

    APIRET rc = DosAllocMem((PPVOID)&orig_file,
                            flen + 1, PAG_READ | PAG_WRITE | PAG_COMMIT);

    if(rc)
        return 0;

    orig_file[flen] = 0;

    if(in >= 0)
    {
        _lread(in, orig_file, flen);
        _lclose(in);
    }

    return orig_file;
}

void _fr_file(char *data)
{
    DosFreeMem(data);
}

int get_ea(char *FileName, char *ea_name, char **ea_value)
{
    LONG rc;                    /* Ret code                   */
    UCHAR geabuff[300];         /* buffer for GEA             */
    PVOID fealist;              /* fealist buffer             */
    EAOP2 eaop;                 /* eaop structure             */
    PGEA2 pgea;                 /* pgea structure             */
    PFEA2 pfea;                 /* pfea structure             */
    HFILE handle;               /* file handle                */
    ULONG act;                  /* open action                */
    char  *content;

    content = 0;

    rc = DosOpen((PCH)FileName, &handle, &act,
                 0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_ACCESS_READONLY |
                 OPEN_SHARE_DENYREADWRITE |
                 OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_FLAGS_WRITE_THROUGH, NULL);
    if(rc)
    {
        return -1;
    }                           /* get the file status info   */

    if (DosAllocMem((PPVOID) & fealist, 0x00010000L, PAG_COMMIT | PAG_WRITE))
    {
        return -2;
    }

    /* FEA and GEA lists          */
    eaop.fpGEA2List = (PGEA2LIST) geabuff;
    eaop.fpFEA2List = (PFEA2LIST) fealist;
    eaop.oError = 0;
    pgea = &eaop.fpGEA2List->list[0];
    eaop.fpGEA2List->cbList = sizeof(ULONG) + sizeof(GEA2) + strlen(ea_name);
    eaop.fpFEA2List->cbList = (ULONG) 0xffff;

    /* fill in the EA name length */

    pgea->cbName = (BYTE) strlen(ea_name);
    strcpy(pgea->szName, ea_name);  /* fill in the name           */
    pgea->oNextEntryOffset = 0;     /* fill in the next offset    */

    /* read the extended attribute */

    rc = DosQueryFileInfo(handle, 3, (PSZ) & eaop, sizeof(EAOP2));
    DosClose(handle);           /* close the file             */

    if (eaop.fpFEA2List->cbList <= sizeof(ULONG))
        rc = -5;                /* this is error also         */

    if (rc)
    {
        DosFreeMem(fealist);    /* error, get out             */
        return -6;
    }

    pfea = &(eaop.fpFEA2List->list[0]);     /* point to the first FEA     */
    content = new char[pfea->cbValue + 1];

    content[pfea->cbValue] = 0;

    memcpy(content, ((PSZ) pfea->szName + (pfea->cbName + 1)), pfea->cbValue);
    *ea_value = content;

    DosFreeMem(fealist);        /* free our buffer            */
    return 0;
}

int put_ea(char *FileName, char *ea_name, char *ea_value)
{
    LONG rc;                    /* Ret code                   */
    PVOID fealist;              /* fealist buffer             */
    EAOP2 eaop;                 /* eaop structure             */
    PFEA2 pfea;                 /* pfea structure             */
    HFILE handle;               /* file handle                */
    ULONG act;                  /* open action                */

    rc = DosOpen((PCH)FileName, &handle, &act,
                  0L, 0, OPEN_ACTION_OPEN_IF_EXISTS,
//                  OPEN_ACCESS_READWRITE |
                  OPEN_ACCESS_WRITEONLY    |
                  OPEN_SHARE_DENYWRITE     |
                  OPEN_FLAGS_FAIL_ON_ERROR |
                  OPEN_FLAGS_WRITE_THROUGH, NULL);
    if (rc)
    {
        return -1;
    }

    if (DosAllocMem((PPVOID) & fealist, 0x00010000L, PAG_COMMIT | PAG_WRITE))
    {
        return -2;
    }

    eaop.fpFEA2List = (PFEA2LIST) fealist;  /* Set memory for the FEA     */
    eaop.fpGEA2List = NULL;                 /* GEA is unused              */
    pfea = &eaop.fpFEA2List->list[0];       /* point to first FEA         */
    pfea->fEA = '\0';                       /* set the flags              */

    /* Size of FEA name field     */
    pfea->cbName = (BYTE) strlen(ea_name);

    /* Size of Value for this one */
    pfea->cbValue = (SHORT) strlen(ea_value);

    /* Set the name of this FEA   */
    strcpy(pfea->szName, ea_name);

    /* Set the EA value           */
    memcpy(pfea->szName + (pfea->cbName + 1), ea_value, strlen(ea_value));

    pfea->oNextEntryOffset = 0; /* no next entry              */

    /* Set the total size var     */
    eaop.fpFEA2List->cbList = sizeof(ULONG) + sizeof(FEA2) +
                              pfea->cbName  + pfea->cbValue;

    /* set the file info          */

    rc = DosSetFileInfo(handle, 2, (PSZ)&eaop, sizeof(EAOP2));

    DosClose(handle);           /* Close the File             */
    DosFreeMem(fealist);        /* Free the memory            */

    return 0;
}

char* parse_pos(char *ptr, int *x, int *y)
{
    if(!ptr || !x || !y)
        return 0;

    int cX = 0;
    int cY = 0;

    while(*ptr && __isdd(*ptr))
    {
        cX *= 10;
        cX += *ptr - '0';
        ptr++;
    }

    if(!*ptr || *ptr != 'x')
        return 0;

	ptr++;

    while(*ptr && __isdd(*ptr))
    {
        cY *= 10;
        cY += *ptr - '0';
        ptr++;
    }
    *x = cX;
    *y = cY;

    return ptr;
}

char* mk_pos(char *ptr, int x, int y)
{
    if(!ptr)
        return 0;

	char num[16];

    strcpy(ptr, u2s(x, 0, num));
    strcat(ptr, "x");
    strcat(ptr, u2s(y, 0, num));

    return ptr + strlen(ptr);
}

char *get_full_name(char *fname)
{
    char *name = new char[FED_MAXPATH];

    name[0] = 0;

    DosQueryPathInfo((PCH)fname, FIL_QUERYFULLNAME, name, FED_MAXPATH);

    return name;
}

//-----------------------------------------------
// Buffered writer
//-----------------------------------------------

BlockWrite::BlockWrite(int size)
{

    buff = new char[size];
    handle = used_len = 0;
    len = size;
}

BlockWrite::~BlockWrite()
{
    if(handle)
        flush();

    delete buff;
}

void BlockWrite::Add(char *data, int size)
{
    if(!handle || !size)
        return;

    if((size+used_len) > len)
        flush();

    if(size > len) //buffer still too small to fit pice
    {
        _lwrite(handle, data, size);
        return;
    }

    memcpy(&buff[used_len], data, size);
    used_len += size;
}

void BlockWrite::flush()
{
    if(!handle || !used_len)
        return;

    _lwrite(handle, buff, used_len);
    used_len = 0;
}

void BlockWrite::Open(int hnd)
{
    handle = hnd;
}

void BlockWrite::Close()
{
    if(used_len)
        flush();
}

