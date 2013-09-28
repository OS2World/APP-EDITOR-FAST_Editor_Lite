/*
** Module   :RX.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  10/04/1998 Created
**
*/

#include <string.h>

#include <boxcoll.h>
#include <stddlg.h>
#include <keynames.h>
#include <fio.h>
#include <_search.h>
#include <version.h>
//#include <_regex.h>

#define INCL_REXXSAA
#define INCL_DOS

#ifdef __EMX__
extern "C" {
#include <os2emx.h>
}
#else
#include <rexxsaa.h>
#endif

//-----------------------------------------
// Data Types
//-----------------------------------------

typedef struct
{
    char *pszName;
    PFN pfnFunc;
} FuncListItem;

//-----------------------------------------
// Function Prototypes
//-----------------------------------------

ULONG APIENTRY fedNoparm  (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedFind    (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedInsChar (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedGetChar (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedOpen    (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedJumpList(PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedMsgBox  (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);

ULONG APIENTRY fedVarSet  (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedVarGet  (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedVarDrop (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);

ULONG APIENTRY fedInput   (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedGetClip (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedSetClip (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);

ULONG APIENTRY fedBind    (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedMenu    (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedSendKey (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);

ULONG APIENTRY fedRxMatch (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);
ULONG APIENTRY fedExec    (PCSZ, ULONG, PRXSTRING, PCSZ, PRXSTRING);

int makeRC   (PRXSTRING, LONG );
int makeRCstr(PRXSTRING, char*);
int makeRCstr(PRXSTRING, int  );

APIRET GetRexxVariable(char* name, char* value, LONG len);
APIRET SetRexxVariable(char* name, char* value, LONG len);

static char* mk_var(char *buff, char *beg, int num, char *end, int mode);

extern Buffer* launcher(char* command, char* expr, int type, int* prc);

//-----------------------------------------
// Static and Extern Data
//-----------------------------------------

extern EditBoxCollection Editor;
extern JumpList JList[];
extern DispatchEntry dispatch_list[];

FuncListItem FunctionList[]=
{
    { "fedBind"        , (PFN)fedBind    },
    { "fedFillJumpList", (PFN)fedJumpList},
    { "fedFind"        , (PFN)fedFind    },
    { "fedExec"        , (PFN)fedExec    },
    { "fedGetChar"     , (PFN)fedGetChar },
    { "fedGetClip"     , (PFN)fedGetClip },
    { "fedInput"       , (PFN)fedInput   },
    { "fedInsChar"     , (PFN)fedInsChar },
    { "fedMenu"        , (PFN)fedMenu    },
    { "fedMsgBox"      , (PFN)fedMsgBox  },
    { "fedOpenFile"    , (PFN)fedOpen    },
    { "fedOpenJumpList", (PFN)fedJumpList},
    { "fedSendKey"     , (PFN)fedSendKey },
    { "fedSetClip"     , (PFN)fedSetClip },
    { "fedVarDrop"     , (PFN)fedVarDrop },
    { "fedVarGet"      , (PFN)fedVarGet  },
    { "fedVarSet"      , (PFN)fedVarSet  },
    { "fedRxMatch"     , (PFN)fedRxMatch },
};

//-----------------------------------------
// API's
//-----------------------------------------

void InitREXX(void)
{
	int i;

	//Regular REXX API's
	for(i = 0; i < (sizeof(FunctionList)/sizeof(FunctionList[0])); i++)
	{
		RexxRegisterFunctionExe((PCSZ)FunctionList[i].pszName,
#ifdef __EMX__
								(RexxFunctionHandler*)FunctionList[i].pfnFunc);
#else
								FunctionList[i].pfnFunc);
#endif
	}

	char cName[64] = "fed";

	for(i = 0; dispatch_list[i].key; i++)
	{
		if(dispatch_list[i].key[0] == '-')
			continue;

		strcpy(cName + 3, dispatch_list[i].key);

		RexxRegisterFunctionExe((PCSZ)cName,
#ifndef __EMX__
		(PFN)
#endif
		fedNoparm);
	}
}

//-----------------------------------------------------------------------
// REXX API
//-----------------------------------------------------------------------

//-----------------------------------------
// fedRxMatch(string, regexp[, [flags] [, pat0[, pat1[...]]]])
//
// string - source string
// regexp - regular expression
// flags  - search options ('I' - ignore case)
// pat0   - variable which will contain full matching string,
// pat1...patN - variables where matching subexpressions will be stored
//-----------------------------------------

ULONG APIENTRY fedRxMatch(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    int rc = 0;
    int ignore_case = 0;

    //At least two arguments must be present
    if(lCnt < 2)
        return 40;

    if(lCnt > 2 && argv[2].strptr)
    {
    	char* s = (char*)argv[2].strptr;

    	while(*s)
    		if(__to_upper(*s++) == 'I')
    			ignore_case = 1;
    }

    RXSearch flt;

    do
    {
        rc = flt.init((char*)argv[1].strptr, ignore_case);

        if(rc)
        {
            rc = -1;
            break;
        }

        int len = 0;
        char* start = flt.search((char*)argv[0].strptr, len);

        if(!start)
        {
        	rc = 0;
        	break;
        }

        rc = 1; //Match found

        for(int i = 3; i < lCnt; i++)
        {
			int beg = 0;
			int end = 0;

			flt.get_match(i - 3, beg, end);

			SetRexxVariable((char*)argv[i].strptr, (char*)&argv[0].strptr[beg], end - beg);
        }
    }
    while(0);

    return makeRC(RC, rc);
}

//-----------------------------------------
// fedExec(command, varname [, filter[, option]])
//-----------------------------------------

ULONG APIENTRY fedExec(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    int rc = 0;
    char* flt = 0;
    int mode = 0;

    //At least two arguments must be present
    if(lCnt < 2)
        return 40;

    if(lCnt > 2 && argv[2].strptr)
    	flt = (char*)argv[2].strptr;

	if(lCnt > 3 && argv[3].strptr)
		mode = (*argv[3].strptr == '2') ? 1:0;

   	char cNum[16];
    Buffer* buf = 0;
    char* cname = new char[argv[1].strlength + 128];

    do
    {
		buf = launcher((char*)argv[0].strptr, flt, mode, &rc);

		if(!buf || rc)
			break;

        for(int i = 0; i < buf->Count(); i++)
        {
			mk_var(cname, (char *)argv[1].strptr, i + 1, 0, 1);
			SetRexxVariable(cname, buf->line(i)->str, buf->line(i)->size());
        }

		mk_var(cname, (char *)argv[1].strptr, 0, 0, 1);
		SetRexxVariable(cname, u2s(buf->Count(),0, cNum), -1);
    }
    while(0);

    delete cname;

    return makeRC(RC, rc);
}

//-----------------------------------------
// fedInput()
//-----------------------------------------

ULONG APIENTRY fedInput(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    char *hdr = 0;
    char *res = 0;
    int r = 5;
    int c = 5;
    int rc;

    if(lCnt > 3)
        return 40;

    if(lCnt > 0)
        hdr = (char*)argv[0].strptr;

    if(lCnt > 1)
        r = s2u((char*)argv[1].strptr);

    if(lCnt > 2)
        c = s2u((char*)argv[2].strptr);

    rc = AskString(r, c, &res, hdr);

    if(!rc)
    {
        int Len = strlen(res);

        makeRCstr(RC, Len + 1);

        strcpy((char*)RC->strptr, res);
        RC->strlength = Len;

        delete res;
        return 0;
    }

    RC->strlength = 0;
    return 0;
}

ULONG APIENTRY fedGetClip(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt)
        return 40;

    if(!Clipboard)
    {
        RC->strlength = 0;
        return 0;
    }

    Clipboard->from_pm();

    char *res = Clipboard->as_text();

    RC->strlength = strlen(res);
    RC->strptr    = (PCH)res;

    return 0;
}

ULONG APIENTRY fedSetClip(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt != 1 || !argv[0].strptr)
        return 40;

    if(!Clipboard)
        return 0;

    Clipboard->from_text((char*)argv[0].strptr);
    Clipboard->to_pm();

    return 0;
}

ULONG APIENTRY fedSendKey(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt != 1 || !argv[0].strlength || !argv[0].strptr)
        return 40;

    Editor.SendKey((char*)argv[0].strptr);

    RC->strlength = 0;
    return 0;
}

ULONG APIENTRY fedBind(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt)
    {
        if(!argv[0].strptr || !argv[0].strlength)
            return 40;

        char *file = _ld_file((char*)argv[0].strptr);

        if(file)
            Editor.load_profile(file);

        _fr_file(file);
    }
    else
        EditBoxCollection::doLoadProfile(Editor, 0);

    RC->strlength = 0;
    return 0;
}

ULONG APIENTRY fedVarSet(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt < 1 || lCnt > 2)
        return 40;

    if(lCnt == 1)
        Editor.get_bindery().delVar((char*)argv[0].strptr);
    else
        Editor.get_bindery().setVar((char*)argv[0].strptr, (char*)argv[1].strptr);

    RC->strlength = 0;
    return 0;
}

ULONG APIENTRY fedVarGet(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt < 1 || lCnt > 2)
        return 40;

    char *var = Editor.get_bindery().getVar((char*)argv[0].strptr);

    if(var)
    {
        int mLen = strlen(var) + 1;

        if(mLen > RC->strlength)
        {
            if(DosAllocMem((PPVOID)&RC->strptr, mLen, PAG_COMMIT | PAG_WRITE))
            {
                RC->strlength = 0;
                return 0;
            }
        }
        strcpy((char*)RC->strptr, var);
    }
    else
    {
        if(lCnt == 1)
            RC->strptr[0] = 0;
    	else
        	if(lCnt == 2)
                strcpy((char*)RC->strptr, (char*)argv[1].strptr);
    }

    RC->strlength = strlen((char*)RC->strptr);
    return 0;
}

ULONG APIENTRY fedVarDrop(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt != 1)
        return 40;

    Editor.get_bindery().dropVar((char*)argv[0].strptr);

    RC->strlength = 0;
    return 0;
}

ULONG APIENTRY fedOpen(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt > 1)
        return makeRC(RC, -1);

    if(!lCnt || !argv[0].strptr || !argv[0].strlength ||
       strchr((char*)argv[0].strptr, '?') ||
       strchr((char*)argv[0].strptr, '*'))
    {
        Editor.open();
        return makeRC(RC, 1);
    }
    else
        Editor.doOpenFile((char*)argv[0].strptr);

    return makeRC(RC, 0);
}

ULONG APIENTRY fedMsgBox(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt < 1 || lCnt > 2 || !argv[0].strptr || !argv[0].strlength)
        return makeRC(RC, -1);

    int timeout = 0;

    if(lCnt > 1)
        timeout = s2u((char*)argv[1].strptr);

    MessageBox((char*)argv[0].strptr, 0, timeout);

    return makeRC(RC, 0);
}

ULONG APIENTRY fedNoparm(PCSZ pName, ULONG lCnt, PRXSTRING, PCSZ, PRXSTRING RC)
{
    if(lCnt != 0) //No arguments should be given
        return makeRC(RC, -1);

	PDEntry pFunc = Editor.GetFunction((char*)(pName + 3));

    if(pFunc)
		pFunc->proc(Editor, 0);

    return makeRC(RC, 0);
}

ULONG APIENTRY fedInsChar(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt != 1) //One argument should be given
        return makeRC(RC, -1);

    Editor.play_macro((char*)argv[0].strptr);
    return makeRC(RC, 0);
}

ULONG APIENTRY fedGetChar(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    //If no arguments given,
    //return current string
    //With one argument, return full string
    //With two arguments, return rest of string from given pos
    //With three arguments, return part of string from given pos;

    int Row = Editor.current()->abs_row();
    int Col = 0;
    int Len = -1;

    switch(lCnt)
    {
        case 3:
            if(argv[2].strlength != 0) //Length
            {
                Len = s2u((char*)argv[2].strptr);

                if(Len == 0)
                    return makeRC(RC, -1);
            }

        case 2:

            if(argv[1].strlength != 0) //Col
            {
                Col = s2u((char*)argv[1].strptr);

                if(Col == 0)
                    return makeRC(RC, -2);

                Col--;

                if(Len < 0)
                    Len = -2;
            }

        case 1:

            if(argv[0].strlength != 0) //Row
            {
                Row = s2u((char*)argv[0].strptr);

                if(Row == 0)
                    return makeRC(RC, -3);
            }

            Row--;

        case 0:
            if(Editor.current()->Count() < Row)
                return makeRC(RC, -1);

            if(Len == -2)
                Len = Editor.current()->line(Row)->len() - Col;

            if(Len == -1)
                Len = Editor.current()->line(Row)->len();

            makeRCstr(RC, Len);

            Editor.current()->line(Row)->get_print(Col, (char*)RC->strptr, Len);
            RC->strlength = Len;
            break;

        default:
            return makeRC(RC, -4);
    }

    return 0;
}

ULONG APIENTRY fedFind(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    if(lCnt < 2)
        return makeRC(RC, -1);

    char* repl = (char*)((lCnt > 2 && argv[2].strptr) ? (char*)argv[2].strptr:"");

    int match_len = Editor.current()->search((char*)argv[1].strptr,
                                             (char*)argv[0].strptr,
                                             repl);

    return makeRC(RC, match_len);
}

ULONG APIENTRY fedMenu(PCSZ, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    int r = 0;
    int c = 0;

    if(lCnt < 3 ||
       !argv[0].strlength ||
       !argv[1].strlength ||
       !argv[2].strlength)
    {
        return makeRC(RC, -1);
    }

    r = s2u((char*)argv[0].strptr);
    c = s2u((char*)argv[1].strptr);

    int choice = -1;
    int i;

    Dialog dlg(r, c, 1, 1);
    Menu* menu = new Menu(1, 1, 0);

    for(i = 2; i < lCnt; i++)
    {
        if(argv[i].strlength)
            menu->add_menu((char*)argv[i].strptr);
    }

    dlg.Ins(menu);
    dlg.rows = menu->rows+2;
    dlg.cols = menu->cols+2;

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);

        if((k.skey & 0x00FF) == kbEnter || (k.skey & 0x00FF) == kbGrEnter)
        {
            choice = menu->first_selected();
            break;
        }

        dlg.do_key(k);

        if(menu->selected_by_key())
        {
            choice = menu->first_selected();
            break;
        }
    }
    while(k.skey != (kbEsc | shIsCtrl));

    return makeRC(RC, choice + 1);
}

ULONG APIENTRY fedJumpList(PCSZ pName, ULONG lCnt, PRXSTRING argv, PCSZ, PRXSTRING RC)
{
    char varname[512];
    char varvalue[512];
    char header[512];
    APIRET rc;
    int len;
    int i;
    int list;

    if(lCnt < 1 || argv[0].strlength == 0 || !argv[0].strptr)
        return makeRC(RC, -1);

    if(!strcmp((char*)pName, "FEDOPENJUMPLIST"))
    {
        if(lCnt < 1)
            return makeRC(RC, -2);

        list = s2u((char*)argv[0].strptr);

        if(list > 9)
            return makeRC(RC, -3);

        int r  = 0;
        int c  = 0;
        int nr = 0;
        int nc = 0;

        if(lCnt >= 2)
            r = s2u((char*)argv[1].strptr);
        if(lCnt >= 3)
            c = s2u((char*)argv[2].strptr);
        if(lCnt >= 4)
            nr = s2u((char*)argv[3].strptr);
        if(lCnt >= 5)
            nc = s2u((char*)argv[4].strptr);

        return makeRC(RC, JumpListBox(&JList[list], r, c, nr, nc));
    }

    if(lCnt != 2)
        return makeRC(RC, -2);

    list = s2u((char*)argv[1].strptr);

    if(list > 9)
        return makeRC(RC, -3);

    rc = GetRexxVariable(mk_var(varname, (char*)argv[0].strptr,0,"jump_len",0),
                         varvalue, sizeof(varvalue)-1);

    if(rc)
        return makeRC(RC, -4);

    len = s2u(varvalue);

    JList[list].RemoveAll();

    if(!len)
        return makeRC(RC, 0);

    int new_pos = -1;

    rc = GetRexxVariable(mk_var(varname, (char*)argv[0].strptr,0,"jump_start",0),
                         varvalue, sizeof(varvalue)-1);

    if(!rc)
        new_pos = s2u(varvalue);

    for(i = 0; i < len; i++)
    {
        int row;
        int col;
        APIRET rc2 = 1;

        rc = GetRexxVariable(mk_var(varname,(char*)argv[0].strptr, i, "jump_row", 1),
                             varvalue, sizeof(varvalue)-1);

        if(rc)
            return makeRC(RC, -5);

        row = s2u(varvalue);

        rc = GetRexxVariable(mk_var(varname,(char*)argv[0].strptr, i, "jump_col", 1),
                             varvalue, sizeof(varvalue)-1);

        if(rc)
            return makeRC(RC, -6);

        col = s2u(varvalue);

        rc2 = GetRexxVariable(mk_var(varname,(char*)argv[0].strptr, i, "jump_header", 1),
                             header, sizeof(header)-1);

        rc = GetRexxVariable(mk_var(varname,(char*)argv[0].strptr, i, "jump_file", 1),
                             varvalue, sizeof(varvalue)-1);
        if(rc)
            return makeRC(RC, -7);

        if(rc2)
            strcpy(header, varvalue);

        JList[list].add_entry(row, col, header, varvalue);
    }

    JList[list].set_last_pos(new_pos);

    return makeRC(RC, 0);
}



//-----------------------------------------
// Utility functions
//-----------------------------------------

int makeRC(PRXSTRING RC, LONG lData)
{
	RC->strlength = strlen(i2s(lData, 0, (char*)RC->strptr));
    return 0;
}

int makeRCstr(PRXSTRING RC, char *str)
{
    if(!str)
        return 0;

    makeRCstr(RC, strlen(str));
    strcpy((char*)RC->strptr, str);
    RC->strlength = strlen(str);
    return 0;
}

int makeRCstr(PRXSTRING RC, int Len)
{
    if(Len > RC->strlength)
    {
        DosFreeMem(RC->strptr);

        DosAllocMem((PPVOID)&RC->strptr, Len, PAG_COMMIT | PAG_WRITE);
    }

    RC->strlength = Len;
    return 0;
}

APIRET SetRexxVariable(char* name, char* value, LONG len)
{
	if(!name || !value)
		return 1;

	if(len < 0)
		len = strlen(value);

    SHVBLOCK block = {0};

	block.shvcode = RXSHV_SYSET;

    MAKERXSTRING(block.shvname, name, strlen(name));
    MAKERXSTRING(block.shvvalue, value, len);

    block.shvnamelen  = block.shvname.strlength;
	block.shvvaluelen = block.shvvalue.strlength;

	return RexxVariablePool(&block);
}

APIRET GetRexxVariable(char* name, char* value, LONG len)
{
    APIRET rc;
    SHVBLOCK block = {0};

    block.shvcode = RXSHV_SYFET;

    MAKERXSTRING(block.shvname, name, strlen(name));
    MAKERXSTRING(block.shvvalue, value, len);

    block.shvvaluelen=len;

    rc = RexxVariablePool(&block);

    if(!rc)
        value[block.shvvalue.strlength]=0;
    else
        value[0]=0;
    return rc;
}

char* mk_var(char *buff, char *beg, int num, char *end, int mode)
{
    strcpy(buff, beg);

    if(mode)
    {
    	char cNum[16];

        strcat(buff, ".");
        strcat(buff, u2s(num,0, cNum));
    }

    if(end)
    {
    	strcat(buff, ".");
    	strcat(buff, end);
    }
    return buff;
}

