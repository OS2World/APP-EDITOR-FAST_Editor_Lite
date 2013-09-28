/*
** Module   :FED.CPP
** Abstract :Fast Editor Main module
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  11/05/1997   	Created from 'RC.CPP'
**      Sun  09/11/1997   	This is first comment written in FED! Version 0.1.5a
**      Wed  12/11/1997   	Updated to V0.1.6
**      Sun  16/11/1997   	Updated to V0.1.7
**      Tue  18/11/1997   	Updated to V0.1.8
**      Tue  17/03/1998     Updated to V0.1.9l (first fully functional version)
**      Mon  03/08/1998     Updated to V0.2.0n
**      Sat  09/01/1999     Updated to V0.2.0p
**      Thu  03/06/1999     Updated to V0.2.5 (FED Open Source)
*/

/*
TODO:
    Redesign language support

*/

#include <string.h>
#include <stdio.h>

#include <stddlg.h>
#include <keynames.h>
#include <pmproc.h>

#define TRACE_ENABLED 1
#include <version.h>

#include <fio.h>

#define MK_CLR(clr)     (app_pal[CL_APPLICATION_START+(clr)])


EditBoxCollection Editor;
JumpList JList[10];

//---------------------------------------------------

#define RESERVED_SIZE       16000

static char *mem_reserved = 0;

ULONG APIENTRY XHandler(PEXCEPTIONREPORTRECORD,
                        PEXCEPTIONREGISTRATIONRECORD,
                        PCONTEXTRECORD,
                        PVOID);

int main(int argc, char **argv)
{
    EXCEPTIONREGISTRATIONRECORD RegRec = {0};
    APIRET rc = 0;
    int i = 0;
	char* cName = new char[1024];

#if TRACE_ENABLED
    setvbuf(stderr, 0, _IONBF, 0);
#endif

	Editor.Init();

    if(Editor.isDown())
    {
        MessageBox("ERROR:\nCan't open INI file");
        return -1;
    }

    if(iForce)
        init_pm(iForce);

    Editor.LoadHistory();

    Editor.lock();
    EditBoxCollection::doCopyright(Editor, 0);
    Editor.unlock();

#ifndef __FED_DEBUG__
    RegRec.ExceptionHandler = (_ERR*) XHandler;
    rc = DosSetExceptionHandler(&RegRec);

    if (rc)
    {
        MessageBox("ERROR:\nCan't install exception handler");
        return -1;
    }
#endif

    if(argc > 1)
    {
        int rc = 1;

        Editor.lock();

        for(i = 1; i < argc; i++)
        {
            if(argv[i][0]=='-' && __isdd(argv[i][1]))
            {
                Editor.set_xy(&argv[i][1]);
                continue;
            }

            if(argv[i][0]=='-' && __to_lower(argv[i][1]) == 'p')
            {
                i++;

                if(i >= argc)
                    break;

                //Note: opening via pipe have sense only if our pipe is
                //      not open, and therefore other instance uses it
                if(!Editor.npFED.IsValid())
                {
                    char* fname = get_full_name(argv[i]);

                    int handle = _lopen("\\PIPE\\FED", OP_PIPE);

                    _lwrite(handle, (void*)"open ", 5);
                    _lwrite(handle, fname, strlen(fname));
                    _lclose(handle);

                    delete fname;

                    //Make sure that editor will go down after opening files
                    EditBoxCollection::doAbort(Editor, 0);
                    continue;
                }
                //Note: if pipe is open, we just fall through here
                //      and open file as usual
            }

            if(strchr(argv[i], '*') || strchr(argv[i], '?'))
            {
                int frc = FileDialog(2, 2, cName, 0);

                if(!frc)
                    Editor.doOpenFile(cName);

                continue;
            }

            if(!rc)
                Editor.select(Editor.open());

            rc = Editor.current()->load(argv[i]);

            Editor.SendKey("kbOpen");

        }
        Editor.select(0);
        Editor.unlock();
    }

    /* Send pseudo key kbStart */
    Editor.lock();
    Editor.SendKey("kbInit");
    Editor.unlock();

    KeyInfo k;

    while(!Editor.isDown())
    {
        Editor.lock();
        Editor.draw();
        Editor.unlock();
        vio_read_key(&k);
        Editor.lock();

        while(k.rep_count--)
            Editor.Dispatcher(k,1);

        Editor.unlock();
    }

    Editor.lock();
    Editor.SendKey("kbDone");
    Editor.unlock();

#ifndef __FED_DEBUG__

    rc = DosUnsetExceptionHandler(&RegRec);
#endif

    Editor.Done();

	delete cName;
    return 0;
}

ULONG APIENTRY XHandler(PEXCEPTIONREPORTRECORD p1,
                        PEXCEPTIONREGISTRATIONRECORD,
                        PCONTEXTRECORD,
                        PVOID)
{
    int i;
    extern HAB hab;
    extern HMQ hmq;
    static int single = 0;

    //delete mem_reserved;

    if(single)
        return XCPT_CONTINUE_SEARCH;

    if(p1->ExceptionNum == XCPT_SIGNAL)
    {
        if(iCtrlBrk == 1)
        {
            //Silently die
            single++;
            return XCPT_CONTINUE_SEARCH;
        }

        if(iCtrlBrk == 2)
        {
            //Save all and exit
            single++;
            EditBoxCollection::doSaveAll(Editor, 0);

            return XCPT_CONTINUE_SEARCH;
        }

        if(iCtrlBrk == 3)
        {
            //Ignore
            return XCPT_CONTINUE_EXECUTION;
        }

        MessageBox("Ctrl-Break received.\nTerminating.");
    }
    else
    {
        printf("Exception occured at: %08x\r\n", p1->ExceptionAddress);

        MessageBox("  WARNING!!!\n"
                   "FED will be terminated \n"
                   "due to unrecoverable error!\n"
                   "All opened files will be saved\n"
                   "with names DEADFED.xxx\n"
                   "Where xxx is number starting from 000.");

    }
    for(i = 0; i < Editor.Count(); i++)
    {
        static char savename[16];
        strcpy(savename,"DEADFED.");
        u2s(i, 3, savename + 8);
        Editor.current()->set_changed(1);
        Editor.current()->save_as(savename);
        Editor.next();
    }

    if(hab)
    {
        _inCloseClipbrd(hab);
        deinit_pm();
    }

    single = 1;
    return XCPT_CONTINUE_SEARCH;
}

