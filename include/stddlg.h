/*
** Module   :STDDLG.H
** Abstract :Standard controls
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  12/11/1997   	Created
*/

#include <boxcoll.h>
#include <dialog.h>
#include <history.h>

#ifndef  __STDDLG_H
#define  __STDDLG_H

#define MAX_HISTORY 32

//-----------------------------------------------------------
// Standard controls
//-----------------------------------------------------------

// Input:
//      r        - Row
//      c        - Col
//      itemlist - array of pointers to text strings, last item must be NULL
//      title    - dialog title
// Output:
//      number of selected item or -1 (if Esc key was pressed)
int AChoice(int r, int c, char **itemlist, char* title = 0);

// Input:
//      r         - Row
//      c         - Col
//      searchstr - Initial value for search string
//      replacestr- Initial value for replace string
//      flags     - Initial value for flags string
//
// Output:
//      0 if success, -1 cancel
//      if success searchstr, replacestr and flags filled with new values.
// Attention:
//      function assumes size of searchstr, replacestr for 1024 bytes,
//      flags - 32 bytes.
int SearchReplace(int r, int c, char *searchstr, char *replacestr, char* flags);
int FileDialog(int r, int c, char *name, int flags);
int AskNumber(int r, int c, int *number, char *hdr);
int AskString(int r, int c, char** res, char *hdr);
int MessageBox(char *text, KeyInfo* lastkey = 0, int timeout = 0);

int JumpListBox(PJumpList, int r, int c, int nr, int nc);

int DoHistory(Control* pCtrl, History* pHist);
int LoadHistory(char* file);
int StoreHistory(char* file);

int CompletionListBox(Buffer* pSrc, int r, int c, int nr, int nc);

#endif //__STDDLG_H
