/*
** Module   :BSEARCH.CPP
** Abstract :Search routine
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  15/03/1998     Created
*/

#include <stdlib.h>

#include <buffer.h>
#include <version.h>
#include <_regex.h>
#include <_search.h>

#define UNDO    1

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

//-----------------------------------------
// Local helper class
//-----------------------------------------

struct match_pair
{
	int start;
	int len;

	match_pair(int s = 0, int l = 0):start(s),len(l) { }
};

class MatchCollection : public Collection
{
		match_pair guard;
	public:

		MatchCollection():Collection(10, 10) {}

		match_pair& operator[] (unsigned ndx) { return (ndx < Count()) ? *((match_pair*)Get(ndx)):guard;}
};

//----------------------------------------------------------------------
// Search routines
//
//----------------------------------------------------------------------

int Buffer::search(char* flags, char *pattern, char* repl)
{
    SearchEngine *s = 0;
    char *str = flags;
    int s_row = abs_row();
    int e_row = Count();
    int incr  = 1;
    int replace = 0;
    int ignore_case = 0;
    int i;

    if(!pattern || !pattern[0])
        return 0;

    while(*str)
    {
        switch(__to_upper(*str))
        {
            case 'E':
                if(!s)
                	s = new RXSearch;
                break;

            case 'I':
                if(!s)
                    s = new BMHISearch;

                ignore_case = 1;
                break;
/*
            case 'G':
                s_row = 0;
                e_row = Count();
                break;
*/
            case 'B':
                e_row = -1;

                if(s_row == 0)
                    s_row = abs_row();

                incr  = -1;
                break;
        }
        str++;
    }

    if(!s)
        s = new BMHSearch;

    str = 0;

    if(s->init(pattern, ignore_case))
    {
        delete s;
        return 0;
    }

	MatchCollection list;

    for(i = s_row; i != e_row; i += incr)
    {
        int match_len = 0;

        Line ln(line(i), 0, line(i)->len(), 1);

        if(!ln.len())
            continue;

        ln.xlat(cp_out);

        s->middle(0);

        int first_time = 1;
        char* start = s->search(ln.str, match_len);

        if(!start)
        	continue;

		//Collect start column for matching lines
		list.RemoveAll();

        s->middle(1);

		do
		{
			list.Add(new match_pair(start - ln.str, match_len));
			start = s->search(start + match_len, match_len);
		}
		while(start);

		int offset = (incr > 0) ? 0:ln.len();

		if(i == abs_row())
		{
			offset = abs_col();

			if(incr > 0 && found_len && found_row == i && found_col == offset)
				offset += found_len;

			if(incr < 0)
			{
				if(offset)
					offset--;
				else
				{
					//We're at the beginning of the string, so we
					//can't match anything if backward search is performed
					continue;
				}
			}
		}

		int found = -1;

		if(incr > 0)
		{
			//Forward search
			for(int j = 0; j < list.Count(); j++)
			{
				if(list[j].start >= offset)
				{
					found = j;
					break;
				}
			}
		}
		else
		{
			//Backward search
			for(int j = list.Count() - 1; j >= 0; j--)
			{
				if(list[j].start <= offset)
				{
					found = j;
					break;
				}
			}
		}

		if(found < 0)
			continue;

		if(!list[found].start)
			s->middle(0);

		//Refresh internal variables to make replace work properly
		start = s->search(ln.str + list[found].start, match_len);

		if(!start)
			continue;

		goto_line(i);
		goto_col(list[found].start);
		set_found(i, list[found].start, match_len);

		//Update replace string
		s->subst(repl, ln.str + list[found].start, &rep_string);

		rep_string.xlat(cp_in);

		delete s;

		return match_len;
    }

    delete s;

    return 0;
}

int Buffer::replace()
{
    if(abs_col() != found_col || abs_row() != found_row)
       return 0;

    set_changed(1);

    track(opRestoreLine,(void *)abs_line(),(void *)abs_row());
    abs_line()->del_char(abs_col(), found_len);

    abs_line()->ins_char(abs_col(), &rep_string);

    check_hiliting();

    found_len = rep_string.len();

    return 1;
}

