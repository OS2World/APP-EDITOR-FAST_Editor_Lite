/*
** Module   :LINE.CPP
** Abstract :Editor Line methods
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Wed  05/03/1997   	Updated to V0.5
**      Sun  09/11/1997     Updated to V0.9
**      Sun  29/02/2004     Updated to V2.28
*/

#include <string.h>

#include <buffer.h>
#include <version.h>

#define max(a,b)        (((a) > (b)) ? (a):(b))

#define GET_OP(c)   (c & 0xC0)
#define GET_CNT(c)  (c & 0x3F)
#define OP_NOP      0x00
#define OP_EQ       0x40
#define OP_SUB      0x80
#define OP_ADD      0xC0
#define MAX_CNT     0x3F

void Line::init()
{
    hl_state  = ST_INITIAL;
    buf_len   = 0;
    str_width = 0;
    str_size  = 0;
    str       = "";
    token_cache_type  = -1;
    token_cache_start = -1;
    token_cache_len   = 0;
}

Line::Line()
{
    init();
}

Line::Line(PLine master)
{
    init();

    if(master->buf_len)
    {
        str = new char[master->buf_len];

        memcpy(str, master->str, master->buf_len);

        buf_len = master->buf_len;
    }
    else
        str = master->str;

    recalc_sz();
}

Line::Line(PLine master, int start, int width, int force)
{
    init();

    if(width <= 0)
        return;

    expand_by(width + 1);

	if(force || (start % iTabWidth))	//Not on the tab boundary
	    master->get_print(start, str, width);
	else
    {
        int pos = 0;

        char* out = str;
        char* src = master->str;

        //Locate starting point in the buffer;

		while(pos < start)
	    {
	        if(!*src)
	            break;

	        if(*src == '\t')
				pos = ((pos / iTabWidth + 1) * iTabWidth);
			else
		        pos++;

		    src++;
	    }

	    if(*src)
	    {
	        while(pos < (start + width))
	        {
	            if(!*src)
	                break;

	            if(*src == '\t')
	            {
					int fill = pos;

					pos = ((pos / iTabWidth + 1) * iTabWidth);

                    if(pos <= (start + width))
                    {
                        *out++ = *src++;
                        continue;
                    }

                    while(fill++ < (start + width))
                        *out++ = ' ';
	            }
	            else
	            {
	                pos++;
	                *out++ = *src++;
	            }
	        }
	    }

		while(pos < (start + width))
	    {
	        if((pos + iTabWidth) < (start + width))
	        {
	            pos += iTabWidth;
	            *out++ = '\t';
	        }
	        else
	        {
	            pos++;
	            *out++ = ' ';
	        }
	    }

	    *out = 0;
    }

    recalc_sz();
}

Line::Line(char *new_str)
{
    init();
    str = new_str;
    recalc_sz();
}

Line::~Line()
{
    if(str && buf_len)
        delete str;
}

void Line::expand_by(int len)
{
    len = (len / CHUNK_SIZE + 1) * CHUNK_SIZE;

    char* tmp = new char [buf_len + len];

    if(buf_len)
    	delete str;

    buf_len += len;
    str = tmp;
}

void Line::set(char* str_new)
{
    int len = strlen(str_new)+1;

    check_size(len + 1);

    memcpy(str, str_new, len + 1);

    recalc_sz();
}

int Line::export_buf(char* dst)
{
	if(!dst)
	    return 0;

	int sz = size();

	memcpy(dst, str, sz);

	return sz;
}

char* Line::get_print(int start, char *buff, int width)
{
    char *str2 = str;
    int pos = 0;
    int end = start + width;
    int tabw = iTabWidth;

    for(pos = 0; *str2; str2++)
    {
        if(*str2 == '\t')
        {
            int fill = pos;
            pos = ((pos / tabw + 1) * tabw);

            while(fill < pos)
            {
                if(fill >= start && fill < end)
                    buff[fill - start] = ' ';

                fill++;
            }
        }
        else
        {
            if(pos >= start && pos < end)
                buff[pos - start] = *str2;
            pos++;
        }

        if(pos >= end)
            break;
    }

    if(pos < start) // Oops, string is too short
        pos = start;

    while(pos < end)
        buff[pos++ - start] = ' ';

    buff[pos - start] = 0;

    return buff;
}

void Line::touch()
{
    char* tmp = strchr(str, '\t');

    if(!tmp && buf_len)
        return;

    int slen = len();

    slen = (slen/CHUNK_SIZE + 1) * CHUNK_SIZE;

    tmp = new char[slen];
    build_print(tmp);

    if(buf_len)
        delete str;

    buf_len = slen;
    str = tmp;
    recalc_sz();
}

void Line::recalc_sz()
{
	str_width = 0;
	str_size  = 0;

	char *str2 = str;

	int tabw = iTabWidth;

	for(str_width = 0; *str2; str2++)
	{
		if(*str2 == '\t')
			str_width = ((str_width / tabw + 1) * tabw);
		else
			str_width++;

		str_size++;
	}
}

int Line::ins_char(int pos, int chr, int num, Buffer* pBuf)
{
    if(num <= 0 || pos < 0)
        return 0;

    char* c = new char[num];

    memset(c, chr, num);

    return ins_char(pos, c, num, pBuf);
}

int Line::ins_char(int pos, PLine src, Buffer* pBuf)
{
    if(!src->size())
        return 0;

    return ins_char(pos, src->str, src->size(), pBuf);
}

int Line::ins_char(int ins_pos, char* chr, int num, Buffer* pBuf)
{
    if(ins_pos < 0 || !chr || num <= 0)
        return 1;

    int slen = len();

    //Make a copy of the existing string

    Line ln = this;

    if(!ln.buf_len)  //Line points to original buffer
        expand_by(slen);

    int new_len = max(slen, ins_pos);
    int tabw = iTabWidth;
    int ins_len = 0;

    for(int i = 0; i < num; i++)
        ins_len += (chr[i] == '\t') ? tabw:1;

    check_size(new_len + ins_len + tabw);

    char *str2 = ln.str;
    char *out  = str;
    int pos = 0;
    int rc = (chr[0] == '\t') ? ((ins_pos / iTabWidth + 1) * iTabWidth - ins_pos) : 1;

    for(pos = 0; pos <= new_len;)
    {
        if(pos >= slen)     //Insertion is past the end of the line
        {
            if(pos == ins_pos)
                while(num--)
                    *out++ = *chr++;
            else
                if(pos < new_len)
                    *out++ = ' ';
            pos++;
            continue;
        }

        if(*str2 == '\t')   //Process TAB
        {
            int fill = pos;

            pos = ((pos / tabw + 1) * tabw);

            if(pos <= ins_pos || fill >= ins_pos)
            {
                if(fill == ins_pos)
                {
                    while(num--)
                        *out++ = *chr++;
                }

                *out++ = *str2++;
                continue;
            }

            while(fill < pos)
            {
                if(fill == ins_pos)
                    while(num--)
                        *out++ = *chr++;

                *out++ = ' ';
                fill++;
            }

            str2++;
            continue;
        }

        if(*str2)       //Usual character
        {
            if(pos == ins_pos)
                while(num--)
                    *out++ = *chr++;

            *out++ = *str2++;
            pos++;
        }
        else
            pos++;
    }

    *out = 0;
    recalc_sz();

    if(pBuf)
    {
        PLine diff = ln.gen_diff(this);
        pBuf->track_line(diff, this);
    }

    return rc;
}

int Line::del_char(int del_pos, int num, Buffer* pBuf)
{
    if(del_pos < 0 || num <= 0)
        return 0;

    int slen = len();

    if(del_pos >= slen)
        return 0;

    Line ln = this;

    if(!ln.buf_len)  //Line points to original buffer
        expand_by(slen);

    int tabw = iTabWidth;

    char *str2 = ln.str;
    char *out  = str;
    int pos = 0;
    int chr = 0;

    for(pos = 0; pos < slen;)
    {
        if(*str2 == '\t')
        {
            int fill = pos;

            pos = ((pos / tabw + 1) * tabw);

            if(pos <= del_pos || fill >= del_pos)
            {
                if(fill == del_pos)
                {
                    chr = *str2++;

                    while(fill++ < pos && --num)
                        del_pos++;
                }
                else
                	*out++ = *str2++;
                continue;
            }

            while(fill < pos)
            {
                if(fill == del_pos)
                {
                    chr = ' ';

                    if(--num)
                        del_pos++;
                }
                else
                    *out++ = ' ';

                fill++;
            }

            str2++;
            continue;
        }

        if(*str2)
        {
            if(pos == del_pos)
            {
                chr = *str2++;

                if(--num)
                    del_pos++;
            }
            else
	            *out++ = *str2++;
            pos++;
        }
        else
            pos++;
    }

    *out = 0;
    recalc_sz();

	if(pBuf)
    {
        PLine diff = ln.gen_diff(this);
        pBuf->track_line(diff, this);
    }

    return chr;
}

PLine Line::split(int split_pos, Buffer* pBuf)
{
    if(split_pos < 0)
        return 0;

    PLine ln = new Line(this);

    del_char(split_pos, len(), pBuf);
    ln->del_char(0, split_pos);

    return ln;
}

int Line::char_at(int from_pos)
{
    if(from_pos >= len())
        return 0;

    char *str2 = str;
    int pos = 0;
    int tabw = iTabWidth;

    for(pos = 0; *str2; str2++)
    {
        if(*str2 == '\t')
        {
            int fill = pos;
            pos = ((pos / tabw + 1) * tabw);

            if(fill <= from_pos && pos > from_pos)
                return '\t';
        }
        else
        {
            if(pos == from_pos)
                return *str2;

            pos++;
        }
    }

    return 0;
}

void Line::build_print(char *buff)
{
    char *str2 = str;
    int pos = 0;
    int tabw = iTabWidth;

    if(str2)
    {
        for(pos = 0; *str2; str2++)
        {
            if(*str2 == '\t')
            {
                int fill = pos;
                pos = ((pos / tabw + 1) * tabw);

                while(fill++ < pos)
                    *buff++ = ' ';
            }
            else
            {
                *buff++ = *str2;
                pos++;
            }
        }
    }
    *buff = 0;
}

void Line::detach()
{
    if(!buf_len)    //str points to a file buffer
    {
        char* save = str;
        int sz     = size();

        expand_by(sz);
        memcpy(str, save, sz+1);
    }
}

void Line::xlat(char *cvt_tbl)
{
    int slen;

	detach();

    for(int x = 0; str[x]; x++)
        str[x] = cvt_tbl[str[x]];
}

void Line::set_hiliting(Parser* parser, IDColl* plist, int& initial_state, int pos)
{
    hl_state = initial_state;

    Line ln(this, 0, len(), 1);

    parser->reset(ln.str, hl_state);

    if(pos < 0)
        reset_type_cache();

    if(pos > 0)
        pos--;

    while(*parser->tok)
    {
        int iSz = parser->next_token();

        if(pos >= parser->offset() && pos < (parser->offset() + iSz))
        {
            token_cache_type  = parser->color;
            token_cache_start = parser->offset();
            token_cache_len   = iSz;
        }

        if(plist && pos < 0 && (iSz >= iMinComplLen) && token_is_completable(parser->color))
        {
            plist->AddID(parser->tok, iSz);
        }

        parser->tok += iSz;
    }

    initial_state = parser->state;
}

void Line::get_first_token(Parser* parser)
{
    parser->reset(str, hl_state);

    while(*parser->tok)
    {
        int iSz = parser->next_token();

        //Smart indent token must be a reserved (standard) word
        if(parser->color == CL_STDWORD)
        	break;

        parser->tok += iSz;
    }
}

int Line::token_type(Parser* parser, int pos)
{
    Line ln(this, 0, len(), 1);

    parser->reset(ln.str, hl_state);

    while(*parser->tok)
    {
        int iSz = parser->next_token();

        if(pos >= parser->offset() && pos < (parser->offset() + iSz))
            return parser->color;

        parser->tok += iSz;
    }
    return CL_DEFAULT;
}

int Line::is_empty(int end_pos)
{
    char *str2 = str;
    int pos = 0;
    int tabw = iTabWidth;

    if(str2)
    {
        for(pos = 0; pos <= end_pos && *str2; str2++)
        {
            if(!__issp(*str2))
                return 0;   //Line is not empty before this pos

            if(*str2 == '\t')
            {
                int fill = pos;
                pos = ((pos / tabw + 1) * tabw);

                if(fill <= end_pos && pos > end_pos)
                    break;
            }
            else
                pos++;
        }
    }

    return 1;
}

int Line::token_valid(int pos)
{
    if(token_cache_type < 0)
        return 0;

    if(token_cache_start < 0)
        return 0;

    if(token_cache_len <= 0)
        return 0;

    if(pos > 0)
        pos--;

    if(pos >= token_cache_start && pos < (token_cache_start + token_cache_len))
        return 1;

    return 0;
}

int Line::token_is_completable(int type)
{
    return (type == CL_IDENT || type == CL_STDWORD || type == CL_FUNCTION) ? 1:0;
}

int Line::indent_offset()
{
    int res  = 0;
    char* in = str;
    int tabw = iTabWidth;

    while(__issp(*in))
        res += (*in++ == '\t') ? tabw:1;

    return res;
}

void Line::indent(PLine src, int offset)
{
    if(!src || offset <= 0)
        return;

    char* in = src->str;
    int pos = 0;

    if(!iIndUseTab)
    {
        ins_char(0, ' ', offset);
        return;
    }

    while(pos < offset)
    {
        if(!__issp(*in))
            break;

        int delta = (*in == '\t') ? iTabWidth:1;

        if((pos + delta) > offset)
            break;

        pos += delta;
        in++;
    }

    if(pos) /* found something to insert */
        ins_char(0, src->str, in - src->str);

    if((offset - pos) >= iTabWidth)
    {
        int ntabs = (offset - pos)/iTabWidth;

        ins_char(pos, '\t', ntabs);

        pos += iTabWidth * ntabs;
    }

    if((offset - pos) > 0)
	    ins_char(pos, ' ', (offset - pos));
}

int Line::get_prev_pos(int from_pos)
{
    if(from_pos <= 1)
        return 0;

    if(from_pos > len())
        return from_pos - 1;

    char *str2 = str;
    int pos = 0;
    int tabw = iTabWidth;

    from_pos--;

    for(pos = 0; *str2; str2++)
    {
        if(*str2 == '\t')
        {
            int fill = pos;
            pos = ((pos / tabw + 1) * tabw);

            if(fill <= from_pos && pos > from_pos)
                return fill;
        }
        else
        {
            if(pos == from_pos)
                return pos;

            pos++;
        }
    }

    return 0;
}

int Line::find_word(int start_pos, int& from, int& to)
{
	if(start_pos >= len())
	    return 0;

	Line ln(this, 0, len(), 1);

	if(!__isic(ln.str[start_pos]))
		return 0;

	while(start_pos)
	{
	    if(__issp(ln.str[start_pos - 1]))
	        break;

	    start_pos--;
	}

	from = start_pos;

	while(start_pos < len())
	{
        if(__issp(ln.str[start_pos]))
            break;

        start_pos++;
    }
    to = start_pos;

    return 1;
}

//-----------------------------------------
// Line diff support
//-----------------------------------------

int Line::diff_width(void)
{
    int sz = 0;

    char* diff  = str;

    if(!diff)
        return 0;

    while(*diff)
    {
        int op  = GET_OP(*diff);
        int cnt = GET_CNT(*diff);

        diff++;

        switch(op)
        {
            case OP_ADD:
            case OP_SUB:
                diff += cnt;
            case OP_EQ:
                sz   += cnt;
                break;
        }
    }

    return sz;
}

static void diff_str(char* s1, char* s2, char* s3)
{
    char* start = s2;
    char* j;
    char* last_op = s3;

    *last_op = 0;

    for(; *s1; s1++)
    {
        int res = 1;

        for(j = start; *j; j++)
        {
            res = (*s1 - *j);

            if(!res)    //Match found
                break;
        }

        if(!res)    //Match found
        {
            //Fill the gap, if any

            if(start < j)
            {
                last_op = s3;
                *s3++ = OP_SUB;

                while(start < j)
                {
                    if(GET_CNT(*last_op) == MAX_CNT)
                    {
                        last_op = s3;
                        *s3++ = OP_SUB;
                    }

                    *s3++ = *start++;
                    *last_op = *last_op + 1;
                }
            }

            if(GET_OP(*last_op) == OP_EQ)
            {
                if(GET_CNT(*last_op) == MAX_CNT)
                {
                    last_op = s3;
                    *s3++ = OP_EQ;
                }
                *last_op = *last_op + 1;
            }
            else
            {
                last_op = s3;
                *s3++ = OP_EQ | 0x01;
            }

            start++;
        }
        else        //No match
        {
            if(GET_OP(*last_op) == OP_ADD)
            {
                if(GET_CNT(*last_op) == MAX_CNT)
                {
                    last_op = s3;
                    *s3++ = OP_ADD;
                }
                *s3++ = *s1;
                *last_op = *last_op + 1;
            }
            else
            {
                last_op = s3;
                *s3++ = OP_ADD | 0x01;
                *s3++ = *s1;
            }
        }
    }

    if(*start)
    {
        last_op = s3;
        *s3++ = OP_SUB;

        while(*start)
        {
            if(GET_CNT(*last_op) == MAX_CNT)
            {
                last_op = s3;
                *s3++ = OP_SUB;
            }

            *s3++ = *start++;
            *last_op = *last_op + 1;
        }
    }

    *s3 = 0;
}

Line* Line::gen_diff(Line* other)
{
    if(!other)
        return 0;

    char* diff = new char[(size() + other->size() + 1) * 2];

	diff_str(str, other->str, diff);

    PLine ln = new Line;

    ln->set(diff);
    delete diff;

    return ln;
}

static int apply(char* diff, char* base, char* dst, int direction)
{
    while(*diff)
    {
        int op  = GET_OP(*diff);
        int cnt = GET_CNT(*diff);

        diff++;

        if(direction)   //reverse
        {
            if(op == OP_SUB)
                op = OP_ADD;
            else
                if(op == OP_ADD)
                    op = OP_SUB;
        }

        switch(op)
        {
            case OP_EQ:
                while(cnt--)
                    *dst++ = *base++;
                break;

            case OP_SUB:
                while(cnt--)
                {
                    diff++;
                    base++;
                }
                break;

            case OP_ADD:
                while(cnt--)
                    *dst++ = *diff++;
                break;
        }
    }
    *dst = 0;

    return 0;
}

int Line::apply_diff(Line* diff, int dir)
{
    if(!diff)
        return -1;

    Line ln(this);

    check_size(diff->diff_width());

	apply(diff->str, ln.str, str, dir);

    recalc_sz();

    return 0;
}

int Line::find_char(int chr)
{
    char *str2 = str;

	while(*str2)
	{
		if(*str2 == chr)
			return 1;

		str2++;
	}

	return 0;
}
