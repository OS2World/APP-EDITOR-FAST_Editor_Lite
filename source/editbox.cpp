/*
** Module   :EDITBOX.CPP
** Abstract :Complete interface for single editor window
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  03/05/1997   	Created
*/

#include <string.h>
#include <stdio.h>

#include <fio.h>
#include <stddlg.h>
#include <version.h>

#define INCL_DOSFILEMGR
#include <os2.h>

short EditBox::used = 0;

EditBox::EditBox(int i, int j, int k, int l)
{
	cName = new char[1024];

    row = i;
    col = j;
    rows = k;
    cols = l;
    name = 0;
    orig_file = 0;
    num = 10;

    short bit = 1;

    for(i = 0; i < 10; i++)
    {
        if(!(used & bit))
        {
            num = i;
            used |= bit;
            break;
        }
        bit <<= 1;
    }

    new_file();
}

EditBox::~EditBox()
{
    if(num < 10)
    {
        short bit = 1;
        bit <<= num;
        used &= ~bit;
    }
    delete name;
    _fr_file(orig_file);
    delete cName;
}

int EditBox::load(char* new_name)
{
    char *str, *ptr;
    int old_completed = -1;
    int _unx = 1;

    close();

    if(!new_name)
        new_name = untitled;

    name = get_full_name(new_name);

    set_hiliting(0);
//-------------------------------------
// Guess hiliting

    orig_file = _ld_file(name);

    long flen = strlen(orig_file);

    if(!flen)
    {
        _unx = iDefType;
        add_line(new Line);
        set_hiliting(Parser::GuessType(name));
        set_custom_syntax(0);

        if(!_file_exists(name))
            set_changed(1);

        return -1;
    }

//-------------------------------------
// scan loaded file and prepare them for displaying
//
    for(ptr = str = orig_file; (str - orig_file) < flen;)
    {
        int i = 1;

        long completed = ((long)(str - orig_file) * 40 + 1)/flen;

        if(completed != old_completed)
        {
            vio_print2(Rows - 1,
            	       0,
                       &ProgressBar[40 - completed],
	                   Cols,
                       app_pal[CL_APPLICATION_START+CL_STATUSLINE]);
            old_completed = completed;
        }

        while(*str && *str != '\n')
        {
            if(str[0] == '\r' && str[1] != '\n')
            {
                str++;
                break;
            }
            str++;
        }

        if(*str)
        {
            if((str > orig_file) && str[-1] == '\r')
            {
                _unx = 0;
                str[-1] = 0;
            }

            if(str[0] == '\n')
	            *str++ = 0;
        }

        Add(new Line(ptr));

        ptr = str;
    }

    if(!Count())
    {
        _unx = iDefType;
        add_line(new Line);
    }

    set_hiliting(Parser::GuessType(name));
    set_custom_syntax(0);       //This is our guess, not a user setting

    char *ea = 0;

    if(!get_ea(get_name(), ".FEDPOS",&ea) && ea)
    {
        int cX = 0;
        int cY = 0;

        parse_pos(ea, &cX, &cY);

        if(cX > 0)
            goto_line(cX - 1);

        if(cY > 0)
            goto_col(cY - 1);

        delete ea;
        ea = 0;
    }

    int i;

    for(i = 0; i < BMK_NUM; i++)
    {
        char eaname[128];
        char num[16];

        strcpy(eaname, ".FEDMARK");
        strcat(eaname, u2s(i,0,num));

        if(!get_ea(get_name(), eaname, &ea) && ea)
        {
            bookmark[i].row = bookmark[i].col = 0;
            parse_pos(ea, &bookmark[i].row, &bookmark[i].col);
            delete ea;
            ea = 0;
        }
    }

    int cp_set = 0;

    if(!get_ea(get_name(), ".FEDSTATE",&ea) && ea)
    {
        int cX = 0;
        int cY = 0;
        char *ptr;

        ptr = parse_pos(ea, &cX, &cY);

        if(cX > 0)
            goto_line(cX - 1);

        if(cY > 0)
            goto_col(cY - 1);

        if(ptr && *ptr == ' ')
            ptr++;

        for(i = 0; i < BMK_NUM && ptr; i++)
        {
            bookmark[i].row = bookmark[i].col = 0;
            ptr = parse_pos(ptr, &bookmark[i].row, &bookmark[i].col);

            if(ptr && *ptr == ' ')
                ptr++;
        }

        if(ptr && *ptr == ' ')
            ptr++;

        if(ptr && *ptr == '+')
        {
            ptr++;
            int num = 0;

            while(*ptr && *ptr >= '0' && *ptr <= '9')
            {
                num *= 10;
                num += *ptr - '0';
                ptr++;
            }

            if(num > 0)
                set_hiliting(num - 1);

            if(ptr && *ptr == ' ')
                ptr++;
        }

        if(ptr && *ptr == ' ')
            ptr++;

        if(ptr && *ptr == '*')
        {
            set_xlate(ptr + 1);
            cp_set = 1;
        }

        delete ea;
        ea = 0;
    }

    set_unix(_unx);

    do
    {
        if(cp_set)
            break;

        if(iCurCP != 866 && iCurCP != 1125)
            break;

        //Guess codepage

        char c1251[256];
        char c878 [256];
        char cSrc [256];

        for(i = 0; i < 256; i++)
            cSrc[i] = c878[i] = c1251[i] = i;

        if(cp2cp("IBM-878" , "IBM-866", &cSrc[1], &c878 [1], 255))
            break;

        if(cp2cp("IBM-1251", "IBM-866", &cSrc[1], &c1251[1], 255))
            break;

        unsigned rate1251 = 0;
        unsigned rate866  = 0;
        unsigned rate878  = 0;

        for(i = 0; i < Count(); i++)
        {
            char save = 0;
            str = line(i)->str;

            while(*str)
            {
                if(*str == save)
                {
                    if(__isnru(save))
                        rate866++;

                    str++;
                    continue;
                }
                save = *str;

                rate866  += (__isru(save)       ) ? 1:0;
                rate878  += (__isru(c878[save]) ) ? 1:0;
                rate1251 += (__isru(c1251[save])) ? 1:0;

                str++;
            }
        }

//fprintf(stderr, "1251:%d 866:%d 878:%d\n", rate1251, rate866, rate878);

        if(rate866 >= rate878 && rate866 >= rate1251)
            break;

        if(rate1251 > rate878)
            set_xlate("IBM-1251");
        else
            set_xlate("IBM-878");
    }
    while(0);

// Finally fill hiliting

    if(get_hiliting())
       fill_hiliting(0, ST_INITIAL);

    return 0;
}

int EditBox::new_file()
{
    close();
    add_line(new Line);
    set_unix(iDefType);

    name = get_full_name(untitled);

    return 0;
}

int EditBox::save_as(char *new_name)
{
    set_changed(1);
    set_name(new_name);
    return save();
}

void EditBox::set_name(char *new_name)
{
    delete name;
    name = get_full_name(new_name);
}

void EditBox::put_xy()
{
    char ea[256];

    if(iNoEA)
        return;

    char *ptr = ea;

    ptr = mk_pos(ptr, get_edit_row(), get_edit_col());
    *ptr++ = ' ';

    for(int i = 0; i < BMK_NUM; i++)
    {
        ptr = mk_pos(ptr, bookmark[i].row, bookmark[i].col);
        *ptr++ = ' ';
    }

    *ptr = 0;

    //Save syntax

    if(iSaveSyntax && (iSaveSyntax > 1 || get_custom_syntax()))
    {
    	char num[16];

        strcat(ptr, "+");
        strcat(ptr, u2s(get_hiliting() + 1,0,num));
    }

    strcat(ptr, " ");
    strcat(ptr, "*");
    strcat(ptr, get_cur_cp());

    put_ea(get_name(), ".FEDSTATE", ea);
}

int EditBox::save()
{
    if(get_saved())
    {
        //Preserve original time and date
        APIRET rc;
        FILESTATUS3 stFStatus;

        memset(&stFStatus, 0, sizeof(FILESTATUS3));

        rc = DosQueryPathInfo((PSZ)get_name(), FIL_STANDARD,
                              &stFStatus, sizeof(FILESTATUS3));

        if(!rc)
        {
            put_xy();

            DosSetPathInfo((PSZ)get_name(), FIL_STANDARD,
                           &stFStatus, sizeof(FILESTATUS3), DSPI_WRTTHRU);
        }
        return 0;
    }

    int out;
    int old_completed = -1;

    out = _lopen(name, OP_WRITE);

    if(out <= 0)
        return -1;

    BlockWrite blk = 32000;

    blk.Open(out);

    for(int i = 0; i < Count(); i++)
    {
        char *str = line(i)->str;
        char *non_blank = str;
        int len;
        int completed = (i * 40 + 1)/Count();

        if(completed != old_completed)
        {
        	vio_print2(Rows - 1, 0, &ProgressBar[40 - completed], Cols,
                       app_pal[CL_APPLICATION_START+CL_STATUSLINE]);
            old_completed = completed;
        }
        while(*str)
        {
            if(!__issp(*str))
                non_blank = str;
            str++;
        }

        if(!__issp(*non_blank) && *non_blank != 0)
            non_blank++;

        len = non_blank - line(i)->str;

        if(len)
            blk.Add(line(i)->str, len);

        if(i < (Count()-1))
        {
            if(get_unix())
                blk.Add("\n", 1);
            else
                blk.Add("\r\n", 2);
        }
        else
        {
            switch(iEOF)
            {
                case EOF_MODE_CRLF:
                    if(get_unix())
                        blk.Add("\n", 1);
                    else
                        blk.Add("\r\n", 2);
                    break;

                case EOF_MODE_EOF:
                    blk.Add("\x1A", 1);
                    break;

                default:
                    break;
            }
        }
    }

    blk.Close();

    _lclose(out);

    set_changed(0);
    set_saved(1);

    put_xy();

    return 0;
}

int EditBox::close()
{
    _fr_file(orig_file);
    delete name;

    name      = 0;
    orig_file = 0;

    track_cancel();
    clear_undobuff();
    RemoveAll();

    set_saved(1);

    return 0;
}

void EditBox::draw()
{
    Buffer::draw();
    vio_cursor_pos(row + get_cur_row(), col + get_cur_col());
    vio_cursor_type((get_ins_mode()) ? Underline:BigCursor);
}

int EditBox::check_save()
{
    if(get_saved())
        return 0;

    int rc = AChoice(5, 5, Yes_No, "Save?");

    switch(rc)
    {
        case -1:
        case  2:
            return -1;
        case  0:

            if(is_untitled())
            {
                rc = FileDialog(5, 5, cName, 2);
                if(!rc)
                    save_as(cName);
                else
                    return -1;
            }
            save();
    }
    return 0;
}

int EditBox::is_untitled()
{
    char buff[FED_MAXPATH];
    char buff2[FED_MAXPATH];
    make_short_name(get_name(), buff);
    make_short_name(untitled, buff2);
    return (!strcmp(buff, buff2)) ? 1:0;
}

char *EditBox::get_fmt_name(int minlen, int fill)
{
    static char buff[FED_MAXPATH];
    char fname[FED_MAXPATH];
    char *tmp;
    char *src;

    if(minlen > FED_MAXPATH)
        minlen = FED_MAXPATH;

    memset(buff, 0, FED_MAXPATH);

    if(minlen)
        memset(buff, fill, minlen);

    tmp = buff;

    src = get_name();

    if(src)
    {
    	strcpy(fname, src);
        fname[minlen] = 0;

        if(strlen(src) > minlen && minlen > 7 && iFileName)
        {
            char *dst = fname;

            while(*src != '\\')
                *dst++ = *src++;

            while(strlen(src) > (minlen - 6))
            {
                char *slash = strchr(src + 1, '\\');

                if(slash)
                    src = slash;
                else
                    break;  //no more slashes, leave
            }

            *dst++ = '\\';
            *dst++ = '.' ;
            *dst++ = '.' ;
            *dst++ = '.' ;

            strcpy(dst, src);
            fname[minlen] = 0;
        }

        src = fname;

        while(*src)
        {
            *tmp++ = *src++;
        }
    }

    return buff;
}

