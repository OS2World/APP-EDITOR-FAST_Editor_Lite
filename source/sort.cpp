/*
** Module   :SORT.CPP
** Abstract :Block sort routine
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  29/03/1998 	Created
*/

#include <string.h>

#include <buffer.h>
#include <version.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

void Buffer::sort()
{
    if(!get_mark_state())
        return;

    if(!get_column_block())
        return;

    int mark_beg_row   = min(old_abs_row, abs_row());
    int mark_end_row   = max(old_abs_row, abs_row());
    int mark_col_start = min(old_abs_col, abs_col());
    int mark_col_end   = max(old_abs_col, abs_col());

    int len   = mark_end_row - mark_beg_row;
    int width = mark_col_end - mark_col_start;

    if(!len)
        return;

    len++;

    if(!width)
        return;

    set_changed(1);

    //Sort using shell sort

    unsigned long gap, i, j;
    char *buf1;
    char *buf2;

    for(i = mark_beg_row; i <= mark_end_row; i++)
        track(opRestoreLine, (void *)line(i), (void *)i);

    buf1 = new char[width + 1];
    buf2 = new char[width + 1];

    for (gap = 0; ++gap < len;)
          gap *= 3;

    while ((gap /= 3) != 0)
    {
        for (i = gap; i < len; i += 1)
        {
            for (j = i - gap; ;j -= gap)
            {
                //Compare
                line(j + mark_beg_row)->get_print(mark_col_start, buf1, width);
                line(j + gap + mark_beg_row)->get_print(mark_col_start, buf2, width);

                if (__nstrcmp(buf1, buf2, cp_out) <= 0 )
                    break;

                //Swap
                Ptr tmp = ppData[j + mark_beg_row];
                ppData[j + mark_beg_row]     = ppData[j + gap + mark_beg_row];
                ppData[j + gap+mark_beg_row] = tmp;

                if (j < gap)
                      break;
            }
        }
    }
    delete buf1;
    delete buf2;

    fill_hiliting(mark_beg_row, line(mark_beg_row)->state());
}

