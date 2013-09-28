/*
** Module   :EDITCFG.H
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  23/08/2002 Created
**
*/

#ifndef __EDITCFG_H
#define __EDITCFG_H

class EditConfig
{
    public:

        EditConfig();
        ~EditConfig();

        char* GetInitCP();  //Get initial code page
        int GetTabSz();     //Get TAB size
        int GetInTabSz();   //Get TAB size in loaded file

        Parser* GetParser();

        char* GetWordChars();
};


#endif  /*__EDITCFG_H*/

