/*
** Module   :DIALOG.CPP
** Abstract :Simple Text UI
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  07/11/1997   	Created
*/

#include <string.h>

#include <dialog.h>
#include <_ctype.h>
#include <keynames.h>
#include <version.h>

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
Control::Control(int r, int c, int nr, int nc)
{
	row = r; col = c; rows = nr; cols = nc;
    options = active = hotkey = 0;
}

void Control::set_option(int option)
{
    options |= option;
}

void Control::reset_option(int option)
{
    options &= ~option;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

Dialog::Dialog(int r, int c, int nr, int nc)
    :Control(r, c, nr, nc), savebuff(0), current(-1)
{
    set_option(Dialog::dfBorder);
}

void Dialog::draw()
{
    int i;

    vio_cursor_type(NoCursor);

    if(!savebuff)
        savebuff = vio_save_box(row, col, rows + 1, cols + 1);

    if(is_opt_set(Dialog::dfBorder))
    {
	    vio_box(row, col, rows, cols, 0, mk_clr(CL_DEFAULT));

    	for(i = 1; i < rows - 1; i++)
        	vio_hdraw(row + i, col + 1, ' ', cols - 2, mk_clr(CL_BORDER));
	}
    for(i = 0; i < Count(); i++)
    {
        Control* ctrl = (Control*)Get(i);

        if(ctrl)
            ctrl->draw();
    }

    for(i = 0; i < rows; i++)
        vio_show_str(row + i, col, cols);
}

Dialog::~Dialog()
{
    if(savebuff)
        vio_restore_box(savebuff);

    RemoveAll();
}

void Dialog::Free(Ptr p)
{
    delete (Control *)p;
}

void Dialog::do_key(KeyInfo& k)
{
    switch(k.skey & 0x00FF)
    {
        case kbTab:
            if(k.skey & shShift)
                prev();
            else
                next();
            return;

        case kbEnter:
            next();
            return;
    }

    PControl ctl = in_focus();

    if(ctl)
        ctl->do_key(k);
}

void Dialog::next()
{
    int i;

    //1: from current to end
    for(i = current+1; i < Count(); i++)
    {
        if(control(i)->select(1))
        {
            control(current)->select(0);
            current = i;
            return;
        }
    }

    //2: from start to current
    for(i = 0; i < current; i++)
    {
        if(control(i)->select(1))
        {
            control(current)->select(0);
            current = i;
            return;
        }
    }
}

void Dialog::prev()
{
    int i;

    //1: from current to start
    for(i = current-1; i >= 0; i--)
    {
        if(control(i)->select(1))
        {
            control(current)->select(0);
            current = i;
            return;
        }
    }

    //2: from end to current
    for(i = Count()- 1; i > current ; i--)
    {
        if(control(i)->select(1))
        {
            control(current)->select(0);
            current = i;
            return;
        }
    }
}

void Dialog::Ins(Control* ctrl)
{
    ctrl->move(ctrl->row + row, ctrl->col + col);

    Collection::Add(ctrl);

    if(current < 0 && ctrl->select(1))
        current = Count()-1;
}

void Dialog::move(int r, int c)
{
    int off_r = r - row;
    int off_c = c - col;

    for(int i = 0; i < Count(); i++)
    {
        Control* ctrl = (Control*)Get(i);

        if(ctrl)
            ctrl->move(ctrl->row + off_r, ctrl->col + off_c);
    }
	Control::move(r,c);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

StaticText::StaticText(int r, int c, int nr, int nc, char *aText)
    :Control(r,c,nr,nc)
{
    text = 0;
    set_text(aText);
}

void StaticText::draw()
{
    vio_printh(row, col, text, cols, mk_clr(CL_DEFAULT), mk_clr(CL_HILITE));
}

StaticText::~StaticText()
{
    delete text;
}

void StaticText::set_text(char *aText)
{
    delete text;
    text = new char[strlen(aText)+1];
    strcpy(text, aText);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
void ListBoxItem::set_text(char *aText)
{
    if(text)
    {
        delete text;
        text = 0;
        len = 0;
    }
    if(aText)
    {
        len = strlen(aText);
        text = new char[len + 1];
        strcpy(text, aText);
    }
}

ListBoxItem::~ListBoxItem()
{
    set_text(0);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
ListBox::ListBox(int r, int c, int nr, int nc, int opt)
    :Control(r,c,nr,nc),Collection(128,128)
{
    start_row = cur_row = 0;
    start_x   = 0;
    index_ptr = -1;
    last_sel_was_key = 0;

    set_option(opt);

    if(is_opt_set(lbMultiSel))
        reset_option(lbWrap);
}

void ListBox::draw()
{
    if(is_opt_set(lbHilite))
    {
        for(int i = 0; i < rows; i++)
        {
            int color1 = (cur_row == i) ? CL_CURRENT:CL_DEFAULT;
            int color2 = (cur_row == i) ? CL_CURRSEL:CL_SELECTION;

            char *txt = get_item_text(start_row + i);

            if(!txt)
                txt = "";

            if(start_x > cstrlen(txt))
                txt = "";
            else
                txt = &txt[start_x];

            vio_printh(row+i, col, txt, cols, mk_clr(color1), mk_clr(color2));
        }
    }
    else
    {
        for(int i = 0; i < rows; i++)
        {
            int color = (cur_row == i) ? CL_CURRENT:CL_DEFAULT;

            if(get_item(start_row + i))
                if(get_item(start_row + i)->flags & ListBoxItem::lbiSelected)
                    color = (cur_row == i) ? CL_CURRSEL:CL_SELECTION;

            char *txt = get_item_text(start_row + i);

            if(!txt)
                txt = "";

            if(start_x > strlen(txt))
                txt = "";
            else
                txt = &txt[start_x];

            //
            if(get_item(start_row + i) && get_item(start_row + i)->get_mark())
            {
            	vio_hdraw(row+i, col, get_item(start_row + i)->get_mark(), 1, mk_clr(color));
            	vio_print(row+i, col+1, txt, cols - 2, mk_clr(color));
            }
            else
            	vio_print(row+i, col, txt, cols - 1, mk_clr(color));
        }
        vio_vdraw(row, col + cols - 1, '\xB0', rows, mk_clr(CL_DEFAULT));

        int mark = Count() ? ((abs_row() * rows)/Count()):0;

        vio_vdraw(row + mark, col + cols - 1, '\xB2', 1, mk_clr(CL_DEFAULT));
    }
}

int ListBox::first_selected()
{
    if(!is_opt_set(lbMultiSel))
        return abs_row();

    for(index_ptr = 0; index_ptr < Count(); index_ptr++)
    {
        if(get_item(index_ptr)->flags & ListBoxItem::lbiSelected)
            return index_ptr;
    }
    return (index_ptr = -1);
}

int ListBox::next_selected()
{
    if(index_ptr < 0)
        return -1;

    for(++index_ptr; index_ptr < Count(); index_ptr++)
    {
        if(get_item(index_ptr)->flags & ListBoxItem::lbiSelected)
            return index_ptr;
    }
    return (index_ptr = -1);
}

char* ListBox::get_item_text(int ndx)
{
    PListBoxItem p = get_item(ndx);

    if(p)
        return p->get_text();
    return 0;
}

unsigned ListBox::get_item_key(int ndx)
{
    PListBoxItem p = get_item(ndx);

    if(p)
        return p->get_user_data();
    return 0;
}

PListBoxItem ListBox::add_item(char *text, int pos)
{
    if(pos < 0 || pos > Count())
        return 0;

	PListBoxItem p = new ListBoxItem(text);

    At(p, pos);
    return p;
}

int ListBox::del_item(int index)
{
	if(index == (Count() - 1))
	{
		go_item(index - 1);
		Free(Remove(index));
		return index - 1;
	}
	else
    	Free(Remove(index));

    return index;
}

void ListBox::go_item(int index)
{
    if(index < 0 || index > Count())
        return;
    while(index < abs_row())
        prev();
    while(index > abs_row())
        next();
}

void ListBox::Free(Ptr p)
{
    delete PListBoxItem(p);
}

void ListBox::prev()
{
    if(is_opt_set(lbWrap) && abs_row() == 0)
    {
        while(abs_row() < Count() - 1)
        {
            cur_row++;
            if(cur_row + start_row >= Count())
                cur_row = Count() - start_row - 1;

            if(cur_row >= rows)
            {
                start_row += cur_row - rows + 1;
                cur_row    = rows-1;
            }
        }
        return;
    }


    if(!is_opt_set(lbMultiSel))
        get_item(abs_row())->flags &= ~ListBoxItem::lbiSelected;


    cur_row--;
    if(cur_row < 0)
    {
        start_row += cur_row;
        cur_row    = 0;
    }
    if(start_row <0)
        start_row = 0;

    if(!is_opt_set(lbMultiSel))
        get_item(abs_row())->flags |= ListBoxItem::lbiSelected;
}

void ListBox::next()
{
    if(is_opt_set(lbWrap) && abs_row() == (Count() - 1))
    {
        cur_row = start_row = 0;
        return;
    }

    if(!is_opt_set(lbMultiSel))
        get_item(abs_row())->flags &= ~ListBoxItem::lbiSelected;

    cur_row++;

    if(cur_row + start_row >= Count())
        cur_row = Count() - start_row - 1;

    if(cur_row >= rows)
    {
        if(is_opt_set(lbCanScroll))
        {
            start_row += cur_row - rows + 1;
            cur_row    = rows-1;
        }
        else
            cur_row = rows-1;
    }

    if(!is_opt_set(lbMultiSel))
        get_item(abs_row())->flags |= ListBoxItem::lbiSelected;
}

void ListBox::sel(KeyInfo& k)
{
    if(is_opt_set(lbMultiSel))
    {
        if((k.skey & shCtrl) == shCtrl)
            get_item(abs_row())->flags |= ListBoxItem::lbiSelected;
        if((k.skey & shShift) == shShift)
            get_item(abs_row())->flags &= ~ListBoxItem::lbiSelected;
        if((k.skey & shAlt) == shAlt)
            get_item(abs_row())->flags ^= ListBoxItem::lbiSelected;
    }
}

void ListBox::do_key(KeyInfo& k)
{
    int i;

    last_sel_was_key = 0;

    switch(k.skey & 0x00FF)
    {
        case kbLeft:
            if(is_opt_set(lbXScroll))
            {
                if(start_x > 0)
                    start_x--;
            }
            break;

        case kbRight:
            if(is_opt_set(lbXScroll))
            {
                start_x++;
            }
            break;

        case kbUp:
            sel(k);
            prev();
            break;

        case kbDown:
            sel(k);
            next();
            break;

        case kbPgUp:

            for(i = 0; i < rows; i++)
            {
                sel(k);
                prev();
            }
            break;

        case kbPgDown:

            for(i = 0; i < rows; i++)
            {
                sel(k);
                next();
            }
            break;

        case kbEnd:

            if(!(k.skey & shCtrl) || !is_opt_set(lbXScroll))
            {
                while(abs_row() < (Count() - 1))
                {
                    sel(k);
                    next();
                }
            }
            else
            {
                if(is_opt_set(lbXScroll))
                {
                    char *s = get_item(abs_row())->text;
                    int  i  = cstrlen(s);

                    while(start_x < (i - cols))
                        start_x++;
                }
            }

            break;

        case kbHome:

            if(!(k.skey & shCtrl) || !is_opt_set(lbXScroll))
            {
                while(abs_row())
                {
                    sel(k);
                    prev();
                }

            }
            else
                if(is_opt_set(lbXScroll))
                	start_x = 0;

            break;

        case kbSpace:
            if(is_opt_set(lbMultiSel))
                get_item(abs_row())->flags ^= ListBoxItem::lbiSelected;
            break;

        default:
            if(!(k.skey & shIsCtrl)) //Usual key
            {
                //Search forward
                int i;

                for(i = abs_row() + 1; i < Count(); i++)
                {
                    if(item_char(i) == __to_upper((char)k.key))
                        break;
                }

                if(i < Count())
                {
                    go_item(i);
                    last_sel_was_key = 1;
                    break;
                }

                for(i = 0; i < abs_row(); i++)
                {
                    if(item_char(i) == __to_upper((char)k.key))
                        break;
                }

                if(i < abs_row())
                {
                    last_sel_was_key = 1;
                    go_item(i);
                    break;
                }

                if(item_char(abs_row()) == __to_upper((char)k.key))
                    last_sel_was_key = 1;

                break;
            }
    }
}

char ListBox::item_char(int ndx)
{
    if(ndx < 0 || ndx >= Count())
        return 0;

    char* s = get_item(ndx)->text;

    if(is_opt_set(lbHilite))
    {
        while(*s && *s != SWITCH_CHAR)
            s++;

        if(*s)
            s++;
    }
    else
    {
        while(*s && __issp(*s))
            s++;
    }

    return __to_upper(*s);
}

char ListBox::mk_clr(int index)
{
    return app_pal[(active ? CL_LISTBOX_ACTIVE:CL_LISTBOX_INACTIVE) + index];
}

ListBox::~ListBox()
{
    RemoveAll();
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

Menu::Menu(int r, int c, char **itemlist)
    :ListBox(r,c,0,1,ListBox::lbHilite | ListBox::lbWrap)
{
    int i;

    if(itemlist)
	    for(i = 0; itemlist[i]; i++)
    	    add_menu(itemlist[i]);
}

void Menu::add_menu(char* item)
{
    int len = cstrlen(item);

    if(len > cols)
        cols = len;

    add_at_end(item);
    rows++;
}

char Menu::mk_clr(int index)
{
    return app_pal[CL_MENU+index];
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

JumpEntryHolder::JumpEntryHolder(int arow, int acol, char *name, char *file):
                row(arow), col(acol)
{
    if(name)
    {
        hdr = new char[strlen(name)+1];
        strcpy(hdr, name);
    }
    else
    {
        hdr = new char[1];
        hdr[0] = 0;
    }

    if(file)
    {
        text = new char[strlen(file)+1];
        strcpy(text, file);
    }
    else
    {
        text = new char[1];
        text[0] = 0;
    }
}

JumpEntryHolder::~JumpEntryHolder()
{
    delete text;
    delete hdr;
}

JumpList::~JumpList()
{
    RemoveAll();
}

void JumpList::add_entry(int row, int col, char *name, char *file)
{
    Add(new JumpEntryHolder(row, col, name, file));
}

//-----------------------------------------
// Menu Bar
//-----------------------------------------

MenuBarItem::MenuBarItem(char* aText):Control(0, 0, 1, cstrlen(aText)), text(0), action(0)
{
    text = str_dup(aText);
}

MenuBarItem::~MenuBarItem()
{
    delete text;
    delete action;
}

void MenuBarItem::set_action(char* anAction)
{
    delete action;
    action = str_dup(anAction);
}

void MenuBarItem::draw()
{
    int color1 = (active) ? CL_CURRENT:CL_DEFAULT;
    int color2 = (active) ? CL_CURRSEL:CL_SELECTION;

    char *txt = text;

	if(!txt)
    	txt = "";

    vio_printh(row, col, txt, cols, mk_clr(color1), mk_clr(color2));
}

char MenuBarItem::mk_clr(int index)
{
    return app_pal[(active ? CL_LISTBOX_ACTIVE:CL_LISTBOX_INACTIVE) + index];
}

MenuBar::MenuBar(int r, int c):Dialog(r, c, 1, 0)
{
    reset_option(Dialog::dfBorder);
}

MenuBarItem* MenuBar::AddItem(char* aText)
{
    MenuBarItem* pItem = new MenuBarItem(aText);

    pItem->move(0, cols);
    size(rows, cols + pItem->cols);

    Ins(pItem);

    return pItem;
}

char MenuBar::mk_clr(int index)
{
    return app_pal[CL_MENU+index];
}

void MenuBar::do_key(KeyInfo& k)
{
    switch(k.skey & 0x00FF)
    {
        case kbLeft:
        	prev();
            return;

        case kbRight:
            next();
            return;
/*
        case kbEnter:
            next();
            return;
*/
    }
/*
    PControl ctl = in_focus();

    if(ctl)
        ctl->do_key(k);
*/
}

