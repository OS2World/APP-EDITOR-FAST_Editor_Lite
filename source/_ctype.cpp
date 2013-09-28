/*
** Module   :_CTYPE.CPP
** Abstract : NLS support routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  03/05/1997   	Updated
*/

#define INCL_DOSNLS
#include <os2.h>

#include <_ctype.h>
#include <version.h>

void __nls_init(void)
{
    COUNTRYCODE Country;
    COUNTRYINFO CtryInfo = {0};
    ULONG ulLen;
    unsigned i;
    APIRET rc;

    for(i = 0; i < 256; i++)
    {
        toupper_cvt_table[i] = (char)i;
        tolower_cvt_table[i] = (char)i;
        collate_cvt_table[i] = (char)i;
    }

    for(i = 'a'; i <= 'z'; i++)
    {
        toupper_cvt_table[i] = (char)(i - 'a' + 'A');
    }

    for(i = 'A'; i <= 'Z'; i++)
    {
        tolower_cvt_table[i] = (char)(i - 'A' + 'a');
    }

    Country.country  = 0;
    Country.codepage = 0;

    rc = DosMapCase(256,
                    &Country,
                    toupper_cvt_table);

    DD_TRACE("DosMapCase",rc);

    if(rc)
        return;

    rc = DosQueryCollate(256,
                         &Country,
                         collate_cvt_table,
                         &ulLen);

    DD_TRACE("DosQueryCollate",rc);

    rc = DosQueryCtryInfo(sizeof(CtryInfo), &Country,
                          &CtryInfo, &ulLen);

    DD_TRACE("DosQueryCtryInfo",rc);

    if(!rc)
    {
        iCurCP   = CtryInfo.codepage;
        iDateFmt = CtryInfo.fsDateFmt;
      //  cDateSep = CtryInfo.szDateSeparator[0];
    }
    for(i = 128; i < 256; i++)
    {
        if((unsigned char)toupper_cvt_table[i] > (unsigned char)0x7F)
        	tolower_cvt_table[(unsigned char)toupper_cvt_table[i]] = (char)i;
    }
    for(i = 128; i < 256; i++)
    {
        __r_ctype[i] |= (tolower_cvt_table[i] != toupper_cvt_table[i]) ?
                         (IS_AL | IS_IS) : (IS_PU);
    }
}

int __nstrcmp(char *s0, char *s1, char *cvt_tbl)
{
    if(!s0 || !s1)
        return (s0) ? 1:-1;

    while(*s0 || *s1)
    {
        if(collate_cvt_table[cvt_tbl[*s0]] == collate_cvt_table[cvt_tbl[*s1]])
        {
            s0++;
            s1++;
            continue;
        }

        if(collate_cvt_table[cvt_tbl[*s0]] > collate_cvt_table[cvt_tbl[*s1]])
            return 1;
        if(collate_cvt_table[cvt_tbl[*s0]] < collate_cvt_table[cvt_tbl[*s1]])
            return -1;
    }
    return 0;
}

int __cstrcmp(char *s0, char *s1)
{
    if(!s0 || !s1)
        return (s0) ? 1:-1;
    while(*s0 || *s1)
    {
        if(__to_lower(*s0) == __to_lower(*s1))
        {
            *s0++;
            *s1++;
            continue;
        }

        if(__to_lower(*s0) > __to_lower(*s1))
            return 1;
        if(__to_lower(*s0) < __to_lower(*s1))
            return -1;
    }
    return 0;
}

int __cnstrcmp(char *s0, char *s1, int n)
{
    if(!s0 || !s1 || n <= 0)
        return (s0) ? 1:-1;

    while(n--)
    {
        if(__to_lower(*s0) == __to_lower(*s1))
        {
            *s0++;
            *s1++;
            continue;
        }

        if(__to_lower(*s0) > __to_lower(*s1))
            return 1;
        if(__to_lower(*s0) < __to_lower(*s1))
            return -1;
    }
    return 0;
}

char* __cstrchr(char* s0, int chr)
{
    if(!s0)
        return 0;

    chr = __to_lower(chr);

    while(*s0)
    {
        if(__to_lower(*s0) == chr)
            return s0;

        s0++;
    }
    return 0;
}

#ifdef __GNUC__
extern "C" void __rtti_user(void);
void __rtti_user(void) {}
extern "C" void __rtti_si(void);
void __rtti_si(void) {}
extern "C" void __rtti_class(void);
void __rtti_class(void) {}
extern "C" void __eh_pc(void);
void __eh_pc(void){}
//extern "C" void terminate(void)
void terminate(void) {}
#endif

//-----------------------------------------
// Global data
//-----------------------------------------

char toupper_cvt_table[256];
char tolower_cvt_table[256];
char collate_cvt_table[256];

int __r_ctype[]=
{
     0, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP,
 IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP,
 IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP,
 IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP, IS_SP,
 IS_SP                          /*' ' - 0x20*/,
 IS_PU                          /*'!' - 0x21*/,
 IS_PU | IS_QU                  /*'"' - 0x22*/,
 IS_PU | IS_IS                  /*'#' - 0x23*/,
 IS_PU                          /*'$' - 0x24*/,
 IS_PU                          /*'%' - 0x25*/,
 IS_PU                          /*'&' - 0x26*/,
 IS_PU | IS_QU                  /*''' - 0x27*/,
 IS_PU                          /*'(' - 0x28*/,
 IS_PU                          /*')' - 0x29*/,
 IS_PU                          /*'*' - 0x2A*/,
 IS_PU                          /*'+' - 0x2B*/,
 IS_PU                          /*',' - 0x2C*/,
 IS_PU                          /*'-' - 0x2D*/,
 IS_PU                          /*'.' - 0x2E*/,
 IS_PU                          /*'/' - 0x2F*/,
 IS_DI | IS_XD | IS_HD          /*'0' - 0x30*/,
 IS_DI | IS_XD | IS_HD          /*'1' - 0x31*/,
 IS_DI | IS_XD | IS_HD          /*'2' - 0x32*/,
 IS_DI | IS_XD | IS_HD          /*'3' - 0x33*/,
 IS_DI | IS_XD | IS_HD          /*'4' - 0x34*/,
 IS_DI | IS_XD | IS_HD          /*'5' - 0x35*/,
 IS_DI | IS_XD | IS_HD          /*'6' - 0x36*/,
 IS_DI | IS_XD | IS_HD          /*'7' - 0x37*/,
 IS_DI | IS_XD | IS_HD          /*'8' - 0x38*/,
 IS_DI | IS_XD | IS_HD          /*'9' - 0x39*/,
 IS_PU                          /*':' - 0x3A*/,
 IS_PU | IS_SE                  /*';' - 0x3B*/,
 IS_PU                          /*'<' - 0x3C*/,
 IS_PU                          /*'=' - 0x3D*/,
 IS_PU                          /*'>' - 0x3E*/,
 IS_PU                          /*'?' - 0x3F*/,
 IS_PU                          /*'@' - 0x40*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'A' - 0x41*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'B' - 0x42*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'C' - 0x43*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'D' - 0x44*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'E' - 0x45*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'F' - 0x46*/,
 IS_AL                 | IS_IS  /*'G' - 0x47*/,
 IS_AL                 | IS_IS  /*'H' - 0x48*/,
 IS_AL                 | IS_IS  /*'I' - 0x49*/,
 IS_AL                 | IS_IS  /*'J' - 0x4A*/,
 IS_AL                 | IS_IS  /*'K' - 0x4B*/,
 IS_AL                 | IS_IS  /*'L' - 0x4C*/,
 IS_AL                 | IS_IS  /*'M' - 0x4D*/,
 IS_AL                 | IS_IS  /*'N' - 0x4E*/,
 IS_AL                 | IS_IS  /*'O' - 0x4F*/,
 IS_AL                 | IS_IS  /*'P' - 0x50*/,
 IS_AL                 | IS_IS  /*'Q' - 0x51*/,
 IS_AL                 | IS_IS  /*'R' - 0x52*/,
 IS_AL                 | IS_IS  /*'S' - 0x53*/,
 IS_AL                 | IS_IS  /*'T' - 0x54*/,
 IS_AL                 | IS_IS  /*'U' - 0x55*/,
 IS_AL                 | IS_IS  /*'V' - 0x56*/,
 IS_AL                 | IS_IS  /*'W' - 0x57*/,
 IS_AL                 | IS_IS  /*'X' - 0x58*/,
 IS_AL                 | IS_IS  /*'Y' - 0x59*/,
 IS_AL                 | IS_IS  /*'Z' - 0x5A*/,
 IS_PU                          /*'[' - 0x5B*/,
 IS_PU                          /*'\' - 0x5C*/,
 IS_PU                          /*']' - 0x5D*/,
 IS_PU                          /*'^' - 0x5E*/,
 IS_PU                 | IS_IS  /*'_' - 0x5F*/,
 IS_PU                          /*'`' - 0x60*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'a' - 0x61*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'b' - 0x62*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'c' - 0x63*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'d' - 0x64*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'e' - 0x65*/,
 IS_AL | IS_XD | IS_HD | IS_IS  /*'f' - 0x66*/,
 IS_AL                 | IS_IS  /*'g' - 0x67*/,
 IS_AL                 | IS_IS  /*'h' - 0x68*/,
 IS_AL                 | IS_IS  /*'i' - 0x69*/,
 IS_AL                 | IS_IS  /*'j' - 0x6A*/,
 IS_AL                 | IS_IS  /*'k' - 0x6B*/,
 IS_AL                 | IS_IS  /*'l' - 0x6C*/,
 IS_AL                 | IS_IS  /*'m' - 0x6D*/,
 IS_AL                 | IS_IS  /*'n' - 0x6E*/,
 IS_AL                 | IS_IS  /*'o' - 0x6F*/,
 IS_AL                 | IS_IS  /*'p' - 0x70*/,
 IS_AL                 | IS_IS  /*'q' - 0x71*/,
 IS_AL                 | IS_IS  /*'r' - 0x72*/,
 IS_AL                 | IS_IS  /*'s' - 0x73*/,
 IS_AL                 | IS_IS  /*'t' - 0x74*/,
 IS_AL                 | IS_IS  /*'u' - 0x75*/,
 IS_AL                 | IS_IS  /*'v' - 0x76*/,
 IS_AL                 | IS_IS  /*'w' - 0x77*/,
 IS_AL                 | IS_IS  /*'x' - 0x78*/,
 IS_AL                 | IS_IS  /*'y' - 0x79*/,
 IS_AL                 | IS_IS  /*'z' - 0x7A*/,
 IS_PU                          /*'{' - 0x7B*/,
 IS_PU                          /*'|' - 0x7C*/,
 IS_PU                          /*'}' - 0x7D*/,
 IS_PU                          /*'~' - 0x7E*/,
 IS_SP | IS_PU                  /*' ' - 0x7F*/,
 0                              /*' ' - 0x80*/,
 0                              /*' ' - 0x81*/,
 0                              /*' ' - 0x82*/,
 0                              /*' ' - 0x83*/,
 0                              /*' ' - 0x84*/,
 0                              /*' ' - 0x85*/,
 0                              /*' ' - 0x86*/,
 0                              /*' ' - 0x87*/,
 0                              /*' ' - 0x88*/,
 0                              /*' ' - 0x89*/,
 0                              /*' ' - 0x8A*/,
 0                              /*' ' - 0x8B*/,
 0                              /*' ' - 0x8C*/,
 0                              /*' ' - 0x8D*/,
 0                              /*' ' - 0x8E*/,
 0                              /*' ' - 0x8F*/,
 0                              /*' ' - 0x90*/,
 0                              /*' ' - 0x91*/,
 0                              /*' ' - 0x92*/,
 0                              /*' ' - 0x93*/,
 0                              /*' ' - 0x94*/,
 0                              /*' ' - 0x95*/,
 0                              /*' ' - 0x96*/,
 0                              /*' ' - 0x97*/,
 0                              /*' ' - 0x98*/,
 0                              /*' ' - 0x99*/,
 0                              /*' ' - 0x9A*/,
 0                              /*' ' - 0x9B*/,
 0                              /*' ' - 0x9C*/,
 0                              /*' ' - 0x9D*/,
 0                              /*' ' - 0x9E*/,
 0                              /*' ' - 0x9F*/,
 IS_RU                          /*' ' - 0xA0*/,
 IS_RU                          /*' ' - 0xA1*/,
 IS_RU                          /*' ' - 0xA2*/,
 IS_RU                          /*' ' - 0xA3*/,
 IS_RU                          /*' ' - 0xA4*/,
 IS_RU                          /*' ' - 0xA5*/,
 IS_RU                          /*' ' - 0xA6*/,
 IS_RU                          /*' ' - 0xA7*/,
 IS_RU                          /*' ' - 0xA8*/,
 IS_RU                          /*' ' - 0xA9*/,
 IS_RU                          /*' ' - 0xAA*/,
 IS_RU                          /*' ' - 0xAB*/,
 IS_RU                          /*' ' - 0xAC*/,
 IS_RU                          /*' ' - 0xAD*/,
 IS_RU                          /*' ' - 0xAE*/,
 IS_RU                          /*' ' - 0xAF*/,
 IS_NRU                         /*' ' - 0xB0*/,
 IS_NRU                         /*' ' - 0xB1*/,
 IS_NRU                         /*' ' - 0xB2*/,
 IS_NRU                         /*' ' - 0xB3*/,
 IS_NRU                         /*' ' - 0xB4*/,
 IS_NRU                         /*' ' - 0xB5*/,
 IS_NRU                         /*' ' - 0xB6*/,
 IS_NRU                         /*' ' - 0xB7*/,
 IS_NRU                         /*' ' - 0xB8*/,
 IS_NRU                         /*' ' - 0xB9*/,
 IS_NRU                         /*' ' - 0xBA*/,
 IS_NRU                         /*' ' - 0xBB*/,
 IS_NRU                         /*' ' - 0xBC*/,
 IS_NRU                         /*' ' - 0xBD*/,
 IS_NRU                         /*' ' - 0xBE*/,
 IS_NRU                         /*' ' - 0xBF*/,
 IS_NRU                         /*' ' - 0xC0*/,
 IS_NRU                         /*' ' - 0xC1*/,
 IS_NRU                         /*' ' - 0xC2*/,
 IS_NRU                         /*' ' - 0xC3*/,
 IS_NRU                         /*' ' - 0xC4*/,
 IS_NRU                         /*' ' - 0xC5*/,
 IS_NRU                         /*' ' - 0xC6*/,
 IS_NRU                         /*' ' - 0xC7*/,
 IS_NRU                         /*' ' - 0xC8*/,
 IS_NRU                         /*' ' - 0xC9*/,
 IS_NRU                         /*' ' - 0xCA*/,
 IS_NRU                         /*' ' - 0xCB*/,
 IS_NRU                         /*' ' - 0xCC*/,
 IS_NRU                         /*' ' - 0xCD*/,
 IS_NRU                         /*' ' - 0xCE*/,
 IS_NRU                         /*' ' - 0xCF*/,
 IS_NRU                         /*' ' - 0xD0*/,
 IS_NRU                         /*' ' - 0xD1*/,
 IS_NRU                         /*' ' - 0xD2*/,
 IS_NRU                         /*' ' - 0xD3*/,
 IS_NRU                         /*' ' - 0xD4*/,
 IS_NRU                         /*' ' - 0xD5*/,
 IS_NRU                         /*' ' - 0xD6*/,
 IS_NRU                         /*' ' - 0xD7*/,
 IS_NRU                         /*' ' - 0xD8*/,
 IS_NRU                         /*' ' - 0xD9*/,
 IS_NRU                         /*' ' - 0xDA*/,
 IS_NRU                         /*' ' - 0xDB*/,
 IS_NRU                         /*' ' - 0xDC*/,
 IS_NRU                         /*' ' - 0xDD*/,
 IS_NRU                         /*' ' - 0xDE*/,
 IS_NRU                         /*' ' - 0xDF*/,
 IS_RU                          /*' ' - 0xE0*/,
 IS_RU                          /*' ' - 0xE1*/,
 IS_RU                          /*' ' - 0xE2*/,
 IS_RU                          /*' ' - 0xE3*/,
 IS_RU                          /*' ' - 0xE4*/,
 IS_RU                          /*' ' - 0xE5*/,
 IS_RU                          /*' ' - 0xE6*/,
 IS_RU                          /*' ' - 0xE7*/,
 IS_RU                          /*' ' - 0xE8*/,
 IS_RU                          /*' ' - 0xE9*/,
 IS_RU                          /*' ' - 0xEA*/,
 IS_RU                          /*' ' - 0xEB*/,
 IS_RU                          /*' ' - 0xEC*/,
 IS_RU                          /*' ' - 0xED*/,
 IS_RU                          /*' ' - 0xEE*/,
 IS_RU                          /*' ' - 0xEF*/,
 IS_RU                          /*' ' - 0xF0*/,
 IS_RU                          /*' ' - 0xF1*/,
 IS_RU                          /*' ' - 0xF2*/,
 IS_RU                          /*' ' - 0xF3*/,
 IS_RU                          /*' ' - 0xF4*/,
 IS_RU                          /*' ' - 0xF5*/,
 IS_RU                          /*' ' - 0xF6*/,
 IS_RU                          /*' ' - 0xF7*/,
 0                              /*' ' - 0xF8*/,
 0                              /*' ' - 0xF9*/,
 0                              /*' ' - 0xFA*/,
 0                              /*' ' - 0xFB*/,
 0                              /*' ' - 0xFC*/,
 0                              /*' ' - 0xFD*/,
 0                              /*' ' - 0xFE*/,
 0                              /*' ' - 0xFF*/,
 0
};

