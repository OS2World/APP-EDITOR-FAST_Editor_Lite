/*
** Module   :CVT.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  02/04/2004	Created
**
*/

#ifndef __CVT_H
#define __CVT_H

#define SWITCH_CHAR     '~'

//-----------------------------------------
// String
//-----------------------------------------

int cstrlen(char *str);
char* str_dup(char*, int = -1);
char* merge2str(char* str1, char* str2, char* between = 0);

//-----------------------------------------
// Number <-> string conversion
//-----------------------------------------

char* u2s(unsigned int val, int minlen, char* buff);
char* i2s(int val, int minlen, char* buff);
char* c2x(int, char* buff);

unsigned s2u(char *str);
int s2i(char *str);

#endif  /*__CVT_H*/

