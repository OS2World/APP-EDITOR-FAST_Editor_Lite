/*
** Module   :BRACKET.CPP
** Abstract :Bracket matching routine
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  15/03/1998     Created
*/

#include <buffer.h>
#include <version.h>

#define UNDO    	1

#define MAX_ROWS    50

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif


//-----------------------------------------
// Bracket matching v 0.0.1a
//-----------------------------------------

struct brackets_pair
{
    char cLB;
    char cRB;
};

static brackets_pair brackets[]=
{
    {'(',')'},
    {'{','}'},
    {'[',']'},
    {'<','>'}
};

#define BM_FORWARD  0
#define BM_BACKWARD 1
#define BM_NONE     2

void Buffer::match_bracket()
{
    int r;
    int c;

    find_matching_bracket(r, c);

    if(r >= 0 && c >= 0)
        goto_row_col(r, c);
}

void Buffer::flash_bracket()
{
    int r;
    int c;

    if(!iFlash)
        return;

    find_matching_bracket(r, c, rows);

	set_found(r, c, 1); //Assume bracket has length == 1
}

void Buffer::find_matching_bracket(int& r, int& c, int rows)
{
    int i;
    int direction = BM_NONE;
    int pair  = 0;
    int level = 0;
    int an_const = 0;

    r = -1;
    c = -1;

    char bracket = abs_char();

    for(i = 0; i < (sizeof(brackets)/sizeof(brackets[0])); i++)
    {
        if(bracket == brackets[i].cLB)
        {
            // Bracket found, direction forward
            direction = BM_FORWARD;
            pair = i;
            break;
        }

        if(bracket == brackets[i].cRB)
        {
            // Bracket found, direction backward
            direction = BM_BACKWARD;
            pair = i;
            break;
        }
    }

    if(direction == BM_NONE)
        return;

    Parser *res = Parser::GenParser(hiliting);
    int ttype   = abs_line()->token_type(res, abs_col());
    int found   = 0;

    if(direction == BM_FORWARD)
    {
        //Direction: forward
        //quickly looking for bracket, then check other parameters
        int _row = abs_row();
        int _col = abs_col()+1;

        while(!found && _row < Count())
        {
            PLine ln = line(_row);

            for(; !found && _col < ln->len(); _col++)
            {
                if(ln->char_at(_col) != brackets[pair].cLB &&
                   ln->char_at(_col) != brackets[pair].cRB)
                {
                    continue;
                }

                if(ttype != ln->token_type(res, _col))
                    continue;

                if(ln->char_at(_col) == brackets[pair].cLB)
                {
                    level++;
                    continue;
                }

                if(ln->char_at(_col) == brackets[pair].cRB)
                {
                    if(level)
                    {
                    	level--;
                        continue;
                    }
                    r = _row;
                    c = _col;
                    found = 1;
                    break;
                }
            }
            _row++;
            _col = 0;

            if(rows > 0 && _row > (abs_row() + rows))
                break;
        }
    }
    else
    {
        int _row = abs_row();
        int _col = abs_col()-1;

        while(!found && _row >= 0)
        {
            PLine ln = line(_row);

            for(; !found && _col >= 0; _col--)
            {
                if(ln->char_at(_col) != brackets[pair].cLB &&
                   ln->char_at(_col) != brackets[pair].cRB)
                {
                    continue;
                }

                if(ttype != ln->token_type(res, _col))
                    continue;

                if(ln->char_at(_col) == brackets[pair].cRB)
                {
                    level++;
                    continue;
                }

                if(ln->char_at(_col) == brackets[pair].cLB)
                {
                    if(level)
                    {
                        level--;
                        continue;
                    }
                    r = _row;
                    c = _col;
                    found = 1;
                    break;
                }
            }
            _row--;
            _col = line(_row) ? line(_row)->len() : 0;

            if(rows > 0 && _row < (abs_row() - rows))
                break;
        }
    }

    delete res;
}

