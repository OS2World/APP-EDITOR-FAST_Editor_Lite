/*
** Module   :PARS_HTM.CPP
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
// Class Parser_HTML
//
//----------------------------------------------------------------------

int Parser_HTML::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;

    if(state == ST_COMMENT) //HTML style comment
    {
        color = CL_COMMENT;

        for(;*tmp; tmp++)
        {
            if(tmp[0] == '-' && tmp[1] == '-' && tmp[2] == '>')
            {
                tmp += 3;
                state = ST_INITIAL;
                break;
            }
        }

        return (tok_len = (tmp - tok));
    }

    while(state == ST_QUOTE1)  /* HTML code */
    {
        switch(*tmp)
        {
            case '\t':
            case ' ':
                for(++tmp; __issp(*tmp);)
                    tmp++;

                return (tok_len = (tmp - tok));

            case '\'':
            case '"':
                {
                    int q_chr = *tmp;

                    for(++tmp; *tmp != q_chr && *tmp;)
                    {
                        if(*tmp == '\\' && tmp[1])
                        {
                            tmp += 2;
                            continue;
                        }
                        tmp++;
                    }

                    if(*tmp == q_chr)
                        tmp++;

                    color = CL_CONST;

                    return (tok_len = (tmp - tok));
                }

            case '&':
                tmp++;
                while(*tmp && *tmp != ';' && __isan(*tmp) != ' ')
                    tmp++;

                if(*tmp == ';')
                    tmp++;

                color = CL_XNUMBER;
                return (tok_len = (tmp - tok));

            case '>':
                state = ST_INITIAL;
                break;

            default:
                if(__isic(*tmp))
                {
                    for(++tmp;__isic(*tmp);)
                        tmp++;

                    color = CL_IDENT;

                    return (tok_len = (tmp - tok));
                }
        }
        return (tok_len = 1);
    }

    while(state >= ST_PHP_START && state <= ST_PHP_END)   /* PHP code */
    {
        int q_chr = (state == ST_STR_PHP1) ? '"': ((state == ST_STR_PHP2) ? '\'':0);

        if(state == ST_COMMENT_PHP)
        {
            while(*tmp)
            {
                if(*tmp == '*' && *(tmp+1) == '/')
                {
                    tmp += 2;
                    state = ST_PHP_START;
                    break;
                }
                tmp++;
            }
            color = CL_COMMENT;

            return (tok_len = (tmp - tok));
        }

        if(state == ST_STR_PHP1 || state == ST_STR_PHP2)
        {
            while(*tmp)
            {
                if(*tmp == '\\' && tmp[1])
                {
                    tmp += 2;
                    continue;
                }
                if(*tmp == q_chr)
                {
                    tmp++;
                    state = ST_PHP_START;
                    break;
                }
                tmp++;
            }
            color = CL_CONST;
            return (tok_len = (tmp - tok));
        }

        if(!__cnstrcmp(tmp, "</script",8))
        {
            tmp += 8;

            while(*tmp && *tmp != '>')
                tmp++;

            if(*tmp == '>')
                tmp++;

            state = ST_INITIAL;
            color = CL_PREPROC;

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

                state = (q_chr == '"') ? ST_STR_PHP1:ST_STR_PHP2;
                tmp++;

                color = CL_CONST;
                return (tok_len = (tmp - tok));

            case '#':

                while(*tmp)
                        tmp++;

                color = CL_COMMENT;
                return (tok_len = (tmp - tok));

            case '/':

                if(tok[1] == '*')
                {
                    tmp += 2;

                    state = ST_COMMENT_PHP;
                    color = CL_COMMENT;

                    return (tok_len = (tmp - tok));
                }
                if(tok[1] == '/')
                {
                    tmp += 2;
                    while(*tmp)
                        tmp++;

                    color = CL_COMMENT;
                    return (tok_len = (tmp - tok));
                }
                break;

            case '?':
                if(tmp[1] != '>')
                    break;

                tmp += 2;

                state = ST_INITIAL;
                color = CL_PREPROC;

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

                color = CL_NUMBER;

                while(__ishd(*tmp))
                {
                    tmp++;

                    if(*tmp == 'x' || *tmp == 'X')
                    {
                        color = CL_XNUMBER;
                        tmp++;
                    }
                }

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

                    color = CL_IDENT;

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

    if(*tmp == '<')
    {
        if(tmp[1] == '!' && tmp[2] == '-' && tmp[3] == '-')
        {
            if(tmp[4] != '#')
            {
            	tmp += 4;
            	state = ST_COMMENT;
            	color = CL_COMMENT;

                return (tok_len = (tmp - tok));
            }
        }

        if(tmp[1] == '?')
        {
            tmp += 2;

            state = ST_PHP_START;  //PHP
            color = CL_PREPROC;

            return (tok_len = (tmp - tok));
        }

        if(!__cnstrcmp(tmp, "<script",7))
        {
            //Search for closing '>'
            while(*tmp && *tmp != '>')
                tmp++;

            if(*tmp == '>')
            {
                tmp++;
	            state = ST_PHP_START;  //PHP
    	        color = CL_PREPROC;
                return (tok_len = (tmp - tok));
            }
        }

        state = ST_QUOTE1;

        //NOTE: caller will be ought to issue another call
        //for the same position. But state already changed
        //and next time token will be processed by other
        //part of code

        return 0;
    }

    if(*tmp == '&')
    {
        tmp++;
        while(*tmp && *tmp != ';' && __isan(*tmp) != ' ')
            tmp++;

        if(*tmp == ';')
        	tmp++;

        color = CL_XNUMBER;
        return (tok_len = (tmp - tok));
    }

    while(*tmp)
    {
        if(*tmp == '<' || *tmp == '&')
            break;

        tmp++;
    }

    return (tok_len = (tmp - tok));
}

