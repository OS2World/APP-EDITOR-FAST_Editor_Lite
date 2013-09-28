/*
** Module   :CVT.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  02/04/2004	Created
**
*/

#include <string.h>

#include <cvt.h>
#include <_ctype.h>
#include <version.h>

//-----------------------------------------
// String
//-----------------------------------------

int cstrlen(char *str)
{
    int rc = 0;
    if(str)
    {
        while(*str)
        {
            if(*str != SWITCH_CHAR)
                rc++;
            str++;
        }
    }
    return rc;
}

char *str_dup(char* str, int len)
{
    if(len < 0)
        len = (str) ? strlen(str):0;

    char *ptr = new char[len+1];

    if(len)
    {
        if(str)
            memcpy(ptr, str, len);
        else
            memset(ptr, ' ', len);
    }

    ptr[len] = 0;

    return ptr;
}

char* merge2str(char* str1, char* str2, char* between)
{
	int sz = strlen(str1) + strlen(str2) + 1;

	if(between)
		sz += strlen(between);

	char* new_var = new char[sz];

	strcpy(new_var, str1);

	if(between)
		strcat(new_var, between);

	strcat(new_var, str2);

	return new_var;
}

//-----------------------------------------
// Number <-> string conversion
//-----------------------------------------

char* u2s(unsigned int ulID, int minlen, char* numbuff)
{
	if(!numbuff)
		return 0;

    char *buff = numbuff;
    char *str;
    int  slen = 0;

    str = buff;

    do
    {
        *str++ = (char)((ulID % 10) + '0');
        ulID /= 10;
        slen++;
    }
    while(ulID);

    while(slen < minlen)
    {
        *str++ = '0';
        slen++;
    }

    *str-- = 0;

    for(; str > buff; str--, buff++)
    {
        *buff ^= *str;
        *str  ^= *buff;
        *buff ^= *str;
    }
    return numbuff;
}

char* i2s(int i, int minlen, char* numbuff)
{
	char* p = numbuff;

	if(!numbuff)
		return 0;

	if(i < 0)
	{
		*numbuff++ = '-';

		if(minlen > 0)
			minlen--;

		i = -i;
	}

	u2s(i, minlen, numbuff);

	return p;
}

char* c2x(int chr, char* numbuff)
{
    char nibble;

    if(!numbuff)
    	return 0;

    nibble = (char)(chr >> 4);

    numbuff[0] = (char)((nibble > 9) ? (nibble + 'A' - 10) : (nibble + '0'));

    nibble = (char)(chr & 0x0F);

    numbuff[1] = (char)((nibble > 9) ? (nibble + 'A' - 10) : (nibble + '0'));

    numbuff[2] = 0;

    return numbuff;
}

unsigned s2u(char *str)
{
    unsigned ulRes = 0;

    if(!str)
        return ulRes;

	while(*str == ' ')
		str++;

	if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		str += 2;

		while(__ishd(*str))
		{
			ulRes <<= 4;

			if(__isdd(*str))
				ulRes |= *str - '0';
			else
				ulRes |= __to_upper(*str) - 'A' + 0x0A;
			str++;
		}
	}
	else
	{
		while(__isdd(*str))
		{
			ulRes *= 10;
			ulRes += *str - '0';
			str++;
		}
	}
    return ulRes;
}

int s2i(char *str)
{
	int sign = 0;

	if(!str)
		return 0;

	while(*str == ' ' || *str == '+' || *str == '-')
	{
		if(*str == '-')
			sign = 1 - sign;

		str++;
	}

	unsigned u = s2u(str);

	return (sign) ? -u:u;
}

