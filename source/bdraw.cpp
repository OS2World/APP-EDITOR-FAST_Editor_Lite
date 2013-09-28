/*
** Module   :BDRAW.CPP
** Abstract :Buffer::draw implementation
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  15/03/1998     Created
*/

#ifdef __FED_DEBUG__
#define INCL_DOS
#define INCL_VIO
#include <os2.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <buffer.h>
#include <version.h>

#define UNDO    	1
#define USE_SCROLL 	1

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

void Buffer::draw()
{
    int i,j,x;
    Parser * parser = gen_parser();

    char *save_buff;

    int mark_col_start = min(old_abs_col, abs_col());
    int mark_col_end   = max(old_abs_col, abs_col());
    int mark_beg_row   = min(old_abs_row, abs_row());
    int mark_end_row   = max(old_abs_row, abs_row());

    mark_col_start = max(mark_col_start, start_col);
    mark_col_end   = min(mark_col_end  , start_col + cols);

    char *print_buffer2 = new char[(cols+start_col + 5)];
    char *print_buffer  = &print_buffer2[start_col];

    memset(AlignedBuffer, 0, BufLen);
    save_buff = vio_set_work_buff(AlignedBuffer);

    for(j = start_row, i = 0; i < rows; i++, j++)
    {
        if(j >= Count())
        {
            vio_print(row + i, col, "<EOF>", cols, app_pal[pal_start + CL_EOF]);
            break;
        }

        print_buffer[0] = '\x0';

        if(hiliting && start_col < line(j)->len())
        {
#define ScreenOffset(Row,Col) (((Row) * Cols + (Col)) << 1)
            char *scr_start = &Screen[ScreenOffset(row + i, col)];
            PLine ln = line(j);

            ln->get_print(0, print_buffer2, cols+start_col);

            print_buffer2[cols+start_col] = 0;

            for(x = 0; x < cols+start_col; x++)
                    print_buffer2[x] = chr_out(print_buffer2[x]);

            parser->reset(print_buffer2, line(j)->state());

            int tok_pos = 0;
            int delta = 0;

            while(*parser->tok)
            {
                delta = parser->next_token();
                tok_pos = (parser->prev_token() - print_buffer2);
                if((tok_pos + parser->tok_len) < start_col)
                {
                    parser->tok += delta;
                    continue;
                }
                else
                    break;
            }
            if(tok_pos < start_col)
            {
                parser->tok_len -= (start_col - tok_pos);
                parser->tok     += (start_col - tok_pos);
                delta   -= (start_col - tok_pos);
                tok_pos  = start_col;
            }
            while(*parser->tok)
            {
                char *scr = parser->tok;
                char clr  = app_pal[parser->color+pal_start];

                for(int counter = 0; counter < parser->tok_len; counter++)
                {
                    *scr_start++ = *scr++;
                    *scr_start++ = clr;
                }
                parser->tok += delta;
                delta = parser->next_token();
                tok_pos = (parser->tok-print_buffer2);
            }
        }
        else
        {
            if(start_col < line(j)->len())
            {
                line(j)->get_print(start_col, print_buffer, cols);

                for(x = 0; x < cols; x++)
                    print_buffer[x] = chr_out(print_buffer[x]);
            }

            vio_print(row + i, col, print_buffer, cols, app_pal[pal_start + CL_DEFAULT]);
        }

        if(get_mark_state())
        {
            if(!print_buffer[0] && start_col < line(j)->len())
            {
                line(j)->get_print(start_col, print_buffer, cols);

                for(x = 0; x < cols; x++)
                    print_buffer[x] = chr_out(print_buffer[x]);

            }

            if(!print_buffer[0])
                print_buffer[mark_col_start - start_col] = 0;

            if(get_column_block())
            {
                if(j >= mark_beg_row && j <= mark_end_row)
                    vio_print(row + i, col + (mark_col_start - start_col),
                              &print_buffer[mark_col_start - start_col],
                              (mark_col_end - mark_col_start),
                               app_pal[pal_start + CL_SELECTION]);
            }
            else
            {
                if(j > mark_beg_row && j < mark_end_row) // full line
                {
                    vio_print(row + i, col, print_buffer, cols,
                              app_pal[pal_start + CL_SELECTION]);
                }
                else
                {
                    if(j == mark_beg_row || j == mark_end_row)
                    {
                        int hi_start = 0;
                        int hi_end   = 0;

                        if(mark_beg_row != mark_end_row)
                        {
                            if(j == mark_beg_row)
                            {
                                hi_start = (mark_beg_row == old_abs_row) ?
                                			old_abs_col : abs_col();
                                hi_start = max(hi_start, start_col);
                                hi_end   = start_col+cols;
                            }
                            if(j == mark_end_row)
                            {
                                hi_start = start_col;
                                hi_end   = (mark_end_row == old_abs_row) ?
                                            old_abs_col : abs_col();
                                hi_end   = min(hi_end, start_col+cols);
                            }
                        }
                        else
                        {
                            hi_start = min(old_abs_col, abs_col());
                            hi_start = max(hi_start, start_col);
                            hi_end   = max(old_abs_col, abs_col());
                            hi_end   = min(hi_end, start_col+cols);
                        }

                        if(hi_end - hi_start)
                        {
                            if(!print_buffer[0])
                                print_buffer[hi_start - start_col] = 0;

                            vio_print(row + i, col + (hi_start - start_col),
                                      &print_buffer[hi_start - start_col],
                                      (hi_end - hi_start),
                                      app_pal[pal_start + CL_SELECTION]);
                        }
                    }
                }
            }
        }

        if(j == found_row && found_len > 0)
        {
            if(start_col <= found_col &&
               start_col + cols >= found_col)
            {
                int draw_len = found_len;
                if(found_col + found_len > start_col + cols)
                    draw_len = (start_col + cols) - found_col;

                vio_draw_attr(row + i, col + (found_col - start_col),
                              draw_len, app_pal[pal_start + CL_MATCHING]);
            }
        }


        if(iDrawEOL &&
           line(j)->len() >= start_col &&
           line(j)->len() < (start_col + cols))
        {
            vio_draw_chr(row + i, col + line(j)->len() - start_col, '\xFA');

            char attr = vio_get_attr(row + i, col + line(j)->len() - start_col);

            if(attr != app_pal[pal_start + CL_SELECTION] &&
               attr != app_pal[pal_start + CL_MATCHING])
            {
                vio_draw_attr(row + i, col + line(j)->len() - start_col, 1,
                              app_pal[pal_start + CL_EOL]);
            }
        }
    }
    {
        vio_set_work_buff(save_buff);

        //try to guess which part of screen can be reused
        //if old start_col and start_row differs from current
        //try to shift screen buffer to proper destination

        if(start_row > draw_save.start_row &&
           (start_row - draw_save.start_row) < rows)
        {
            vio_scroll(SCROLL_UP, *this,
                       start_row - draw_save.start_row,
                       app_pal[pal_start + CL_DEFAULT]);
        }

        if(start_row < draw_save.start_row &&
           (draw_save.start_row - start_row) < rows)
        {
            vio_scroll(SCROLL_DN, *this,
                       draw_save.start_row - start_row,
                       app_pal[pal_start + CL_DEFAULT]);
        }

        if(start_col > draw_save.start_col &&
           (start_col - draw_save.start_col) < cols)
        {
            vio_scroll(SCROLL_LT, *this,
                       start_col - draw_save.start_col,
                       app_pal[pal_start + CL_DEFAULT]);
        }

        if(start_col < draw_save.start_col &&
           (draw_save.start_col - start_col) < cols)
        {
            vio_scroll(SCROLL_RT, *this,
                       draw_save.start_col - start_col,
                       app_pal[pal_start + CL_DEFAULT]);
        }
        for(i = 0; i < rows; i++)
        {
            int rowstart = ScreenOffset(row + i, col);

            if(memcmp(&Screen[rowstart], &AlignedBuffer[rowstart], cols * 2))
            {
                memcpy(&Screen[rowstart], &AlignedBuffer[rowstart], cols * 2);
                vio_show_buf(rowstart, cols * 2);
            }
        }

        draw_save.start_row = start_row;
        draw_save.start_col = start_col;
    }
    delete print_buffer2;
    delete parser;
}

