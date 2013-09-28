/**/
'@echo off'

parse arg key fname
tex='vtex.exe'

select
    when key='ps'  then command="-ov -q -n1 -pu0 -ofps -$s(g=a @latex"
/*    when key='pdf' then command="-ov -ox2 -ob2 -n1 -pu0 -ofpdf -$p(h=29.7cm,w=21cm,f=a,t=b,g=a @latex"*/
    when key='pdf' then command="-ov -ox2 -ob2 -n1 -pu0 -ofpdf -$p(h=29.7cm,w=21cm,f=cf,t=f,g=f @latex"
    when key='dvi' then command="-q -n1 -pu0 -$d(g=a @latex"
    otherwise return -1;
end
'@vtex' command fname

