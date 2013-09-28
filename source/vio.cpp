/*
** Module   :VIO.C
** Abstract :Screen and keyboard I/O routines
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  29/01/1997   	Created
**      Sun  09/11/1997   	Some minor changes and updates
*/

#include <string.h>
#include <stdio.h>

#include <vio.h>
//#include <fio.h>
#include <cvt.h>
#include <_ctype.h>
#include <keynames.h>
#include <version.h>

#include <pmproc.h>

//-----------------------------------------
// local defs
//-----------------------------------------

#define ScreenOffset(Row,Col) (((Row) * Cols + (Col)) << 1)

#define ScreenPtr(Row,Col) \
            ((((Row) * Cols + (Col)) << 1) < BufLen ? \
            (&Screen[(((Row) * Cols + (Col)) << 1)]): \
            0)


//-----------------------------------------
// Local data structures
//-----------------------------------------

struct stKeyMap
{
    unsigned char code;
    char *name;
};

//-----------------------------------------
// Pointer to PM API's
//-----------------------------------------

BOOL (APIENTRY *_inCloseClipbrd)(HAB);
HMQ  (APIENTRY *_inCreateMsgQueue)(HAB, LONG);
BOOL (APIENTRY *_inDestroyMsgQueue)(HMQ);
BOOL (APIENTRY *_inEmptyClipbrd)(HAB);
HAB  (APIENTRY *_inInitialize)(ULONG);
BOOL (APIENTRY *_inOpenClipbrd)(HAB);
HWND (APIENTRY *_inQueryActiveWindow)(HWND);
ULONG(APIENTRY *_inQueryClipbrdData)(HAB, ULONG);
BOOL (APIENTRY *_inQueryClipbrdFmtInfo)(HAB, ULONG, PULONG);
BOOL (APIENTRY *_inSetClipbrdData)(HAB, ULONG, ULONG, ULONG);
BOOL (APIENTRY *_inTerminate)(HAB);

HSWITCH (APIENTRY *_inQuerySwitchHandle)(HWND, PID);
ULONG (APIENTRY *_inQuerySwitchEntry)(HSWITCH, PSWCNTRL);
ULONG (APIENTRY *_inChangeSwitchEntry)(HSWITCH, PSWCNTRL);

LONG (APIENTRY *_inQueryWindowText)(HWND, LONG, PCH);
BOOL (APIENTRY *_inSetWindowText)(HWND, PCSZ);


HAB hab = 0;
HMQ hmq = 0;
HWND hwndFrame = 0;
char cOldTitle[257] = "";
HSWITCH hSw = 0;
SWCNTRL swOldData = {0};

extern stKeyMap AltKey[];

static int VioInitComplete = 0;
static VIOCURSORINFO CUR_SAVE;
static ULONG ulType = 0;
static PPIB pib = 0;
static PTIB tib = 0;
static USHORT save_row = 0;
static USHORT save_col = 0;

char* Screen        = 0;
char* AlignedBuffer = 0;
int Rows            = 25;
int Cols            = 80;
int BufLen;

static void importKey(KeyInfo*, USHORT, USHORT);

KeyInfo kiLastKey;

int init_pm(int)
{
    APIRET rc;
    HMODULE hMte = 0;
    char loadErr[256];

    if(hab || hmq)
        return 0;

    rc = DosGetInfoBlocks(&tib, &pib);
    DD_TRACE("DosGetInfoBlocks",rc);

    rc = DosQueryModuleName(pib->pib_hmte, FED_MAXPATH, _cFedPATH);
    DD_TRACE("DosQueryModuleName",rc);

//    rc = DosQueryModuleHandle("PMWIN", &hMte);

//    mutex   \SEM32\PMDRAG.SEM

    rc = 0;

    if(pib->pib_ultype < 2) //If this is an full screen session
    {
        HMTX hMtx = 0;

        rc = DosOpenMutexSem((PCH)"\\SEM32\\PMDRAG.SEM", &hMtx);

        if(!rc)
            DosCloseMutexSem(hMtx);
    }

    if(!rc)
    {
        ulType = pib->pib_ultype;
        pib->pib_ultype = 3;

        rc = DosLoadModule((PCH)loadErr, 256, (PCH)"PMWIN", &hMte);

        if(rc)
            return 1;

        rc = DosQueryProcAddr(hMte, 707, 0, (PFN*)&_inCloseClipbrd);
        rc = DosQueryProcAddr(hMte, 716, 0, (PFN*)&_inCreateMsgQueue);
        rc = DosQueryProcAddr(hMte, 726, 0, (PFN*)&_inDestroyMsgQueue);
        rc = DosQueryProcAddr(hMte, 733, 0, (PFN*)&_inEmptyClipbrd);
        rc = DosQueryProcAddr(hMte, 763, 0, (PFN*)&_inInitialize);
        rc = DosQueryProcAddr(hMte, 793, 0, (PFN*)&_inOpenClipbrd);
        rc = DosQueryProcAddr(hMte, 799, 0, (PFN*)&_inQueryActiveWindow);
        rc = DosQueryProcAddr(hMte, 806, 0, (PFN*)&_inQueryClipbrdData);
        rc = DosQueryProcAddr(hMte, 807, 0, (PFN*)&_inQueryClipbrdFmtInfo);
        rc = DosQueryProcAddr(hMte, 854, 0, (PFN*)&_inSetClipbrdData);
        rc = DosQueryProcAddr(hMte, 888, 0, (PFN*)&_inTerminate);
        rc = DosQueryProcAddr(hMte, 841, 0, (PFN*)&_inQueryWindowText);
        rc = DosQueryProcAddr(hMte, 877, 0, (PFN*)&_inSetWindowText);

        rc = DosLoadModule((PCH)loadErr, 256, (PCH)"PMSHAPI", &hMte);

        if(rc)
            return 1;

        rc = DosQueryProcAddr(hMte, 123, 0, (PFN*)&_inChangeSwitchEntry);
        rc = DosQueryProcAddr(hMte, 124, 0, (PFN*)&_inQuerySwitchEntry );
        rc = DosQueryProcAddr(hMte, 125, 0, (PFN*)&_inQuerySwitchHandle);

        hab = _inInitialize(0);
        hmq = _inCreateMsgQueue(hab, 0);

        hSw = _inQuerySwitchHandle(0, pib->pib_ulpid);
        if(hSw)
        {
            _inQuerySwitchEntry(hSw, &swOldData);
            hwndFrame = swOldData.hwnd;

            if(hwndFrame)
                _inQueryWindowText(hwndFrame, sizeof(cOldTitle), (PCH)cOldTitle);
        }

        return 0;
    }
    return 1;
}

void deinit_pm(void)
{
    if(hwndFrame)
        set_title(cOldTitle);
    _inChangeSwitchEntry(hSw, &swOldData);

    if(hmq)
        _inDestroyMsgQueue(hmq);
    if(hab)
        _inTerminate(hab);

    if(pib && ulType)
        pib->pib_ultype = ulType;
}

int get_fg_state(void)
{
    int rc = 0;

    switch(pib->pib_ultype)
    {
        case 0: /* FS  */
            break;

        case 2: /* VIO */
        case 3: /* PM  */
            rc = (hwndFrame == _inQueryActiveWindow(HWND_DESKTOP)) ? 1:0;
            break;
    }
    return rc;
}

void set_title(char *title)
{
    if(hmq)
    {
        static char vTitle[FED_MAXPATH]="";
        if(strcmp(vTitle, title))
        {
            SWCNTRL swData = swOldData;
            strcpy(vTitle, title);
            strncpy(swData.szSwtitle, title, MAXNAMEL);
            _inChangeSwitchEntry(hSw, &swData);
            _inSetWindowText(hwndFrame, (PCH)title);
        }
    }
}

void vio_init(void)
{
    USHORT blen = 0;
	ULONG badr = 0;
    HKBD hKbd = 0;
    VIOMODEINFO MI;
    KBDINFO kbInfo;
    APIRET rc;

    if(VioInitComplete)
        return;

    memset(&MI, 0, sizeof(MI));

    MI.cb = sizeof(MI);

    rc = VioGetMode(&MI, 0);
    VioGetBuf(&badr, &blen, 0);

    Rows = MI.row;
    Cols = MI.col;

    Screen = (char *) (((badr >> 3) & 0xffff0000L) | (badr & 0xffffL));
    BufLen = blen;
    kbInfo.cb = sizeof(KBDINFO);

    rc = KbdFlushBuffer(hKbd);
    rc = KbdGetStatus(&kbInfo, hKbd);

    if(!rc)
    {
        kbInfo.fsMask = KEYBOARD_BINARY_MODE | KEYBOARD_SHIFT_REPORT ;
        rc = KbdSetStatus(&kbInfo, hKbd);
    }

    rc = KbdFlushBuffer(hKbd);

    rc = DosAllocMem((PPVOID)&AlignedBuffer, BufLen,
     	             PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_TILE);

    rc = VioGetCurType(&CUR_SAVE,0);
    rc = VioGetCurPos(&save_row, &save_col, 0);

    init_pm(0);

    VioInitComplete = 1;
}

void vio_shutdown(void)
{
    if(!VioInitComplete)
        return;

    deinit_pm();
    vio_cls(0x07);
    vio_show();
    vio_cursor_pos(save_row, save_col);

    VioSetCurType(&CUR_SAVE, 0);

    VioInitComplete = 0;
}

void vio_read_key(KeyInfo* key)
{
    KBDTRANS   skInfo    = {0};
    KBDKEYINFO skNewInfo = {0};
    HKBD   hKbd    = 0;
    APIRET rc      = 0;
    USHORT usKey   = 0;
    USHORT usShift = 0;

    key->rep_count = 1;

    rc = KbdCharIn((PKBDKEYINFO)&skInfo, IO_WAIT, hKbd);

    if(!rc)
    {
        usKey   = (USHORT)(skInfo.chScan << 8) | skInfo.chChar;
        usShift = skInfo.fsState;

        key->old_key = usKey;

        do
        {
            rc = KbdPeek(&skNewInfo, hKbd);
            if(!rc)
            {
                if(   skInfo.chScan == skNewInfo.chScan
                   && skInfo.chChar == skNewInfo.chChar
                   && skInfo.fsState== skNewInfo.fsState
                   && skInfo.time   != skNewInfo.time)
                {
                    rc = KbdCharIn((PKBDKEYINFO)&skInfo, IO_WAIT, hKbd);
                    key->rep_count++;
                }
                else
                    rc = (APIRET)-1;
            }
        }
        while(!rc);
    }
    importKey(key, usKey, usShift);
}

void vio_cls(int Color)
{
    vio_fill(Color, ' ');
}

void vio_fill(int Color, int Char)
{
    unsigned short *uScr = (unsigned short *)Screen;
    int i;
    for(i = 0; i < (BufLen /2); i++)
        *uScr++ = (unsigned short)((Color << 8) | (Char & 0xFF));
}

void vio_print2(int Row, int Col, char *String, int MaxLen, int Color)
{
    vio_print(Row, Col, String, MaxLen, Color);
    vio_show_str(Row, Col, MaxLen);
}

void vio_print(int Row, int Col, char *String, int MaxLen, int Color)
{
    int i;
    char *sptr = ScreenPtr(Row, Col);

    if(!String)
        return;
    if((Cols - Col) < MaxLen)
        MaxLen = Cols - Col;

    for(i = 0; i < MaxLen; i++)
    {
        *sptr ++ = (*String) ? (*String):' ';
        *sptr++  = (char)Color;

        if(*String)
            String++;
    }
}

void vio_printh(int Row, int Col, char *String, int MaxLen, int Color, int ColorH)
{
    int i;
    char *sptr = ScreenPtr(Row, Col);
    char *Ccolor = (char *) &Color;

    if(!String)
        return;

    if((Cols - Col) < MaxLen)
        MaxLen = Cols - Col;

    for(i = 0; i < MaxLen; i++)
    {
        if(*String == SWITCH_CHAR)
        {
            /*Swap colors*/
            Ccolor = (Ccolor == (char *) &Color) ?
                        ((char *) &ColorH):
                        ((char *) &Color);
            i--;
        }
        else
        {
            *sptr ++ = (*String) ? (*String):' ';
            *sptr ++ = *Ccolor;
        }
        if(*String)
            String++;
    }
}
void vio_hdraw(int Row, int Col, char Char, int Len, int Color)
{
    char *sptr = ScreenPtr(Row, Col);
    int i;

    if((Cols - Col) < Len)
        Len = Cols - Col;

    for(i = 0; i < Len; i++)
    {
        *sptr ++ = Char;
        *sptr ++ = (char)Color;
    }
}

char vio_get_attr(int Row, int Col)
{
    char *sptr = ScreenPtr(Row, Col) + 1;
    return *sptr;
}

void vio_vdraw(int Row, int Col, char Char, int Len, int Color)
{
    char *sptr = ScreenPtr(Row, Col);
    int Shift = (Cols - 1) << 1;
    int i;

    if((Rows - Row) < Len)
        Len = Rows - Row;
    for(i = 0; i < Len; i++)
    {
        *sptr++ = Char;
        *sptr++ = (char)Color;
        sptr += Shift;
    }
}

void vio_box(int Row, int Col, int Hight, int Width, int Type, int Color)
{
    static char* sideTypes[]=
    {
        /*Corners, Top, Bottom, Left, Right sides */
        "Ú¿ÀÙÄÄ³³", /*Single line             */
        "Õ¸ÀÙÍÄ³³", /*Double top              */
        "É»È¼ÍÍºº", /*Double                  */
        "Õ¸À¼ÍÄ³³", /*Double top, resizeable  */
        "³³ÀÙÛÄ³³", /*Single line, bold top   */
        "        "
    };
#define UL_Corner  0
#define UR_Corner  1
#define BL_Corner  2
#define BR_Corner  3
#define T_Side     4
#define B_Side     5
#define L_Side     6
#define R_Side     7

    if(Type < (sizeof(sideTypes)/sizeof(char *)))
    {
        vio_hdraw(Row            , Col            , sideTypes[Type][UL_Corner], 1, Color);
        vio_hdraw(Row            , Col + Width - 1, sideTypes[Type][UR_Corner], 1, Color);
        vio_hdraw(Row + Hight - 1, Col            , sideTypes[Type][BL_Corner], 1, Color);
        vio_hdraw(Row + Hight - 1, Col + Width - 1, sideTypes[Type][BR_Corner], 1, Color);
        vio_hdraw(Row            , Col + 1        , sideTypes[Type][T_Side], Width - 2, Color);
        vio_hdraw(Row + Hight - 1, Col + 1        , sideTypes[Type][B_Side], Width - 2, Color);
        vio_vdraw(Row + 1        , Col            , sideTypes[Type][L_Side], Hight - 2, Color);
        vio_vdraw(Row + 1        , Col + Width - 1, sideTypes[Type][R_Side], Hight - 2, Color);
    }
}
void vio_show(void)
{
    VioShowBuf((USHORT) 0, (USHORT)BufLen, 0);
}
void vio_cursor_pos(int row, int col)
{
    VioSetCurPos((USHORT)row, (USHORT)col, 0);
}
void vio_cursor_type(int shape)
{
    VIOCURSORINFO cursor;
    cursor.cEnd   = (USHORT)-100;
    cursor.cx     = 1;
    cursor.attr   = (USHORT)-1;

    if(iShape[0] <= 0)
        iShape[0] = 10;

    if(iShape[0] >= 100)
        iShape[0] = 90;

    if(iShape[1] <= 0)
        iShape[1] = 10;

    if(iShape[1] >= 100)
        iShape[1] = 90;

    cursor.yStart = (USHORT)(iShape[0] - 100);

    switch(shape)
    {
        case BigCursor:
            cursor.yStart = (USHORT)(iShape[1] - 100);
        case Underline :
            cursor.attr   = 0;
        case NoCursor :
            break;
    }
    VioSetCurType(&cursor, 0);
}

struct stScreen
{
    int Row;
    int Col;
    int Rows;
    int Cols;
    char Data[1];
};

void *vio_save_box(int Row, int Col, int Hight, int Width)
{
    stScreen * save_data;

    if(Hight * Width <= 0 || Row < 0 && Col < 0)
        return 0;

    save_data = (stScreen *) new char[(sizeof(stScreen) + (Hight*Width) << 1)];
    save_data->Row  = Row ;
    save_data->Col  = Col ;
    save_data->Rows = Hight;
    save_data->Cols = Width;
    int offset_sb = 0;
    int offset = ScreenOffset(Row, Col);

    for(int i = 0; i < Hight; i++)
    {
        memcpy(&save_data->Data[offset_sb], &Screen[offset], Width << 1);
        offset    += Cols << 1;
        offset_sb += Width << 1;
    }
    return save_data;
}

void vio_restore_box(void *data)
{
    stScreen * save_data = (stScreen *)data;
    if(!save_data)
        return;
    int offset_sb = 0;
    int offset = ScreenOffset(save_data->Row, save_data->Col);
    for(int i = 0; i < save_data->Rows; i++)
    {
        memcpy(&Screen[offset], &save_data->Data[offset_sb], save_data->Cols << 1);
        VioShowBuf((USHORT)offset, (USHORT)(save_data->Cols << 1), 0);
        offset    += Cols << 1;
        offset_sb += save_data->Cols << 1;
    }

    delete save_data;
}

void vio_show_str(int Row, int Col, int Len)
{
    USHORT offset = (USHORT)ScreenOffset(Row,Col);
    VioShowBuf((USHORT)offset, (USHORT)(Len << 1), 0);
}

void vio_show_buf(int Offset, int Len)
{
    VioShowBuf((USHORT)Offset, (USHORT)Len, 0);
}

char *vio_set_work_buff(char *buff)
{
    char *old = Screen;
    Screen = buff;
    return old;
}

void importKey(KeyInfo* key, USHORT usKey, USHORT usShift)
{
    key->skey = AltKey[usKey >> 8].code;
    key->key  = (unsigned short)(usKey & 0x00FF);
    key->KeyName[0] = 0;

    if(key->skey == kbEsc)      //ESC-key handling
        key->skey |= shIsCtrl;

    if(usShift & 8)
        key->skey |= shIsCtrl | shAlt;

    if(usShift & 4)
        key->skey |= shIsCtrl | shCtrl;

    if(usShift & 3) //'Shift' pressed
    {
        key->skey |= shShift;

        //Shift can be pressed in many situations
        //We will try to separate them into two categories:
        //1. Shift pressed with usual alpha-numeric key
        //2. Shift pressed with control key

        //First try:
        // If this key don't have ASCII code this is control key
        //Second try:
        // Another similar case: ASCII code for this key is 0xE0
        //Last try:
        // If ASCII code for this key between '0' and '9', or is '.',
        // and Shift is pressed, we assume that this is control key.
        // By the way: these codes can't come from other source, because
        // from the other sources they come _without_ Shift pressed

        if( (usKey & 0x00FF) == 0 ||
            (usKey & 0x00FF) == 0xE0 ||
            ((usKey & 0x00FF) >= '0' && (usKey & 0x00FF) <= '9') ||
            ((usKey & 0x00FF) == '.' && (usKey & 0xFF00) != 0))
        {
            key->skey |= shIsCtrl;
        }
    }

    if((usKey & 0x00FF) == 0)
    {
        key->skey |= shIsCtrl;
    }

    if((usKey & 0x00FF) == 0xE0)
    {
        //Check if this is control key.
        //Control key has long (at least 2 chars) name.

        char* ptr = AltKey[usKey >> 8].name;

        if(ptr[0] && ptr[1])
	        key->skey |= shIsCtrl;
    }

    if((usKey & 0x00FF) == 0x0D)
        key->skey |= shIsCtrl;
    if((usKey & 0x00FF) == 0x08)
        key->skey |= shIsCtrl;
    if((usKey & 0x00FF) == 0x09)	//Tabs
        key->skey |= shIsCtrl;
    if((usKey & 0xFF00) == 0x4A00)
        key->skey |= shIsCtrl;
    if((usKey & 0xFF00) == 0x4E00)
        key->skey |= shIsCtrl;

    if(key->skey & shIsCtrl)
    {
        char *ptr = key->KeyName;
        static char cAlt  []="Alt";
        static char cCtrl []="Ctrl";
        static char cShift[]="Shift";

        *ptr++ = 'k';
        *ptr++ = 'b';
        if(key->skey & shAlt)
        {
            memcpy(ptr, cAlt, sizeof(cAlt) - 1);
            ptr += sizeof(cAlt) - 1;
        }

        if(key->skey & shCtrl)
        {
            memcpy(ptr, cCtrl, sizeof(cCtrl) - 1);
            ptr += sizeof(cCtrl) - 1;
        }

        if(key->skey & shShift)
        {
            memcpy(ptr, cShift, sizeof(cShift) - 1);
            ptr += sizeof(cShift) - 1;
        }
        // Copy string from array, including '\x00' terminator

        memcpy(ptr, AltKey[usKey >> 8].name, strlen(AltKey[usKey >> 8].name) + 1);
    }
    else
    {
        key->KeyName[0] = key->key;
        key->KeyName[1] = 0;
    }

    memcpy(&kiLastKey, key, sizeof(kiLastKey));
}

void vio_draw_attr(int Row, int Col, int Len, int Color)
{
    char *sptr = ScreenPtr(Row, Col);
    int i;

    if((Cols - Col) < Len)
        Len = Cols - Col;
    for(i = 0; i < Len; i++)
    {
        sptr ++;
        *sptr ++ = (char)Color;
    }
}

void vio_draw_chr(int Row, int Col, int Char)
{
    *ScreenPtr(Row, Col) = (char)Char;
}

void vio_scroll(int Dir, Rect& rect, int Num, int Attr)
{
    static BYTE cell[]= {' ', ' '};

    cell[1] = (BYTE)Attr;

    switch(Dir)
    {
        case SCROLL_UP:
            VioScrollUp((USHORT)rect.row, (USHORT)rect.col,
                        (USHORT)(rect.row + rect.rows - Num),
                        (USHORT)(rect.col + rect.cols),
                        (USHORT)Num, (PBYTE)cell, 0);
                        break;
        case SCROLL_DN:
            VioScrollDn((USHORT)rect.row, (USHORT)rect.col,
                        (USHORT)(rect.row + rect.rows - Num),
                        (USHORT)(rect.col + rect.cols),
                        (USHORT)Num, (PBYTE)cell, 0);
                        break;
        case SCROLL_LT:
            VioScrollLf((USHORT)rect.row, (USHORT)rect.col,
                        (USHORT)(rect.row + rect.rows - 1),
                        (USHORT)(rect.col + rect.cols - Num),
                        (USHORT)Num, (PBYTE)cell, 0);
                        break;
        case SCROLL_RT:
            VioScrollRt((USHORT)rect.row, (USHORT)rect.col,
                        (USHORT)(rect.row + rect.rows - 1),
                        (USHORT)(rect.col + rect.cols - Num),
                        (USHORT)Num, (PBYTE)cell, 0);
                        break;
    }
}

//-----------------------------------------
// Thread level HAB/HMQ handler
//-----------------------------------------

PMObj::PMObj():_hmq(0)
{
    if(!hab)
        return;

    _hmq = _inCreateMsgQueue(hab, 0);
}

PMObj::~PMObj()
{
    if(_hmq)
    {
        _inDestroyMsgQueue(_hmq);
        _hmq = 0;
    }
}

char _cZero[]="";

stKeyMap AltKey[] =
{
    { 0x00, _cZero },
    { 0x1A, "Esc" },
    { 0x02, "1" },
    { 0x03, "2" },
    { 0x04, "3" },
    { 0x05, "4" },
    { 0x06, "5" },
    { 0x07, "6" },
    { 0x08, "7" },
    { 0x09, "8" },
    { 0x0A, "9" },
    { 0x01, "0" },
    { 0x38, "Minus" },
    { 0x19, "Equal" },
    { 0x0E, "Bksp" },
    { 0x48, "Tab" },
    { 0x3F, "Q" },
    { 0x4D, "W" },
    { 0x16, "E" },
    { 0x41, "R" },
    { 0x47, "T" },
    { 0x4F, "Y" },
    { 0x4A, "U" },
    { 0x30, "I" },
    { 0x3A, "O" },
    { 0x3B, "P" },
    { 0x35, "Lbracket" },
    { 0x42, "Rbracket" },
    { 0x18, "Enter" },
    { 0x00, _cZero },
    { 0x0B, "A" },
    { 0x44, "S" },
    { 0x12, "D" },
    { 0x1B, "F" },
    { 0x28, "G" },
    { 0x2E, "H" },
    { 0x32, "J" },
    { 0x33, "K" },
    { 0x34, "L" },
    { 0x45, "Semicolon" },
    { 0x40, "Quote" },
    { 0x49, "Tilde" },
    { 0x00, _cZero },
    { 0x0D, "BackSlash" },
    { 0x50, "Z" },
    { 0x4E, "X" },
    { 0x0F, "C" },
    { 0x4C, "V" },
    { 0x0C, "B" },
    { 0x39, "N" },
    { 0x37, "M" },
    { 0x11, "Comma" },
    { 0x3E, "Point" },
    { 0x14, "Div" },
    { 0x00, _cZero },
    { 0x2C, "GrMul" },
    { 0x00, _cZero },
    { 0x46, "Space" },
    { 0x00, _cZero },
    { 0x1C, "F1" },
    { 0x20, "F2" },
    { 0x21, "F3" },
    { 0x22, "F4" },
    { 0x23, "F5" },
    { 0x24, "F6" },
    { 0x25, "F7" },
    { 0x26, "F8" },
    { 0x27, "F9" },
    { 0x1D, "F10" },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x2F, "Home" },
    { 0x4B, "Up" },
    { 0x3D, "PgUp" },
    { 0x2B, "GrMinus" },
    { 0x36, "Left" },
    { 0x10, "Center" },
    { 0x43, "Right" },
    { 0x2D, "GrPlus" },
    { 0x17, "End" },
    { 0x15, "Down" },
    { 0x3C, "PgDown" },
    { 0x31, "Ins" },
    { 0x13, "Del" },
    { 0x1C, "F1" },
    { 0x20, "F2" },
    { 0x21, "F3" },
    { 0x22, "F4" },
    { 0x23, "F5" },
    { 0x24, "F6" },
    { 0x25, "F7" },
    { 0x26, "F8" },
    { 0x27, "F9" },
    { 0x1D, "F10" },
    { 0x1C, "F1" },
    { 0x20, "F2" },
    { 0x21, "F3" },
    { 0x22, "F4" },
    { 0x23, "F5" },
    { 0x24, "F6" },
    { 0x25, "F7" },
    { 0x26, "F8" },
    { 0x27, "F9" },
    { 0x1D, "F10" },
    { 0x1C, "F1" },
    { 0x20, "F2" },
    { 0x21, "F3" },
    { 0x22, "F4" },
    { 0x23, "F5" },
    { 0x24, "F6" },
    { 0x25, "F7" },
    { 0x26, "F8" },
    { 0x27, "F9" },
    { 0x1D, "F10" },
    { 0x00, _cZero },
    { 0x36, "Left" },
    { 0x43, "Right" },
    { 0x17, "End" },
    { 0x3C, "PgDown" },
    { 0x2F, "Home" },
    { 0x02, "1" },
    { 0x03, "2" },
    { 0x04, "3" },
    { 0x05, "4" },
    { 0x06, "5" },
    { 0x07, "6" },
    { 0x08, "7" },
    { 0x09, "8" },
    { 0x0A, "9" },
    { 0x01, "0" },
    { 0x38, "Minus" },
    { 0x19, "Equal" },
    { 0x3D, "PgUp" },
    { 0x1E, "F11" }, //Norm
    { 0x1F, "F12" },
    { 0x1E, "F11" }, //Shift
    { 0x1F, "F12" },
    { 0x1E, "F11" }, //Ctrl
    { 0x1F, "F12" },
    { 0x1E, "F11" }, //Alt
    { 0x1F, "F12" },
    { 0x4B, "Up" },
    { 0x2B, "GrMinus" },
    { 0x10, "Center" },
    { 0x2D, "GrPlus" },
    { 0x15, "Down" },
    { 0x31, "Ins" },
    { 0x13, "Del" },
    { 0x48, "Tab" },
    { 0x29, "GrDiv" },
    { 0x2C, "GrMul" },
    { 0x2F, "Home" },
    { 0x4B, "Up" },
    { 0x3D, "PgUp" },
    { 0x00, _cZero },
    { 0x36, "Left" },
    { 0x00, _cZero },
    { 0x43, "Right" },
    { 0x00, _cZero },
    { 0x17, "End" },
    { 0x15, "Down" },
    { 0x3C, "PgDown" },
    { 0x31, "Ins" },
    { 0x13, "Del" },
    { 0x29, "GrDiv" },
    { 0x00, _cZero },
    { 0x2A, "GrEnter" },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x2A, "GrEnter" },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero },
    { 0x00, _cZero }
};

