/*
** Module   :LINE.H
** Abstract :Class Line handles one line of editor
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  05/03/1997   	Updated to V0.5
*/

#include <idcoll.h>
#include <common.h>
#include <parser.h>

#ifndef  __LINE_H
#define  __LINE_H

class Line;
typedef Line* PLine;

class Buffer;

class Line
{
        int buf_len;
        int str_width;
        int str_size;

        int hl_state;
        int token_cache_type;
        int token_cache_start;
        int token_cache_len;

        void expand_by(int len);
        void check_size(int sz) { if(sz >= buf_len) expand_by(sz - buf_len); }
        void init();

    public:
        char *str;

        Line();
        Line(char *);
        Line(PLine);
        Line(PLine master, int start, int width, int force);

        ~Line();
        void set(char *str);

        char * get_print(int start, char *buffer, int width);

        int ins_char(int pos, PLine src, Buffer* = 0);
        int ins_char(int pos, char* chr, int num, Buffer* = 0);
        int ins_char(int pos, int chr, int num = 1, Buffer* = 0);

        int del_char(int pos, int num = 1, Buffer* = 0);

        int len()		{ return str_width;}	/* size with account of tabs */
        int size()		{ return str_size;}		/* size in bytes */
        int memory()	{ return (buf_len) ? buf_len:(str_size+2);}
        void touch();							/* replace all tabs with spaces */
        void detach();							/* make own copy of data buffer */
														/* instead of one provided in (char*) constructor */
        void recalc_sz();

        int char_at(int pos);
        int fchar_at(char *tmp, int pos) { return (pos < len()) ? tmp[pos]:0;}
        int find_char(int chr);

        void build_print(char*);        //Buld printable version of the string
        void xlat(char *cvt_tbl);       //Transtale string using given table

        int state() { return hl_state;} //Highlighting FSM state

        void set_hiliting(Parser*, IDColl*, int& initial_state, int offset);
        void get_first_token(Parser* parser);
        int token_type(Parser* parser, int pos);
        int is_empty(int pos);
        int export_buf(char* dst);

        //Token cache API

        void reset_type_cache() { token_cache_type = token_cache_start = token_cache_len = -1; }

        void set_start(int n)	{ token_cache_start = n;}
        void set_type(int n)   	{ token_cache_type  = n;}

        int token_type()        { return token_cache_type;}
        int token_start()       { return token_cache_start;}
        int token_length()      { return token_cache_len;}
        int token_valid(int pos);

        static int token_is_completable(int type);

        //High level API

        PLine split(int split_pos, Buffer* pBuf);	//Split line at given pos

        int indent_offset();                    //Find indent offset (position)
        void indent(PLine src, int offset);     //Indent line using src as a template
        int get_prev_pos(int from_pos);         //Get previous cursor pos (account tabs)
        int find_word(int start_pos, int& from, int& to);

        //Diffs
		Line* gen_diff(Line* other);
		int diff_width(void);
		int apply_diff(Line* diff, int dir);
};

#endif //__LINE_H

