/*
** Module   :PROFILE.CPP
** Abstract :FED profile (FED.INI) reader/parser
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  17/03/1998     Created
*/

#include <string.h>

#define INCL_DOS
#include <os2.h>

#include <stddlg.h>
#include <boxcoll.h>
#include <fio.h>
#include <version.h>

int get_ini_name(char *cProgName, char *ini_ext)
{
    char* lastdot;
    char* lastslash;

    strcpy(cProgName, _cFedPATH);

    lastdot   = strrchr(cProgName, '.');
    lastslash = strrchr(cProgName, '\\');

    if(lastdot > lastslash) //dot really in extension, not in path
        strcpy(lastdot, ini_ext);
    else
        strcat(cProgName, ini_ext);

    return 0;
}

void EditBoxCollection::LoadHistory(void)
{
    char cProgName[FED_MAXPATH];

    if(get_ini_name(cProgName, ".ini"))
    	return;

    ::LoadHistory(cProgName);
}

void EditBoxCollection::StoreHistory(void)
{
    char cProgName[FED_MAXPATH];

    if(get_ini_name(cProgName, ".ini"))
        return;

    ::StoreHistory(cProgName);
}

void EditBoxCollection::load_profile(int mode)
{
    char cProgName[FED_MAXPATH];

    if(!mode)
    {
        if(get_ini_name(cProgName, ".ini"))
            return;
    }
    else
    {
        strcpy(cProgName, "fed.ini");
    }

    load_profile_file(cProgName);
}

static char* skip_to_eol(char* ptr)
{
	while(*ptr && *ptr != '\n')
        ptr++;

    return ptr;
}

void EditBoxCollection::load_profile_file(char* cFile)
{
	char* fname = get_full_name(cFile);

	if(get_ini_list().Find(fname))	//Prevent recursion, file already loaded
	{
		delete fname;
		return;
	}

	char *orig_file = _ld_file(fname);

	if(orig_file)
		load_profile(orig_file);

	_fr_file(orig_file);

	get_ini_list().Add(fname);
}

void EditBoxCollection::load_profile(char* orig_file)
{
    char *ptr;
    char *str;

    for(ptr = str = orig_file; *ptr;)
    {
        int j = 0;

        //Skip whitespaces
        //Skip empty lines
        while(__issp(*ptr))
            ptr++;

        // Ignore comments
        if(*ptr == ':' || *ptr == '#' || (ptr[0] == '/' && ptr[1] == '/'))
        {
            ptr = skip_to_eol(ptr);
            continue;
        }

        if(*ptr == '@') // '@' - commands
        {
            char cCmd[32];
            int i;

            ptr++;

            while(__isic(ptr[j]) || ptr[j] == '.')
                j++;

            if(j == 0 || j >= sizeof(cCmd))  /* something wrong, skip line */
            {
                ptr = skip_to_eol(ptr);
                continue;
            }

            for(i = 0; i < j; i++)
                cCmd[i] = __to_lower(ptr[i]);

            cCmd[i] = 0;

            ptr += j;

            while(__issp(*ptr))
            	ptr++;

            if(!strcmp(cCmd, "include"))    //@include "some.file"
            {
                char cProgName[FED_MAXPATH];

                do
                {
                    int iDelim = *ptr;

                    if(iDelim != '\'' && iDelim != '"')
                    {
                        ptr = skip_to_eol(ptr);
                        break;
                    }

                    ptr++;
                    str = ptr;

                    while(*ptr && *ptr != iDelim && *ptr != '\r' && *ptr != '\n')
                        ptr++;

                    if(*ptr != iDelim)
                    {
                        if(*ptr != '\n')
	                        ptr = skip_to_eol(ptr);
                        break;
                    }

                    int iSz = (ptr - str);

                    if(iSz <= 0 || iSz >= sizeof(cProgName))
                    {
                        ptr = skip_to_eol(ptr);
                        break;
                    }

                    memcpy(cProgName, str, iSz);
                    cProgName[iSz] = 0;

                    //Check if file with such a name can be loaded
                    if(!_file_exists(cProgName))
                    {
                        //Try other name

                        strcpy(cProgName, _cFedPATH);

                        char* lastslash = strrchr(cProgName, '\\');

                        if(!lastslash)
                            lastslash=cProgName;
                        else
                            lastslash++;

                        memcpy(lastslash, str, iSz);

                        cProgName[iSz + (lastslash - cProgName)] = 0;
                    }

                    load_profile_file(cProgName);

                    ptr = skip_to_eol(ptr);
                }
                while(0);
            }
            else if(!strcmp(cCmd, "mode"))
            {
                do
                {
                    j = 0;

                    while(__isic(ptr[j]) || ptr[j] == '+')
                        j++;

                    if(j == 0 || j >= sizeof(cCmd))  /* something wrong, wkip line */
                    {
                        ptr = skip_to_eol(ptr);
                        break;
                    }

    	            for(i = 0; i < j; i++)
                        cCmd[i] = ptr[i];

    	            cCmd[i] = 0;

                    ptr += j;

                    while(__issp(*ptr))
                        ptr++;

                    if(*ptr != '{')
                    {
                        ptr = skip_to_eol(ptr);
                        break;
                    }

                    ptr++;

                    str = ptr;

	                while(*ptr)
                    {
                        while(*ptr && *ptr != '}')
            	            ptr++;

                	    if(!*ptr)
                    	    break;

	                    char* tmp = ptr + 1;

        	            while(__issp(*tmp) && *tmp != '\n') //skip trailing spaces
            	            tmp++;

                        if(*ptr == '}' && *tmp == '\n')   //at the end of line!
                    	    break;
	                    ptr++;
                    }

                    if(*ptr && *ptr != '}')
                    {
                        ptr = skip_to_eol(ptr);
                        break;
                    }

                    int iSz = (ptr - str);

                    char* compile = new char[iSz + 1];

                    for(i = 0; str < ptr; i++)
                        compile[i] = *str++;

                    compile[i] = 0;

                    //Parse it using recursive call to ourselves
                    //load_profile(compile, cCmd);

                    delete compile;
                }
                while(0);
            }
            else
                ptr = skip_to_eol(ptr);

            continue;
        }

        // Look for variable name

        while(__isic(ptr[j]) || ptr[j] == '.')
        	j++;

        if(j == 0)  /* something wrong, wkip line */
        {
            ptr = skip_to_eol(ptr);
            continue;
        }

		Line ln;

		ln.ins_char(0, ptr, j);

		if(ln.find_char('.'))	//Looks like a variable name
		{
            //Process lines
            //Skip everything up to first non-space character after delimiter

            ptr += j;

            while(__issp(*ptr))
    	        ptr++;

            if(*ptr != ':' && *ptr != '=')
            {
	            ptr = skip_to_eol(ptr);
    	        continue;
            }

            ptr++;

            while(__issp(*ptr))
                ptr++;

            do
            {
                int iDelim = ptr[0];
                char *pEOL = 0;

                if(iDelim != '\'' && iDelim != '"')
                {
                	//Take one word
                	j = 0;

		            while(!__issp(ptr[j]))
        		        j++;

					Line var;

					var.ins_char(0, ptr, j);

					get_bindery().setVar(ln.str, var.str);

                    ptr = skip_to_eol(ptr);
                    break;
                }

                //First pass - calculate length
                int iLen    = 0;
                int iInside = 0;

                str = ptr;

                while(*ptr && !pEOL)
                {
                    if(!iInside)
                    {
                        if(*ptr == '\'' || *ptr == '"')
                        {
                            iDelim = *ptr;
                            iInside = 1;
                            ptr++;
                            continue;
                        }

                        if(ptr[0] == ',' &&
                           (ptr[1] == '\r' || ptr[1] == '\n'))
                        {
                            //Include next line too
                            ptr++;

                            while(__issp(*ptr))
                                ptr++;

                            continue;
                        }

                        if(*ptr == '\n')
                        {
                            pEOL = ptr;
                            break;
                        }

                        ptr++;
                        continue;
                    }

                    if(*ptr == iDelim)
                    {
                        iInside = 0;
                        ptr++;
                        continue;
                    }

                    iLen++;

                    //Inside string

                    if(*ptr == '\\')
                        ptr++;

                    ptr++;
                }

                if(!pEOL)
                    pEOL = ptr;

                //Second pass - copy string

                char* pOut = new char[iLen + 1];

                ptr = str;
                str = pOut;

                iInside = 0;

                while(ptr != pEOL)
                {
                    if(!iInside)
                    {
                        if(*ptr == '\'' || *ptr == '"')
                        {
                            iDelim = *ptr;
                            iInside = 1;
                            ptr++;
                            continue;
                        }

                        ptr++;
                        continue;
                    }

                    if(*ptr == iDelim)
                    {
                        iInside = 0;
                        ptr++;
                        continue;
                    }

                    //Inside string
                    if(*ptr == '\\')
                    {
                        ptr++;

                        char chr = *ptr;

                        if(chr == 'b') chr = '\b';
                        if(chr == 't') chr = '\t';
                        if(chr == 'n') chr = '\n';
                        if(chr == 'r') chr = '\r';

                        if(*ptr == 'x' || *ptr == 'X')
                        {
                            ptr++;
                            int iValid = 0;
                            int iValue = 0;
                            int iCnt = 2;

                            while(__ishd(*ptr) && iCnt--)
                            {
                                iValue <<= 4;

                                if(__to_upper(*ptr) > '9')
                                    iValue |= __to_upper(*ptr) - 'A' + 0x0A;
                                else
                                    iValue |= *ptr - '0';

                                ptr++;

                                iValid = 1;
                            }

                            if(iValid)
                            {
                                chr = iValue;
                                ptr--;
                            }
                        }

                        *str++ = chr;

                        ptr++;

                        continue;
                    }

                    *str++ = *ptr++;
                }
                *str = 0;

                //*(char **)(pProf->value) = pOut;

                get_bindery().setVar(ln.str, pOut);
                delete pOut;
            }
            while(0);

			continue;
		}

		if(j == (sizeof("library") - 1) && !__cnstrcmp(ptr, "library",j))
		{
            ptr += j;

        	while(__issp(*ptr))
                ptr++;

            if(*ptr != ':' && *ptr != '=')
            {
                ptr = skip_to_eol(ptr);
                continue;
            }

            ptr++;

            while(__issp(*ptr))
                ptr++;

            if(*ptr != '{')
            {
                ptr = skip_to_eol(ptr);
                continue;
            }

            ptr++;

            str = ptr;

            while(*ptr)
            {
                while(*ptr && *ptr != '}')
                    ptr++;

                if(!*ptr)
                    break;

                char* tmp = ptr + 1;

                while(__issp(*tmp) && *tmp != '\n') //skip trailing spaces
                    tmp++;

                if(*ptr == '}' && *tmp == '\n')   //at the end of line!
                    break;

                ptr++;
            }

            if(*ptr)
            {
                *ptr = 0;
                ptr++;
            }

            get_bindery().setVar(ln.str, str);

			continue;
		}

        if(__to_upper(ptr[0]) == 'K' &&
           __to_upper(ptr[1]) == 'B' &&
           j < KEY_NAME_LEN)
        {
            //Process keydef
            str = ptr;

            ptr += j;

        	while(__issp(*ptr))
                ptr++;

            if(*ptr != ':' && *ptr != '=')
            {
                ptr = skip_to_eol(ptr);
                continue;
            }

            ptr++;

            while(__issp(*ptr))
                ptr++;

            if(*ptr == '{') // REXX macro
            {
                while(*ptr)
                {
                    while(*ptr && *ptr != '}')
                        ptr++;

                    if(!*ptr)
                        break;

                    char* tmp = ptr + 1;

                    while(__issp(*tmp) && *tmp != '\n') //skip trailing spaces
                        tmp++;

                    if(*ptr == '}' && *tmp == '\n')   //at the end of line!
                        break;

                    ptr++;
                }
            }
            else            //Ordinary keydef
            {
               while(*ptr)
               {
                    if(ptr[0] == ',' &&
                       (ptr[1] == '\r' || ptr[1] == '\n'))
                    {
                        //Include next line too
                        ptr++;

                        while(__issp(*ptr))
                            ptr++;

                        continue;
                    }

                    if(*ptr == '\n')
                        break;

                	ptr++;
                }
            }

            if(*ptr)
            {
                *ptr = 0;
                ptr++;
            }

            keys.InsKey(str, j);
            continue;
        }

        //Ignore unrecognized line
        ptr = skip_to_eol(ptr);
    }
}

