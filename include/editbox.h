/*
** Module   :EDITBOX.H
** Abstract :EditBox control representation class
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  03/05/1997   	Created
*/

#include <buffer.h>

#ifndef  __EDITBOX_H
#define  __EDITBOX_H

//-------------------------------------------------------
// class EditBox

class EditBox: public Buffer
{
        char *name;
        char *orig_file;
        int num;
        static short used;
        char* cName;

    public:

        EditBox(int, int, int, int);
        virtual ~EditBox();

        int load(char* name);
        int save_as(char* name);
        int new_file();
        int save();
        int close();
        void draw();
        int number()                { return num;}
        void set_name(char *name);
        char *get_name()            { return name;}
        char *get_fmt_name(int, int = ' ');
        int check_save();
        int is_untitled();
        void put_xy();
};

#endif //__EDITBOX_H
