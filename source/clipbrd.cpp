/*
** Module   :CLIPBRD.CPP
** Abstract :Clipboard-related methods of class Buffer
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  05/03/1997     Updated to V0.5
**      Sat  12/04/1997   	Updated to V0.8
**      Wed  12/11/1997     Updated to V0.9, mostly rewritten
**      Tue  12/12/2000     Single-char ops replaced by group operations
**                          to improve performance
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <buffer.h>
#include <version.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

Buffer* Buffer::copy()
{
    if(!get_mark_state())
        return 0;

    int mark_beg_row = min(old_abs_row, abs_row());
    int mark_end_row = max(old_abs_row, abs_row());

    Buffer* clip = new Buffer;

    if(get_column_block() || mark_beg_row == mark_end_row)
    {
        int mark_col_start = min(old_abs_col, abs_col());
        int mark_col_end   = max(old_abs_col, abs_col());

        if(get_column_block())
            clip->set_column_block(1);

        for(int i = mark_beg_row; i <= mark_end_row; i++)
        {
            PLine ln = new Line(line(i), mark_col_start,
                                mark_col_end - mark_col_start, 0);
            ln->xlat(cp_out);
            clip->add_line(ln);
        }
    }
    else
    {
        for(int i = mark_beg_row; i <= mark_end_row; i++)
        {
            if(i == mark_beg_row)
            {
                int col_start = (mark_beg_row == old_abs_row)
                                ? old_abs_col : abs_col();

                PLine ln = new Line(line(i), col_start, line(i)->len() - col_start, 0);

                ln->xlat(cp_out);
                clip->add_line(ln);
                continue;
            }
            if(i == mark_end_row)
            {
                int col_end = (mark_end_row == old_abs_row)
                              ? old_abs_col : abs_col();

                if(col_end > line(i)->len())
                    col_end = line(i)->len();

                PLine ln = new Line(line(i), 0, col_end, 0);

                ln->xlat(cp_out);
                clip->add_line(ln);
                continue;
            }

            //Default action

            PLine ln = new Line(line(i));

            ln->xlat(cp_out);
            clip->add_line(ln);
        }
    }

    if(iNoTabsIntoClip)
        clip->touch_all();

    clip->to_pm();
    return clip;
}

Buffer* Buffer::cut()
{
    if(!get_mark_state())
        return 0;
    Buffer* clip = copy();
    clear();
    set_changed(1);

    return clip;
}

void Buffer::clear()
{
    if(!get_mark_state())
        return;

    set_changed(1);

    int mark_beg_row = min(old_abs_row, abs_row());
    int mark_end_row = max(old_abs_row, abs_row());

    if(get_column_block() || mark_beg_row == mark_end_row)
    {
        int mark_col_start = min(old_abs_col, abs_col());
        int mark_col_end   = max(old_abs_col, abs_col());

        for(int i = mark_beg_row; i <= mark_end_row; i++)
        {
            track(opRestoreLine, line(i), (void *)i);
            line(i)->del_char(mark_col_start, mark_col_end - mark_col_start);
        }

        if(abs_col() != mark_col_start)
            goto_col(mark_col_start);

        if(abs_row() != mark_beg_row)
            goto_line(mark_beg_row);
    }
    else
    {
        int col_start;
        int col_end  ;

        /* mark_beg_row */

        col_start = (mark_beg_row == old_abs_row) ? old_abs_col : abs_col();
        col_end   = line(mark_beg_row)->len();

        goto_line(mark_beg_row);
        line(mark_beg_row)->del_char(col_start, col_end - col_start, this);

        /* mark_end_row */

        col_start = 0;
        col_end   = (mark_end_row == old_abs_row) ? old_abs_col : abs_col();

		goto_line(mark_end_row);
		line(mark_end_row)->del_char(col_start, col_end - col_start, this);

        /* other lines */

        int i = mark_end_row - (mark_beg_row + 1);

        if(i > 0)
        {
            Buffer *tmp_buf = new Buffer(i);
            remove_items(tmp_buf,(mark_beg_row + 1), i);
            track(opInsBlock, tmp_buf, (void *)(mark_beg_row + 1));
           	line_removed(mark_beg_row + 1, i + 1);
        }

        goto_line(mark_beg_row);
        goto_col((mark_beg_row == old_abs_row) ? old_abs_col : abs_col());

        del_char();
    }
    unmark();

    fill_hiliting(mark_beg_row, line(mark_beg_row)->state());
}

void Buffer::paste(Buffer* clip)
{
    if(!clip)
        return;

    set_changed(1);
    int recalc_row = abs_row()-1;

    if(recalc_row < 0)
        recalc_row = 0;

    int i,j;

    clip->from_pm();

    int as_col_block = clip->get_column_block();

    if(as_col_block && iBlockIns)
        as_col_block = get_column_block() ? 1:0;

    if(as_col_block || clip->Count() == 1)
    {
        for(i = 0; i < clip->Count(); i++)
        {
            PLine ln = new Line(clip->line(i));
            ln->xlat(cp_in);
			track(opRestoreLine, line(abs_row() + i), (void *)(abs_row() + i));

            line(abs_row() + i)->ins_char(abs_col(), ln);

            if((i < clip->Count() - 1) && (abs_row() + i) == (Count() - 1))
            {
                track(opDelLine, Count());
				line_added(Count());
                Add(new Line);
            }

            delete ln;
        }

        if(iAfterBlock && clip->Count() == 1)
            goto_col(abs_col() + clip->line(clip->Count()-1)->len());
    }
    else
    {
        //Split line at insertion point
		track(opRestoreLine, abs_line(), (void *)abs_row());
        track(opDelLine, abs_row()+1);

        PLine ln = abs_line()->split(abs_col(), 0);
    	ins_line(ln, abs_row() + 1);

		//Insert data from first line of the clipboard
    	ln = new Line(clip->line(0));
        ln->xlat(cp_in);
        abs_line()->ins_char(abs_col(), ln, 0);

		//Insert "middle" lines from block
        for(i = 1; i < clip->Count() - 1; i++)
        {
            track(opDelLine, abs_row() + i);
            ln = new Line(clip->line(i));
            ln->xlat(cp_in);
            At(ln, abs_row() + i);
			line_added(abs_row() + i);
        }

        //Insert data from the last line of the clipboard

        goto_line(abs_row() + clip->Count()-1);

        ln = new Line(clip->line(clip->Count() - 1));
        ln->xlat(cp_in);
        abs_line()->ins_char(0, ln);

        goto_col(clip->line(clip->Count()-1)->len());
    }

    fill_hiliting(recalc_row, line(recalc_row)->state());
}

void Buffer::paste_over(Buffer*)
{
    //Still unimplemented
    set_changed(1);
}

