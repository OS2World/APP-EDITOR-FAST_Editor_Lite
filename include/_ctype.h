/*
** Module   :_CTYPE.H
** Abstract :NLS support routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  04/05/1997   	Updated
*/

#include <common.h>

#ifndef  __CTYPE_H
#define  __CTYPE_H

#define IS_NO       0x000
#define IS_PU       0x001
#define IS_SP       0x002
#define IS_AL       0x004 /* 'a'-'z', 'A'-'Z', ' '-'ï', '€'-'Ÿ' */
#define IS_DI       0x008 /* '0'-'9' */
#define IS_XD       0x010 /* '0'-'9', 'a'-'f', 'A'-'F' */
#define IS_HD       0x020 /* '0'-'9', 'a'-'f', 'A'-'F', 'x', 'X', 'u','U', 'l', 'L'*/
#define IS_QU       0x040 /* '\'', '\"' */
#define IS_SE       0x080 /* ';' */
#define IS_IS       0x100 /* 'a'-'z', 'A'-'Z', ' '-'ï', '€'-'Ÿ', '#', '_' */
#define IS_IC       0x108 /* 'a'-'z', 'A'-'Z', ' '-'ï', '€'-'Ÿ', '0'-'9', '#', '_' */
#define IS_OP       0x200 /* '%', '&', '*', '+', ',', '-', '.', '<', '>', '?', '{', '|', '}', '~' */
#define IS_RU       0x400
#define IS_NRU      0x800

#define __ispu(c) (__r_ctype[(unsigned char)(c)] & IS_PU)
#define __isis(c) (__r_ctype[(unsigned char)(c)] & IS_IS)
#define __isic(c) (__r_ctype[(unsigned char)(c)] & IS_IC)
#define __issp(c) (__r_ctype[(unsigned char)(c)] & IS_SP)
#define __ishd(c) (__r_ctype[(unsigned char)(c)] & IS_HD)
#define __isdd(c) (__r_ctype[(unsigned char)(c)] & IS_DI)
#define __isru(c) (__r_ctype[(unsigned char)(c)] & IS_RU)
#define __isnru(c) (__r_ctype[(unsigned char)(c)] & IS_NRU)
#define __isan(c) (__r_ctype[(unsigned char)(c)] & (IS_AL | IS_DI))

inline char __to_lower(char chr)
{
    return tolower_cvt_table[(chr)];
}

inline char __to_upper(char chr)
{
    return toupper_cvt_table[(chr)];
}

void __nls_init(void);
int __cstrcmp(char *, char *);
int __cnstrcmp(char *s0, char *s1, int n);
int __nstrcmp(char *, char *, char *);  /* str1, str2, cvt_tbl */
char* __cstrchr(char* s0, int chr);

#endif
