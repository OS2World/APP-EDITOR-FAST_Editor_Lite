/*
** Module   :PARS_RX.CPP
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
// Class Parser_REXX
//
//----------------------------------------------------------------------

int Parser_REXX::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;

    int q_chr = (state == ST_QUOTE1) ? '"': ((state == ST_QUOTE2) ? '\'':0);

    if(state >= ST_COMMENT)
    {
        while(*tmp)
        {
            if(*tmp == '*' && *(tmp+1) == '/')
            {
                tmp += 2;
                state = ST_INITIAL;
                break;
            }
            tmp++;
        }
        color = CL_COMMENT;

        if(state < ST_COMMENT)
            state = ST_INITIAL;

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
            ++tmp;
            color = CL_CONST;
            return (tok_len = (tmp - tok));

        case '/':
            if(tok[1] == '*')
            {
                tmp += 2;

                state = ST_COMMENT;

                while(*tmp)
                {
                    if(*tmp == '*' && *(tmp + 1) == '/')
                    {
                        tmp += 2;
                        state--;
                        break;
                    }
                    tmp++;
                }
                color = CL_COMMENT;

                if(state < ST_COMMENT)
                    state = ST_INITIAL;

                return (tok_len = (tmp - tok));
            }
            break;

        case ';':
            color = CL_SEMICOL;
            break;

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

        case '~':
            tmp++;
            color = CL_IDENT;
            return (tok_len = (tmp - tok));

        case ':':
            if(tmp[1] != ':')
                break;

            tmp += 2;

            if(!__isis(*tmp) || !*tmp)
            {
                color = CL_IDENT;
                return tok_len = (tmp - tok);
            }

        default:
            if(__isis(*tmp))
            {
                for(++tmp;__isic(*tmp);)
                {
                    tmp++;

                    if(*tmp == '~')
                    {
                        tmp++;
                        color = CL_FUNCTION;
                    }
                }

                tok_len = (tmp - tok);

                if(color != CL_DEFAULT)
                    return tok_len;

                if(is_kwd())
                {
                    color = CL_STDWORD;
                    return tok_len;
                }

                if(*tmp && (*tmp == '(' || *tmp == ':'))
                    color = CL_FUNCTION;
                else
                    color = CL_IDENT;
                return tok_len;
            }
    }
    return (tok_len = 1);
}


