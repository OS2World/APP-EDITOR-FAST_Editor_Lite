/*
** Module   :COMPILE.CPP
** Abstract :Keyboard macro compiler
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  27/03/1998     Created
*/

#include <string.h>

#include <boxcoll.h>
#include <version.h>

static int cx2n(char chr)
{
    int n = 0;

    if(__isdd(chr))
        return chr - '0';

    if(chr >= 'a' || chr <= 'z')
        return chr - 'a' + 0x0A;

    if(chr >= 'A' || chr <= 'Z')
        return chr -'A' + 0x0A;

    return 0;
}

int KeyDefCollection::compile_keydef(char *str, char *out)
{
    int cnt = 0;

    while(*str)
    {
        switch(*str)
        {
            case ' ':
            case ',':
            case '\r':
            case '\n':
            case '\t':
                break;

            case '"':
            case '{':
            case '\'':
                {
                    int iRexx = 0;

                    char delim = *str++;

                    if(delim == '{') //This is rexx?
                    {
                        delim = '}';

                        cnt += sizeof(PEditProc) + 1;

                        if(out)
                        {
                            PEditProc proc = EditBoxCollection::doRexx;
                            *out++ = FUNC_ESCAPE;
                            memcpy(out, &proc, sizeof(PEditProc));
                            out += sizeof(PEditProc);
                        }

                        iRexx++;
                    }

                    while(*str)
                    {
                        if(delim != '}' && *str == delim)
                            break;

                        if(delim == '}' && *str == delim && !str[1])
                            break;

                        char chr = *str;

                        if(*str == '\\' && !iRexx) //Escape sequences
                        {
                            str++;

                            switch(*str)
                            {
                                case 'n':
                                    chr = '\n';
                                    break;

                                case 'r':
                                    chr = '\r';
                                    break;

                                case 't':
                                	chr = '\t';
                                    break;

                                case 'x':
                                    {
                                        str++;
                                        chr = 0;
                                        int k;

                                        for(k = 0; k < 2; k++)
                                        {
                                            if(!__ishd(*str))
                                                break;

                                            chr <<= 4;

                                            chr |= cx2n(*str++);
                                        }

                                        if(out)
                                            *out++ = chr++;
                                    }
                                    continue;

                                default:
                                	chr = *str;
                                	break;
                            }
                        }

                        if(out)
                            *out++ = chr;

                        cnt++;
                        str++;
                    }

                    if(*str && delim == '}')
                        return cnt;
                }
                break;

            default:
                if(__isis(*str))
                {
                    char *ptr = str;
                    while(*str && __isic(*str))
                        str++;

                    PDEntry pFunc = (PDEntry)Func_DIC.IsIn(ptr, str - ptr, 0);

                    if(pFunc)
                    {
                        cnt += sizeof(PEditProc) + 1; //function code

                        if(out)
                        {
                            *out++ = FUNC_ESCAPE;
                            memcpy(out, &pFunc->proc, sizeof(PEditProc));
                            out += sizeof(PEditProc);
                        }
                        if(!memcmp(pFunc->key, "markgo", 6) ||
                           !memcmp(pFunc->key, "markset", 7) ||
                           !memcmp(pFunc->key, "openjumplist", 12))
                        {
                            //Parameters
							cnt++;

							if(out)
							{
								char* num = pFunc->key + strlen(pFunc->key) - 1;

								*out++ = (*num - '0');
							}
                        }
                    }
                    else
                        return -2; //Wrong function name
                }
                else
                    return -1; //Wrong char in keydef
        }
        if(*str)
        	str++;
    }
    return cnt;
}

int KeyDefCollection::decompile_keydef(char *macro, char* buf)
{
    if(!macro)
        return 0;

    int len = 3;	//make some spare space

    int plain_text = 0;

    while(*macro)
    {
        if(*macro == FUNC_ESCAPE)
        {
            macro++;

            if(plain_text)	//Function name encountered inside plain text
            {
        		plain_text = 0;

            	if(buf)
            		*buf++ = '"';

            	len++;
            }

            if(buf)
            	*buf++ = ' ';

            len++;

            char* fname = FunctionDictionary::locate_name(macro);

            if(!fname)
            	fname = "bug_bug_bug";

            fname = str_dup(fname);

            int delta = 0;
            int sz = strlen(fname);

            static PEditProc plist[4] = { EditBoxCollection::doGotoMark,
            	                          EditBoxCollection::doSetMark ,
            	                          EditBoxCollection::doJumpList,
            	                          EditBoxCollection::doRexx};


			if(!memcmp(macro, &plist[0], sizeof(PEditProc)) ||
			   !memcmp(macro, &plist[1], sizeof(PEditProc)) ||
			   !memcmp(macro, &plist[2], sizeof(PEditProc)))
			{
				delta = 1;
				fname[sz - 1] = '0' + macro[sizeof(PEditProc)];
			}

			if(!memcmp(macro, &plist[3], sizeof(PEditProc)))
				delta = strlen(macro + sizeof(PEditProc));

            len += sz + 1;

			if(fname[0] == '-')
				len += delta + 2;

            if(buf)
            {
            	//strcpy(buf, fname);
            	if(fname[0] == '-')
            	{
            		*buf++ = '{';
					strcpy(buf, macro + sizeof(PEditProc));
					buf += delta;
            		*buf++ = '}';
            	}
            	else
            	{
            		strcpy(buf, fname);
            		buf += sz;
            	}

            	*buf++ = ' ';
            }

            delete fname;

            macro += sizeof(PEditProc) + delta;
        }
        else
        {
        	if(!plain_text) //Switching from function to text
        	{
        		plain_text = 1;

            	if(buf)
            		*buf++ = '"';

            	len++;
        	}

           	switch(*macro)
           	{
           		case '\n':
           			if(buf)
           			{
           				*buf++ = '\\';
           				*buf++ = 'n';
           				*buf++ = '"';
           				*buf++ = '\n';
           				*buf++ = '"';
           			}
           			len += 5;
           			break;

           		case '\r':
           			if(buf)
           			{
           				*buf++ = '\\';
           				*buf++ = 'r';
           			}
           			len += 2;
           			break;

           		case '\t':
           			if(buf)
           			{
           				*buf++ = '\\';
           				*buf++ = 't';
           			}
           			len += 2;
           			break;

           		case '\"':
           			if(buf)
           			{
           				*buf++ = '\\';
           				*buf++ = '"';
           			}
           			len += 2;
           			break;

           		default:
           		    if(buf)
	           			*buf++ = *macro;
					len++;
           			break;
           	}

           	macro++;
        }
    }

    if(plain_text)	//trailing quote
    {
   		if(buf)
    		*buf++ = '"';

    	len++;
    }

    if(buf)
    	*buf = 0;

    return len;
}

