/*
** Module   :OUT.CMD
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Sat  25/07/1998 Created
**
*/
'@echo off'
'lxlite .\obj\*.exe'

fin='.\include\version.h'
arcname=''

d0='0'
d1='0'
d2='0'

do while lines(fin) > 0
    parse value linein(fin) with def varname varvalue rest
    if strip(def) = '#define' & strip(varvalue)<>''
    then do
        varvalue=strip(varvalue)
        parse value varname with prefix'_'suffix rest
        if suffix='MAJ'
            then d0=varvalue;
        if suffix='MID'
            then d1=varvalue;
        if suffix='MIN'
            then d2=varvalue;
    end
end
call stream fin, 'c', 'close'

arcname='fed'||d0||d1||d2's.zip'

'noea * /R'
'zip -r 'arcname' * -x *.obj -x *.o -x *.a -x *.rar -x *.zip -x *.map -x *.lib -x *.bak -x *.@* -x CVS\* -x *CVS\* -x misc\* -x obj\*. -x pcre\*.exe'

