/*
** Module   :BACKUP.CMD
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
** Last Update :Fri  12-04-96
*/
'@echo off'

parse arg opt rest

if translate(opt)='EA' then
do
    'noea * /r'
end

parse value time() with hh ':' mn ':' ss

filename=date(sorted)'-'hh||mn||ss'.RAR'
say filename
'rar32 a -r -std -mdG -se -m5 -xout\* -x*.ics -x*.lib -x*.bak -x*.exe -x*.obj -x*.o -x*.a -x*.map -x*.dll -x*.rar -x*.zip -x*.pch -x*.sy* -x*.res -x*.idb -xCVS\* -x*CVS\* -xobj\*. -x*.@* .\backup\'filename '*'

