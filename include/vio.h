/*
** Module   :VIO.H
** Abstract :Console I/O
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  29/01/1997     Created
*/

#ifndef  __VIO_H
#define  __VIO_H

#define KEY_NAME_LEN    32

struct Rect
{
    int row;
    int col;
    int rows;
    int cols;
};

typedef struct _st_key
{
    unsigned short key;
    unsigned short skey;
    unsigned short shift;
    unsigned short old_key;
    unsigned short rep_count;
    char KeyName[KEY_NAME_LEN];
} KeyInfo;

class PMObj
{
        unsigned long _hmq;
    public:

        PMObj();
        ~PMObj();
};

#define NoCursor        0
#define Underline       1
#define BigCursor       2

#define SCROLL_UP       0
#define SCROLL_DN       1
#define SCROLL_LT       2
#define SCROLL_RT       3

void vio_read_key(KeyInfo*);
void vio_show(void);
void vio_show_str(int Row, int Col, int Len);
void vio_show_buf(int Offset, int Len);
void vio_draw_attr(int Row, int Col, int Len, int Color);
void vio_draw_chr(int Row, int Col, int Char);
void vio_vdraw(int Row, int Col, char Char, int Len, int Color);
void vio_vdraw2(int Row, int Col, char Char, int Len, int Color);
void vio_hdraw(int Row, int Col, char Char, int Len, int Color);
void vio_hdraw2(int Row, int Col, char Char, int Len, int Color);
void vio_printh(int Row, int Col, char *String, int MaxLen, int Color, int ColorH);
void vio_printh2(int Row, int Col, char *String, int MaxLen, int Color, int ColorH);
void vio_print(int Row, int Col, char *String, int MaxLen, int Color);
void vio_print2(int Row, int Col, char *String, int MaxLen, int Color);
void vio_fill(int Color, int Char);
void vio_cls(int Color);
void vio_init(void);
void vio_shutdown(void);
void vio_cursor_pos(int row, int col);
void vio_cursor_type(int shape);
void vio_box(int Row, int Col, int Hight, int Width, int Type, int Color);
void vio_box2(int Row, int Col, int Hight, int Width, int Type, int Color);
char* vio_set_work_buff(char *buff);
void *vio_save_box(int Row, int Col, int Hight, int Width);
void vio_restore_box(void *data);
char vio_get_attr(int Row, int Col);
void vio_scroll(int Dir, Rect& rect, int Num, int Attr);
int init_pm(int);
void deinit_pm(void);
void set_title(char *title);
int get_fg_state(void);
int cp2cp(char *cp1, char *cp2, char *src, char *dst, int len);

#endif
