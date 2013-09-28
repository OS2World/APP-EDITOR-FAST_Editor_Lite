/*
** Module   :BOXCOLL.CPP
** Abstract :Edit box collection
**
** Copyright (C) Sergey I. Yevtushenko
** Copyright (C) Dmitry Froloff
**
** Log: Mon  16/06/1997 	Created
**      Tue  17/04/2001     Added mouse support
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include <fio.h>
#include <stddlg.h>
#include <keynames.h>

#define TRACE_ENABLED       1
#include <version.h>
#include <rx.h>

#define INCL_DOS
#define INCL_MOU
#include <os2.h>

#define MK_CLR(clr)     (app_pal[CL_APPLICATION_START+(clr)])

#ifdef __IBMCPP__
#define CC _Optlink
#else
#define CC
#endif

static PMObj* ppTmr = 0;
static PMObj* ppMou = 0;
static PMObj* ppPip = 0;

//static void APIENTRY thTimer(ULONG data)
static void CC thTimer(void* data)
{
    EditBoxCollection* pEdit = (EditBoxCollection*) data;
    int i = 0;
    int state = get_fg_state();

    if(!pEdit)
        return;

    ppTmr = new PMObj;

    while(!pEdit->isDown())
    {
        DosSleep(100);

        int new_state = get_fg_state();

        if(new_state != state)
        {
            state = new_state;

            pEdit->lock();
            pEdit->SendKey(state ? "kbActivate":"kbDeactivate");
            pEdit->draw();
            pEdit->unlock();
        }

        i++;

        if(i == 10)
        {
            pEdit->lock();
            pEdit->SendKey("kbTimer");
            pEdit->draw();
            pEdit->unlock();
            i = 0;
        }
    }
}

//static void APIENTRY thMouse(ULONG data)
static void CC thMouse(void* data)
{
    MOUEVENTINFO Event;
    //USHORT MouHandle = 0;
    USHORT ReadType = 1;
    APIRET rc;

    ppMou = new PMObj;

    EditBoxCollection* pEdit = (EditBoxCollection*) data;

    rc = MouOpen( 0L, &pEdit->MouHandle);

    if(rc)
        return;

    rc = MouDrawPtr(pEdit->MouHandle);

    while(!pEdit->isDown())
    {
        rc = MouReadEventQue(&Event, &ReadType, pEdit->MouHandle);

        if(Event.fs & iMouseMask)
        {
            pEdit->lock();

            if(pEdit->current())
            {
                pEdit->track_beg();

                if(iSenseShift)
                {
                    if(kiLastKey.skey & shShift)
                        pEdit->current()->mark();
                    else
                        pEdit->current()->unmark();
                }

                if(iUpperStatus)
                    pEdit->SetMouse(Event.row - 1, Event.col);
                else
                    pEdit->SetMouse(Event.row, Event.col);

                pEdit->track_end();

                pEdit->draw();
            }
            pEdit->unlock();
        }
    }

    MouClose(pEdit->MouHandle);
}

//static void APIENTRY thPipe(ULONG data)
static void CC thPipe(void* data)
{
    EditBoxCollection* pEdit = (EditBoxCollection*) data;

    if(!pEdit)
        return;

    ppPip = new PMObj;

    while(!pEdit->isDown())
    {
        if(!pEdit->npFED.IsValid())
        {
            DosSleep(200);
            pEdit->npFED.Open(cPipe);
        }
        else
        {
            char* s = 0;

            do
            {
                s = pEdit->npFED.ReadMsg();

//printf("%d - %d - %d\n", s, pEdit->npFED.IsValid(), pEdit->isDown());

	            if(!s || !pEdit->npFED.IsValid() || pEdit->isDown())
                    break;

                char* p = s;

//printf("[%s]\n", s);

                if(!memicmp(p, "open ", 5))
                {
                    p += 5;

                    while(__issp(*p))
                        p++;

                    if(!*p)
                        break;

                    pEdit->lock();
                    pEdit->doOpenFile(p);
                    pEdit->draw();
                    pEdit->unlock();
                    break;
                }

                if(!memicmp(p, "cursor ", 7))
                {
                    p += 7;

                    while(__issp(*p))
                        p++;

                    if(!*p)
                        break;

                    pEdit->lock();
                    pEdit->set_xy(p);
                    pEdit->draw();
                    pEdit->unlock();
                    break;
                }
                             // kbPipe =
                if(!memicmp(p, "execute ", 8))
                {
                    memcpy(p, "kbPipe =", 8);
                    pEdit->lock();
                    pEdit->CompileKey(p, 6);
                    pEdit->SendKey("kbPipe");
                    pEdit->draw();
                    pEdit->unlock();
                    break;
                }

//printf("no match!\n", s);
                //Future extensions may go here..
            }
            while(0);
            delete s;
        }
    }
}

//-----------------------------------------
//
//-----------------------------------------

Initor::Initor()
{
    __nls_init();
    vio_init();
    InitREXX();
}

//-----------------------------------------
//
//-----------------------------------------

int INIList::Compare(Ptr p1, Ptr p2)
{
	return stricmp((char*)p1, (char*)p2);
}

//-----------------------------------------
//
//-----------------------------------------

EditBoxCollection::EditBoxCollection():Collection(10,2), init(), keys(), vars(),
										npFED(cPipe), MouHandle(0)
{
	cName = new char[1024];

    cur_box   = 0;
    shutdown  = 0;
    recording = 0;
    last_box  = 0;

    head = tail = 0;

    DosCreateMutexSem(0, &hMtx, 0, 0);

    Clipboard = new Buffer;
    Clipboard->add_line(new Line);

	save_screen = vio_save_box(0, 0, Rows, Cols);
}

EditBoxCollection::~EditBoxCollection()
{
	delete cName;
	vio_restore_box(save_screen);
}

void EditBoxCollection::show_startup_screen()
{
	vio_restore_box(save_screen);
	vio_show();

    KeyInfo k;
    vio_read_key(&k);

	save_screen = vio_save_box(0, 0, Rows, Cols);
}

void EditBoxCollection::Init()
{
	vars.LoadDefaults();

    load_profile(0);    //Global profile
    load_profile(1);    //Local profile

    if(pDef && pDef[0])
        strcpy(Flags, pDef);

    if(!keys.Count())
    {
        shutdown = 1;
    }

    open();

    if(iMouseThrd)
        mTID = _beginthread(thMouse, 0, 16384, this);
    tTID = _beginthread(thTimer, 0, 16384, this);
    pTID = _beginthread(thPipe , 0, 16384, this);
}

void EditBoxCollection::lock()
{
    DosRequestMutexSem(hMtx, (ULONG)-1);
}

void EditBoxCollection::unlock()
{
    DosReleaseMutexSem(hMtx);
}

void EditBoxCollection::CompileKey(char* str, int j)
{
    keys.InsKey(str, j);
}

void EditBoxCollection::Done()
{
    StoreHistory();
    shutdown = 1;

    lock();
    RemoveAll();

//    MouClose(MouHandle);
    npFED.Close();
    DosCloseMutexSem(hMtx);

    DosSleep(1);
    DosSleep(1);
    DosSleep(1);

    delete ppTmr;
    delete ppMou;
    delete ppPip;

    vio_shutdown();
}

void EditBoxCollection::Free(Ptr p)
{
    delete (EditBox*)p;
}

void EditBoxCollection::SendKey(const char *key)
{
    KeyInfo k;

    k.skey = shIsCtrl;
    strcpy(k.KeyName, key);
    Dispatcher(k);
}

void EditBoxCollection::SetMouse(int new_row, int new_col)
{
    current()->put_mouse(new_row, new_col);
}

EditBox* EditBoxCollection::current()
{
    return GetBox(cur_box);
}

EditBox* EditBoxCollection::next()
{
	int box = cur_box + 1;

    if(box >= Count())
        box = 0;

    return select(box);
}

EditBox* EditBoxCollection::prev()
{
    int box = cur_box - 1;

    if(box < 0)
        box = Count()-1;

    return select(box);
}

EditBox* EditBoxCollection::select(int new_cur)
{
    if(new_cur < Count())
    {
        if(cur_box < Count())
	        last_box = cur_box;

		vars.setDefaultProfile();
        cur_box = new_cur;
        vars.setCurrentProfile();
    }
    return current();
}

EditBox* EditBoxCollection::select_last()
{
	return select(last_box);
}

EditBox* EditBoxCollection::locate(int num)
{
    for(int i = 0; i < Count(); i++)
    {
        if(GetBox(i)->number() == num)
        {
            select(i);
            break;
        }
    }
    return current();
}

void EditBoxCollection::select(EditBox* new_box)
{
    for(int i = 0; i < Count(); i++)
    {
        if(new_box == GetBox(i))
        {
            select(i);
            break;
        }
    }
}

EditBox* EditBoxCollection::open()
{
    EditBox* new_box = new EditBox(0,0, Rows - 1, Cols);

    if(iUpperStatus)
        new_box->row++;

    Add(new_box);

    return new_box;
}

void EditBoxCollection::close()
{
    EditBox* box = (EditBox *)Remove(cur_box);
    delete box;

    if(!Count())
        open();
}

int EditBoxCollection::opened(char *fname)
{
    char* _cname = get_full_name(fname);

    for(int i = 0; i < Count(); i++)
    {
        if(!__cstrcmp(GetBox(i)->get_name(), _cname))
        {
            delete _cname;
            return i;
        }
    }
    delete _cname;
    return -1;
}

void EditBoxCollection::draw()
{
    if(!current())
        return;

    current()->draw();

    char* str = statusline;
    char* ptr = cName;

    if(!str || !ptr)
        return;

    int fminlen = 0;

    char cNum[16];

    for(;*str; str++)
    {
        if(*str == '%')
        {
            char *out = 0;
            int minlen = 0;
            str++;

            if(!*str)
                break;

            while(__isdd(*str))
            {
                minlen *= 10;
                minlen += *str++ - '0';
            }

            switch(*str)
            {
                case 'n': out = u2s(current()->number(), 0, cNum);      	            break;
                case 'r': out = u2s(current()->get_edit_row(), minlen, cNum);           break;
                case 'c': out = u2s(current()->get_edit_col(), minlen, cNum);           break;
                case 'd': out = u2s(current()->get_cur_char(), minlen, cNum);           break;
                case 'p': out = u2s(current()->get_rel_pos() , minlen, cNum);           break;
                case 'x': out = c2x(current()->get_cur_char(), cNum);                   break;
                case 'h': out = Parser::NameByKey(current()->get_hiliting());           break;
                case 'l': out = current()->get_cur_cp();                                break;
                case 'a': out = (current()->get_auto_indent() ? cStInd:cStNoInd); 		break;
                case 'o': out = (current()->get_auto_completion() ? cStAcOn:cStAcOff);	break;
                case 'u': out = (current()->get_saved() ? cStNoChg:cStChange);			break;
                case 't': out = (current()->get_unix() ? cStUnix:cStDOS);				break;
                case 'm': out = (current()->get_column_block() ? cStColumn:cStStream);	break;
                case 'f': out = current()->get_fmt_name(minlen, '°'); fminlen = minlen;	break;
                case 'w':
                    if(!(current()->ww_get() & WW_STATE))
                        out = cStUnwrap;
                    else
                        out = (current()->ww_get() & WW_MERGE ? cStMerge:cStWrap);
                    break;

                default:
                    if(*str)
                        *ptr++ = *str;
                    break;
            }

            if(out)
                while(*out)
                    *ptr++ = *out++;
            continue;
        }
        *ptr++ = *str;
    }
    *ptr = 0;

    int sRow = (iUpperStatus) ? (0):(Rows - 1);

    vio_print2(sRow, 0, cName, Cols, MK_CLR(CL_STATUSLINE));

    if(recording)
        vio_print2(sRow, Cols - 1, "R", 1, 0xF0);

    strcpy(cName, "FED-");
    strcat(cName, current()->get_fmt_name(fminlen));
    set_title(cName);
}

int EditBoxCollection::check_save()
{
    if(!current())
        return 0;

    EditBox* cur = current();

    do
    {
        //if(current()->get_changed())
        if(!current()->get_saved())
        {
            draw();

            int rc = AChoice(5, 5, Yes_No, "Save?");

            switch(rc)
            {
                case -1:
                case  2:
                    select(cur);
                    return -1;
                case  0:

                    if(current()->is_untitled())
                    {
                        rc = FileDialog(5, 5, cName, 2);
                        if(!rc)
                            current()->save_as(cName);
                        else
                            return -1;
                    }
                    if(current()->save())
                    {
                        MessageBox("Error saving file!");
                        return -1;
                    }
            }
        }
        else
            current()->save(); //just put EA and ignore errors

        next();
    }
    while(current() != cur);
    return 0;
}

void EditBoxCollection::search_proc()
{
    if(!current())
        return;

    if(strchr(Flags, 'g') || strchr(Flags, 'G'))    //Global search
        doFileBegin(*this, 0);

    int rc = current()->search(Flags, Search, Replace);

    if(!rc)
    {
        if(iVSearch)
            MessageBox("\n No occurences found! \n");
        return;
    }

    if(rc && (strchr(Flags, 'r') || strchr(Flags, 'R')))
    {
        // Replace mode
        if((strchr(Flags, 'n') ||
            strchr(Flags, 'N'))) //Noask
        {
            do
            {
                current()->replace();   //Now replace string stored in buffer
            }
            while(current()->search(Flags, Search, Replace));

            if(iVSearch)
                MessageBox("\n Done. \n");
        }
        else //Confirmation
        {
            do
            {
                int rr;
                int cc;

                if(current()->get_cur_row() < (Rows - 7))
                    rr = current()->get_cur_row() + 2;
                else
                    rr = current()->get_cur_row() - 7;

                if(current()->get_cur_col() < (Cols - 12))
                    cc = current()->get_cur_col();
                else
                    cc = current()->get_cur_col() - 12;

                draw();

                rc = AChoice(rr, cc, Yes_No, "Replace?");
                if(rc == 0)
                    current()->replace();
                if(rc == 2 || rc == -1)
                    break;
            }
            while(current()->search(Flags, Search, Replace));

            if(rc != 2 && rc != -1 && iVSearch)
                MessageBox("\n Done. \n");
        }
    }
}

void EditBoxCollection::track_beg()
{
    for(int i = 0; i < Count(); i++)
    {
        GetBox(i)->track_beg();
    }
}

void EditBoxCollection::track_end()
{
    for(int i = 0; i < Count(); i++)
    {
        GetBox(i)->track_end();
    }
}

void EditBoxCollection::track_cancel()
{
    for(int i = 0; i < Count(); i++)
    {
        GetBox(i)->track_cancel();
    }
}

void EditBoxCollection::usual_key(KeyInfo& k)
{
    if(current()->get_mark_state())
        current()->clear();

    if(current()->get_ins_mode())
        current()->ins_char(k.key);
    else
        current()->replace_char(k.key);
}

void EditBoxCollection::set_xy(char *ptr)
{
    int cX = 0;
    int cY = 0;

    if(!current())
        return;

    while(*ptr && __isdd(*ptr))
    {
        cX *= 10;
        cX += *ptr - '0';
        ptr++;
    }

    if(*ptr)
        ptr++;

    while(*ptr && __isdd(*ptr))
    {
        cY *= 10;
        cY += *ptr - '0';
        ptr++;
    }

    if(cX > 0)
        current()->goto_line(cX - 1);
    if(cY > 0)
        current()->goto_col(cY - 1);
}

int EditBoxCollection::doOpenFile(char *pName)
{
    int rc = opened(pName);

    if(rc < 0)
    {
        if (!current()->is_untitled() ||
            current()->get_changed())
                select(open());

        current()->load(pName);
        SendKey("kbOpen");
    }
    else
        select(rc);
    return 0;
}

void EditBoxCollection::recalc_sz()
{
   for(int i = 0; i < Count(); i++)
      GetBox(i)->recalc_sz();
}

PDEntry EditBoxCollection::GetFunction(char* name)
{
	return (PDEntry)keys.Func_DIC.IsIn(name, 0);
}

int EditBoxCollection::get_status_pos()
{
	return iUpperStatus;
}

void EditBoxCollection::set_status_pos(int pos)
{
	int iSaveStatus = iUpperStatus;

	if(pos > 0)
		iUpperStatus = 1;
	else
		iUpperStatus = 0;

    if(iSaveStatus != iUpperStatus)
    {
        int iShift = (iUpperStatus) ? 1:-1;

        for(int i = 0; i < Count(); i++)
            GetBox(i)->row += iShift;
    }
}

int EditBoxCollection::get_tab_width()
{
	return iTabWidth;
}

void EditBoxCollection::set_tab_width(int width)
{
	if(width > 1)
		iTabWidth = width;

	recalc_sz();
}
