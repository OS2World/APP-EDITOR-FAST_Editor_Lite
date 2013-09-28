/*
** Module   :KEYNAMES.H
** Abstract :Names for syntetic keys
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sun  04/05/1997   	Created
*/

#ifndef  __KEYNAMES_H
#define  __KEYNAMES_H

enum keyFlags
{
    shIsCtrl = 0x0100,
    shCtrl   = 0x0200,
    shShift  = 0x0400,
    shAlt    = 0x0800
};

enum keyNames
{
    kb0         =0x01,
    kb1         =0x02,
    kb2         =0x03,
    kb3         =0x04,
    kb4         =0x05,
    kb5         =0x06,
    kb6         =0x07,
    kb7         =0x08,
    kb8         =0x09,
    kb9         =0x0A,
    kbA         =0x0B,
    kbB         =0x0C,
    kbBackSlash =0x0D,
    kbBksp      =0x0E,
    kbC         =0x0F,
    kbCenter    =0x10,
    kbComma     =0x11,
    kbD         =0x12,
    kbDel       =0x13,
    kbDiv       =0x14,
    kbDown      =0x15,
    kbE         =0x16,
    kbEnd       =0x17,
    kbEnter     =0x18,
    kbEqual     =0x19,
    kbEsc       =0x1A,
    kbF         =0x1B,
    kbF1        =0x1C,
    kbF10       =0x1D,
    kbF11       =0x1E,
    kbF12       =0x1F,
    kbF2        =0x20,
    kbF3        =0x21,
    kbF4        =0x22,
    kbF5        =0x23,
    kbF6        =0x24,
    kbF7        =0x25,
    kbF8        =0x26,
    kbF9        =0x27,
    kbG         =0x28,
    kbGrDiv     =0x29,
    kbGrEnter   =0x2A,
    kbGrMinus   =0x2B,
    kbGrMul     =0x2C,
    kbGrPlus    =0x2D,
    kbH         =0x2E,
    kbHome      =0x2F,
    kbI         =0x30,
    kbIns       =0x31,
    kbJ         =0x32,
    kbK         =0x33,
    kbL         =0x34,
    kbLbracket  =0x35,
    kbLeft      =0x36,
    kbM         =0x37,
    kbMinus     =0x38,
    kbN         =0x39,
    kbO         =0x3A,
    kbP         =0x3B,
    kbPgDown    =0x3C,
    kbPgUp      =0x3D,
    kbPoint     =0x3E,
    kbQ         =0x3F,
    kbQuote     =0x40,
    kbR         =0x41,
    kbRbracket  =0x42,
    kbRight     =0x43,
    kbS         =0x44,
    kbSemicolon =0x45,
    kbSpace     =0x46,
    kbT         =0x47,
    kbTab       =0x48,
    kbTilde     =0x49,
    kbU         =0x4A,
    kbUp        =0x4B,
    kbV         =0x4C,
    kbW         =0x4D,
    kbX         =0x4E,
    kbY         =0x4F,
    kbZ         =0x50
};

#endif //__KEYNAMES_H

