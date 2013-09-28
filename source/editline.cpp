/*
** Module   :EDITLINE.CPP
** Abstract :Editable character string implementation
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  16/06/1997   	Created
*/

#include <string.h>

#include <buffer.h>
#include <keynames.h>
#include <version.h>

EditLine::EditLine(int Row, int Col, int nRows, int nCols)
{
	//Control(Row, Col, nRows, nCols)
	move(Row, Col);
	size(nRows, nCols);
    pal_start = CL_EDITLINE_INACTIVE;
    add_line(new Line);
    set_hiliting(0);
};

int EditLine::select(int i)
{
    pal_start = (i) ? CL_EDITLINE_ACTIVE : CL_EDITLINE_INACTIVE;
    return Control::select(i);
}

void EditLine::get_text(char *buff, int max_len)
{
    PLine ln = line(0);
    int len = ln->len();
    if(len > max_len)
        len = max_len;
    if(len)
        memcpy(buff, ln->str, len);
    buff[len] = 0;
}

void EditLine::set_text(char *str)
{
    PLine ln = line(0);
    ln->set(str);
    line_begin();
    mark();
    line_end();
}

void EditLine::draw()
{
    Buffer::draw();

    if(active)
    {
        vio_cursor_pos(row+get_cur_row(), col+get_cur_col());
        vio_cursor_type((get_ins_mode()) ? Underline:BigCursor);
    }
}

void EditLine::do_key(KeyInfo& k)
{
    if(k.skey & shIsCtrl)
    {
        switch(k.skey & 0x00FF)
        {
            case kbLeft:

                if(k.skey & shShift)
                    mark();
                else
                    unmark();

                if(k.skey & shCtrl)
                    word_left();
                else
                    cursor_left();
                flash_bracket();
                break;

            case kbRight:
                if(k.skey & shShift)
                    mark();
                else
                    unmark();
                if(k.skey & shCtrl)
                    word_right();
                else
                    cursor_right();
                flash_bracket();
                break;

            case kbC:	/* Copy  */
                if(k.skey & shCtrl)
                {
                    if(Clipboard)
                        delete Clipboard;
                    Clipboard = copy();
                }
                break;

            case kbV:   /* Paste */
                if(k.skey & shCtrl)
                {
                    if(get_mark_state())
                    {
                        clear();
                        unmark();
                    }
                    paste(Clipboard);
                }
                break;

            case kbX:   /* Cut   */
                if(k.skey & shCtrl)
                {
                    if(Clipboard)
                        delete Clipboard;
                    Clipboard = cut();
                }
                break;

            case kbIns:
                if(k.skey & shCtrl) /* Ctlr+Ins */
                {
                    if(Clipboard)
                        delete Clipboard;
                    Clipboard = copy();
                    break;
                }

                if(k.skey & shShift) /* Shift+Ins*/
                {
                    if(get_mark_state())
                    {
                        clear();
                        unmark();
                    }
                    paste(Clipboard);
                    break;
                }
                set_ins_mode(1 - get_ins_mode());
                break;

            case kbBksp:
                if(k.skey & shAlt)
                {
                    undo();
                    break;
                }
                if(k.skey & shCtrl)
                {
                    del_word_left();
                    break;
                }
                back_space();
                flash_bracket();
                break;

            case kbEnd:
                if(k.skey & shShift)
                    mark();
                else
                    unmark();
                line_end();
                flash_bracket();
                break;

            case kbDel:
                if(k.skey & shShift)
                {
                    if(Clipboard)
                        delete Clipboard;
                    Clipboard = cut();
                }
                else
                {
                    if(get_mark_state())
                        clear();
                    else
                        del_char();
                }
                unmark();
                flash_bracket();
                break;

            case kbHome:
                if(k.skey & shShift)
                    mark();
                else
                    unmark();
                line_begin();
                flash_bracket();
                break;

            case kbT:
                if(k.skey & shCtrl)
                    del_word_right();
                flash_bracket();
                break;

            case kbL:
            case kbU:
                if(k.skey & shAlt)
                {
                    if((k.skey & 0xFF) == kbL)
                        tolower();
                    else
                        toupper();
                }
                break;

            default:
                track_cancel();
        }

        if(abs_row() != 0)
            goto_line(0);
    }
    else
    {
        if(get_mark_state())
            clear();
        if(get_ins_mode())
            ins_char(k.key);
        else
            replace_char(k.key);
        flash_bracket();
    }
}
