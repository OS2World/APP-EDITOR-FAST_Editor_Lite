/*
** Module   :CP_CVT.CPP
** Abstract :Unicode API wrapper
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  15/12/1999 Created
**
*/

//#ifdef __EMX__
//#define USE_OS2_TOOLKIT_HEADERS
//#include <os2.h>
//#endif

#include <uconv.h>
#include <stdlib.h>
#include <string.h>

int cp2cp(char *cp1, char *cp2, char *src, char *dst, int len)
{
    UconvObject ucSrc = 0;
    UconvObject ucDst = 0;

    size_t uBytes;
    size_t uUniChars;
    size_t uSubs;

    UniChar* pUniBuf;
    UniChar* pUniPtr;

    UniChar UniCP1[64];
    UniChar UniCP2[64];

    char *pStr;

    int rc = 0;

    /* Check parameters */

    if(!cp1 || !cp2 || !src || !dst || len < 0)
        return -1;

    /* Convert codepage names to UniChar */

    rc = UniCreateUconvObject((UniChar *)L"", &ucSrc);

    if(rc)
        return rc;

    pStr    = cp1;
    uBytes  = strlen(cp1)+1;    /* convert including trailing 0 */
    pUniPtr = UniCP1;
    uUniChars = (sizeof(UniCP1)/sizeof(UniChar)) + 1;

    rc = UniUconvToUcs(ucSrc, (void **)&pStr, &uBytes, &pUniPtr, &uUniChars, &uSubs);

    if(rc)
    {
        UniFreeUconvObject(ucSrc);
        return rc;
    }

    pStr    = cp2;
    uBytes  = strlen(cp2)+1;    /* convert including trailing 0 */
    pUniPtr = UniCP2;
    uUniChars = (sizeof(UniCP2)/sizeof(UniChar)) + 1;

    rc = UniUconvToUcs(ucSrc, (void **)&pStr, &uBytes, &pUniPtr, &uUniChars, &uSubs);

    /* conversion object is no more required */

    UniFreeUconvObject(ucSrc);

    if(rc)
    {
        return rc;
    }

    /* Codepages is prepared, create conversion objects */

    rc = UniCreateUconvObject(UniCP1, &ucSrc);

    if(rc)
    {
        return rc;
    }

    rc = UniCreateUconvObject(UniCP2, &ucDst);

    if(rc)
    {
        UniFreeUconvObject(ucSrc);
        return rc;
    }

    /* Conversion objects are created, prepare buffers */

    pStr      = src;
    uBytes    = len;
    uUniChars = len;

    pUniBuf = (UniChar*) malloc(uUniChars * sizeof(UniChar));

    if(!pUniBuf)
    {
        UniFreeUconvObject(ucSrc);
        UniFreeUconvObject(ucDst);
        return -2;
    }

    pUniPtr = pUniBuf;

    rc = UniUconvToUcs(ucSrc, (void **)&pStr, &uBytes, &pUniPtr, &uUniChars, &uSubs);

    if(rc)
    {
        UniFreeUconvObject(ucSrc);
        UniFreeUconvObject(ucDst);
        free(pUniBuf);
        return rc;
    }

    pStr      = dst;
    uBytes    = len;
    uUniChars = len;

    pUniPtr = pUniBuf;

    rc = UniUconvFromUcs(ucDst, &pUniPtr, &uUniChars, (void **)&pStr, &uBytes, &uSubs);

    UniFreeUconvObject(ucSrc);
    UniFreeUconvObject(ucDst);
    free(pUniBuf);

    return rc;
}

