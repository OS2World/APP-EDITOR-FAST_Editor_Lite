/*
** Module   :BUFFER.H
** Abstract :General purpose edit buffer
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  06/04/1997   	Created from stripped down EDIT.H
**      Fri  07/11/1997   	Updated for UNDO implementation
*/

#include <line.h>
#include <_ctype.h>
#include <idcoll.h>
#include <dialog.h>
//#include <undo.h>

#ifndef  __BUFFER_H
#define  __BUFFER_H

#define WW_STATE        0x01    /* Word wrap state on/off */
#define WW_MERGE        0x02    /* Merge wrapped part with next line */
#define WW_LONG         0x04    /* Reformat until changes are made */

class Buffer;
typedef Buffer* PBuffer;
typedef char (*PerCharFunc)(char);
extern Buffer* Clipboard;

class ListBox;
class Step;

//-----------------------------------------
// class Buffer
//-----------------------------------------

class Buffer: public Control, public Collection
{
    protected:
        Line rep_string;
        int old_abs_col;
        int old_abs_row;
        int start_row;
        int start_col;
        int cur_row;
        int cur_col;
        int undo_count;
        int found_row;
        int found_len;
        int found_col;
        int tracking;

        int hiliting;
        int word_wrap;
        int ww_width;

        unsigned flags;
        char cur_cp[64];

        char cp_in [256];
        char cp_out[256];

        struct
        {
	        int start_row;
    	    int start_col;
        } draw_save;

        IDColl idlist;

        void cursor_left(int);
        void cursor_right(int);

        struct trackinfo
		{
			int op;
			void *arg1;
			void *arg2;
			trackinfo* next;
		};

        trackinfo *track_head;
        trackinfo *undobuff;

        int pal_start;

        void set_flags(unsigned value)			{ flags = value;}
        void set_flag(unsigned mask, int mode);
        int get_flag(unsigned mask)				{ return (flags & mask) ? 1:0;}

        enum enFlags
		{
			flAutoCompletion	= 0x0001,
			flAutoIndent		= 0x0002,
			flChanged			= 0x0004,
			flColumnBlock		= 0x0008,
			flCustomSyntax		= 0x0010,
			flInsMode			= 0x0020,
			flIsUnix			= 0x0040,
			flMarkState			= 0x0080,
			flSaved				= 0x0100,
			flTracking			= 0x0200,
        };

        enum track_cursor_op
        {
            opCurRow,
            opCurCol,
            opStartRow,
            opStartCol,
        };

	protected:

        int is_word_delim()            { return is_word_delim(abs_char());}
        int is_word_delim(int chr);		//Note: it does no NLS translation, char
        								//must be already translated

		int get_tracking(void)		{ return get_flag(flTracking);}

        enum track_op
        {
            opAction,
            opCursor,
            opRestoreLine,
            opDelLine,
            opInsLine,
            opInsBlock,
            opHiliting,
            opMarkPos,
            opUpdateLine,
            opSetFlags
        };

        void track(int op, void *arg1 = 0, void *arg2 = 0);
        void track(int op, int arg1, int arg2 = 0)  { track(op, (void*) arg1, (void*) arg2);}

        struct
        {
            int row;
            int col;
        }
        bookmark[BMK_NUM];

        void line_added(int line_num, int cnt = 1);
        void line_removed(int line_num, int cnt = 1);

    public:

        void ins_line(Line*, int);
        void add_line(Line*);
        Line* del_line(int line_num);

        void dup_line(int line_num);

        void fill_hiliting(int start, int initial_state);
        void check_hiliting(int mode = 0);

        Line* abs_line()        { return line(abs_row());}
        Parser* gen_parser(int =0);

        void track_line(void *p1, void *p2)	{ track(opUpdateLine, p1, p2); }

        void process_block(PerCharFunc);
        void init_buffer();

        Buffer(int sz);
        Buffer();

        virtual ~Buffer();

        virtual void Free(Ptr p);

        virtual void draw();

        void from_pm();
        void to_pm();

        void from_text(char *);
        char* as_text();

        int abs_row()       { return cur_row + start_row;}
        int abs_col()       { return cur_col + start_col;}

        int get_rows()		{ return rows;}
        int get_cols()		{ return cols;}

//-----------------------------------------
// Various flags
//-----------------------------------------

        int get_auto_completion()	{ return get_flag(flAutoCompletion);}
        int get_auto_indent()		{ return get_flag(flAutoIndent);	}
        int get_changed()			{ return get_flag(flChanged);		}
        int get_column_block()  	{ return get_flag(flColumnBlock);	}
        int get_custom_syntax() 	{ return get_flag(flCustomSyntax);	}
        int get_ins_mode()			{ return get_flag(flInsMode);		}
        int get_mark_state()		{ return get_flag(flMarkState);		}
        int get_saved()				{ return get_flag(flSaved);			}
        int get_unix()				{ return get_flag(flIsUnix);		}

        void set_auto_completion(int i)	{ set_flag(flAutoCompletion,i);	}
        void set_auto_indent(int i) 	{ set_flag(flAutoIndent,i);		}
        void set_column_block(int i)    { set_flag(flColumnBlock,i);	}
        void set_custom_syntax(int i)  	{ set_flag(flCustomSyntax,i);	}
        void set_ins_mode(int i)        { set_flag(flInsMode,i);		}
        void set_mark_state(int i)  	{ set_flag(flMarkState, i);		}
        void set_saved(int i)       	{ set_flag(flSaved, i); 		}
        void set_tracking(int i)    	{ set_flag(flTracking, i);		}
        void set_unix(int i)			{ set_flag(flIsUnix, i);		}

        void set_changed(int i);

//-----------------------------------------
// Modes and current values
//-----------------------------------------

        int get_hiliting()   { return hiliting;}
        int get_cur_row()    { return cur_row;}
        int get_cur_col()    { return cur_col;}
        int get_start_row()  { return start_row;}
        int get_start_col()  { return start_col;}
        char* get_cur_cp()   { return cur_cp;   }

        int get_edit_row()   { return abs_row() + 1; }
        int get_edit_col()   { return abs_col() + 1; }

        int get_edit_row_2() { return old_abs_row + 1; }
        int get_edit_col_2() { return old_abs_col + 1; }

        int abs_char(int offset = 0)
                             { return chr_out(abs_line()->char_at(abs_col() + offset));}
        int get_cur_char()   { return abs_line()->char_at(abs_col());}
        int char_at(int i)   { return abs_line()->char_at(i);}
        int get_rel_pos()    { return abs_row()*100/Count();}
        int get_line_count() { return Count();}

        int get_undo_count();

        char chr_in (char c) { return cp_in [c];}
        char chr_out(char c) { return cp_out[c];}

        int  do_smart()      { return (get_auto_indent() && abs_col()) ? 1:0;}
        void recalc_sz();

        void reset_type_cache()   { if(abs_line()) abs_line()->reset_type_cache();}

        int is_cursor_only();   //current undo item contains only cursor moves

        void set_found(int start_row, int start_col, int len);

//-------------------------------------------------------------
        void track_beg();
        void track_end();
        void track_cancel();

        int undo_size();
        int memory();

        Line* line(int num) { return PLine(Get(num));}

//------------------ Cursor movement
        void text_begin();
        void text_end();
        void line_begin();
        void line_end();
        void page_up();
        void page_down();
        void cursor_up();
        void cursor_down();
        void cursor_left();
        void cursor_right();
        void word_left();
        void word_right();
        void goto_line(int);
        void goto_col(int);
        void goto_row_col(int new_row, int new_col);
        void put_mouse(int new_row, int new_col);
        void match_bracket();
        void find_matching_bracket(int& r, int& c, int max_rows = 0);
        void flash_bracket();
        int search(char* flags, char *pattern, char* repl);
        int replace();

        void set_edit_row(int r) { goto_line(r -1);}
        void set_edit_col(int c) { goto_col(c -1);}

//------------------- Editing
        void ins_char(int chr);
        int  del_char(int mode = 0);
        int  replace_char(int chr);
        void del_word_right();
        void del_word_left();
        void del_to_EOL();
        void split_line();
        void back_space();
        void undo();
        void toupper();
        void tolower();
        void indent();
        void unindent();
        void sort();

//------------------- Clipboard
        Buffer* cut();
        Buffer* copy();
        void clear();
        void paste(Buffer*);
        void paste_over(Buffer*);     //Still unimplemented

//------------------- Marking
        void mark();
        void unmark();
        void mark_word();

//------------------ Other actions
        void clear_undobuff();
        void set_xlate(char *cp);

        void touch_all();

        void flip_hiliting();
        void set_hiliting(int mode);
        void next_completion(int n = -1);
        int fill_completion_lb(ListBox*);

//------------------ Bookmarks

        void bmk_place(int i);
        void bmk_get(int i, int &r, int& c);
        void bmk_go(int i);

//------------------ Word wrap
        int ww_need(int ln);
        int ww_perform(int ln);
        int ww_get()                   { return word_wrap;}
        int ww_get_width()             { return ww_width;}
        void ww_set(int flags)         { word_wrap = flags;}
        void ww_set_width(int width)   { ww_width = (width > 1) ? width:ww_width;}

        int ww_get_state()			   { return (word_wrap & WW_STATE) ? 1:0;}
        int ww_get_merge()			   { return (word_wrap & WW_MERGE) ? 1:0;}
        int ww_get_paragraph()		   { return (word_wrap & WW_LONG ) ? 1:0;}

        void ww_set_state(int i)	   { word_wrap &= ~WW_STATE; word_wrap |= (i) ? WW_STATE:0;}
        void ww_set_merge(int i)	   { word_wrap &= ~WW_MERGE; word_wrap |= (i) ? WW_MERGE:0;}
        void ww_set_paragraph(int i)   { word_wrap &= ~WW_LONG ; word_wrap |= (i) ? WW_LONG :0;}
};

//-----------------------------------------------------------
// class EditLine
//-----------------------------------------------------------

class EditLine: public Buffer
{
    public:

        EditLine(int Row, int Col, int nRows, int nCols);

        void get_text(char *buff, int max_len);
        void set_text(char *);

        virtual int  select(int i=0);
        virtual void do_key(KeyInfo&);
        virtual void draw();
};

#endif //__BUFFER_H
