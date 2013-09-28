/*
** Module   :PARS_PAS.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  08/03/1999 Created
**
*/

#include <parser.h>
#include <version.h>

//----------------------------------------------------------------------
//
// Class Parser_PAS
//
//----------------------------------------------------------------------

int Parser_PAS::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;

    int q_chr = (state == ST_QUOTE1) ? '"': ((state == ST_QUOTE2) ? '\'':0);

    if(state == ST_COMMENT)
    {
        while(*tmp)
        {
            if(*tmp == '*' && *(tmp+1) == ')')
            {
                tmp += 2;
                state = ST_INITIAL;
                break;
            }
            tmp++;
        }

        color = CL_COMMENT;
        return (tok_len = (tmp - tok));
    }

    if(state == ST_COMMENT2)
    {
        while(*tmp)
        {
            if(*tmp == '}')
            {
                tmp++;
                state = ST_INITIAL;
                break;
            }
            tmp++;
        }

        color = CL_COMMENT;
        return (tok_len = (tmp - tok));
    }

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
                tmp++;

            if(*tmp == q_chr)
            {
                tmp++;
                state = ST_INITIAL;
            }
            color = CL_CONST;
            return (tok_len = (tmp - tok));

        case '(':
            if(tok[1] == '*')
            {
                tmp += 2;

                state = ST_COMMENT;
                color = CL_COMMENT;

                return (tok_len = (tmp - tok));
            }
            break;

        case '{':

            tmp++;

            state = ST_COMMENT2;
            color = CL_COMMENT;

            return (tok_len = (tmp - tok));

        case ';':
            color = CL_SEMICOL;
            break;

        case '$':

            tmp++;

            if(!__ishd(*tmp))
            {
                color = CL_DEFAULT;
                break;
            }

            color = CL_XNUMBER;

            /* fall through */

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
                tmp++;

            if(color == CL_DEFAULT)
                color = CL_NUMBER;
            return (tok_len = (tmp - tok));

        default:
            if(__isis(*tmp))
            {
                for(++tmp;__isic(*tmp);)
                    tmp++;

                tok_len = (tmp - tok);

                if(is_kwd())
                    color = CL_STDWORD;
                else
                    color = CL_IDENT;

                if(color == CL_IDENT && *tmp && (*tmp == '(' || *tmp == ':'))
                {
                    color = CL_FUNCTION;
                }
                return tok_len;
            }
    }
    return (tok_len = 1);
}

