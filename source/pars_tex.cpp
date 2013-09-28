/*
** Module   :PARS_TEX.CPP
** Abstract :TeX parser module
**
** Copyright (C) Alexander Belyaev
**
** Log: Sun  25/11/2001 Created
**
*/

#include <parser.h>
#include <version.h>

//----------------------------------------------------------------------
//
// Class Parser_TEX
//
//----------------------------------------------------------------------

int Parser_TEX::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;

    switch(*tmp)
    {
        case ' ':
        case '\t':
            for(++tmp; __issp(*tmp);)
                tmp++;
            return (tok_len = (tmp - tok));
/*
        case '{':
        case '}':
//            color = CL_FUNCTION;
            color = CL_IDENT;
            return tok_len = 1;
*/
        case '\\':
            color = CL_IDENT;
            while(*tmp)
            {
                tmp++;

                if(*tmp==' '|| *tmp=='\\' ||
                   *tmp=='[' || *tmp==']' ||
                   *tmp=='(' || *tmp==')' ||
                   *tmp=='{' || *tmp=='}')
                {
                   break;
                }
                if(__ispu(*tmp))
                {
                    tmp++;
                    break;
                }
            }
            return (tok_len = (tmp - tok));

        case '%':
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

                return (tok_len = (tmp - tok));
            }

            return (tok_len = 1);
    }
    return (tok_len = 1);
}


