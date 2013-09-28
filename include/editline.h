/*
** Module   :EDITLINE.H
** Abstract :Input (editable) Line control
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  16/06/1997   	Created
*/

#include <dialog.h>

#ifndef  __EDITLINE_H
#define  __EDITLINE_H

//-------------------------------------------------------
// class EditLine

class EditLine:public Control, public Buffer
{
    public:

        EditLine(int Row, int Col, int nRows, int nCols);

        void get_text(char *buff, int max_len);
        void set_text(char *);

        virtual int  select(int i=0);
        virtual void do_key(KeyInfo&);
        virtual void draw();
};

#endif //__EDITLINE_H

