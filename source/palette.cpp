/*
** Module   :PALETTE.CPP
** Abstract :Color palette for all visible elements of editor
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  09/11/1997   	Updated to V0.1
*/

#include <boxcoll.h>
#include <version.h>

Buffer* Clipboard = 0;
char _cFedPATH[FED_MAXPATH];
char Search[1025]  = {0};
char Replace[1025] = {0};
char Flags[33]     = {0};

char ProgressBar[]=
"\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE"
"\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE"
"\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE"
"\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE\xFE";

char* FileDialogNames[]=
{
    " File Open ",
    " File Save ",
    " File Save As ",
    ""
};

char *Yes_No[]=
{
    " ~Y~es",
    " ~N~o",
    " ~C~ancel ",
    0
};

char cPipe[] = "\\PIPE\\FED";

int cDateSep = '/';

//Configuration variables
char* StartupDir= 0;
char* untitled  = 0;
char* statusline= 0;
char* cStAcOff  = 0;
char* cStAcOn   = 0;
char* cStChange = 0;
char* cStColumn = 0;
char* cStDOS    = 0;
char* cStInd    = 0;
char* cStMerge  = 0;
char* cStNoChg  = 0;
char* cStNoInd  = 0;
char* cStStream = 0;
char* cStUnix   = 0;
char* cStUnwrap = 0;
char* cStWrap   = 0;
char* pDef      = 0;
char* help_text = 0;
char* cWordDelim= 0;
char* hi_map  	= 0;
char* rexx_pool = 0;

int iAfterBlock     = 0;
int iAutoCompl      = 0;
int iAutoUnInd      = 0;
int iBlockIns       = 0;
int iCtrlBrk     	= 0;
int iCurCP          = 0;
int iDateFmt 		= 0;
int iDefType     	= 0;
int iDefWidth    	= 0;
int iDelWS          = 0;
int iDrawEOL        = 0;
int iFileName   	= 0;
int iFlash          = 0;
int iFollowTabs     = 0;
int iForce          = 0;
int iIndBracket     = 0;
int iIndKwd         = 0;
int iIndUseTab      = 0;
int iMinComplLen    = 0;
int iMouseMask 		= 0;
int iMouseThrd      = 0;
int iNoEA       	= 0;
int iNoTabsIntoClip = 0;
int iQUndo          = 0;
int iSaveSyntax     = 0;
int iSenseShift 	= 0;
int iSpacesOnly     = 0;
int iTabWidth    	= 0;
int iUnIndBrackets  = 0;
int iUnIndKwd       = 0;
int iUpperStatus 	= 0;
int iVSearch 		= 0;
int iWWDef       	= 0;
int iEOF            = 0;

int iShape[2]       = {0};

char app_pal[256];
