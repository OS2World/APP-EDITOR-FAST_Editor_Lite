/*
** Module   :PARS_PL.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  08/12/2000 Created
**
*/

#include <parser.h>
#include <version.h>

//----------------------------------------------------------------------
//
// Class Parser_PL
//
//----------------------------------------------------------------------

int Parser_PL::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;

    int q_chr = (state == ST_QUOTE1) ? '"': ((state == ST_QUOTE2) ? '\'':0);

    if(state == ST_QUOTE1 || state == ST_QUOTE2)
    {

// Processing variables inside quotes temporarily disabled

/*
        if(state == ST_QUOTE1 && *tmp == '$')
        {
            for(++tmp;__isic(*tmp);)
            {
                tmp++;

                if(tmp[0] == ':' && tmp[1] == ':')
                    tmp += 2;
            }

            color = CL_IDENT;

            return (tok_len = (tmp - tok));
        }
*/
        while(*tmp)
        {
            if(*tmp == '\\' && tmp[1])
            {
                tmp += 2;
                continue;
            }

            //if(*tmp == '$')
            //    break;

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

            tmp++;

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

        case ';':
            color = CL_SEMICOL;
            break;

        case '\\': /* Skip next character*/
            return (tok_len = (tok[1]) ? 2:1);

        case '%':
        case '$':
        case '@':
            tmp++;

        default:
            if(__isic(*tmp))
            {
                for(++tmp;__isic(*tmp);)
                {
                    tmp++;

                    if(tmp[0] == ':' && tmp[1] == ':')
                        tmp+= 2;
                }

                tok_len = (tmp - tok);

                if(is_kwd())
                    color = CL_STDWORD;
                else
                    color = CL_IDENT;

                if(color == CL_IDENT)
                {
                    while(*tmp)
                    {
                        if(__issp(*tmp))
                        {
                            tmp++;
                            continue;
                        }
                        if(*tmp == '(')
                            color = CL_FUNCTION;
                        break;
                    }
                }

                return tok_len;
            }
    }
    return (tok_len = 1);
}


