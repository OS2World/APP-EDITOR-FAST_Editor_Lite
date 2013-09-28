/*
** Module   :BCOMMAND.CPP
** Abstract :Editor Commands
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  25/03/1998     Created
*/

#include <stddlg.h>
#include <string.h>
#include <stdio.h>

#include <boxcoll.h>
#include <fio.h>
#include <dialog.h>
#include <keynames.h>
#include <version.h>

#define INCL_REXXSAA
#ifdef __EMX__
extern "C" {
#include <os2emx.h>
}
#else
#include <rexxsaa.h>
#ifndef RXEXIT_HANDLED
#define RXEXIT_HANDLED       0
#define RXEXIT_NOT_HANDLED   1
#define RXEXIT_RAISE_ERROR   (-1)

extern "C" APIRET APIENTRY RexxQueryExit(PCSZ, PCSZ, PUSHORT, PUCHAR);

#endif

#endif

#define MK_CLR(clr)     (app_pal[CL_APPLICATION_START+(clr)])

extern JumpList JList[];
//---- General

int EditBoxCollection::doSave(RBoxColl r, char* macro)
{
    if (r.current()->is_untitled())
		doSaveAs(r, macro);
    else
    {
        if(r.current()->save())
            MessageBox("Error saving file!");
    }
	return 0;
}

int EditBoxCollection::doSaveAs(RBoxColl r, char*)
{
	char* cName = new char[1024];

    int rc = FileDialog(5, 5, cName, 2);

    if (!rc)
    {
        if(r.current()->save_as(cName))
            MessageBox("Error saving file!");
    }

    delete cName;

    return 0;
}

int EditBoxCollection::doLoad(RBoxColl r, char*)
{
	char* cName = new char[1024];

    int rc = FileDialog(2, 2, cName, 0);

	if (!rc)
        r.doOpenFile(cName);

    delete cName;

    return 0;
}

int EditBoxCollection::doNew(RBoxColl r, char*)
{
	r.select(r.open());
    r.SendKey("kbOpen");
    return 0;
}

int EditBoxCollection::doClose(RBoxColl r, char*)
{
    r.SendKey("kbClose");

    if(!r.current()->check_save())
    {
		r.close();
		r.select(0);
    }
    return 0;
}

int EditBoxCollection::doHelpScreen(RBoxColl r, char*)
{
	Dialog dlg(2, 2, Rows - 4, Cols - 4);
	ListBox *list = new ListBox(1, 1, dlg.rows - 2, dlg.cols - 2);

    dlg.Ins(list);

    char *str;
    char *ptr;

    for(ptr = str = help_text; *str;)
    {
        char chr;

        while(*str && *str != '\n')
            str++;

        chr = *str;
        *str = 0;

        list->add_at_end(ptr);

        *str = chr;

        if(*str)
            str++;

        ptr = str;
    }

    KeyInfo k;

	do
    {
		dlg.draw();
		vio_read_key(&k);
		dlg.do_key(k);
    }
	while (k.skey != (kbEsc | shIsCtrl));
    return 0;
}

int EditBoxCollection::doNextFile(RBoxColl r, char*)
{
	r.next();
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doPrevFile(RBoxColl r, char*)
{
	r.prev();
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doSort(RBoxColl r, char*)
{
    r.current()->sort();
    return 0;
}

int EditBoxCollection::doSearch(RBoxColl r, char*)
{
	int rr;
	int rc;

	if (r.current()->get_cur_row() >= Rows / 2)
		rr = 3;
	else
		rr = Rows / 2;

	rc = SearchReplace(rr, 3, Search, Replace, Flags);

	if (!rc)
		r.search_proc();

    return 0;
}

int EditBoxCollection::doSearchAgain(RBoxColl r, char* macro)
{
    if(Search[0])
        r.search_proc();
    else
	    r.doSearch(r, macro);
    return 0;
}

int EditBoxCollection::doFileList(RBoxColl r, char*)
{
	char* cName = new char[1024];

    Dialog dlg(2,2, Rows - 4 , Cols - 4);
    ListBox *list = new ListBox(1, 1, dlg.rows - 2, dlg.cols - 2);

    dlg.Ins(list);

    int i;

    for(i = 0; i < r.Count(); i++)
    {
        EditBox* one = r.GetBox(i);

        cName[0] = one->get_changed() ? '*':' ';
        strcpy(&cName[1], one->get_fmt_name(list->cols - 1));

        list->add_at_end(cName);

        if(one == r.current())
            list->go_item(i);
    }

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);

        if((k.skey & 0x00FF) == kbEnter ||
           (k.skey & 0x00FF) == kbGrEnter)
        {
            r.select(list->first_selected());
            break;
        }
        dlg.do_key(k);
    }
    while(k.skey != (kbEsc | shIsCtrl));

    delete cName;
    return 0;
}

int EditBoxCollection::doSaveAll(RBoxColl r, char*)
{
    EditBox *cur = r.current();

    for(int i = 0; i < r.Count(); i++)
        r.GetBox(i)->save();

    return 0;
}

int EditBoxCollection::doJumpLine(RBoxColl r, char*)
{
	int rr;

	if (r.current()->get_cur_row() >= Rows / 2)
		rr = 3;
	else
		rr = Rows / 2;

	int place = 0;
	int rc = AskNumber(rr, 3, &place, "Goto Line");

	if (!rc)
    {
		r.current()->goto_line(place - 1);
	    r.current()->flash_bracket();
    }
    return 0;
}

int EditBoxCollection::doJumpCol(RBoxColl r, char*)
{
	int rr;

	if (r.current()->get_cur_row() >= Rows / 2)
		rr = 3;
	else
		rr = Rows / 2;

	int place = 0;
	int rc = AskNumber(rr, 3, &place, "Goto Column");

	if (!rc)
    {
		r.current()->goto_col(place - 1);
	    r.current()->flash_bracket();
    }
    return 0;
}

int EditBoxCollection::doExit(RBoxColl r, char*)
{
    if(r.check_save() == -1)
        r.shutdown = 0;
    else
        r.shutdown = 1;
    return 0;
}

int EditBoxCollection::doAbort(RBoxColl r, char*)
{
	r.shutdown = 1;
    return 0;
}

int EditBoxCollection::doCopyright(RBoxColl r, char*)
{
    static char * copyright[]=
    {
        "FAST Editor Lite for OS/2 "VERSION,
        "Copyright (C) Sergey I. Yevtushenko, 1997-2004",
        0
    };

    vio_cls(MK_CLR(CL_DEFAULT));
    int i;

    for(i = 0; copyright[i]; i++)
        vio_print(i + 1, 1,copyright[i], Cols, MK_CLR(CL_DEFAULT));

    vio_show();
    return 0;
}

int EditBoxCollection::doCopyright2(RBoxColl r, char* macro)
{
    doCopyright(r, macro);
    KeyInfo k;
    vio_read_key(&k);
    return 0;
}
//--------------------------------------------------------------
//---- Cursor movement

int EditBoxCollection::doLeft(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->cursor_left();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doWordLeft(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->word_left();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doRight(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->cursor_right();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doWordRight(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->word_right();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doPgUp(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->page_up();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doFileBegin(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->text_begin();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doPgDn(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->page_down();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doFileEnd(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->text_end();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doHome(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->line_begin();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doEnd(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->line_end();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doUp(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->cursor_up();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doDown(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->cursor_down();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doMatchBracket(RBoxColl r, char*)
{
	r.current()->unmark();
	r.current()->match_bracket();
    r.current()->flash_bracket();
    return 0;
}

//---- Block marking
int EditBoxCollection::doLeftMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->cursor_left();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doWordLeftMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->word_left();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doRightMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->cursor_right();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doWordRightMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->word_right();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doPgUpMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->page_up();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doFileBeginMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->text_begin();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doPgDnMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->page_down();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doFileEndMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->text_end();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doHomeMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->line_begin();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doEndMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->line_end();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doUpMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->cursor_up();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doDownMark(RBoxColl r, char*)
{
	r.current()->mark();
    r.current()->cursor_down();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doMatchBracketMark(RBoxColl r, char*)
{
	r.current()->mark();
	r.current()->match_bracket();
    r.current()->flash_bracket();
    return 0;
}

//---- Editing commands

int EditBoxCollection::doUpper(RBoxColl r, char*)
{
    r.current()->toupper();
    return 0;
}
int EditBoxCollection::doLower(RBoxColl r, char*)
{
    r.current()->tolower();
    return 0;
}
int EditBoxCollection::doBksp(RBoxColl r, char*)
{
	r.current()->back_space();
	r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doDel(RBoxColl r, char*)
{
	if (r.current()->get_mark_state())
		r.current()->clear();
	else
		r.current()->del_char();
    r.current()->unmark();
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doDelLine(RBoxColl r, char*)
{
	PLine line = r.current()->del_line(r.current()->abs_row());
	delete line;
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doDupLine(RBoxColl r, char*)
{
    r.current()->dup_line(r.current()->abs_row());
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doDelWordLeft(RBoxColl r, char*)
{
	r.current()->del_word_left();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doDelWordRight(RBoxColl r, char*)
{
	r.current()->del_word_right();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doDelToEOL(RBoxColl r, char*)
{
    r.current()->del_to_EOL();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doIndent(RBoxColl r, char*)
{
    r.current()->indent();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doUnindent(RBoxColl r, char*)
{
    r.current()->unindent();
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doCut(RBoxColl r, char*)
{
	delete Clipboard;

    Clipboard = r.current()->cut();
    r.current()->flash_bracket();
    return 0;
}
int EditBoxCollection::doCopy(RBoxColl r, char*)
{
	delete Clipboard;

    Clipboard = r.current()->copy();
    return 0;
}

int EditBoxCollection::doPaste(RBoxColl r, char*)
{
    if (r.current()->get_mark_state())
	{
        r.current()->clear();
        r.current()->unmark();
	}
    r.current()->paste(Clipboard);
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doUndo(RBoxColl r, char*)
{
    r.current()->track_cancel();

    if(iQUndo && r.current()->is_cursor_only())
    {
        do
        {
            r.current()->undo();
        }
        while(r.current()->is_cursor_only());
    }
    else
        r.current()->undo();
    return 0;
}

//---- State commands
int EditBoxCollection::doIns(RBoxColl r, char*)
{
	r.current()->set_ins_mode(1 - r.current()->get_ins_mode());
    return 0;
}
int EditBoxCollection::doFlipBlockMode(RBoxColl r, char*)
{
	r.current()->set_column_block(1 - r.current()->get_column_block());
    return 0;
}
int EditBoxCollection::doFlipAutoindent(RBoxColl r, char*)
{
	r.current()->set_auto_indent(1 - r.current()->get_auto_indent());
    return 0;
}
int EditBoxCollection::doFlipHiliting(RBoxColl r, char*)
{
	r.current()->flip_hiliting();
    return 0;
}
int EditBoxCollection::doFlipType(RBoxColl r, char*)
{
    r.current()->set_unix(1 - r.current()->get_unix());
    return 0;
}

int EditBoxCollection::doInsDate(RBoxColl r, char*)
{
    char buff[FED_MAXPATH];

    if(!curr_date_str(buff))
        r.play_macro(buff);
    return 0;
}

int EditBoxCollection::doInsFileName(RBoxColl r, char*)
{
    r.play_macro(r.current()->get_name());
    return 0;
}

int EditBoxCollection::doInsFileNameShort(RBoxColl r, char*)
{
    char buff[FED_MAXPATH];
    make_short_name(r.current()->get_name(), buff);
    r.play_macro(buff);
    return 0;
}

//---- Macro Recorder commands

int EditBoxCollection::doMacroRecStart(RBoxColl r, char*)
{
    r.start_recording();
    return 0;
}

int EditBoxCollection::doMacroRecEnd(RBoxColl r, char*)
{
    if(!r.recording)
        return 0;

    KeyInfo k;
    MessageBox("Press key to assign recorded macro\n <ESC> - cancel", &k);

	keydef_pair* def = r.track_recording_done();

    if(k.skey != (kbEsc | shIsCtrl))
        r.keys.AddAssignment(k.KeyName, def);
	else
	    delete def;

    return 0;
}

int EditBoxCollection::doSetMark(RBoxColl r, char* macro)
{
    r.current()->bmk_place(*macro);
    return 1;
}

int EditBoxCollection::doGotoMark(RBoxColl r, char* macro)
{
    r.current()->bmk_go(*macro);
    return 1;
}

int EditBoxCollection::doJumpList(RBoxColl r, char* macro)
{
    if(*macro >= 0 && *macro <= 9)
        JumpListBox(&JList[*macro], 0, 0, 0, 0);
    return 1;
}

int EditBoxCollection::doSetXlat(RBoxColl r, char*)
{
    int rr;

    if (r.current()->get_cur_row() >= Rows / 2)
        rr = 3;
    else
        rr = Rows / 2;

    int place = 0;
    int rc = AskNumber(rr, 3, &place, "Code page ");

    if (!rc && place > 0)
    {
        char cp[20];

        strcpy(cp, "IBM-");
        i2s(place, 0, cp + 4);

        r.current()->set_xlate(cp);
    }
    return 0;
}

int EditBoxCollection::doHilitingChoice(RBoxColl r, char*)
{
    char** hl_names = new char* [Parser::Count()+1];

    int i;

    for(i = 0; i < Parser::Count(); i++)
        hl_names[i] = Parser::Name(i, 1);

    hl_names[i] = 0;

    int rc = AChoice(5, 5, hl_names, "Mode");

    if(rc >= 0)
    {
        r.current()->set_hiliting(Parser::Type(rc));
        r.current()->fill_hiliting(0, ST_INITIAL);
    }

    delete hl_names;
    return 0;
}

int EditBoxCollection::doLoadProfile(RBoxColl r, char*)
{
    int sz = 0;
    int i;

    for(i = 0; i < r.current()->Count(); i++)
    {
        sz += r.current()->line(i)->len();
        sz += 2; //CRLF
    }
    sz++; //NULL terminator

    char *pByte = new char[sz];

    char *ptr = pByte;

    for(i = 0; i < r.current()->Count(); i++)
    {
        r.current()->line(i)->build_print(ptr);
        ptr += r.current()->line(i)->len();
        *ptr++ = '\r';
        *ptr++ = '\n';
        *ptr   = 0;
    }

    r.load_profile(pByte);

    delete pByte;
    return 0;
}

int EditBoxCollection::doFlipWordWrap(RBoxColl r, char*)
{
    r.current()->ww_set(r.current()->ww_get() ^ WW_STATE);
    return 0;
}

int EditBoxCollection::doFlipWWMerge(RBoxColl r, char*)
{
    r.current()->ww_set(r.current()->ww_get() ^ WW_MERGE);
    return 0;
}

int EditBoxCollection::doFlipWWLong(RBoxColl r, char*)
{
    r.current()->ww_set(r.current()->ww_get() ^ WW_LONG);
    return 0;
}

int EditBoxCollection::doFlashBracket(RBoxColl r, char*)
{
    r.current()->flash_bracket();
    return 0;
}

int EditBoxCollection::doTouchAll(RBoxColl r, char*)
{
    r.current()->touch_all();
    return 0;
}

int EditBoxCollection::doNextCompletion(RBoxColl r, char*)
{
    r.current()->next_completion();
    return 0;
}

int EditBoxCollection::doFlipCompletion(RBoxColl r, char*)
{
    r.current()->set_auto_completion(1 - r.current()->get_auto_completion());
    return 0;
}

int EditBoxCollection::doSelectCompletion(RBoxColl r, char*)
{
    int rr;
    int cc;
    int nr = 10;
    int nc = 20;

    if(r.current()->get_cur_row() < (Rows - nr))
        rr = r.current()->get_cur_row() + 2;
    else
        rr = r.current()->get_cur_row() - nr;

    if(r.current()->get_cur_col() < (Cols - nc))
        cc = r.current()->get_cur_col();
    else
        cc = r.current()->get_cur_col() - nc;

    CompletionListBox(r.current(), rr, cc, nr, nc);
    return 0;
}

int EditBoxCollection::doLastFile(RBoxColl r, char*)
{
	r.select_last();
	r.current()->flash_bracket();
	return 0;
}

int EditBoxCollection::doMarkWord(RBoxColl r, char*)
{
	r.current()->mark_word();
    return 0;
}

int EditBoxCollection::doFileStat(RBoxColl r, char*)
{
	char cBuff[128];
	char cNum[16];

	cBuff[0] = 0;
	strcat(cBuff, "Buffer memory usage: ");
	strcat(cBuff, u2s(r.current()->memory(), 0, cNum));
	strcat(cBuff, "\n");
	strcat(cBuff, "Undo memory usage:   ");
	strcat(cBuff, u2s(r.current()->undo_size(), 0, cNum));

	MessageBox(cBuff);
	return 0;
}

int EditBoxCollection::doStartupScreen(RBoxColl r, char*)
{
	r.show_startup_screen();

	return 0;
}

//-----------------------------------------
// REXX macro handler
//-----------------------------------------

#ifdef __EMX__
#define FNIO	(PUCHAR)"FEDREXXIO"
#else
#define FNIO	"FEDREXXIO"
#endif

LONG APIENTRY HandleIO(LONG func, LONG subfunc, PUCHAR parm)
{
	if(func != RXSIO)
		return RXEXIT_NOT_HANDLED;

	if(subfunc != RXSIOTRC)
		return RXEXIT_NOT_HANDLED;

	char** userdata[2] = {0};
	USHORT exists;

	if(RexxQueryExit(FNIO, NULL, &exists, (PUCHAR)userdata))
		return RXEXIT_RAISE_ERROR;

	typedef struct
	{
		RXSTRING text;
	}
	RXPARAM;

	RXPARAM* pParm = (RXPARAM*)parm;

	if(*userdata[0])
	{
		char* new_var = merge2str(*userdata[0], (char*)pParm->text.strptr, "\n");
		delete *userdata[0];
		*userdata[0] = new_var;
	}
	else
	{
		*userdata[0] = merge2str("Error reported by REXX Interpreter:",
								(char*)pParm->text.strptr, "\n");
	}

	return RXEXIT_HANDLED;
}

static RXSYSEXIT exit_list[] =
{
	{ FNIO, RXSIO},
	{ 0   , RXENDLST}
};


int EditBoxCollection::doRexx(RBoxColl r, char* proc)
{
    RXSTRING INSTORE[2];
    RXSTRING arg;
    RXSTRING rexxretval;
    SHORT    rexxrc = 0;
    APIRET   rc;
    char* message = 0;
    char** userdata[2] = {&message, 0};

//-----------------------------------------
// Register IO exit
//-----------------------------------------

	RexxRegisterExitExe(FNIO, (PFN)HandleIO, (PUCHAR)userdata);

//-----------------------------------------
// Call REXX interpreter
//-----------------------------------------

    int proc_len = strlen(proc);
    char* prog = merge2str(proc, rexx_pool, "\r\n");

    INSTORE[0].strptr    = (PCH)prog;
    INSTORE[0].strlength = strlen(prog);
    INSTORE[1].strptr    = 0;
    INSTORE[1].strlength = 0;

    arg.strptr    		 = (PCH)"";
    arg.strlength 		 = 0;
    rexxretval.strptr    = 0;
    rexxretval.strlength = 0;

    rc=RexxStart((LONG)      1,
                (PRXSTRING)  &arg,
                (PSZ)        "FED_RX",
                (PRXSTRING)  INSTORE,
                (PSZ)        "FED",
                (LONG)       RXCOMMAND,
                (PRXSYSEXIT) exit_list,
                (PSHORT)     &rexxrc,
                (PRXSTRING)  &rexxretval);

//-----------------------------------------
// Cleanup
//-----------------------------------------

    DosFreeMem(rexxretval.strptr);

    delete prog;

	RexxDeregisterExit(FNIO, NULL);

	if(message)
	{
		MessageBox(message);
		delete message;
	}

    return proc_len;
}

