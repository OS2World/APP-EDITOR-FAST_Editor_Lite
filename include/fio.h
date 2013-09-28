/*
** Module   :FIO.H
** Abstract :Low level file I/O routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  11/04/1997   	Created
*/

#include <common.h>

#ifndef  __FIO_H
#define  __FIO_H

#define OP_READ     0
#define OP_WRITE    1
#define OP_PIPE     2

int _lopen(char *name, int mode);
int _lread(int handle, void * buff, int len);
int _lwrite(int handle, void * buff, int len);
int _lclose(int handle);
int _lsize(int handle);
int _file_exists(char *name);
int curr_date_str(char *date);
void make_short_name(char *name, char *buff);
char* _ld_file(char *name);
void _fr_file(char *data);
int put_ea(char *FileName, char *ea_name, char *ea_value);
int get_ea(char *FileName, char *ea_name, char **content);
char* parse_pos(char *ptr, int *x, int *y);
char* mk_pos(char *ptr, int x, int y);
char *get_full_name(char *fname);
int match_name(char *name, char *mask);

class BlockWrite
{
        char *buff;
        int handle;
        int used_len;
        int len;

        void flush();

    public:

        BlockWrite(int size);
        ~BlockWrite();

        void Add(char *data, int len);

        void Open(int handle);
        void Close();
};

#endif
