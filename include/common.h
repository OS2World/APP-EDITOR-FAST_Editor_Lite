/*
** Module   :COMMON.H
** Abstract :Common constants and definitions
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  06/04/1997   	Created
*/

#include <cvt.h>
#include <vio.h>

#ifndef  __COMMON_H
#define  __COMMON_H

#define CHUNK_SIZE 	4
#define FED_MAXPATH 270
#define BMK_NUM    	10

#ifdef TRACELOG
#define DD_TRACE(a,rc) printf("info:%s at %s (%d), rc = %d\n",(a),__FILE__,__LINE__,(rc));
#else
#define DD_TRACE(a,rc)
#endif

//Colors, in order

#define CL_APPLICATION_START    (0                        )
#define CL_DIALOG_START         (CL_APPLICATION_START + 2 )
#define CL_STEXT_START          (CL_DIALOG_START      + 2 )
#define CL_EDITBOX_START        (CL_STEXT_START       + 2 )
#define CL_EDITLINE_ACTIVE      (CL_EDITBOX_START     + 20)
#define CL_EDITLINE_INACTIVE    (CL_EDITLINE_ACTIVE   + 20)
#define CL_LISTBOX_ACTIVE       (CL_EDITLINE_INACTIVE + 20)
#define CL_LISTBOX_INACTIVE     (CL_LISTBOX_ACTIVE    + 4 )
#define CL_MENU                 (CL_LISTBOX_INACTIVE  + 4 )

#define CL_STATUSLINE   ( 1)
#define CL_HILITE       ( 1)
#define CL_BORDER       ( 1)
#define CL_CURRENT      ( 2)
#define CL_CURRSEL      ( 3)

#define CL_DEFAULT      ( 0)
#define CL_SELECTION    ( 1)
#define CL_EOF          ( 2)
#define CL_COMMENT      ( 3)
#define CL_IDENT        ( 4)
#define CL_CONST        ( 5)
#define CL_PREPROC      ( 6)
#define CL_NUMBER       ( 7)
#define CL_STDWORD      ( 8)
#define CL_SEMICOL      ( 9)
#define CL_FUNCTION     (10)
#define CL_XNUMBER      (11)
#define CL_MATCHING     (12)
#define CL_EOL          (13)
#define CL_CUSTOM_0     (14)
#define CL_CUSTOM_1     (15)
#define CL_CUSTOM_2     (16)
#define CL_CUSTOM_3     (17)
#define CL_CUSTOM_4     (18)
#define CL_CUSTOM_5     (19)

//Hiliting modes

#define HI_CPP      1
#define HI_REXX     2
#define HI_MAKE     3
#define HI_ASM      4
#define HI_HTML     5
#define HI_MAIL     6
#define HI_PAS      7
#define HI_PL       8
#define HI_TEX      9
#define HI_LAST     HI_TEX
#define MASK_CPP    (1 << HI_CPP)
#define MASK_JAVA   (1 << HI_CPP)
#define MASK_REXX   (1 << HI_REXX)
#define MASK_MAKE   (1 << HI_MAKE)
#define MASK_ASM    (1 << HI_ASM)
#define MASK_HTML   (1 << HI_HTML)
#define MASK_MAIL   (1 << HI_MAIL)
#define MASK_PAS    (1 << HI_PAS)
#define MASK_PL     (1 << HI_PL)
#define MASK_TEX    (1 << HI_TEX)

#define MASK_UNINDENT       0x0800000
#define MASK_UNINDENT_C     0x1000000
#define MASK_INDENT         0x2000000
#define MASK_AUTOUNINDENT   0x4000000

#define FUNC_ESCAPE           0x01
#define FUNC_ABORT            0x02
#define FUNC_BKSP             0x03
#define FUNC_CLOSE            0x04
#define FUNC_COPY             0x05
#define FUNC_COPYRIGHT2       0x06
#define FUNC_CUT              0x07
#define FUNC_DEL              0x08
#define FUNC_DELLINE          0x09
#define FUNC_DELTOEOL         0x0A
#define FUNC_DELWORDLEFT      0x0B
#define FUNC_DELWORDRIGHT     0x0C
#define FUNC_DOWN             0x0D
#define FUNC_DOWNMARK         0x0E
#define FUNC_END              0x0F
#define FUNC_ENDMARK          0x10
#define FUNC_EXIT             0x11
#define FUNC_FILEBEGIN        0x12
#define FUNC_FILEBEGINMARK    0x13
#define FUNC_FILEEND          0x14
#define FUNC_FILEENDMARK      0x15
#define FUNC_FILELIST         0x16
#define FUNC_FLIPAUTOINDENT   0x17
#define FUNC_FLIPBLOCKMODE    0x18
#define FUNC_FLIPHILITING     0x19
#define FUNC_HELPSCREEN       0x1A
#define FUNC_HOME             0x1B
#define FUNC_HOMEMARK         0x1C
#define FUNC_INDENT           0x1D
#define FUNC_INS              0x1E
#define FUNC_INSDATE          0x1F
#define FUNC_INSFILENAME      0x20
#define FUNC_INSFILENAMESHORT 0x21
#define FUNC_JUMPCOL          0x22
#define FUNC_JUMPLINE         0x23
#define FUNC_LEFT             0x24
#define FUNC_LEFTMARK         0x25
#define FUNC_LOAD             0x26
#define FUNC_LOWER            0x27
#define FUNC_MACRORECEND      0x28
#define FUNC_MACRORECSTART    0x29
#define FUNC_MATCHBRACKET     0x2A
#define FUNC_MATCHBRACKETMARK 0x2B
#define FUNC_NEW              0x2C
#define FUNC_NEXTFILE         0x2D
#define FUNC_PASTE            0x2E
#define FUNC_PGDN             0x2F
#define FUNC_PGDNMARK         0x30
#define FUNC_PGUP             0x31
#define FUNC_PGUPMARK         0x32
#define FUNC_PREVFILE         0x33
#define FUNC_RIGHT            0x34
#define FUNC_RIGHTMARK        0x35
#define FUNC_SAVE             0x36
#define FUNC_SAVEALL          0x37
#define FUNC_SAVEAS           0x38
#define FUNC_SEARCH           0x39
#define FUNC_SEARCHAGAIN      0x3A
#define FUNC_SORT             0x3B
#define FUNC_UNDO             0x3C
#define FUNC_UNINDENT         0x3D
#define FUNC_UP               0x3E
#define FUNC_UPMARK           0x3F
#define FUNC_UPPER            0x40
#define FUNC_WORDLEFT         0x41
#define FUNC_WORDLEFTMARK     0x42
#define FUNC_WORDRIGHT        0x43
#define FUNC_WORDRIGHTMARK    0x44
#define FUNC_REXX             0x45
#define FUNC_FLIPTYPE         0x46
#define FUNC_HILITE_ACHOICE   0x47
#define FUNC_DUPLICATE_LINE   0x48
#define FUNC_SET_XLAT         0x49

#define FUNC_BMK_PUT_0        0x50
#define FUNC_BMK_PUT_1        0x51
#define FUNC_BMK_PUT_2        0x52
#define FUNC_BMK_PUT_3        0x53
#define FUNC_BMK_PUT_4        0x54
#define FUNC_BMK_PUT_5        0x55
#define FUNC_BMK_PUT_6        0x56
#define FUNC_BMK_PUT_7        0x57
#define FUNC_BMK_PUT_8        0x58
#define FUNC_BMK_PUT_9        0x59

#define FUNC_BMK_GET_0        0x60
#define FUNC_BMK_GET_1        0x61
#define FUNC_BMK_GET_2        0x62
#define FUNC_BMK_GET_3        0x63
#define FUNC_BMK_GET_4        0x64
#define FUNC_BMK_GET_5        0x65
#define FUNC_BMK_GET_6        0x66
#define FUNC_BMK_GET_7        0x67
#define FUNC_BMK_GET_8        0x68
#define FUNC_BMK_GET_9        0x69

#define FUNC_JMP_LST_0        0x70
#define FUNC_JMP_LST_1        0x71
#define FUNC_JMP_LST_2        0x72
#define FUNC_JMP_LST_3        0x73
#define FUNC_JMP_LST_4        0x74
#define FUNC_JMP_LST_5        0x75
#define FUNC_JMP_LST_6        0x76
#define FUNC_JMP_LST_7        0x77
#define FUNC_JMP_LST_8        0x78
#define FUNC_JMP_LST_9        0x79

#define FUNC_LOAD_KEYS        0x80
#define FUNC_FLIPWORDWRAP     0x81
#define FUNC_FLIPWWMERGE      0x82
#define FUNC_FLIPWWLONG       0x83
#define FUNC_TOUCHALL         0x84
#define FUNC_NEXT_COMPL       0x85
#define FUNC_FLIP_COMPL       0x86
#define FUNC_SELECT_COMPL     0x87
#define FUNC_LASTFILE         0x88

#define EOF_MODE_CRLF         0
#define EOF_MODE_EOF          1
#define EOF_MODE_NONE         2

#define P_INT  1
#define P_CHAR 2
#define P_PSZ  3

extern int iTabWidth;
extern int iIndUseTab;
extern int iFollowTabs;
extern int iWWDef;
extern int iDefWidth;
extern int iUpperStatus;
extern int iNoEA;
extern int Rows;
extern int Cols;
extern int BufLen;
extern int __r_ctype[];
extern int iDateFmt;
extern int cDateSep;
extern int iShape[2];
extern int iForce;
extern int iDefType;
extern int iVSearch;
extern int iCtrlBrk;
extern int iFileName;
extern int iSaveSyntax;
extern int iMouseMask;
extern int iSenseShift;
extern int iMouseThrd;
extern int iAutoCompl;
extern int iMinComplLen;
extern int iNoTabsIntoClip;

extern int iUnIndBrackets;
extern int iUnIndKwd;
extern int iAutoUnInd;
extern int iIndKwd;
extern int iIndBracket;
extern int iFlash;
extern int iAfterBlock;
extern int iCurCP;
extern int iDelWS;
extern int iDrawEOL;
extern int iEOF;
extern int iSpacesOnly;
extern int iBlockIns;
extern int iQUndo;

extern char cPipe[];
extern char app_pal[];
extern char Search[];
extern char Replace[];
extern char Flags[];
extern char sbuff[];
extern char toupper_cvt_table[];
extern char tolower_cvt_table[];
extern char collate_cvt_table[];
extern char ProgressBar[];
extern char _cFedPATH[];

extern char* StartupDir;
extern char* untitled;
extern char* Screen;
extern char* AlignedBuffer;
extern char* FileDialogNames[];
extern char* Yes_No[];
extern char* help_text;
extern char* hi_map;
extern char* statusline;
extern char* rexx_pool;
extern char* cStColumn;
extern char* cStStream;
extern char* cStWrap  ;
extern char* cStUnwrap;
extern char* cStMerge ;
extern char* cStDOS   ;
extern char* cStUnix  ;
extern char* cStChange;
extern char* cStNoChg ;
extern char* cStInd   ;
extern char* cStNoInd ;
extern char* cWordDelim;
extern char* pDef     ;
extern char* cStAcOn  ;
extern char* cStAcOff ;

extern KeyInfo kiLastKey;

//-----------------------------------------
// Pair object for Dictionary class
//-----------------------------------------

struct kwdPair
{
    char *key;
    unsigned mask;      //Language mask
    unsigned no_ind;	//Disable indent
    unsigned no_auto;   //Disable autounindent
    unsigned no_un;     //Disable unindent
    unsigned no_un_c;   //Disable conditional unindent
};

typedef kwdPair* Pkwd;

extern kwdPair keywords[];

#endif
