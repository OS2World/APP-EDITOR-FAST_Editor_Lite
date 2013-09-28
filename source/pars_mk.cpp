/*
** Module   :PARS_MK.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  09/04/1998 Created
**
*/

#include <parser.h>
#include <version.h>

//----------------------------------------------------------------------
//
// Class Parser_MAKE
//
//----------------------------------------------------------------------

int Parser_MAKE::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;

    int q_chr = (state == ST_QUOTE1) ? '"': ((state == ST_QUOTE2) ? '\'':0);

    if(state == ST_QUOTE1 || state == ST_QUOTE2)
    {
        while(*tmp)
        {
            if(*tmp == q_chr)
            {
                tmp++;
                state = ST_INITIAL;
                break;
            }
            tmp++;
        }
        color = CL_CONST;
        return (tok_len = (tmp - tok));
    }
    switch(*tmp)
    {
        case ' ':
        case '\t':
            for(++tmp; __issp(*tmp);)
                tmp++;
            return (tok_len = (tmp - tok));

        case '"':
        case '\'':
            q_chr = *tmp;

            state = (q_chr == '"') ? ST_QUOTE1:ST_QUOTE2;

            for(++tmp; *tmp != q_chr && *tmp;)
            {
                if(*tmp == '\\')
                    tmp++;
                tmp++;
            }

            if(*tmp == q_chr)
            {
                tmp++;
                state = ST_INITIAL;
            }
            color = CL_CONST;
            return (tok_len = (tmp - tok));

        case '#':
            while(*tmp)
                tmp++;

            color = CL_COMMENT;
            return (tok_len = (tmp - tok));

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

            while(__isdd(*tmp))
                tmp++;
            color = CL_NUMBER;
            return (tok_len = (tmp - tok));

        default:
            if(__isis(*tmp))
            {
                for(++tmp;__isic(*tmp);)
                    tmp++;

                tok_len = (tmp - tok);

                color = CL_IDENT;

                return tok_len;
            }
    }
    return (tok_len = 1);
}


