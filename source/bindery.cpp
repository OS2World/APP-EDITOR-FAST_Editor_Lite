/*
** Module   :BINDERY.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Fri  02/04/2004	Created
**
*/

#include <string.h>
#include <bindery.h>
#include <_ctype.h>
#include <boxcoll.h>
#include <fio.h>
#include <version.h>

#include <stdio.h>

extern EditBoxCollection Editor;

//-----------------------------------------
// Variable
//-----------------------------------------

Variable::Variable(char* aName)
{
	pName = str_dup(aName);
	pValue = str_dup(aName);
}

Variable::~Variable()
{
	delete pName;
	delete pValue;
}

int Variable::setValue(char* val)
{
	delete pValue;
	pValue = str_dup(val);
	return 0;
}

void Variable::fromInt(int i)
{
	char cBuff[16];

	Variable::setValue(i2s(i, 0, cBuff));
}

int Variable::asInt()
{
	return s2i(pValue);
}

//-----------------------------------------
// Global variables
//-----------------------------------------

typedef int FnGlobal(Ptr, char* );
typedef FnGlobal* PFnGlobal;

class VarGlobal: public Variable
{
		Ptr 	  pData;
		PFnGlobal pFn;
	public:

		VarGlobal(char* aName, PFnGlobal aFn, Ptr aData):
				Variable(aName), pFn(aFn), pData(aData) {}

		virtual int setValue(char* aVal);
		virtual int isPermanent()	{ return 1;}
};

int VarGlobal::setValue(char* aVal)
{
	int rc = pFn(pData, aVal);

	return (rc) ? rc:Variable::setValue(aVal);
}

static int FnInt(Ptr aData, char* pValue)
{
	int* pData = (int*)aData;
	*pData = s2i(pValue);

	return 0;
}

//Replace on new assigniment

static int FnStr(Ptr aData, char* pValue)
{
	char** pData = (char**)aData;

	if(*pData)
		delete *pData;

	*pData = str_dup(pValue);

	return 0;
}

//Construct new value from existing value and new one

static int FnStr2(Ptr aData, char* pValue)
{
	char** pData = (char**)aData;

	if(!*pData)
		*pData = str_dup(pValue);
	else
	{
		char* new_var = merge2str(*pData, pValue, "\r\n");
		delete *pData;
		*pData = new_var;
	}
	return 0;
}

static int FnClr(Ptr aData, char* pValue)
{
	char* pData = (char*)aData;

	int iValue = 0;
	int iValid = 0;

	char* ptr = pValue;

	if(ptr[0] == '0' && __to_upper(ptr[1]) == 'X')
	{
		ptr += 2;

		while(__ishd(*ptr))
		{
			iValue <<= 4;

			if(__to_upper(*ptr) > '9')
				iValue |= __to_upper(*ptr) - 'A' + 0x0A;
			else
				iValue |= *ptr - '0';

			ptr++;

			iValid = 1;
		}
	}
	else
	{
		while(__isdd(*ptr))
		{
			iValue *= 10;
			iValue += *ptr - '0';

			ptr++;

			iValid = 1;
		}
	}

	if(!iValid)
		return 1;

	*pData =  iValue;

	return 0;
}

//-----------------------------------------
// Global variables init data
//-----------------------------------------

static struct
{
	char* pName;
	PFnGlobal pFn;
	Ptr pData;
	char* pInit;
}
globals[]=
{
    //CL_APPLICATION_START
    {"color.app.default"             , FnClr, app_pal + CL_APPLICATION_START+CL_DEFAULT   ,"0x07"},
    {"color.app.status"              , FnClr, app_pal + CL_APPLICATION_START+CL_STATUSLINE,"0x70"},

    //CL_DIALOG_START
    {"color.dialog.default"          , FnClr, app_pal + CL_DIALOG_START+CL_DEFAULT        ,"0x70"},
    {"color.dialog.hilite"           , FnClr, app_pal + CL_DIALOG_START+CL_BORDER         ,"0x70"},

    //CL_STEXT_START
    {"color.static.default"          , FnClr, app_pal + CL_STEXT_START+CL_DEFAULT         ,"0x70"},
    {"color.static.hilite"           , FnClr, app_pal + CL_STEXT_START+CL_HILITE          ,"0x7C"},

    //CL_EDITBOX_START
    {"color.edit.default"            , FnClr, app_pal + CL_EDITBOX_START+CL_DEFAULT       ,"0x08"},
    {"color.edit.selection"          , FnClr, app_pal + CL_EDITBOX_START+CL_SELECTION     ,"0x70"},
    {"color.edit.eof"                , FnClr, app_pal + CL_EDITBOX_START+CL_EOF           ,"0x1B"},
    {"color.edit.comment"            , FnClr, app_pal + CL_EDITBOX_START+CL_COMMENT       ,"0x06"},
    {"color.edit.ident"              , FnClr, app_pal + CL_EDITBOX_START+CL_IDENT         ,"0x07"},
    {"color.edit.const"              , FnClr, app_pal + CL_EDITBOX_START+CL_CONST         ,"0x0E"},
    {"color.edit.preproc"            , FnClr, app_pal + CL_EDITBOX_START+CL_PREPROC       ,"0x0A"},
    {"color.edit.number"             , FnClr, app_pal + CL_EDITBOX_START+CL_NUMBER        ,"0x0C"},
    {"color.edit.stdword"            , FnClr, app_pal + CL_EDITBOX_START+CL_STDWORD       ,"0x0F"},
    {"color.edit.semicol"            , FnClr, app_pal + CL_EDITBOX_START+CL_SEMICOL       ,"0x09"},
    {"color.edit.function"           , FnClr, app_pal + CL_EDITBOX_START+CL_FUNCTION      ,"0x0F"},
    {"color.edit.xnumber"            , FnClr, app_pal + CL_EDITBOX_START+CL_XNUMBER       ,"0x0D"},
    {"color.edit.matching"           , FnClr, app_pal + CL_EDITBOX_START+CL_MATCHING      ,"0x3F"},
    {"color.edit.eol"                , FnClr, app_pal + CL_EDITBOX_START+CL_EOL           ,"0x03"},
    {"color.edit.custom.0"           , FnClr, app_pal + CL_EDITBOX_START+CL_CUSTOM_0      ,"0x0F"},
    {"color.edit.custom.1"           , FnClr, app_pal + CL_EDITBOX_START+CL_CUSTOM_1      ,"0x0F"},
    {"color.edit.custom.2"           , FnClr, app_pal + CL_EDITBOX_START+CL_CUSTOM_2      ,"0x0F"},
    {"color.edit.custom.3"           , FnClr, app_pal + CL_EDITBOX_START+CL_CUSTOM_3      ,"0x0F"},
    {"color.edit.custom.4"           , FnClr, app_pal + CL_EDITBOX_START+CL_CUSTOM_4      ,"0x0F"},
    {"color.edit.custom.5"           , FnClr, app_pal + CL_EDITBOX_START+CL_CUSTOM_5      ,"0x0F"},

    //CL_EDITLINE_ACTIVE
    {"color.line.active.default"     , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_DEFAULT     ,"0x0F"},
    {"color.line.active.selection"   , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_SELECTION   ,"0x30"},
    {"color.line.active.eof"         , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_EOF         ,"0x07"},
    {"color.line.active.comment"     , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_COMMENT     ,"0x07"},
    {"color.line.active.ident"       , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_IDENT       ,"0x07"},
    {"color.line.active.const"       , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CONST       ,"0x07"},
    {"color.line.active.preproc"     , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_PREPROC     ,"0x07"},
    {"color.line.active.number"      , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_NUMBER      ,"0x07"},
    {"color.line.active.stdword"     , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_STDWORD     ,"0x07"},
    {"color.line.active.semicol"     , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_SEMICOL     ,"0x07"},
    {"color.line.active.function"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_FUNCTION    ,"0x07"},
    {"color.line.active.xnumber"     , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_XNUMBER     ,"0x07"},
    {"color.line.active.matching"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_MATCHING    ,"0x0C"},
    {"color.line.active.eol"         , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_EOL         ,"0x03"},
    {"color.line.active.custom.0"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CUSTOM_0    ,"0x0C"},
    {"color.line.active.custom.1"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CUSTOM_1    ,"0x0C"},
    {"color.line.active.custom.2"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CUSTOM_2    ,"0x0C"},
    {"color.line.active.custom.3"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CUSTOM_3    ,"0x0C"},
    {"color.line.active.custom.4"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CUSTOM_4    ,"0x0C"},
    {"color.line.active.custom.5"    , FnClr, app_pal + CL_EDITLINE_ACTIVE+CL_CUSTOM_5    ,"0x0C"},

    //CL_EDITLINE_INACTIVE
    {"color.line.inactive.default"   , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_DEFAULT   ,"0x1B"},
    {"color.line.inactive.selection" , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_SELECTION ,"0x3B"},
    {"color.line.inactive.eof"       , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_EOF       ,"0x07"},
    {"color.line.inactive.comment"   , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_COMMENT   ,"0x07"},
    {"color.line.inactive.ident"     , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_IDENT     ,"0x07"},
    {"color.line.inactive.const"     , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CONST     ,"0x07"},
    {"color.line.inactive.preproc"   , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_PREPROC   ,"0x07"},
    {"color.line.inactive.number"    , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_NUMBER    ,"0x07"},
    {"color.line.inactive.stdword"   , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_STDWORD   ,"0x07"},
    {"color.line.inactive.semicol"   , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_SEMICOL   ,"0x07"},
    {"color.line.inactive.function"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_FUNCTION  ,"0x07"},
    {"color.line.inactive.xnumber"   , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_XNUMBER   ,"0x07"},
    {"color.line.inactive.matching"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_MATCHING  ,"0x0C"},
    {"color.line.inactive.eol"       , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_EOL       ,"0x13"},
    {"color.line.inactive.custom.0"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CUSTOM_0  ,"0x0C"},
    {"color.line.inactive.custom.1"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CUSTOM_1  ,"0x0C"},
    {"color.line.inactive.custom.2"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CUSTOM_2  ,"0x0C"},
    {"color.line.inactive.custom.3"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CUSTOM_3  ,"0x0C"},
    {"color.line.inactive.custom.4"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CUSTOM_4  ,"0x0C"},
    {"color.line.inactive.custom.5"  , FnClr, app_pal + CL_EDITLINE_INACTIVE+CL_CUSTOM_5  ,"0x0C"},

    //CL_LISTBOX_START_ACTIVE
    {"color.list.active.default"     , FnClr, app_pal + CL_LISTBOX_ACTIVE+CL_DEFAULT      ,"0x30"},
    {"color.list.active.selection"   , FnClr, app_pal + CL_LISTBOX_ACTIVE+CL_SELECTION    ,"0x3B"},
    {"color.list.active.current"     , FnClr, app_pal + CL_LISTBOX_ACTIVE+CL_CURRENT      ,"0x07"},
    {"color.list.active.currsel"     , FnClr, app_pal + CL_LISTBOX_ACTIVE+CL_CURRSEL      ,"0x0B"},

    //CL_LISTBOX_START_INACTIVE
    {"color.list.inactive.default"   , FnClr, app_pal + CL_LISTBOX_INACTIVE+CL_DEFAULT    ,"0x30"},
    {"color.list.inactive.selection" , FnClr, app_pal + CL_LISTBOX_INACTIVE+CL_SELECTION  ,"0x3B"},
    {"color.list.inactive.current"   , FnClr, app_pal + CL_LISTBOX_INACTIVE+CL_CURRENT    ,"0x1B"},
    {"color.list.inactive.currsel"   , FnClr, app_pal + CL_LISTBOX_INACTIVE+CL_CURRSEL    ,"0x1B"},

    //CL_MENU
    {"color.menu.active.default"     , FnClr, app_pal + CL_MENU+CL_DEFAULT                ,"0x70"},
    {"color.menu.active.selection"   , FnClr, app_pal + CL_MENU+CL_SELECTION              ,"0x7C"},
    {"color.menu.active.current"     , FnClr, app_pal + CL_MENU+CL_CURRENT                ,"0x07"},
    {"color.menu.active.currsel"     , FnClr, app_pal + CL_MENU+CL_CURRSEL                ,"0x0C"},

    {"cursor.after.block"            , FnInt , &iAfterBlock    ,"0"},
    {"cursor.follow_tabs"            , FnInt , &iFollowTabs    ,"0"},
    {"cursor.shape.insert"           , FnInt , iShape + 0      ,"90"},
    {"cursor.shape.overwrite"        , FnInt , iShape + 1      ,"0"},

    {"mouse.event.mask"              , FnInt , &iMouseMask     ,"0x06"},
    {"mouse.sense.shift"             , FnInt , &iSenseShift    ,"1"},
    {"mouse.thread"                  , FnInt , &iMouseThrd     ,"1"},

    {"editor.autounindent"           , FnInt , &iAutoUnInd     ,"1"},
    {"editor.block.ins_mode"         , FnInt , &iBlockIns      ,"0"},
    {"editor.clipboard.spaces_only"  , FnInt , &iNoTabsIntoClip,"1"},
    {"editor.completion.enable"      , FnInt , &iAutoCompl     ,"0"},
    {"editor.completion.minlen"      , FnInt , &iMinComplLen   ,"5"},
    {"editor.ctrlbreak.action"       , FnInt , &iCtrlBrk       ,"0"},
    {"editor.default.format"         , FnInt , &iDefType       ,"0"},
    {"editor.default.wordwrap.margin", FnInt , &iDefWidth      ,"78"},
    {"editor.default.wordwrap.state" , FnInt , &iWWDef         ,"0x02"},	//WW_MERGE
    {"editor.draw.eol"               , FnInt , &iDrawEOL       ,"1"},
    {"editor.ea.disable"             , FnInt , &iNoEA	       ,"0"},
    {"editor.eof"                    , FnInt , &iEOF           ,"0"},		//EOF_MODE_CRLF
    {"editor.file.name.reduce"       , FnInt , &iFileName      ,"1"},
    {"editor.flash.bracket"          , FnInt , &iFlash         ,"1"},
    {"editor.helptext"               , FnStr , &help_text      ,"No Help available"},
    {"editor.indent.bracket"         , FnInt , &iIndBracket    ,"1"},
    {"editor.indent.keywords"        , FnInt , &iIndKwd        ,"1"},
    {"editor.indent.use_tab"         , FnInt , &iIndUseTab     ,"1"},
    {"editor.search.flags.default"   , FnStr , &pDef           ,""},
    {"editor.startup.directory"      , FnStr , &StartupDir     , get_full_name(".")},
    {"editor.statusline"             , FnStr , &statusline     ,"L%3r:C%2c %h [%u%f]"},
    {"editor.statusline.block.column", FnStr , &cStColumn      ,"C"},
    {"editor.statusline.block.string", FnStr , &cStStream      ,"S"},
    {"editor.statusline.changed"     , FnStr , &cStChange      ,"*"},
    {"editor.statusline.compl.off"   , FnStr , &cStAcOff       ," "},
    {"editor.statusline.compl.on"    , FnStr , &cStAcOn        ,"A"},
    {"editor.statusline.file.dos"    , FnStr , &cStDOS         ,"D"},
    {"editor.statusline.file.unix"   , FnStr , &cStUnix        ,"U"},
    {"editor.statusline.indent.off"  , FnStr , &cStNoInd       ," "},
    {"editor.statusline.indent.on"   , FnStr , &cStInd         ,"I"},
    {"editor.statusline.unchanged"   , FnStr , &cStNoChg       ," "},
    {"editor.statusline.wrap.merge"  , FnStr , &cStMerge       ,"M"},
    {"editor.statusline.wrap.off"    , FnStr , &cStUnwrap      ," "},
    {"editor.statusline.wrap.on"     , FnStr , &cStWrap        ,"W"},
    {"editor.syntax"                 , FnStr , &hi_map         ,"" },
    {"editor.syntax.save.mode"       , FnInt , &iSaveSyntax    ,"1"},
    {"editor.undo.compress.moves"    , FnInt , &iQUndo         ,"0"},
    {"editor.unindent.brackets"      , FnInt , &iUnIndBrackets ,"2"},
    {"editor.unindent.keywords"      , FnInt , &iUnIndKwd      ,"1"},
    {"editor.unindent.spaces.only"   , FnInt , &iSpacesOnly    ,"0"},
    {"editor.untitled"               , FnStr , &untitled       ,".Untitled"},
    {"editor.verbose.search"         , FnInt , &iVSearch       ,"0"},
    {"editor.word.delete_ws"         , FnInt , &iDelWS         ,"0"},
    {"editor.word.delimiter"         , FnStr , &cWordDelim},
    {"library"                       , FnStr2, &rexx_pool      ,"\r\n/* separate library from main code */\r\nexit\r\n"},
    {0}
};

//-----------------------------------------
// Editor integer variables/values
//-----------------------------------------

typedef void (EditBox::* PFnSet)(int);
typedef int  (EditBox::* PFnGet)(void);

class VarEditorInt: public Variable
{
		PFnGet pFnGet;
		PFnSet pFnSet;
	public:

		VarEditorInt(char* aName, PFnGet aGet, PFnSet aSet);

		virtual char* getValue();
		virtual int setValue(char* p);
		virtual int isPermanent()	{ return 1;}
};

//-----------------------------------------

VarEditorInt::VarEditorInt(char* aName, PFnGet aGet, PFnSet aSet):
			Variable(aName), pFnGet(aGet), pFnSet(aSet)
{
}

char* VarEditorInt::getValue()
{
	fromInt((Editor.current()->*pFnGet)());
	return Variable::getValue();
}

int VarEditorInt::setValue(char* p)
{
	if(!pFnSet)
		return 1;

	Variable::setValue(p);
	(Editor.current()->*pFnSet)(asInt());

	return ((Editor.current()->*pFnGet)() == asInt()) ? 0:1;
}

//-----------------------------------------
// Editor integer variables init data
//-----------------------------------------

static struct
{
	char* pName;
	PFnGet pFnGet;
	PFnSet pFnSet;
}
editor_ints[]=
{
    {"editor.current.completion"     , &Buffer::get_auto_completion	, &Buffer::set_auto_completion},
    {"editor.current.autoindent"     , &Buffer::get_auto_indent		, &Buffer::set_auto_indent    },
    {"editor.current.column.block"   , &Buffer::get_column_block	, &Buffer::set_column_block   },
    {"editor.current.syntax.user"    , &Buffer::get_custom_syntax	, &Buffer::set_custom_syntax  },
    {"editor.current.ins.mode"       , &Buffer::get_ins_mode		, &Buffer::set_ins_mode       },
    {"editor.current.mark.state"     , &Buffer::get_mark_state		, &Buffer::set_mark_state     },
    {"editor.current.saved"          , &Buffer::get_saved			, &Buffer::set_saved          },
    {"editor.current.file.mode"      , &Buffer::get_unix			, &Buffer::set_unix           },
    {"editor.current.syntax"         , &Buffer::get_hiliting		, &Buffer::set_hiliting       },
    {"editor.current.ww.state"       , &Buffer::ww_get_state		, &Buffer::ww_set_state       },
    {"editor.current.ww.merge"       , &Buffer::ww_get_merge		, &Buffer::ww_set_merge       },
    {"editor.current.ww.paragraph"   , &Buffer::ww_get_paragraph    , &Buffer::ww_set_paragraph   },
    {"editor.current.ww.width"       , &Buffer::ww_get_width		, &Buffer::ww_set_width       },
    {"editor.current.row"            , &Buffer::get_edit_row		, &Buffer::set_edit_row       },
    {"editor.current.col"            , &Buffer::get_edit_col		, &Buffer::set_edit_col       },
    {"editor.current.start.row"      , &Buffer::get_start_row		, 0},
    {"editor.current.start.col"      , &Buffer::get_start_col		, 0},
    {"editor.current.cursor.row"     , &Buffer::get_cur_row			, 0},
    {"editor.current.cursor.col"     , &Buffer::get_cur_col			, 0},
    {"editor.current.marking.row"    , &Buffer::get_edit_row_2		, 0},
    {"editor.current.marking.col"    , &Buffer::get_edit_col_2		, 0},
    {"editor.current.undo.count"     , &Buffer::get_undo_count		, 0},
    {"editor.current.memory.undo"    , &Buffer::undo_size			, 0},
    {"editor.current.memory"  	     , &Buffer::memory				, 0},
    {"editor.current.relative.pos.y" , &Buffer::get_rel_pos			, 0},
    {"editor.current.lines"			 , &Buffer::get_line_count		, 0},
    {"editor.current.window.rows"	 , &Buffer::get_rows      		, 0},
    {"editor.current.window.cols"	 , &Buffer::get_cols      		, 0},
    {0}
};

//-----------------------------------------
// Editor (collection) integer variables
//-----------------------------------------

//-----------------------------------------
// Editor integer variables/values
//-----------------------------------------

typedef void (EditBoxCollection::* PCFnSet)(int);
typedef int  (EditBoxCollection::* PCFnGet)(void);

class VarEditorCInt: public Variable
{
		PCFnGet pFnGet;
		PCFnSet pFnSet;
	public:

		VarEditorCInt(char* aName, PCFnGet aGet, PCFnSet aSet);

		virtual char* getValue();
		virtual int setValue(char* p);
		virtual int isPermanent()	{ return 1;}
};

//-----------------------------------------

VarEditorCInt::VarEditorCInt(char* aName, PCFnGet aGet, PCFnSet aSet):
			Variable(aName), pFnGet(aGet), pFnSet(aSet)
{
}

char* VarEditorCInt::getValue()
{
	fromInt((Editor.*pFnGet)());
	return Variable::getValue();
}

int VarEditorCInt::setValue(char* p)
{
	if(!pFnSet)
		return 1;

	Variable::setValue(p);
	(Editor.*pFnSet)(asInt());

	return ((Editor.*pFnGet)() == asInt()) ? 0:1;
}

//-----------------------------------------
// Editor integer variables init data
//-----------------------------------------

static struct
{
	char* pName;
	PCFnGet pFnGet;
	PCFnSet pFnSet;
	char* pInit;
}
editor_coll_ints[]=
{
    {"editor.box.count"      , &EditBoxCollection::get_box_count , 0},
    {"editor.box.current"    , &EditBoxCollection::get_cur_box   , &EditBoxCollection::set_cur_box},
    {"editor.status.position", &EditBoxCollection::get_status_pos, &EditBoxCollection::set_status_pos, "0"},
    {"editor.tab.width"      , &EditBoxCollection::get_tab_width , &EditBoxCollection::set_tab_width , "4"},
    {0}
};

//-----------------------------------------
// Editor string variable/settings
//-----------------------------------------

typedef void  (EditBox::* PFnSetStr)(char*);
typedef char* (EditBox::* PFnGetStr)(void);

class VarEditorStr: public Variable
{
		PFnGetStr pFnGet;
		PFnSetStr pFnSet;
	public:

		VarEditorStr(char* aName, PFnGetStr aGet, PFnSetStr aSet);

		virtual char* getValue();
		virtual int setValue(char* p);
		virtual int isPermanent()	{ return 1;}
};

//-----------------------------------------

VarEditorStr::VarEditorStr(char* aName, PFnGetStr aGet, PFnSetStr aSet):
			Variable(aName), pFnGet(aGet), pFnSet(aSet)
{
}

char* VarEditorStr::getValue()
{
	Variable::setValue((Editor.current()->*pFnGet)());
	return Variable::getValue();
}

int VarEditorStr::setValue(char* p)
{
	if(!pFnSet)
		return 1;

	(Editor.current()->*pFnSet)(p);
	return Variable::setValue(p);
}

//-----------------------------------------
// Editor string variables init data
//-----------------------------------------

static struct
{
	char* pName;
	PFnGetStr pFnGet;
	PFnSetStr pFnSet;
}
editor_strings[]=
{
    {"editor.current.codepage"	, &Buffer::get_cur_cp, &Buffer::set_xlate},
    {"editor.current.file.name"	, &EditBox::get_name , &EditBox::set_name},
    {0}
};

//-----------------------------------------
// Editor bookmarks
//-----------------------------------------

class VarEditorBmk: public Variable
{
	public:

		VarEditorBmk(char* aName):Variable(aName) {}

		virtual char* getValue(char* key);

		virtual int isPermanent()	{ return 1;}
		virtual int isGroup()		{ return 1;}
};

//-----------------------------------------

char* VarEditorBmk::getValue(char* key)
{
	int r = 0;
	int c = 0;

	int ord = 0;

	char* ptr = key;

	while(__isdd(*ptr))
	{
		ord *= 10;
		ord += *ptr - '0';
		ptr++;
	}

	if(*ptr != '.')
		return 0;

	int mode;

	if(!strcmp(ptr, ".row"))
		mode = 1;
	else
		if(!strcmp(ptr, ".col"))
			mode = 0;
		else
			return 0;

	char cBuff[32];

	Editor.current()->bmk_get(ord, r, c);

    u2s((mode) ? r:c, 0, cBuff);

	Variable::setValue(cBuff);

	return Variable::getValue();
}

//-----------------------------------------
// Editor key bindings
//-----------------------------------------

class VarEditorKeys: public Variable
{
		KeyDefCollection& Keys;
		int iPerm;

	public:

		VarEditorKeys(char* aName, KeyDefCollection& keys, int perm):
					Variable(aName), Keys(keys), iPerm(perm) {}

		virtual char* getValue(char* key);
		virtual int setValue(char*, char*);

		virtual int isPermanent()	{ return iPerm;}
		virtual int isGroup()		{ return 1;}
};

//-----------------------------------------

char* VarEditorKeys::getValue(char* key)
{
	int ord = -1;

	char* ptr = key;

	if(__isdd(*ptr))
	{
		ord = 0;

		while(__isdd(*ptr))
		{
			ord *= 10;
			ord += *ptr - '0';
			ptr++;
		}

		int mode = 0;

		if(!strcmp(ptr, ".key"))
			mode = 1;
		else
			if(!strcmp(ptr, ".macro"))
				mode = 2;

		if(!mode && ord)	//invalid request like .1, .2
			return 0;

		if(!ord)	//Return number of keys
		{
			char cBuff[16];

			i2s(Keys.Count(), 0, cBuff);

			Variable::setValue(cBuff);
			return Variable::getValue();
		}
		else
		{
    		keydef_pair* p = Keys.GetDef(ord - 1);

	    	if(!p)
    			return 0;

	    	if(mode == 1)
				Variable::setValue(p->key);
	    	else
	    	{
	    		char* prog = Keys.GetDefText(p->prog);

	    		if(prog)
					Variable::setValue(prog);

				delete prog;
			}
		}
	}
	else
	{
		//Get key by name
   		keydef_pair* p = Keys.GetDef(key);

		if(!p)
			return 0;

		//Variable::setValue(p->prog);
		char* prog = Keys.GetDefText(p->prog);

   		if(prog)
			Variable::setValue(prog);

		delete prog;
	}

	return Variable::getValue();
}

int VarEditorKeys::setValue(char* key, char* val)
{
	int len  = strlen(key);
	int vlen = strlen(val);

	if(len <= 3 || len >= KEY_NAME_LEN ||
		__to_upper(key[0]) != 'K' || __to_upper(key[1]) != 'B')
	{
		return 1;
	}

	if(vlen)	//Remove assignment
		Keys.RemoveDef(key);
	else
	{
		char* pBuff = new char[vlen + len + 16];

		strcpy(pBuff, key);
		strcat(pBuff, " = ");
		strcat(pBuff, val);

		Keys.InsKey(pBuff, len);

		delete pBuff;
	}

	return 0;
}

//-----------------------------------------
// Bindery
//-----------------------------------------

Bindery::Bindery()
{
    bDuplicates = 0;
}

void Bindery::LoadDefaults()
{
    int i;

	//Fill registry with predefined entries
	//Global variables

	for(i = 0; globals[i].pName; i++)
	{
		PVariable pVar = new VarGlobal(globals[i].pName, globals[i].pFn, globals[i].pData);

		if(globals[i].pInit)
			pVar->setValue(globals[i].pInit);

		Add(pVar);
	}

	//Editor ints variables

	for(i = 0; editor_ints[i].pName; i++)
		Add(new VarEditorInt(editor_ints[i].pName, editor_ints[i].pFnGet, editor_ints[i].pFnSet));

	//Editor collection ints

	for(i = 0; editor_coll_ints[i].pName; i++)
	{
		PVariable pVar = new VarEditorCInt(editor_coll_ints[i].pName, editor_coll_ints[i].pFnGet, editor_coll_ints[i].pFnSet);

		if(editor_coll_ints[i].pInit)
			pVar->setValue(editor_coll_ints[i].pInit);

		Add(pVar);
	}

	//Editor strings variables

	for(i = 0; editor_strings[i].pName; i++)
		Add(new VarEditorStr(editor_strings[i].pName, editor_strings[i].pFnGet, editor_strings[i].pFnSet));

	//Editor bookmarks

	Add(new VarEditorBmk("editor.current.bookmark"));
	Add(new VarEditorKeys("editor.keys", Editor.get_keys(), 1));
}

Bindery::~Bindery()
{
	RemoveAll();
}

int Bindery::Compare(Ptr p1, Ptr p2)
{
	return stricmp(PVariable(p1)->getName(), PVariable(p2)->getName());
}

PVariable Bindery::LocateVar(char* name, int iCreate)
{
    Variable i = name;

    unsigned dPos = Look(&i);

    if(dPos < Count() && !Compare(&i, Get(dPos)))
        return PVariable(Get(dPos));

	if(dPos < Count() && dPos > 0 && PVariable(Get(dPos - 1))->isGroup())
	{
		PVariable pVar = PVariable(Get(dPos - 1));
		int grpLen = strlen(pVar->getName());

		if(strlen(name) > grpLen && name[grpLen] == '.' &&
		   !memicmp(pVar->getName(), name, grpLen))
		{
			//Found group variable
			return pVar;
		}
	}

    if(iCreate)
    {
        PVariable p = new Variable(name);
        Add(p);
        return p;
    }

    return 0;
}

void Bindery::delVar(char* name)
{
    Variable i = name;

    unsigned dPos = Look(&i);

    if(dPos < Count() && !Compare(&i, Get(dPos)))
    {
    	if(!PVariable(Get(dPos))->isPermanent())
        	Free(Remove(dPos));
    }
}

void Bindery::dropVar(char* namestart)
{
    unsigned len = strlen(namestart);
    unsigned dPos;

    {
        Variable i = namestart;
        dPos = Look(&i);
    }

    while(dPos < Count() && strlen(PVariable(Get(dPos))->getName()) >= len &&
    	  (PVariable(Get(dPos))->getName()[len] == '.' ||
    	   PVariable(Get(dPos))->getName()[len] == '\0') &&
    	  !memicmp(namestart, PVariable(Get(dPos))->getName(), len))
    {
    	if(!PVariable(Get(dPos))->isPermanent())
        	Free(Remove(dPos));
    }
}

void Bindery::setVar(char* name, char* value)
{
    PVariable pItem = LocateVar(name, 1);

    if(pItem)
    {
    	if(pItem->isGroup())
    	{
    		char* key = name;

    		key += strlen(pItem->getName()) + 1;

    		pItem->setValue(key, value);
    	}
		else
	        pItem->setValue(value);
    }
}

char* Bindery::getVar(char* name)
{
    PVariable pItem = LocateVar(name, 0);

    if(pItem)
    {
    	if(pItem->isGroup())
    	{
    		char* key = name;

    		key += strlen(pItem->getName()) + 1;

    		return pItem->getValue(key);
    	}

        return pItem->getValue();
    }
    return 0;
}

void Bindery::setDefaultProfile()
{
	//Remove key bindings
	dropVar("editor.current.keys");
}

void Bindery::setCurrentProfile()
{
	//Add key bindings
	//Add(new VarEditorKeys("editor.keys", Editor.cur_profile()->get_keys(), 0));
}

