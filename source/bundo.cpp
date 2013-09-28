/*
** Module   :BUNDO.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  15/03/1998     Created
**      Fri  13/06/2003     Removed block processing code (now in block.cpp)
*/

#include <string.h>

#include <buffer.h>
#include <version.h>

//----------------------------------------------------------------------
// Undo processing routines
//
//----------------------------------------------------------------------

void Buffer::track_beg()
{
    set_tracking(1);
}

void Buffer::track_end()
{
    if(!get_tracking())
        return;
    if(!track_head)
    {
        set_tracking(0);
        return;
    }

    trackinfo *head = track_head; //Save track
    track_head = undobuff;        //spoof 'track'

    track(opAction, head);

    undobuff   = track_head;
    track_head = 0;
    set_tracking(0);
    undo_count++;
}

void Buffer::track_cancel()
{
    if(track_head) //cleanup here
    {
        while(track_head)
        {
            switch(track_head->op)
            {
                case opInsBlock:
                    delete (Buffer *)track_head->arg1;
                    break;
                case opRestoreLine:
                case opInsLine:
                case opUpdateLine:
                    Free(track_head->arg1);
                    break;
            }
            trackinfo *tmp = track_head;
            track_head = track_head->next;
            delete tmp;
        }
    }
    set_tracking(0);
}

void Buffer::track(int op, void *arg1, void *arg2)
{
    if(!get_tracking())
    {
		if(op == opInsBlock)
			delete (Buffer *)arg1;
        return;
    }

    trackinfo *item = new trackinfo;

    item->next = track_head;
    item->op   = op;
    item->arg1 = arg1;
    item->arg2 = arg2;

    track_head = item;

    switch(op)
    {
        case opRestoreLine:
        case opInsLine:
            // In these cases arg1 contains pointer to line,
            // we need to store a copy
            item->arg1 = new Line(PLine(arg1));
            break;
    }
}

int Buffer::is_cursor_only()
{
    trackinfo *action = undobuff;

    if(!action)
        return 0;

    action = (trackinfo *)action->arg1;

    int rc = 1;

    while(action)
    {
        if(action->op != opCursor)
        {
            rc = 0;
            break;
        }
        action = action->next;
    }

    return rc;
}

void Buffer::undo()
{
    trackinfo *item = undobuff;
    int need_recalc = 0;

    if(!item)
        return;

	undobuff = item->next;

    trackinfo *action = (trackinfo *)item->arg1;

    while(action)
    {
        // Do action
        switch(action->op)
        {
            case opCursor:
                switch((int)action->arg1)
                {
                    case opCurRow:
                        cur_row = (int)action->arg2;
                        break;

                    case opCurCol:
                        cur_col = (int)action->arg2;
                        break;

                    case opStartRow:
                        start_row = (int)action->arg2;
                        break;

                    case opStartCol:
                        start_col = (int)action->arg2;
                        break;
                }
                break;

            case opHiliting:
                hiliting = (int)action->arg1;
                need_recalc = 1;
                break;

            case opSetFlags:
                set_flags((unsigned)action->arg1);
                break;

            case opDelLine:
                Free(Remove((unsigned)action->arg1));
				line_removed((int)action->arg1);
                need_recalc = 1;
                break;

            case opInsBlock:
                {
                    Buffer* tmp_buf = (Buffer *)action->arg1;
                    int start_pos   = (int)action->arg2;
					line_added(start_pos, tmp_buf->Count());
                    move_items(tmp_buf, start_pos);
                    delete tmp_buf;

                    need_recalc = 1;
                }
                break;

			case opUpdateLine:
				abs_line()->apply_diff(PLine(action->arg1), 0);
				//PLine(action->arg2)->apply_diff(PLine(action->arg1), 0);
				delete PLine(action->arg1);
				need_recalc = 1;
				break;

            case opInsLine:
                At(action->arg1, (unsigned) action->arg2);
				line_added((int)action->arg2);
                need_recalc = 1;
                break;

            case opRestoreLine:
                Free(Remove((unsigned)action->arg2));
                At(action->arg1,(unsigned)action->arg2);
				line_added((int)action->arg2);
                need_recalc = 1;
                break;

            case opMarkPos:
                old_abs_col = (int)action->arg1;
                old_abs_row = (int)action->arg2;
                break;
        }

        trackinfo *temp = action->next;

        delete action;
        action = temp;
    }

    delete item;
    undo_count--;

    if(undo_count < 0)
        undo_count = 0;

    if(hiliting && need_recalc)
        fill_hiliting(0, ST_INITIAL);
}

int Buffer::get_undo_count()
{
    return undo_count;
}

void Buffer::clear_undobuff()
{
    while(undobuff)
    {
        trackinfo *item = undobuff;

        undobuff = item->next;
        trackinfo *action = (trackinfo *)item->arg1;

        while(action)
        {
            // Cleanup action item

            switch(action->op)
            {
                case opRestoreLine:
                case opInsLine:
                    Free(action->arg1);
                    break;
            }

            trackinfo *temp = action->next;

            delete action;
            action = temp;
        }
        delete item;
    }
    undo_count = 0;
}

int Buffer::undo_size()
{
	int sz = 0;

    trackinfo *item = undobuff;

    while(item)
	{
	    trackinfo *action = (trackinfo *)item->arg1;

		int rc = 1;

		while(action)
    	{
			sz ++;	//opcode
    		switch(action->op)
        	{
				case opHiliting:
				case opDelLine:
				case opSetFlags:
					sz += sizeof(unsigned);
					break;

				case opMarkPos:
				case opCursor:
					sz += sizeof(int) * 2;
					break;

				case opInsLine:
				case opUpdateLine:
				case opRestoreLine:
					sz += sizeof(int) * 2;
					sz += PLine(action->arg1)->size();
					break;

				case opInsBlock:
					{
						sz += sizeof(int) * 2; //Start pos and number of lines

						Buffer* pBuff = (Buffer *)action->arg1;

						for(int i = 0 ; i < pBuff->Count(); i++)
							sz += pBuff->line(i)->size() + sizeof(int);
					}
					break;
			}
			action = action->next;
        }

        item = item->next;
    }

    return sz;
}
