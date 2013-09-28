/**/

/*
@echo off

@wcl386 -oneatx dftables.c
dftables.exe chartables.c
@wcl386 -oneatx -c pcreposix.c maketables.c get.c study.c pcre.c
@wlib -n pcre.lib +pcreposix.obj +maketables.obj +get.obj +study.obj +pcre.obj
*/

parse upper arg mode .

say 'Build PCRE using 'mode' mode'

if mode='WC' | mode = 'WCD' then
do
   '@wcl386 -zq -ox dftables.c'
   '@dftables.exe chartables.c'
/*   '@wcl386 -bm -zq -ox -c pcreposix.c maketables.c get.c study.c pcre.c'*/
   '@wcl386 -bm -zq -ot -c pcreposix.c maketables.c get.c study.c pcre.c'
   '@wlib -q -n pcre.lib +pcreposix.obj +maketables.obj +get.obj +study.obj +pcre.obj'
end

if mode='VA' | mode = 'VAD' then
do
   '@icc dftables.c'
   '@dftables.exe chartables.c'
   '@icc -Gm+ -O -c pcreposix.c maketables.c get.c study.c pcre.c'
   '@del pcre.lib'
   '@ilib /nofr /noi /q pcre.lib +pcreposix.obj +maketables.obj +get.obj +study.obj +pcre.obj;'
end

if mode='EMX' then
do
   '@gcc -static -s dftables.c'
   '@dftables.exe chartables.c'
   '@gcc -O -Zmt -s -static -c pcreposix.c maketables.c get.c study.c pcre.c'
   '@del pcre.a'
   '@ar rc pcre.a pcreposix.o maketables.o get.o study.o pcre.o'
end

