/*
** Module   :CURSOR.CPP
** Abstract :Cursor movement routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  15/03/1998     Created
*/

#include <buffer.h>
#include <version.h>

#define UNDO    1

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------
// Cursor movement routines
//
//----------------------------------------------------------------------

void Buffer::text_begin()
{
    reset_type_cache();

    if(cur_row)
        track(opCursor, opCurRow, cur_row);
    if(cur_col)
        track(opCursor, opCurCol, cur_col);
    if(start_row)
        track(opCursor, opStartRow, start_row);
    if(start_col)
        track(opCursor, opStartCol, start_col);

    cur_row = start_row = cur_col = start_col = 0;
}

void Buffer::text_end()
{
    reset_type_cache();

    int _cur_row   = cur_row;
    int _cur_col   = cur_col;
    int _start_row = start_row;
    int _start_col = start_col;

    while(abs_row() < Count() - 1)
    {
        cur_row++;
        if(cur_row + start_row >= Count())
        {
            cur_row = Count() - start_row - 1;
        }
        if(cur_row >= rows)
        {
            start_row += cur_row - rows + 1;
            cur_row   = rows-1;
        }
    }
    line_end();

    if(_cur_row != cur_row)
        track(opCursor, opCurRow, _cur_row);
    if(_cur_col != cur_col)
        track(opCursor, opCurCol, _cur_col);
    if(_start_row != start_row)
        track(opCursor, opStartRow, _start_row);
    if(_start_col != start_col)
        track(opCursor, opStartCol, _start_col);
}

void Buffer::page_up()
{
    reset_type_cache();

    int _cur_row   = cur_row;
    int _start_row = start_row;

    for(int i = 0; i < rows - 1; i++)
    {
        start_row--;
        if(start_row < 0)
        {
            start_row = 0;
            cur_row--;
            if(cur_row < 0)
            {
                cur_row = 0;
                break;
            }
        }
    }
    if(_cur_row != cur_row)
        track(opCursor, opCurRow, _cur_row);
    if(_start_row != start_row)
        track(opCursor, opStartRow, _start_row);

}

void Buffer::page_down()
{
    reset_type_cache();

    int _cur_row   = cur_row;
    int _start_row = start_row;

    for(int i = 0; i < rows - 1; i++)
    {
        start_row++;
        if(abs_row() >= Count())
        {
            start_row--;
            cur_row++;
            if(abs_row() >= Count())
            {
                cur_row--;
                break;
            }
        }
    }
    if(_cur_row != cur_row)
        track(opCursor, opCurRow, _cur_row);
    if(_start_row != start_row)
        track(opCursor, opStartRow, _start_row);
}

void Buffer::cursor_up()
{
    reset_type_cache();

    int _cur_row   = cur_row;
    int _start_row = start_row;

    cur_row--;
    if(cur_row < 0)
    {
        start_row += cur_row;
        cur_row    = 0;
    }
    if(start_row <0)
        start_row = 0;

    if(_cur_row != cur_row)
        track(opCursor, opCurRow, _cur_row);
    if(_start_row != start_row)
        track(opCursor, opStartRow, _start_row);
}

void Buffer::cursor_down()
{
    reset_type_cache();

    int _cur_row   = cur_row;
    int _start_row = start_row;

    cur_row++;

    if(cur_row + start_row >= Count())
        cur_row = Count() - start_row - 1;
    if(cur_row >= rows)
    {
        start_row += cur_row - rows + 1;
        cur_row   = rows-1;
    }

    if(_cur_row != cur_row)
        track(opCursor, opCurRow, _cur_row);
    if(_start_row != start_row)
        track(opCursor, opStartRow, _start_row);
}

void Buffer::line_begin()
{
    reset_type_cache();

    if(cur_col)
        track(opCursor, opCurCol, cur_col);
    if(start_col)
        track(opCursor, opStartCol, start_col);
    cur_col = start_col = 0;
}

void Buffer::line_end()
{
    reset_type_cache();

    int _cur_col = cur_col;
    int _start_col = start_col;

    int slen = abs_line()->len();

    //Skip spaces at end of line
    while(slen && __issp(chr_out(char_at(slen - 1))))
        slen--;

    /* Move cursor right */
    while(slen > cur_col + start_col)
    {
        cur_col++;
        if(cur_col >= cols)
        {
            start_col += cur_col - cols + 1;
            cur_col    = cols - 1;
        }
    }
    /* Move cursor left */
    while(slen < cur_col + start_col)
    {
        cur_col--;
        if(cur_col < 0)
        {
            start_col += cur_col;
            cur_col = 0;
        }
        if(start_col < 0)
            start_col = 0;
    }
    if(_cur_col != cur_col)
        track(opCursor, opCurCol, _cur_col);
    if(_start_col != start_col)
        track(opCursor, opStartCol, _start_col);
}

void Buffer::cursor_left()
{
    int offset = 1;

    if(iFollowTabs)
        offset = abs_col() - abs_line()->get_prev_pos(abs_col());

    cursor_left(offset);
}

void Buffer::cursor_left(int num)
{
    if(num <= 0)
        return;

    reset_type_cache();

    int _cur_col   = cur_col;
    int _start_col = start_col;

    cur_col -= num;

    if(cur_col < 0)
    {
        start_col += cur_col;
        cur_col = 0;
    }
    if(start_col < 0)
        start_col = 0;

    if(_cur_col != cur_col)
        track(opCursor, opCurCol, _cur_col);
    if(_start_col != start_col)
        track(opCursor, opStartCol, _start_col);
}

void Buffer::cursor_right()
{
    int offset = 1;

    if(iFollowTabs && abs_char() == '\t')
    {
        offset = (abs_col() % iTabWidth);

        if(!offset)
            offset = iTabWidth;
    }

    cursor_right(offset);
}

void Buffer::cursor_right(int num)
{
    if(num <= 0)
        return;

    reset_type_cache();

    int _cur_col   = cur_col;
    int _start_col = start_col;

    cur_col += num;

    if(cur_col >= cols)
    {
        start_col += cur_col - cols + 1;
        cur_col    = cols - 1;
    }

    if(_cur_col != cur_col)
        track(opCursor, opCurCol, _cur_col);
    if(_start_col != start_col)
        track(opCursor, opStartCol, _start_col);
}

void Buffer::word_left()
{
    reset_type_cache();

    int save_col = abs_col();

    if(abs_col())
    {
        //Four basic cases:
        //1. Cursor in the middle of word
        //2. Cursor at the leftmost delimiter
        //3. Cursor at the leftmost word character
        //4. Cursor in the middle of delimiters

        int m = 0;

        char c1 = abs_char();
        char c2 = abs_char(-1);

        if(!c1)
            c1 = ' ';

        if(!c2)
            c2 = ' ';

        m |= is_word_delim(c1) ? 0x01:0x00;
        m |= is_word_delim(c2) ? 0x02:0x00;

        switch(m)
        {
            case 1: //Cursor at the leftmost delimiter
                cursor_left(1);
            case 0: //Cursor in the middle of word
                while(!is_word_delim() && abs_col())
                    cursor_left(1);

                if(is_word_delim() && abs_col() != save_col)
                    cursor_right(1);
                break;

            case 2: //Cursor at the leftmost word character
                cursor_left(1);
            case 3: //Cursor in the middle of delimiters
                while(!abs_char() && abs_col())
                    cursor_left(1);

                while(is_word_delim() && abs_col())
                    cursor_left(1);

                if(abs_char() && !is_word_delim() && abs_col() != save_col)
                    cursor_right(1);
                break;
        }
    }
    else
    {
        int save_row = abs_row();

        cursor_up();

        if(abs_row() != save_row)
            line_end();
        else
            if(abs_col() != save_col)
                cursor_right(1);
    }
}

void Buffer::word_right()
{
    reset_type_cache();

    int save_col = abs_col();

    int slen = abs_line()->len();

    if(slen)
        slen--;

    while(__issp(chr_out(char_at(slen))) && slen > 0)
        slen--;

    if(slen > abs_col())
    {
        int m = 0;

        char c1 = abs_char();
        char c2 = abs_char(1);

        if(!c1)
            c1 = ' ';

        if(!c2)
            c2 = ' ';

        m |= is_word_delim(c1) ? 0x01:0x00;
        m |= is_word_delim(c2) ? 0x02:0x00;

        int save_col = abs_col();

        switch(m)
        {
            case 0: //Cursor in the middle of word
                while(!is_word_delim() && abs_col() <= slen)
                    cursor_right(1);
                break;

            case 1: //Cursor at the rightmost delimiter
            case 2: //Cursor at the rightmost character
                cursor_right(1);
                break;

            case 3: //Cursor in the middle of delimiters
                while(!abs_char() && abs_col() <= slen)
                    cursor_right(1);

                while(is_word_delim() && abs_col() <= slen)
                    cursor_right(1);
                break;
        }
    }
    else
    {
        cursor_right(1);

        int save_row = abs_row();

        cursor_down();

        if(abs_row() != save_row)
            line_begin();
        else
            if(abs_col() != save_col)
                cursor_left(1);
    }
}

void Buffer::goto_line(int line_to_go)
{
    reset_type_cache();

    if(line_to_go < 0 || line_to_go >= Count())
        return;

    track(opCursor, opCurRow, cur_row);

    //Special case, line already on the screen, we just move cursor to it

    if(line_to_go >= start_row && line_to_go < start_row + rows)
    {
        cur_row = line_to_go - start_row;
        return;
    }

    //General case

    track(opCursor, opStartRow, start_row);

    //Place this line somewhere around center
    start_row = line_to_go - (rows / 2);
    if(start_row < 0)
        start_row = 0;
    cur_row = line_to_go - start_row;
}

void Buffer::goto_col(int col_to_go)
{
    reset_type_cache();

    if(col_to_go < 0)
        return;

    if(!col_to_go)
    {
        //Use this approach, in some cases it may result to
        //lower number of entries in undo buffer
        line_begin();
        return;
    }

    track(opCursor, opCurCol, cur_col);
    track(opCursor, opStartCol, start_col);

    start_col = col_to_go - (cols / 2);

    if(start_col < 0)
        start_col = 0;

    cur_col = col_to_go - start_col;
}

void Buffer::put_mouse(int new_row, int new_col)
{
    reset_type_cache();

    if(new_row < 0 || new_row >= rows)
        return;

    if(new_col < 0 || new_col >= cols)
        return;

    track(opCursor, opCurRow, cur_row);
    track(opCursor, opCurCol, cur_col);

    cur_col = new_col;
    cur_row = new_row;

    if(cur_row + start_row >= Count())
        cur_row = Count() - start_row - 1;
}

void Buffer::goto_row_col(int new_row, int new_col)
{
    reset_type_cache();

    if(new_row < 0 || new_row >= Count())
        return;

    if(new_col < 0)
        return;

    if(new_row != cur_row)
        goto_line(new_row);

    if(new_col != cur_col)
        goto_col(new_col);
}

