/*
** Module   :BOXCOLL.H
** Abstract :Edit box collection
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  16/06/1997   	Created
**      Thu  26/03/1998   	Interface updated, so all
**                        	improvements will be simpler to implement
*/

#include <editbox.h>
#include <keycoll.h>
#include <pipe.h>
#include <history.h>
#include <bindery.h>

#ifndef  __BOXCOLL_H
#define  __BOXCOLL_H

//-----------------------------------------
// Helper classes and defs
//-----------------------------------------

class Initor
{
    public:
        Initor();
};

class EditBoxCollection;
typedef EditBoxCollection& RBoxColl;

typedef int (* PEditProc)(RBoxColl r, char*);

struct DispatchEntry
{
    char* key;
	PEditProc proc;
};
typedef DispatchEntry* PDEntry;

class INIList: public SortedCollection
{
	public:
		INIList()				{ bDuplicates = 0;}
        virtual ~INIList()		{ RemoveAll(); }

        virtual int Compare(Ptr p1, Ptr p2);
};

//-----------------------------------------
// Main editor class
//-----------------------------------------

class EditBoxCollection:public Collection
{
        EditBox* GetBox(int box)    { return (EditBox*)Get(box); }
        int cur_box;
        int shutdown;
        int recording;
        int last_box;
        unsigned long hMtx;

        int tTID;
        int mTID;
        int pTID;

        struct macro_rec
        {
            keydef_pair* macro;
            char chr;
            macro_rec* next;

            macro_rec(keydef_pair* m, char c, macro_rec* p):macro(m),chr(c),next(p){}
            ~macro_rec()	{ delete macro;}
        };

        macro_rec* head;
        macro_rec* tail;

        void track_recording(keydef_pair* prog, char chr);
        void start_recording();
        keydef_pair* track_recording_done();

        Initor init;
        KeyDefCollection keys;
        Bindery vars;
        char* cName;
        INIList ini_files;
        Ptr save_screen;

		void show_startup_screen();

		void remove_profile();
		void apply_profile();

    public:

        static History hSearch;
        static History hReplace;
        static History hFlags;
        static History hFile;

        NamedPipe npFED;
        unsigned short MouHandle;

        EditBoxCollection();
        virtual ~EditBoxCollection();

        virtual void Free(Ptr p);

        void Init();

        void lock();
        void unlock();

        EditBox* current();
        EditBox* next();
        EditBox* prev();
        EditBox* locate(int);
        EditBox* select(int);
        EditBox* select_last();
        void select(EditBox*);

        int  get_box_count() 	{ return Count();}
        int  get_cur_box()	 	{ return cur_box;}
        void set_cur_box(int i)	{ select(i); 	 }

		int get_status_pos();
		void set_status_pos(int pos);
		int get_tab_width();
		void set_tab_width(int width);

        Bindery& get_bindery()  		{ return vars;}
        KeyDefCollection& get_keys()	{ return keys;}

        INIList& get_ini_list()			{ return ini_files;}

        EditBox* open();
        int opened(char *fname);
        void close();

        int check_save();
        void recalc_sz();

        void draw();
        void search_proc();
        void load_profile(int);
        void load_profile(char *);
        void load_profile_file(char*);
        void play_macro(char *macro);
        void set_xy(char *ptr);
        void track_beg();
        void track_end();
        void track_cancel();

        void usual_key(KeyInfo&);
        void Dispatcher(KeyInfo&, int iMode = 0);
        void CompileKey(char* str, int j);

        int  isDown() { return shutdown;}

        void Done();
        void SendKey(const char *key);
        void SetMouse(int row, int col);

        void LoadHistory(void);
        void StoreHistory(void);

        PDEntry GetFunction(char* name);

//------- exported public methods
// !WARNING!
// Methods listed below assumes at least one edit box available
// for processing, i.e. collection is not empty.

		//----
		int doOpenFile(char *);

//-----------------------------------------
// Members of the dispatch table
//-----------------------------------------

        //---- General

        static int doAbort(RBoxColl r, char*);
        static int doClose(RBoxColl r, char*);
        static int doCopyright(RBoxColl r, char*);
        static int doCopyright2(RBoxColl r, char*);
        static int doStartupScreen(RBoxColl r, char*);
        static int doExit(RBoxColl r, char*);
        static int doFileList(RBoxColl r, char*);
        static int doFileStat(RBoxColl r, char*);
        static int doHelpScreen(RBoxColl r, char*);
        static int doJumpCol(RBoxColl r, char*);
        static int doJumpLine(RBoxColl r, char*);
        static int doLoad(RBoxColl r, char*);

        static int doNew(RBoxColl r, char*);
        static int doNextFile(RBoxColl r, char*);
        static int doPrevFile(RBoxColl r, char*);
        static int doLastFile(RBoxColl r, char*);
        static int doSave(RBoxColl r, char*);
        static int doSaveAll(RBoxColl r, char*);
        static int doSaveAs(RBoxColl r, char*);
        static int doSearch(RBoxColl r, char*);
        static int doSearchAgain(RBoxColl r, char*);

        //---- Cursor movement
        static int doDown(RBoxColl r, char*);
        static int doEnd(RBoxColl r, char*);
        static int doFileBegin(RBoxColl r, char*);
        static int doFileEnd(RBoxColl r, char*);
        static int doHome(RBoxColl r, char*);
        static int doLeft(RBoxColl r, char*);
        static int doMatchBracket(RBoxColl r, char*);
        static int doPgDn(RBoxColl r, char*);
        static int doPgUp(RBoxColl r, char*);
        static int doRight(RBoxColl r, char*);
        static int doUp(RBoxColl r, char*);
        static int doWordLeft(RBoxColl r, char*);
        static int doWordRight(RBoxColl r, char*);
        static int doFlashBracket(RBoxColl r, char*);

        //---- Block marking
        static int doDownMark(RBoxColl r, char*);
        static int doEndMark(RBoxColl r, char*);
        static int doFileBeginMark(RBoxColl r, char*);
        static int doFileEndMark(RBoxColl r, char*);
        static int doHomeMark(RBoxColl r, char*);
        static int doLeftMark(RBoxColl r, char*);
        static int doMatchBracketMark(RBoxColl r, char*);
        static int doPgDnMark(RBoxColl r, char*);
        static int doPgUpMark(RBoxColl r, char*);
        static int doRightMark(RBoxColl r, char*);
        static int doUpMark(RBoxColl r, char*);
        static int doWordLeftMark(RBoxColl r, char*);
        static int doWordRightMark(RBoxColl r, char*);
        static int doMarkWord(RBoxColl r, char*);

        //---- Editing commands
        static int doBksp(RBoxColl r, char*);
        static int doDel(RBoxColl r, char*);
        static int doDelLine(RBoxColl r, char*);
        static int doDelToEOL(RBoxColl r, char*);
        static int doDelWordLeft(RBoxColl r, char*);
        static int doDelWordRight(RBoxColl r, char*);
        static int doDupLine(RBoxColl r, char*);
        static int doIndent(RBoxColl r, char*);
        static int doLower(RBoxColl r, char*);
        static int doUnindent(RBoxColl r, char*);
        static int doUpper(RBoxColl r, char*);
        static int doSort(RBoxColl r, char*);

        static int doCopy(RBoxColl r, char*);
        static int doCut(RBoxColl r, char*);
        static int doPaste(RBoxColl r, char*);

        static int doUndo(RBoxColl r, char*);
        static int doTouchAll(RBoxColl r, char*);
        static int doNextCompletion(RBoxColl r, char*);
        static int doSelectCompletion(RBoxColl r, char*);

        //---- State commands
        static int doFlipAutoindent(RBoxColl r, char*);
        static int doFlipCompletion(RBoxColl r, char*);
        static int doFlipBlockMode(RBoxColl r, char*);
        static int doFlipHiliting(RBoxColl r, char*);
        static int doHilitingChoice(RBoxColl r, char*);
        static int doFlipType(RBoxColl r, char*);
        static int doIns(RBoxColl r, char*);
        static int doSetXlat(RBoxColl r, char*);

        //---- Special functions
        static int doInsDate(RBoxColl r, char*);
        static int doInsFileName(RBoxColl r, char*);
        static int doInsFileNameShort(RBoxColl r, char*);

        //---- Macro Recorder commands
        static int doMacroRecStart(RBoxColl r, char*);
        static int doMacroRecEnd(RBoxColl r, char*);

        //---- Rexx macro processor
        static int doRexx(RBoxColl r, char* prog);

        //---- Bookmarks processing
        static int doSetMark(RBoxColl r, char*);
        static int doGotoMark(RBoxColl r, char*);

        //---- Jump lists processing
        static int doJumpList(RBoxColl r, char*);

        //---- 'In place' profile loading
        static int doLoadProfile(RBoxColl r, char*);

        //---- Word wrap

        static int doFlipWordWrap(RBoxColl r, char*);
        static int doFlipWWMerge(RBoxColl r, char*);
        static int doFlipWWLong(RBoxColl r, char*);
};

#endif //__BOXCOLL_H

