/*
** Module   :PARS_ML.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  31/07/1998 Created
**
*/

#include <parser.h>
#include <version.h>

//----------------------------------------------------------------------
//
// Class Parser_MAIL
//
//----------------------------------------------------------------------

void Parser_MAIL::reset(char *token, int initial_state)
{
    Parser::reset(token, initial_state);
    line_start = token;
}

int Parser_MAIL::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_IDENT;
    char *tmp = tok;

    if(tok == line_start)
    {
        while(*tmp != ' ' && *tmp)
        {
            if(*tmp == '>')
            {
                color = CL_PREPROC;
                while(*tmp)
                    tmp++;

                return (tok_len = (tmp - tok));
            }
            tmp++;
        }
	}
    tok_len = Parser::next_token();
    color = CL_IDENT;
    return tok_len;
}


