/*
** Module   :PARS_ASM
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
// Class Parser_ASM
//
//----------------------------------------------------------------------

int Parser_ASM::next_token()
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
            ++tmp;
            color = CL_CONST;
            return (tok_len = (tmp - tok));

        case ';':
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

            while(__ishd(*tmp))
            {
                if(__ishd(*tmp) && !__isdd(*tmp))
                    color = CL_XNUMBER;
                tmp++;
            }

            if(__to_upper(*tmp) == 'H')
            {
                tmp++;
                color = CL_XNUMBER;
            }

            if(color == CL_DEFAULT)
                color = CL_NUMBER;
            return (tok_len = (tmp - tok));

        default:
            if(__isis(*tmp) || *tmp == '%' || *tmp=='@' || *tmp == '.')
            {
                for(++tmp;__isic(*tmp) || *tmp == '%' || *tmp=='@';)
                    tmp++;

                tok_len = (tmp - tok);

                if(is_kwd())
                    color = CL_STDWORD;
                else
                {
                    color = CL_IDENT;

                    while(*tmp)
                    {
                        if(__issp(*tmp))
                        {
                            tmp++;
                            continue;
                        }
                        if(*tmp == '(' || *tmp == ':')
                            color = CL_FUNCTION;
                        break;
                    }
                }
                return tok_len;
            }
    }
    return (tok_len = 1);
}

