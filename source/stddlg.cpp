/*
** Module   :STDDLG.CPP
** Abstract :Standard dialogs
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  12/11/1997   	Derived from DIALOG.CPP
*/

#include <string.h>

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stddlg.h>
#include <keynames.h>
#include <version.h>

#include <history.h>
#include <fio.h>

extern EditBoxCollection Editor;

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

int AChoice(int r, int c, char **itemlist, char* Hdr)
{
    Dialog dlg(r, c, 1, 1);
    Menu* menu = new Menu(1, 1, itemlist);
    dlg.Ins(menu);
    dlg.rows = menu->rows+2;
    dlg.cols = menu->cols+2;

    if(Hdr)
        dlg.Ins(new StaticText(0, 1, 1, dlg.cols - 2, Hdr));

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);

        if((k.skey & 0x00FF) == kbEnter || (k.skey & 0x00FF) == kbGrEnter)
            return menu->first_selected();

        dlg.do_key(k);

        if(menu->selected_by_key())
            return menu->first_selected();
    }
    while(k.skey != (kbEsc | shIsCtrl));
    return -1;
}

int DoHistory(Control* pCtrl, History* pHist)
{
    Dialog dlg(pCtrl->row - 3, pCtrl->col, 7, pCtrl->cols);
    ListBox* box = new ListBox(1, 1, 5, pCtrl->cols - 2, ListBox::lbCanScroll);

    pHist->FillList(box);

    dlg.Ins(box);

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);

        int key = (k.skey & 0x00FF);

        if(key == kbEnter || key == kbGrEnter)
            return box->first_selected();

		if(key == kbDel && pHist->GetItem(box->first_selected()) &&
		   !pHist->GetItem(box->first_selected())->IsFixed())
		{
			pHist->Free(pHist->Remove(box->first_selected()));
			box->del_item(box->first_selected());
		}

		if(key == kbIns)
		{
			PListBoxItem lbi = box->get_item(box->first_selected());
			pHist->GetItem(box->first_selected())->FlipFixed();
			lbi->set_mark(pHist->GetItem(box->first_selected())->IsFixed() ? '\xFE':' ');
		}

        dlg.do_key(k);

        if(box->selected_by_key())
            return box->first_selected();
    }
    while(k.skey != (kbEsc | shIsCtrl));
    return -1;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

History EditBoxCollection::hSearch  = MAX_HISTORY;
History EditBoxCollection::hReplace = MAX_HISTORY;
History EditBoxCollection::hFlags   = MAX_HISTORY;
History EditBoxCollection::hFile    = MAX_HISTORY;

struct HistoryRec
{
    char*    pName;
    History* pList;
};

static HistoryRec pHList[] =
{
    {".FEDSEARCH" , &EditBoxCollection::hSearch  },
    {".FEDREPLACE", &EditBoxCollection::hReplace },
    {".FEDMODE"   , &EditBoxCollection::hFlags   },
    {".FEDFILE"   , &EditBoxCollection::hFile    },
    {0}
};

int StoreHistory(char* file)
{
    if(!file)
        return 0;

    for(int i = 0; pHList[i].pName; i++)
    {
        int sz = 16;
        int j;

        for(j = 0; j < pHList[i].pList->Count(); j++)
        {
            sz += strlen(pHList[i].pList->GetItemText(j)) + 1;
        }

        char* pList = new char[sz];
        pList[0] = 0;

        char* ptr = pList;

        for(j = pHList[i].pList->Count() - 1; j >= 0 ; j--)
        {
            char* p = pHList[i].pList->GetItemText(j);

            sz = strlen(p);
            memcpy(ptr, p, sz);
            ptr += sz;
            *ptr++ = pHList[i].pList->GetItem(j)->IsFixed() ? '\x02':'\x01';
        }

        *ptr = 0;

        put_ea(file, pHList[i].pName, pList);

        delete pList;
    }

    return 0;
}

int LoadHistory(char* file)
{
    if(!file)
        return 0;

    for(int i = 0; pHList[i].pName; i++)
    {
        char* pList = 0;

        pHList[i].pList->RemoveAll();

        get_ea(file, pHList[i].pName, &pList);

        if(!pList)
            continue;

        char* start = pList;
        char* end   = pList;

        for(;;)
        {
            while(*end && *end != '\x01' && *end != '\x02')
                end++;

            char ch = *end;

            *end = 0;

            PHistoryItem h = pHList[i].pList->PutItem(start);

            if(h && ch == '\x02')
            	h->FlipFixed();

            if(!ch)
                break;

            start = end + 1;
            end   = start;
        }

        delete pList;
    }
    return 0;
}

int SearchReplace(int r, int c, char *searchstr, char *replacestr, char* flags)
{
    int _rows = 7;
    int _cols = 68;

    if(_rows + r >= Rows)
        _rows = Rows - r - 2;

    if(_cols - c >= Cols)
        _cols = Cols - c - 2;

    Dialog dlg(r, c, _rows, _cols);

    dlg.Ins(new StaticText(1,1,1, 7,"Search  "));
    dlg.Ins(new StaticText(2,1,1, 7,"Replace "));
    dlg.Ins(new StaticText(3,1,1,66,"~R~eplace ~B~ackward ~G~lobal ~I~gnore ~N~o-ask r~E~gexp"));
    EditLine* lsearch  = new EditLine(1,  9, 1, 58);
    EditLine* lreplace = new EditLine(2,  9, 1, 58);
    EditLine* lflags   = new EditLine(3, 52, 1, 15);

    lsearch ->set_text(searchstr );
    lreplace->set_text(replacestr);
    lflags  ->set_text(flags     );

    dlg.Ins(lsearch );
    dlg.Ins(lreplace);
    dlg.Ins(lflags  );

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);

        if(k.skey == (kbUp | shIsCtrl) || k.skey == (kbDown | shIsCtrl))
        {
            History* pHst = 0;

            if(dlg.in_focus() == lsearch)
                pHst = &EditBoxCollection::hSearch;
            else if(dlg.in_focus() == lreplace)
                pHst = &EditBoxCollection::hReplace;
            else
                pHst = &EditBoxCollection::hFlags;

            if(pHst->Count())
            {
	            int res = DoHistory(dlg.in_focus(), pHst);

                if(res >= 0)
                {
                    ((EditLine*)dlg.in_focus())->set_text(pHst->GetItemText(res));
                }
            }
            continue;
        }

        if(((k.skey & 0x00FF) == kbEnter || (k.skey & 0x00FF) == kbGrEnter) &&
           (dlg.in_focus() == lflags || (k.skey & shCtrl)))
        {
            lsearch ->get_text(searchstr , 1024);
            lreplace->get_text(replacestr, 1024);
            lflags  ->get_text(flags     ,   32);
            EditBoxCollection::hSearch.PutItem(searchstr);
            EditBoxCollection::hReplace.PutItem(replacestr);
            EditBoxCollection::hFlags.PutItem(flags);

            return 0;
        }
        dlg.do_key(k);
    }
    while(k.skey != (kbEsc | shIsCtrl));
    return -1;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

static void FillFileList(ListBox* list, char *mask)
{
    HDIR hdirFindHandle     = HDIR_SYSTEM;
    FILEFINDBUF3 FindBuffer = {0};
    ULONG ulResultBufLen    = sizeof(FILEFINDBUF3);
    ULONG ulFindCount       = 1;
    ULONG ulDriveNum        = 0;
    ULONG ulDriveMap        = 1;
    int i;
    PListBoxItem p;

    APIRET rc = NO_ERROR;

    list->go_item(0);
    list->RemoveAll();

//--------------------------------
// Fill directory list

    rc = DosFindFirst((PCH)"*",
                      &hdirFindHandle,
                      MUST_HAVE_DIRECTORY,
                      &FindBuffer,
                      ulResultBufLen,
                      &ulFindCount,
                      FIL_STANDARD);

    while(rc == NO_ERROR)
    {
        if(!(  FindBuffer.achName[0]=='.'
          && FindBuffer.achName[1]=='\x00'))
        {
			p = list->add_at_end(FindBuffer.achName);
			p->set_user_data(1);
			p->set_mark('\x10');
        }
        ulFindCount = 1;

        rc = DosFindNext(hdirFindHandle,
                         &FindBuffer,
                         ulResultBufLen,
                         &ulFindCount);

    }

    DosFindClose(hdirFindHandle);

//--------------------------------
// Fill filename list

    ulFindCount    = 1;
    hdirFindHandle = HDIR_SYSTEM;

    rc = DosFindFirst((PCH)mask,
                      &hdirFindHandle,
                      0
                      | FILE_ARCHIVED
                      | FILE_READONLY,
                      &FindBuffer,
                      ulResultBufLen,
                      &ulFindCount,
                      FIL_STANDARD);
    while(rc == NO_ERROR)
    {
        if(!(FindBuffer.attrFile & FILE_DIRECTORY))
        {
			p = list->add_at_end(FindBuffer.achName);
			p->set_mark(' ');
        }

        ulFindCount = 1;

        rc = DosFindNext(hdirFindHandle,
                         &FindBuffer,
                         ulResultBufLen,
                         &ulFindCount);

    }

    DosFindClose(hdirFindHandle);

//--------------------------------
// Fill drives list

	rc = DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap);

	for(i = 0; i < 26; i++)
	{
		if(ulDriveMap & (1 << i))
		{
			char cDrive[8];

			cDrive[0] = '[';
			cDrive[1] = char('A' + i);
			cDrive[2] = ']';
			cDrive[3] = 0;

			p = list->add_at_end(cDrive);
			p->set_user_data(2);
		}
	}
}

int FileDialog(int r, int c, char *name, int flags)
{
    int _rows = 20;
    int _cols = 60;

    if(_rows + r >= Rows)
        _rows = Rows - r - 2;

    if(_cols - c >= Cols)
        _cols = Cols - c - 2;

    Dialog dlg(r, c, _rows, _cols);

    char *hdr = FileDialogNames[flags & 3];

    ListBox *dir_list = new ListBox(4, 1, dlg.rows - 5, dlg.cols - 2,
                                    ListBox::lbCanScroll);

    EditLine* filename  = new EditLine(2, 1, 1, dlg.cols - 2);

    filename->set_text("*");

    StaticText* stTxt1 = new StaticText(filename->row - 1, filename->col,
                                        1, filename->cols, "File Name");

    StaticText* stTxt2 = new StaticText(dir_list->row - 1, dir_list->col,
                                        1, dir_list->cols, "Directory list");

    StaticText* stTxt3 = new StaticText(0, (dlg.cols-strlen(hdr)) / 2,
                                        1, strlen(hdr), hdr);
    dlg.Ins(stTxt1);
    dlg.Ins(stTxt2);
    dlg.Ins(stTxt3);
    dlg.Ins(filename);
    dlg.Ins(dir_list);

    FillFileList(dir_list, filename->line(0)->str);

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);

        if(dlg.in_focus() == filename &&
           (k.skey == (kbUp | shIsCtrl) || k.skey == (kbDown | shIsCtrl)))
        {
            History* pHst = &EditBoxCollection::hFile;

            if(pHst->Count())
            {
                int res = DoHistory(dlg.in_focus(), pHst);

                if(res >= 0)
                {
                    ((EditLine*)dlg.in_focus())->set_text(pHst->GetItemText(res));
                }
            }
            continue;
        }

        if((k.skey & 0x00FF) == kbEnter ||
           (k.skey & 0x00FF) == kbGrEnter)
        {
            if(dlg.in_focus() == dir_list)
            {
                switch(dir_list->get_item_key(dir_list->abs_row()))
                {
                    case 1:

    	                //Change directory
                        DosSetCurrentDir((PCH)dir_list->get_item_text(dir_list->abs_row()));

            	        //Refill directory list

                	    filename->set_text("*");
                    	FillFileList(dir_list, filename->line(0)->str);
	                    continue;

                    case 0:
                        {
                        	char *s = dir_list->get_item_text(dir_list->abs_row());
                            if(name)
                            {
                                strncpy(name, s, FED_MAXPATH);
								char* fname = get_full_name(name);
                                EditBoxCollection::hFile.PutItem(fname);
                                delete fname;
                            }
                        	return 0;
                        }

                    case 2:

                        //Change current drive
                        {
                            char *dsk_name = (dir_list->get_item_text(dir_list->abs_row()) + 1);
                            DosSetDefaultDisk(*dsk_name - 'A' + 1);
                        }
                        //Refill directory list
                        filename->set_text("*");
                    	FillFileList(dir_list, filename->line(0)->str);
	                    continue;
                }
            }
            if(dlg.in_focus() == filename)
            {
                char *s = filename->line(0)->str;

                while(*s && *s == ' ')
                    s++;

                //if(strchr(s, ':'))
                if(s[0] && s[1] && !s[2] && s[1] == ':')
                {
                    //Change current drive
                    {
                        char *dsk_name = s;
                        DosSetDefaultDisk((*dsk_name | 0x20) - 'a' + 1);
                    }
                    //Refill directory list
                    filename->set_text("*");
                    FillFileList(dir_list, filename->line(0)->str);
                    continue;
                }

                if(strchr(s, '*') || strchr(s, '?'))
                {
                    //Refresh list
                    FillFileList(dir_list, s);
                }
                else
                {
                    if(name)
                    {
                        strncpy(name, s, FED_MAXPATH);
                        s = name;
                        s += strlen(name);

                        while(s > name)
                        {
                            if(*s == ' ')
                            {
                                *s-- = 0;
                                continue;
                            }
                            if(!*s)
                            {
                                s--;
                                continue;
                            }
                            break;
                        }
                        if(s > name && *name)
                        {
							char* fname = get_full_name(name);
                            EditBoxCollection::hFile.PutItem(fname);
                            delete fname;
                            return 0;
                        }
                    }
                    else
                    	return 0;
                }
            }
        }

        dlg.do_key(k);

        if(dlg.in_focus() == dir_list)
        {
            if(dir_list->get_item_key(dir_list->abs_row()) == 0)
            	filename->set_text(dir_list->get_item_text(dir_list->abs_row()));
        }
        if(dlg.in_focus() == filename)
        {
            if(!(k.skey & shIsCtrl) ||
                k.skey == (kbBksp | shIsCtrl) ||
                k.skey == (kbDel | shIsCtrl))
            {
                int candidate = -1;
                int candidate_len = 0;
                char *s = filename->line(0)->str;
                int s_len = strlen(s);

                for(int i = 0; i < dir_list->Count(); i++)
                {
                    if(dir_list->get_item_key(i) == 2)
                        break; //skip drive names

                    char * cstr = dir_list->get_item_text(i);

                    int j;

                    for(j = 0; j < s_len; j++)
                    {
                        if(__to_lower(s[j]) != __to_lower(cstr[j]))
                            break;
                    }
                    if(j > candidate_len)
                    {
                        candidate_len = j;
                        candidate     = i;
                    }
                }
                if(candidate >= 0 && candidate != dir_list->abs_row())
                    dir_list->go_item(candidate);
            }
        }
    }
    while(k.skey != (kbEsc | shIsCtrl));
    return -1;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

#define MK_CLR(clr)     (app_pal[CL_APPLICATION_START+(clr)])

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

int AskNumber(int r, int c, int *number, char *hdr)
{
    int _rows = 3;
    int _cols = 30;

    if(_rows + r >= Rows)
        _rows = Rows - r - 2;

    if(_cols - c >= Cols)
        _cols = Cols - c - 2;

    Dialog dlg(r, c, _rows, _cols);
    EditLine* in = new EditLine(1, 1, 1, dlg.cols - 2);
    dlg.Ins(in);

    if(hdr)
        dlg.Ins(new StaticText(0, (dlg.cols - strlen(hdr)) / 2, 1, strlen(hdr), hdr));

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);
        if((k.skey & 0x00FF) == kbEnter ||
           (k.skey & 0x00FF) == kbGrEnter)
        {
            char *s = in->line(0)->str;

            while(__issp(*s))
                s++;
            if(!*s)
                return -1;
            *number = 0;

            while(__isdd(*s))
            {
                *number *= 10;
                *number += *s - '0';
                s++;
            }

            if(!*number)
                return -1;

            return 0;
        }
        dlg.do_key(k);
    }
    while(k.skey != (kbEsc | shIsCtrl));

    return -1;
}

int AskString(int r, int c, char** res, char *hdr)
{
    int _rows = 3;
    int _cols = 65;

    if(!res)
        return -1;

    if(_rows + r >= Rows)
        _rows = Rows - r - 2;

    if(_cols - c >= Cols)
        _cols = Cols - c - 2;

    if(_rows < 3 || _cols < 65)
        return -1;

    Dialog dlg(r, c, _rows, _cols);

    EditLine* in = new EditLine(1, 1, 1, dlg.cols - 2);
    dlg.Ins(in);

    if(hdr)
        dlg.Ins(new StaticText(0, (dlg.cols - strlen(hdr)) / 2, 1, strlen(hdr), hdr));

    KeyInfo k;
    do
    {
        dlg.draw();
        vio_read_key(&k);
        if((k.skey & 0x00FF) == kbEnter ||
           (k.skey & 0x00FF) == kbGrEnter)
        {
            *res = str_dup(in->line(0)->str);

            return 0;
        }
        dlg.do_key(k);
    }
    while(k.skey != (kbEsc | shIsCtrl));

    return -1;
}

int MessageBox(char *orig_text, KeyInfo* lastkey, int timeout)
{
    int r = 0;
    int c = 0;
    int len;
    char *ptr;
    char *str;
    char* text;

    Dialog dlg(r, c, 2, 2);

    if(orig_text)
    {
        text = new char[strlen(orig_text)+1];
        strcpy(text, orig_text);
    }
    else
    {
        text = new char[1];
        *text = 0;
    }

    for(ptr = str = text; *str;)
    {
        while(*str && *str != '\n')
            str++;

        if(*str)
        {
            if(*(str - 1) == '\r')
                *(str - 1) = 0;
            *str++ = 0;
            if(*str == '\r' || *str == '\n')
                *str++ = 0;
        }
        //
        len = strlen(ptr);

        if(len >(Cols - 4))
            len = Cols - 4;

        if(len > c)
            c = len;
        dlg.Ins(new StaticText(r + 1, 1, 1, len, ptr));
        //
        ptr = str;
        r++;

        if((dlg.rows + r) >= (Rows - 2))
        	break;
    }

    if((dlg.rows + r) >= (Rows - 2))
        r = Rows - dlg.rows - 2;

    if((dlg.cols + c) >= (Cols - 2))
        c = Cols - dlg.cols - 2;

    dlg.size(dlg.rows + r, dlg.cols + c);
    dlg.move((Rows - dlg.rows)/2, (Cols - dlg.cols)/2);

    if(dlg.row < 0 || dlg.col < 0)
        dlg.move(0, 0);

    KeyInfo k;
    dlg.draw();

    if(!timeout)
    {
        do
        {
            vio_read_key(&k);
        }
        while(!(k.skey & 0x00FF));

        if(lastkey)
        {
            *lastkey = k;
        }
    }
    else
    {
        DosSleep(timeout);
        k.skey = 0;
    }

    delete text;

    return k.skey;
}

int JumpListBox(PJumpList pList, int r, int c, int nr, int nc)
{
    if(!pList || !pList->Count())
        return -1;

    if(r < 2 || r > (Rows - 7))
        r = 2;
    if(c < 2 || c > (Cols - 7))
        c = 2;

    if(nr <= 4)
        nr = Rows - 4;

    if(nc <= 4)
        nc = Cols - 4;

    if((nr + r) > (Rows - 2))
        nr = (Rows - 2) - r;

    if((nc + c) > (Cols - 2))
        nc = (Cols - 2) - c;

    int iFollow = 0;

    Dialog dlg(r, c, nr, nc);

    ListBox *list = new ListBox(1, 1, dlg.rows - 2, dlg.cols - 2,
                                ListBox::lbCanScroll |
                                ListBox::lbXScroll );

    dlg.Ins(list);

    for(int i = 0; i < pList->Count(); i++)
    {
        PJumpEntry pEntry = pList->get_entry(i);
        list->add_at_end(pEntry->get_name())->set_user_data((unsigned)pEntry);
    }

    list->go_item(pList->get_last_pos());

    KeyInfo k;

    int last_pos = list->first_selected();

    do
    {
        dlg.draw();
        vio_read_key(&k);

        if((k.skey & 0x00FF) == kbEnter ||
           (k.skey & 0x00FF) == kbGrEnter)
        {
        	int pos = list->first_selected();

            PJumpEntry pEntry = (PJumpEntry)list->get_item_key(pos);

            if(!pEntry)
                return -1;

            // Select file if opened or open it

            Editor.doOpenFile(pEntry->get_file());

            if(pEntry->get_row())
                Editor.current()->goto_line(pEntry->get_row() - 1);

            if(pEntry->get_col())
                Editor.current()->goto_col(pEntry->get_col() - 1);

            pList->set_last_pos(pos);
            return pos;
        }

        dlg.do_key(k);

        int pos = list->first_selected();

        if(iFollow && (last_pos != pos))
        {
            PJumpEntry pEntry = (PJumpEntry)list->get_item_key(pos);

            if(!pEntry)
                continue;

            // Select file if opened or open it

            Editor.doOpenFile(pEntry->get_file());

            if(pEntry->get_row())
                Editor.current()->goto_line(pEntry->get_row() - 1);

            if(pEntry->get_col())
                Editor.current()->goto_col(pEntry->get_col() - 1);
            //Editor.draw();
        }
    }
    while(k.skey != (kbEsc | shIsCtrl));

    return -1;
}

int CompletionListBox(Buffer* pSrc, int r, int c, int nr, int nc)
{
    if(!pSrc)
        return -1;

    if(r < 2 || r > (Rows - 7))
        r = 2;
    if(c < 2 || c > (Cols - 7))
        c = 2;

    if(nr <= 4)
        nr = Rows - 4;

    if(nc <= 4)
        nc = Cols - 4;

    if((nr + r) > (Rows - 2))
        nr = (Rows - 2) - r;

    if((nc + c) > (Cols - 2))
        nc = (Cols - 2) - c;

    int iFollow = 0;

    Dialog dlg(r, c, nr, nc);

    ListBox *list = new ListBox(1, 1, dlg.rows - 2, dlg.cols - 2,
                                ListBox::lbCanScroll | ListBox::lbXScroll );

    if(!pSrc->fill_completion_lb(list)) //Completion list is empty
        return -1;

    dlg.Ins(list);

    KeyInfo k;

    int last_pos = list->first_selected();

    do
    {
        dlg.draw();
        vio_read_key(&k);

        if((k.skey & 0x00FF) == kbEnter ||
           (k.skey & 0x00FF) == kbGrEnter)
        {
            int pos = list->first_selected();
            int item = list->get_item_key(pos);

            if(item <= 0)
                return -1;

            Editor.current()->next_completion(item - 1);
            return pos;
        }

        dlg.do_key(k);

        int pos = list->first_selected();

        if(iFollow && (last_pos != pos))
        {
            int item = list->get_item_key(pos);

            if(item <= 0)
                continue;

            // Select file if opened or open it

            Editor.current()->next_completion(item);
        }
    }
    while(k.skey != (kbEsc | shIsCtrl));

    return -1;
}

