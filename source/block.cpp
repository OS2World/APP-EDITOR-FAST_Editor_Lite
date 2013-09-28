/*
** Module   :BLOCK.CPP
** Abstract :Block processing routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  13/06/2003 Created - undo.cpp spilt up into 2 pieces
*/

#include <string.h>

#include <buffer.h>
#include <version.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//----------------------------------------------------------------------
// Massive block processing routines
//
//----------------------------------------------------------------------

void Buffer::process_block(PerCharFunc func)
{
    set_changed(1);

    if(!get_mark_state())
    {
        int chr = chr_out(get_cur_char());
        if(chr)
        {
            replace_char(func((char)chr));
            cursor_left(1);
        }
        return;
    }
    int mark_beg_row = min(old_abs_row, abs_row());
    int mark_end_row = max(old_abs_row, abs_row());
    int i,j;

    for(i = mark_beg_row; i <= mark_end_row; i++)
        track(opRestoreLine, (void *)line(i),(void *)i);

    if(get_column_block() || mark_beg_row == mark_end_row)
    {
        int mark_col_start = min(old_abs_col, abs_col());
        int mark_col_end   = max(old_abs_col, abs_col());

        for(i = mark_beg_row; i <= mark_end_row; i++)
            for(j = mark_col_start; j < mark_col_end; j++)
            {
                int chr = chr_out(line(i)->del_char(j));
                line(i)->ins_char(j, chr_in(func((char)chr)));
            }
    }
    else
    {
        for(i = mark_beg_row; i <= mark_end_row; i++)
        {
            int chr;
            int col_start = 0;
            int col_end = line(i)->len();

            if(i == mark_beg_row)
                col_start = (mark_beg_row == old_abs_row) ? old_abs_col : abs_col();

            if(i == mark_end_row)
                col_end = (mark_end_row == old_abs_row) ? old_abs_col : abs_col();

            for(int j = col_start; j < col_end; j++)
            {
                chr = chr_out(line(i)->del_char(j));
                line(i)->ins_char(j, chr_in(func((char)chr)));
            }
        }
    }

    fill_hiliting(mark_beg_row, line(mark_beg_row)->state());
}

void Buffer::indent()
{
    set_changed(1);

    if(!get_mark_state())
    {
        //track(opRestoreLine,(void *)abs_line(),(void *)abs_row());
        abs_line()->ins_char(0, ' ', 1, this);
        return;
    }

    int mark_beg_row = min(old_abs_row, abs_row());
    int mark_end_row = max(old_abs_row, abs_row());
    int i;

    for(i = mark_beg_row; i <= mark_end_row; i++)
        track(opRestoreLine,(void *)line(i),(void *)i);

    if(get_column_block() || mark_beg_row == mark_end_row)
    {
        int mark_col_start = min(old_abs_col, abs_col());
        for(i = mark_beg_row; i <= mark_end_row; i++)
            line(i)->ins_char(mark_col_start, ' ');
    }
    else
    {
        for(i = mark_beg_row; i <= mark_end_row; i++)
        {
            int col_start = 0;

            if(i == mark_beg_row)
                col_start = (mark_beg_row == old_abs_row) ?
                                old_abs_col : abs_col();

            if(i == mark_end_row)
            {
                int col_end = (mark_end_row == old_abs_row) ?
                                old_abs_col : abs_col();

                if(col_end == 0)
                    break;
            }

            line(i)->ins_char(col_start, ' ');
        }
    }

    fill_hiliting(mark_beg_row, line(mark_beg_row)->state());
}

void Buffer::unindent()
{
    set_changed(1);
    if(!get_mark_state())
    {
        track(opRestoreLine,(void *)abs_line(),(void *)abs_row());
        abs_line()->del_char(0);
        return;
    }
    int mark_beg_row = min(old_abs_row, abs_row());
    int mark_end_row = max(old_abs_row, abs_row());
    int i;

    for(i = mark_beg_row; i <= mark_end_row; i++)
        track(opRestoreLine,(void *)line(i),(void *)i);

    if(get_column_block() || mark_beg_row == mark_end_row)
    {
        int mark_col_start = min(old_abs_col, abs_col());
        for(i = mark_beg_row; i <= mark_end_row; i++)
            line(i)->del_char(mark_col_start);
    }
    else
    {
        for(i = mark_beg_row; i <= mark_end_row; i++)
        {
            int col_start = 0;

            if(i == mark_beg_row)
                col_start = (mark_beg_row == old_abs_row) ? old_abs_col : abs_col();

            if(i == mark_end_row)
            {
                int col_end = (mark_end_row == old_abs_row) ?
                                old_abs_col :
                                abs_col();

                if(col_end == 0)
                    break;
            }

            if(iSpacesOnly)
            {
                if(__issp(chr_out(line(i)->char_at(col_start))))
                    line(i)->del_char(col_start);
            }
            else
            {
                line(i)->del_char(col_start);
            }
        }
    }

    fill_hiliting(mark_beg_row, line(mark_beg_row)->state());
}

void Buffer::toupper()
{
    process_block(__to_upper);
}

void Buffer::tolower()
{
    process_block(__to_lower);
}


