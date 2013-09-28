/*
** Module   :PMPROC.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  05/04/1998 Created
**
*/

#define INCL_BASE
#define INCL_WIN
#define INCL_VIO
#define INCL_KBD
#include <os2.h>

#ifndef __PMPROC_H
#define __PMPROC_H

extern BOOL (APIENTRY *_inCloseClipbrd)(HAB);
extern HMQ  (APIENTRY *_inCreateMsgQueue)(HAB, LONG);
extern BOOL (APIENTRY *_inDestroyMsgQueue)(HMQ);
extern BOOL (APIENTRY *_inEmptyClipbrd)(HAB);
extern HAB  (APIENTRY *_inInitialize)(ULONG);
extern BOOL (APIENTRY *_inOpenClipbrd)(HAB);
extern ULONG(APIENTRY *_inQueryClipbrdData)(HAB, ULONG);
extern BOOL (APIENTRY *_inQueryClipbrdFmtInfo)(HAB, ULONG, PULONG);
extern BOOL (APIENTRY *_inSetClipbrdData)(HAB, ULONG, ULONG, ULONG);
extern BOOL (APIENTRY *_inTerminate)(HAB);

#endif  /*__PMPROC_H*/

