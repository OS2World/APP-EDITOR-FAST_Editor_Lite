/*
** Module   :DIALOG.H
** Abstract :Dialog box controls
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  08/11/1997   	Created
*/

#include <common.h>
#include <collect.h>
//#include <buffer.h>

#ifndef  __DIALOG_H
#define  __DIALOG_H

//-----------------------------------------------------------
// Basic control
//-----------------------------------------------------------

class Control;
typedef Control* PControl;

class Control: public Rect
{
    protected:
        int active;
        int hotkey;
        int options;

    public:

        Control(int r, int c, int nr, int nc);
        virtual ~Control()              {}

        virtual void draw()             {}
        virtual int  select(int i=0)    { active = i; return active;}
        virtual void do_key(KeyInfo&)   {}
        virtual char mk_clr(int)    	{ return 0;}
        virtual void move(int r, int c) { row = r;  col = c; }
        virtual void size(int r, int c) { rows = r; cols = c;}

        virtual void set_option(int option);
        virtual void reset_option(int option);

        int is_opt_set(int option)		{ return options & option;}
        int get_options()             	{ return options;}
};

//-----------------------------------------------------------
// Dialog
//-----------------------------------------------------------

class Dialog: public Collection, public Control
{
        void *savebuff;
        int   current;

        PControl control(int i) { return PControl(Get(i));}

    public:
        enum dFlags
        {
            dfBorder = 0x00000001
        };

        Dialog(int r, int c, int nr, int nc);
        virtual ~Dialog();

        virtual void Free(Ptr p);
        virtual void Ins(Control*);

        virtual void draw();

        PControl in_focus() { return (current < 0) ? 0:control(current);}

        void next();
        void prev();

        virtual void do_key(KeyInfo&);
        virtual char mk_clr(int index)  { return app_pal[CL_DIALOG_START+index];}
        virtual void move(int r, int c);
};

//-----------------------------------------------------------
// Static text control
//-----------------------------------------------------------

class StaticText: public Control
{
    protected:
        char *text;

    public:

        StaticText(int r, int c, int nr, int nc, char *aText);

        virtual ~StaticText();

        virtual int  select(int = 0)    { return 0;}
        virtual void draw();
        virtual char mk_clr(int index)  { return app_pal[CL_STEXT_START+index];}

        void set_text(char *txt);
};

//-----------------------------------------------------------
// List box control
//-----------------------------------------------------------

class ListBoxItem
{
    friend class ListBox;
        enum lbiFlags
        {
            lbiSelected = 0x0001
        };

        unsigned int userdata;
        int flags;
        int len;
        int mark;
        char *text;

    public:

        ListBoxItem(char *aText):flags(0),len(0),text(0),mark(0),userdata(0)
        						{ set_text(aText);}
        ~ListBoxItem();

        char* get_text() { return text;}
        void set_text(char *text);
        void set_user_data(unsigned d)	{ userdata = d;}
        unsigned get_user_data()		{ return userdata;}
        void set_mark(int m)			{ mark = m;}
        int get_mark()					{ return mark;}
};

typedef ListBoxItem* PListBoxItem;

class ListBox: public Collection, public Control
{
    protected:
//        int flags;

        int start_row;
        int cur_row;
        int index_ptr;
        int start_x;
        int last_sel_was_key;

        void prev();
        void next();
        void sel(KeyInfo&);
        PListBoxItem cur_item()    { return PListBoxItem(Get(start_row+cur_row));}

        char item_char(int ndx);

    public:
        enum lbFlags
        {
            lbCanScroll = 0x0001,
            lbMultiSel  = 0x0002,
            lbHilite    = 0x0004,
            lbWrap      = 0x0008,
            lbXScroll   = 0x0010
        };

        ListBox(int r, int c, int nr, int nc, int opt = lbCanScroll);
        virtual ~ListBox();

        virtual void Free(Ptr p);

        virtual void draw();

        int first_selected();
        int next_selected();

        char * get_item_text(int);
        unsigned int get_item_key(int);

        PListBoxItem get_item(int ndx)			{ return PListBoxItem(Get(ndx));}

        PListBoxItem add_item(char *text, int pos);
        PListBoxItem add_at_begin(char *text) 	{ return add_item(text, 0); }
        PListBoxItem add_at_end(char *text)   	{ return add_item(text, Count()); }

        int del_item(int index);
        void go_item(int index);

        virtual void do_key(KeyInfo&);
        virtual char mk_clr(int index);
        int abs_row()               	{ return start_row+cur_row;}

        int selected_by_key()           { return last_sel_was_key;}
};

//-----------------------------------------------------------
// Menu control
//-----------------------------------------------------------

class Menu: public ListBox
{
    public:
        Menu(int r, int c, char **itemlist);
        void add_menu(char* item);
        virtual char mk_clr(int index);
};

//----------------------------------------------------------------------
// class JumpEntryHolder
//----------------------------------------------------------------------
class JumpEntryHolder;
typedef class JumpEntryHolder* PJumpEntry;

class JumpEntryHolder
{
        int row;
        int col;
        char *text;
        char *hdr;
    public:
        JumpEntryHolder(int row, int col, char *name, char *file);
        ~JumpEntryHolder();

        int get_row()    { return row; }
        int get_col()    { return col; }
        char *get_file() { return text;}
        char *get_name() { return hdr; }
};

class JumpList;
typedef JumpList* PJumpList;

class JumpList: public Collection
{
        int last_pos;
    public:
        JumpList():Collection(5,5),last_pos(0) {}
        virtual ~JumpList();

        PJumpEntry get_entry(unsigned ndx) { return (PJumpEntry)Get(ndx);}
        void add_entry(int row, int col, char *name, char *file);

        int  get_last_pos()        { return last_pos;}
        void set_last_pos(int pos) { last_pos = (pos >= 0 && pos < Count()) ? pos: last_pos;}
};

//-----------------------------------------
// class MenuBar
//-----------------------------------------

class MenuBarItem: public Control
{
        char* text;
        char* action;

    public:

        MenuBarItem(char* aText);
        ~MenuBarItem();

        char* get_action()              { return action; }
        void set_action(char* = 0);

        virtual void draw();
        virtual char mk_clr(int index);
};

class MenuBar: public Dialog
{
    public:
        MenuBar(int r, int c);

        MenuBarItem* AddItem(char* aText);

        virtual char mk_clr(int index);
        virtual void do_key(KeyInfo&);
};

#endif //__DIALOG_H

