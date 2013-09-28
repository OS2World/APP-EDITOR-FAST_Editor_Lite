/*
** Module   :BUFFER.CPP
** Abstract :Class buffer methods.
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  05/03/1997     Updated to V0.5
**      Sat  12/04/1997   	Updated to V0.8
**      Fri  07/11/1997   	Updated to V0.9, UNDO implemented
**      Sun  09/11/1997   	Updated to V0.91, fixed memory leaks
*/

#include <string.h>

#include <buffer.h>
#include <version.h>

#define UNDO    1

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------
//
// Class BUFFER
//
//----------------------------------------------------------------------

void Buffer::init_buffer()
{
    flags = 0;

    set_auto_completion(iAutoCompl);
    set_auto_indent(1);
    set_ins_mode(1);
    set_saved(1);

    cur_col     = 0;
    cur_row     = 0;
    old_abs_col = 0;
    old_abs_row = 0;

    start_col   = 0;
    start_row   = 0;
    undo_count  = 0;
    found_row   = 0;
    found_len   = 0;
    found_col   = 0;

    memset(bookmark, 0, sizeof(bookmark));

    word_wrap   = iWWDef;
    ww_width    = iDefWidth;

    draw_save.start_row = 0;
    draw_save.start_col = 0;

    hiliting    = HI_CPP;
    pal_start   = CL_EDITBOX_START;
    track_head  = undobuff = 0;

    set_xlate(0);
}

Buffer::Buffer(int sz):Control(0,0,0,0),Collection(sz, 1024)
{
    init_buffer();
}

Buffer::Buffer():Control(0,0,0,0),Collection(1024, 1024)
{
    init_buffer();
}

Buffer::~Buffer()
{
    clear_undobuff();
    RemoveAll();
}

void Buffer::Free(Ptr p)
{
    delete PLine(p);
}

void Buffer::set_flag(unsigned mask, int mode)
{
    if(!(mask & flTracking))
	    track(opSetFlags, (flags & ~(flTracking | flSaved)));

    flags &= ~mask;

    if(mode)
    	flags |= mask;
}

//----------------------------------------------------------------------
// Editing routines
//
//----------------------------------------------------------------------

void Buffer::ins_char(int chr)
{
    unmark();
    set_changed(1);
    int do_unindent = 0;

    if(chr == '\n' || chr == '\r')
    {
        check_hiliting(1);  //Recompile current line and update idlist
        split_line();
        return;
    }

//-----------------------------------
// Smart indent (Part 1)
//-----------------------------------

    if(abs_row() && do_smart() && iUnIndBrackets)
    {
        if(abs_line()->is_empty(abs_col()) &&
           (chr == '}' || chr == '{')      &&
           (hiliting == HI_CPP || hiliting == HI_HTML|| hiliting == HI_PL))
        {
            do
            {
                if(chr == '{' && (iUnIndBrackets == 1 || iAutoUnInd == 0))
                {
                    do_unindent = 0;
                    break;
                }

                if(chr == '}')
                {
                    do_unindent = 1;
                    break;
                }

                Parser *res = Parser::GenParser(hiliting);

                line(abs_row() - 1)->get_first_token(res);

                if(res->flags & MASK_AUTOUNINDENT)
                    do_unindent = 1;

                delete res;
            }
            while(0);
        }

        if(do_unindent)
        {
            //find previous tabstop
            int offset = abs_col() - ((abs_col()/iTabWidth - 1) * iTabWidth);
            cursor_left(offset);
        }
    }

//-----------------------------------------
//    end
//-----------------------------------------

    cursor_right(abs_line()->ins_char(abs_col(), chr_in(chr), 1, this));

    if((word_wrap & WW_STATE) && ww_need(abs_row()))
    {
        int wrap_pos = 0;

        wrap_pos = ww_perform(abs_row());

        if(wrap_pos)
        {
            if(word_wrap & WW_LONG)
            {
                int i;

                for(i = abs_row() + 1; ww_need(i) && (i < Count()); i++)
                    ww_perform(i);
            }

            if(abs_col() >= wrap_pos)
            {
                int offset = (get_auto_indent()) ? abs_line()->indent_offset():0;

                goto_line(abs_row() + 1);
                goto_col(abs_col() - wrap_pos + offset);
            }
        }
    }

    //Now it's time to process unindent

    if(!do_unindent && iUnIndKwd)
    {
        Parser *res = Parser::GenParser(hiliting);

        abs_line()->get_first_token(res);

        do
        {
            //Check if this is a keyword and it requires unindenting

            if(res->color != CL_STDWORD)
                break;

            if(!(res->flags & (MASK_UNINDENT | MASK_UNINDENT_C)))
                break;

            Line ln(abs_line(), 0, abs_line()->len(), 1);

            ln.get_first_token(res);

            //Check if there is no other tokens in the line
            //...
            if(abs_col() != res->offset() + res->tok_len)
                break;

            if(res->offset() < iTabWidth)
                break;

            if(!ln.is_empty(res->offset() - 1))
                break;

            do_unindent = res->tok_len;

            if(res->flags & MASK_UNINDENT_C)
            {
                line(abs_row()-1)->get_first_token(res);

                if(!(res->flags & MASK_AUTOUNINDENT))
                {
                    do_unindent = 0;
                    break;
                }

                if(hiliting == HI_CPP || hiliting == HI_HTML|| hiliting == HI_PL)
                {
                    //Handle trailing { for "linux" coding style

                    int present = 0;

                    while(*res->tok)
                    {
                        int iSz = res->next_token();

                        if(iSz == 1)
                        {
                            if(*res->tok == '{')
                                present++;
                            if(*res->tok == '}')
                                present--;
                        }

                        res->tok += iSz;
                    }
                    if(present > 0)
                        do_unindent = 0;
                }
            }
        }
        while(0);

        delete res;

        if(do_unindent && (abs_col() - do_unindent - iTabWidth) > 0)
        {
            abs_line()->del_char(abs_col() - do_unindent - iTabWidth, iTabWidth);
            cursor_left(iTabWidth);
        }
    }

    check_hiliting(__isic(chr) ? 0:1);

    next_completion();
}

int Buffer::del_char(int mode)
{
    unmark();
    set_changed(1);
    int chr = 0;

    if(__issp(abs_char()))
    {
        int not_eof = 0;

        //Check if all chars up to end of line are spaces

        for(int i = 1; i < abs_line()->len() - abs_col(); i++)
        {
            if(!__issp(abs_char(i)))
            {
                not_eof = 1;
                break;
            }
        }

        if(!not_eof)
        {
            //Delete to the end of line
            //track(opRestoreLine,(void *)abs_line(),(void *)abs_row());
            abs_line()->del_char(abs_col(), abs_line()->len() - abs_col(), this);
        }
    }

    if(abs_col() >= abs_line()->len()) //Special case, merge lines
    {
        if(!mode)
        {
            PLine ln = PLine(Remove(abs_row() + 1));

            if(ln)
            {
                track(opInsLine, (void*)ln,(void *)(abs_row()+1));
                abs_line()->ins_char(abs_col(), ln, this);
            }
        }
    }
    else
    {
        chr = chr_out(abs_line()->del_char(abs_col(), 1, this));
    }

    check_hiliting();

    return chr;
}

void Buffer::split_line()
{
    unmark();
    int chr;
    int offset = 0;
    int smart_indent = 0;
    int do_unindent  = 0;

    set_changed(1);

//-----------------------------------
// C/C++ Smart indent (Part 2)
//-----------------------------------

    if(do_smart())
    {
        Parser *res = Parser::GenParser(hiliting);

        if(abs_char(-1) == '{' && res->process_brackets())
        {
            smart_indent = 1 * iIndBracket;
        }
        else
        {
            abs_line()->get_first_token(res);

            if(res->color == CL_STDWORD && (res->flags & MASK_INDENT)
               && (res->offset() + res->tok_len) <= abs_col())
            {
                smart_indent = 1 * iIndKwd;
            }
        }

        if(!smart_indent && iAutoUnInd && abs_row())
        {
            //Check if previous line starts with keyword with
            //AUTOUNINDENT flag set

            line(abs_row()-1)->get_first_token(res);

            if(res->color == CL_STDWORD && (res->flags & MASK_AUTOUNINDENT))
                do_unindent = 1;
        }

        if(abs_row() && (hiliting == HI_CPP || hiliting == HI_HTML || hiliting == HI_PL))
        {
            //Handle various special cases such as
            //trailing { for "linux" coding style
            //and complete statement/block in the same line
            //with if/while/for

            int present  = 0;
            int cnt      = 0;

            if(!res->tok)
                line(abs_row()-1)->get_first_token(res);

            while(*res->tok)
            {
                int iSz = res->next_token();

                if(iSz == 1)
                {
                    if(*res->tok == ';')
                        cnt++;            //Count the end of complete statement

                    if(*res->tok == ')')  //Reset counter to workaround for(;;)
                        cnt = 0;

                    if(*res->tok == '{')
                        present++;

                    if(*res->tok == '}')
                    {
                        present--;
                        cnt++;            //Count the end of complete statement
                    }
                }
                res->tok += iSz;
            }
            if(present > 0)
                do_unindent = 0;
            else
                if(cnt && !present)
                {
                    //Code block is present and completely finished
                    //in one line, do not do smart indent
                    smart_indent = 0;
	                do_unindent = 0;
                }
        }

        if(smart_indent && abs_char() == '{' && res->process_brackets())
        {
            //Don't do smart indent if we have bracket
            smart_indent = 0;
        }

        delete res;
    }
//    end

    int pos_state = abs_line()->state();

    PLine ln = abs_line()->split(abs_col(), this);

    if(get_auto_indent())
        offset = abs_line()->indent_offset();

    track(opDelLine,(void *)(abs_row()+1));

    ins_line(ln, abs_row() + 1);

    ln = abs_line();

    if(hiliting)
        fill_hiliting(abs_row(), pos_state);

    cursor_down();

    if((get_auto_indent() && offset) || smart_indent)
    {
        if(do_unindent)   //go to previous tabstop
            offset = (offset / iTabWidth - 1) * iTabWidth;

        if(smart_indent)
            offset = (offset / iTabWidth + 1) * iTabWidth;

        abs_line()->indent(ln, offset);
        goto_col(offset);
    }
    else
    	line_begin();
}

int Buffer::replace_char(int chr)
{
    unmark();
    set_changed(1);

    if(chr == '\n' || chr == '\r')
    {
        cursor_down();
        line_begin();
        return 0;
    }

    int old_chr;

    old_chr = chr_out(abs_line()->del_char(abs_col(), 1, this));
    cursor_right(abs_line()->ins_char(abs_col(), chr_in(chr), 1, this));

    check_hiliting();

    return old_chr;
}

void Buffer::del_word_right()
{
    unmark();
    set_changed(1);

    if(abs_col() >= abs_line()->len())
    {
        del_char();
        return;
    }
    int chr;
    int deleted = 0;
    track(opRestoreLine,(void *)abs_line(),(void *)abs_row());

    if(is_word_delim())
    {
        if(iDelWS)
        {
            if(__issp(abs_char()))
            {
                while(__issp(abs_char()) && abs_col() < abs_line()->len())
                    abs_line()->del_char(abs_col());
            }
            else
            {
                while(is_word_delim() && !__issp(abs_char()))
                    abs_line()->del_char(abs_col());
            }
        }
        else
        {
            while(is_word_delim())
                abs_line()->del_char(abs_col());
        }
    }
    else
    {
        while(!is_word_delim() && abs_col() < abs_line()->len())
            abs_line()->del_char(abs_col());
    }
    check_hiliting();
}

void Buffer::del_word_left()
{
    unmark();
    set_changed(1);

    if(abs_col() > abs_line()->len())
    {
        line_end();
        return;
    }

    if(!abs_col())
    {
        back_space();
        return;
    }

    track(opRestoreLine,(void *)abs_line(),(void *)abs_row());

    int i;

    if(is_word_delim(abs_char(-1)))
    {
        if(iDelWS)
        {
            if(__issp(abs_char(-1)))
            {
                for(i = abs_col(); i > 0; i--)
                {
                    if(!__issp(abs_char(-1)))
                        break;

                    cursor_left(1);
                    abs_line()->del_char(abs_col());
                }
            }
            else
            {
                for(i = abs_col(); i > 0; i--)
                {
                    if(!is_word_delim(abs_char(-1)))
                        break;

                    if(__issp(abs_char(-1)))
                        break;

                    cursor_left(1);
                    abs_line()->del_char(abs_col());
                }
            }
        }
        else
        {
            for(i = abs_col(); i > 0; i--)
            {
                if(!is_word_delim(abs_char(-1)))
                    break;

                cursor_left(1);
                abs_line()->del_char(abs_col());
            }
        }
    }
    else
    {
        for(i = abs_col(); i > 0; i--)
        {
            if(is_word_delim(abs_char(-1)))
                break;

            cursor_left(1);
            abs_line()->del_char(abs_col());
        }
    }

    check_hiliting();
}

void Buffer::del_to_EOL()
{
    unmark();
    set_changed(1);

    if(abs_col() > abs_line()->len())
        line_end();
    else
    {
        int deleted = 0;

        abs_line()->del_char(abs_col(), abs_line()->len() - abs_col(), this);
    }

    check_hiliting();
}

void Buffer::dup_line(int line_num)
{
    if(line_num >= Count())
        return;

    set_changed(1);

    PLine pLn = new Line(line(line_num));

    track(opDelLine,(void *)(line_num+1));
    ins_line(pLn, line_num+1);
    check_hiliting();
}

Line* Buffer::del_line(int line_num)
{
    set_changed(1);

    if(line_num >= Count())
        return 0;
    if(line_num == Count() - 1 && line_num == abs_row())
        return 0;

    track(opInsLine,(void *)line(line_num),(void *)line_num);

    if(hiliting)
    {
        // Removing this line may affect rest of lines in buffer

        // If start conditions in this and next line doesn't equal
        // then recalculate rest of lines with new conditions

        PLine ln0 = line(line_num);
        PLine ln1 = line(line_num + 1);

        //Use start conditions of line which will be removed,
        //and recalculate rest of lines
        if(ln0 && ln1 && ln0->state() != ln1->state())
            fill_hiliting(line_num + 1, ln0->state());
    }

	line_removed(line_num);
    return PLine(Remove(line_num));
}

void Buffer::back_space()
{
    set_changed(1);

    if(get_mark_state())
        clear();

    if(abs_col())
    {
        int offset = 1;

        if(!(abs_col() % iTabWidth))
            offset = abs_col() - abs_line()->get_prev_pos(abs_col());

        cursor_left(offset);

        abs_line()->del_char(abs_col(), offset, this);
	    check_hiliting();
    }
    else
    {
        if(abs_row())
        {
            cursor_up();
            line_end();
            del_char();
        }
    }
}

//----------------------------------------------------------------------
// Utility routines
//
//----------------------------------------------------------------------

Parser* Buffer::gen_parser(int i)
{
    Parser *res = Parser::GenParser(hiliting);

    if(!res)
        res = new Parser;

    if(i)
        res->SetXlat(cp_out);

    return res;
}

void Buffer::mark()
{
    if(!get_mark_state())
    {
        //track(opMarking,(void *)mark_state);
        old_abs_col = abs_col();
        old_abs_row = abs_row();
    }
    set_mark_state(1);
}

void Buffer::unmark()
{
    if(get_mark_state())
        track(opMarkPos, old_abs_col, old_abs_row);

    set_mark_state(0);
}

void Buffer::set_hiliting(int mode)
{
    set_custom_syntax(1);
    track(opHiliting, hiliting);
    hiliting = (mode <= HI_LAST && mode >= 0) ? mode : hiliting;
}

void Buffer::set_changed(int i)
{
	set_flag(flChanged, i);

    if(i)
        set_saved(0);

    reset_type_cache();
}

void Buffer::set_found(int h_row, int h_col, int h_len)
{
    found_row  = h_row;
    found_len  = h_len;
    found_col  = h_col;
}

void Buffer::ins_line(Line* ln, int line_num)
{
    if(!ln || line_num < 0 || line_num > Count() + 1)
        return;

    At(ln, line_num);
	line_added(line_num);
}

void Buffer::add_line(Line* ln0)
{
    if(Count())
    {
        PLine ln = line(Count() - 1);
        int pos_state = ln->state();

        Parser* parser = gen_parser(1);

        ln0->set_hiliting(parser, 0, pos_state, -1);

    	delete parser;
    }
	line_added(Count());
    Add(ln0);
}

void Buffer::fill_hiliting(int start, int pos_state)
{
    Parser* parser = gen_parser(1);
    IDColl* p = 0;

    if(start == 0 && pos_state == ST_INITIAL)
    {
        idlist.RemoveAll();
        idlist.Refill(parser->get_mask());
        p = &idlist;
    }

    for(int i = start;i < Count(); i++)
        line(i)->set_hiliting(parser, p, pos_state, -1); //Fill idlist

    delete parser;
}

void Buffer::check_hiliting(int mode)
{
    if(!hiliting)
        return;

    PLine ln0 = abs_line();

    int pos_state = ln0->state();

    Parser* parser = gen_parser(1);

    switch(mode)
    {
        case 0: //Just check highlighting
            ln0->set_hiliting(parser, 0, pos_state, -1);
            break;

        case 1: //Just add tokens
            ln0->set_hiliting(parser, &idlist, pos_state, -1);
            break;

        case 2: //Update token cache
            ln0->set_hiliting(parser, 0, pos_state, abs_col());
    }

    delete parser;

    PLine ln1 = line(abs_row() + 1);

    if(ln1 && ln1->state() != pos_state)
        fill_hiliting(abs_row() + 1, pos_state);
}

void Buffer::flip_hiliting()
{
    set_custom_syntax(1);

    hiliting++;
    if(hiliting > HI_LAST)
        hiliting = 0;

    if(hiliting)
        fill_hiliting(0, ST_INITIAL);
}

void Buffer::set_xlate(char *cp)
{
    int i;

    cur_cp[0] = 0;

    for(i = 0; i < 256; i++)
        cp_in[i] = cp_out[i] = i;

    if(!cp)
        return;

    char cTmp[256];

    for(i = 0; i < 256; i++)
        cTmp[i] = i;

    int rc1;
    int rc2;

    rc1 = cp2cp(cp, "", &cTmp[1], &cp_out[1], 255);
    rc2 = cp2cp("", cp, &cTmp[1], &cp_in [1], 255);

    if(rc1 || rc2)
    {
        for(i = 0; i < 256; i++)
            cp_in[i] = cp_out[i] = i;
    }
    else
    {
        strcpy(cur_cp, cp);
    }
}

//-----------------------------------------
// Word wrap
//-----------------------------------------

int Buffer::ww_need(int i)
{
    PLine ln = line(i);

    if(!ln)
        return 0;

    int slen = ln->len();

    while(slen && __issp(ln->char_at(slen - 1)))
        slen--;

    if(slen > ww_width)
        return 1;

    return 0;
}

int Buffer::ww_perform(int row)
{
    PLine ln = line(row);

    if(!ln)
        return 0;

    int break_pos = 0;
    int i = 0;
    int slen = ln->len();

    while(slen && __issp(chr_out(ln->char_at(slen - 1))))
        slen--;

    if(slen < ww_width)
        return 0;

    //Look for place where we will break line

    for(i = ww_width - 1; i > 0; i--)
    {
        if(__issp(chr_out(ln->char_at(i))))
        {
            break_pos = i + 1;
            break;
        }
    }

    if(!break_pos) //Non-breakable line
    {
        //Try other way
        for(i = ww_width; i < slen; i++)
        {
            if(__issp(chr_out(ln->char_at(i))))
            {
                break_pos = i + 1;
                break;
            }
        }

        if(!break_pos)
	        return 0;
    }

    int save_flags = word_wrap;
    int save_row   = abs_row();
    int save_col   = abs_col();

    word_wrap = 0;

    goto_line(row);

	if((save_flags & WW_MERGE) && abs_row() < (Count() - 1))
	{
    	//Merge next line

		if(line(abs_row() + 1)->len())
		{
			line_end();
			del_char();

			if(__issp(abs_char()))
				del_word_right();

			ins_char(' ');
		}
	}

    goto_col(break_pos);
    ins_char('\n');

    word_wrap = save_flags;

    goto_line(save_row);
    goto_col(save_col);

    return break_pos;
}

int Buffer::is_word_delim(int chr)
{
    if(cWordDelim)
        return strchr(cWordDelim, chr) ? 1:0;

    if(!chr)
        return 0;

    return __isic(chr) ? 0:1;
}

void Buffer::touch_all()
{
    set_changed(1);

    for(unsigned i = 0; i < Count(); i++)
    {
        track(opRestoreLine,(void *)line(i),(void *)i);
        line(i)->touch();
    }
}

//-----------------------------------------
// Autocompletion stuff
//-----------------------------------------

int Buffer::fill_completion_lb(ListBox* lb)
{
    if(!get_auto_completion())
        return 0;

    if(!abs_line()->token_valid(abs_col()))
        check_hiliting(2);                      //Update token cache

    if(!abs_line()->token_valid(abs_col()))
        return 0;

    if(!abs_line()->token_is_completable(abs_line()->token_type()))
        return 0;

    int rc = 0;
    int minlen = abs_col() - abs_line()->token_start();
    Line ln(abs_line(), abs_line()->token_start(),
			abs_line()->token_length(), 1);

    rc = idlist.FillList(lb, &ln, minlen);

    return rc;
}

void Buffer::next_completion(int n)
{
    if(!get_auto_completion())
        return;

    if(!abs_line()->token_valid(abs_col()))
        check_hiliting(2);                  	//Update token cache

    if(!abs_line()->token_valid(abs_col()))
        return;

    if(!abs_line()->token_is_completable(abs_line()->token_type()))
        return;

    do
    {
        int start   = abs_line()->token_start();
        int tok_len = abs_line()->token_length();
        int minlen  = abs_col() - start;

        Line ln(abs_line(), start, tok_len, 1);

        if(!idlist.NextID(&ln, minlen, n))
            break;

        //Edit token

        set_changed(1);

        clear();
        unmark();

        abs_line()->del_char(start, minlen, this);
        abs_line()->ins_char(start, &ln, this);

		mark();
		old_abs_col = abs_col() + ln.len() - minlen;
    }
    while(0);
}

void Buffer::recalc_sz()
{
	for(int i = 0; i < Count(); i++)
		line(i)->recalc_sz();
}

void Buffer::mark_word()
{
	int from;
	int to;

	if(abs_line()->find_word(abs_col(), from, to))
	{
	    unmark();
	    cursor_left(abs_col() - from);
	    mark();
	    cursor_right(to - from);
	}
}

int Buffer::memory()
{
	int sz = 0;

	for(int i = 0; i < Count(); i++)
	{
		sz += line(i)->memory();
	}

	return sz;
}

void Buffer::line_added(int line_num, int cnt)
{
	line_num++;

	for(int i = 0; i < BMK_NUM; i++)
	{
		if(!bookmark[i].row)
			continue;

		if(line_num < bookmark[i].row)
			bookmark[i].row += cnt;
	}
}

void Buffer::line_removed(int line_num, int cnt)
{
	line_num++;

	for(int i = 0; i < BMK_NUM; i++)
	{
		if(!bookmark[i].row)
			continue;

		if(line_num <= bookmark[i].row && (line_num + cnt) > bookmark[i].row)
		{
			//Remove bookmark
			bookmark[i].row = 0;
			bookmark[i].col = 0;
			continue;
		}

		if(line_num < bookmark[i].row)
			bookmark[i].row -= cnt;
	}
}

void Buffer::bmk_go(int i)
{
    if(i < 0 || i >= BMK_NUM)
        return;

    if(bookmark[i].row > 0)
        goto_line(bookmark[i].row - 1);

    if(bookmark[i].col > 0)
        goto_col(bookmark[i].col - 1);
}

void Buffer::bmk_place(int i)
{
    if(i < 0 || i >= BMK_NUM)
        return;

    bookmark[i].row = get_edit_row();
    bookmark[i].col = get_edit_col();
}

void Buffer::bmk_get(int i, int &r, int& c)
{
    if(i < 0 || i >= BMK_NUM)
    	r = c = 0;
    else
    {
    	r = bookmark[i].row;
    	c = bookmark[i].col;
    }
}

