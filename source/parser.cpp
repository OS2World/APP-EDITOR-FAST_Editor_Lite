/*
** Module   :PARSER.CPP
** Abstract :Sintax hiliting language-specific parsers
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Tue  11/03/1997   	Created
**      Sat  12/04/1997   	Added REXX parser
*/

#include <string.h>

#include <fio.h>
#include <parser.h>
#include <version.h>

RegParser<Parser     > reg_NONE("none"  ," ~N~o hiliting ", 0);
RegParser<Parser_CPP > reg_CPP ("C++"   ," ~C~++"         , HI_CPP );
RegParser<Parser_REXX> reg_REXX("REXX"  ," ~R~EXX"        , HI_REXX);
RegParser<Parser_MAKE> reg_MAKE("MAKE"  ," ~M~ake"        , HI_MAKE);
RegParser<Parser_ASM > reg_ASM ("ASM"   ," ~A~sm"         , HI_ASM );
RegParser<Parser_HTML> reg_HTML("HTML"  ," ~H~TML"        , HI_HTML);
RegParser<Parser_MAIL> reg_MAIL("MAIL"  ," Ma~i~l"        , HI_MAIL);
RegParser<Parser_PAS > reg_PAS ("PASCAL"," ~P~ascal"      , HI_PAS );
RegParser<Parser_PL  > reg_PL  ("PERL"  ," P~E~RL"        , HI_PL  );
RegParser<Parser_TEX > reg_TEX ("LATEX" ," ~L~aTeX"       , HI_TEX );

//-----------------------------------------
// Class Parser
//-----------------------------------------

Dictionary *Parser::pKeywords = 0;
Dictionary *Parser::pParsers  = 0;

char Parser::def_tbl[256];
int Parser::tbl_ready = 0;

Parser::Parser()
{
    if(!pKeywords)
    {
        pKeywords = new Dictionary(1,0,0);
        for(int i = 0; keywords[i].key; i++)
            pKeywords->Add(&keywords[i]);
    }

    reset(0, ST_INITIAL);
    preserve_case = 0;
    uses_bracket  = 0;
    mask          = 0;

    if(!tbl_ready)
    {
        for(int j = 0; j < 256; j++)
            def_tbl[j] = j;

        tbl_ready = 1;
    }

    cvt_tbl = def_tbl;

    reset(0,0);
}

//-----------------------------------------
// Static methods for Parser dictionary
//-----------------------------------------

void Parser::RegParser(RegRecord* pReg)
{
    if(!pParsers)
        pParsers = new Dictionary(1, 0, 0);

    pParsers->Add(pReg);
}

Parser* Parser::GenParser(int key)
{
    if(!pParsers)
        return 0;

    for(int i = 0; i < pParsers->Count(); i++)
    {
        RegRecord* pReg = (RegRecord*)pParsers->Get(i);

        if(pReg->iType == key)
            return pReg->pGen();
    }

    return 0;
}

Parser* Parser::GenParser(char* key)
{
    if(!pParsers)
        return 0;

    RegRecord* pReg = (RegRecord*)pParsers->IsIn(key, 0);

    if(pReg)
        return pReg->pGen();

    return 0;
}

int Parser::Count()
{
    if(!pParsers)
        return 0;

    return pParsers->Count();
}

char* Parser::Name(int iNdx, int iHint)
{
    if(!pParsers)
        return 0;

    RegRecord* pReg = (RegRecord*)pParsers->Get(iNdx);

    if(pReg)
        return (iHint) ? pReg->cHName:pReg->cName;

    return 0;
}

int Parser::Type(int iNdx)
{
    if(!pParsers)
        return 0;

    RegRecord* pReg = (RegRecord*)pParsers->Get(iNdx);

    if(pReg)
        return pReg->iType;

    return 0;
}

char* Parser::NameByKey(int key, int iHint)
{
    if(!pParsers)
        return 0;

    for(int i = 0; i < pParsers->Count(); i++)
    {
        RegRecord* pReg = (RegRecord*)pParsers->Get(i);

        if(pReg->iType == key)
            return (iHint) ? pReg->cHName:pReg->cName;
    }

    return "";
}

int Parser::Index(char* key)
{
    if(!pParsers)
        return 0;

    RegRecord* pReg = (RegRecord*)pParsers->IsIn(key, 0);

    if(pReg)
        return pReg->iType;

    return 0;
}

int Parser::GuessType(char *name)
{
    char *str;
    char *ptr;
    int mode = 0;
    char mapname[FED_MAXPATH];

    for(ptr = str = hi_map; *str;)
    {
        char chr;

        while(*str && *str != '\n')
            str++;

        chr = *str;
        *str = 0;

        //String ready, parse it
        char *dots = strchr(ptr, ':');

        if(dots)
        {
            *dots = 0;

            while(__issp(*ptr))
                ptr++;

            mode = Index(ptr);

            *dots = ':';

            if(mode)
            {
                ptr = dots + 1;
                while(*ptr)
                {
                    int i = 0;

                    while(__issp(*ptr))
                        ptr++;

                    for(; *ptr && *ptr != ';' && i < FED_MAXPATH; i++, ptr++)
                    {
                        mapname[i] = *ptr;
                    }
                    mapname[i] = 0;

                    if(match_name(name, mapname))
                    {
                        *str = chr;
                        return mode;
                    }

                    if(*ptr)
                        ptr++;
                }
            }
        }

        *str = chr;

        if(*str)
            str++;

        ptr = str;
        mode = 0;
    }

    return mode;
}

//-----------------------------------------
// Regular methods
//-----------------------------------------

void Parser::reset(char *token, int initial_state)
{
    flags   = 0;
    tok_len = 0;
    color   = 0;
    state   = initial_state;
    tok     = token;
    old_tok = token;
    start   = token;
}

int Parser::next_token()
{
    old_tok = tok;
    tok_len = 0;
    color = CL_DEFAULT;
    char *tmp = tok;
    while(*tmp)
    {
        tmp++;
        tok_len++;
    }
    return tok_len;
}

int Parser::is_kwd()
{
    Pkwd pKwd = (Pkwd) pKeywords->IsIn(tok, tok_len, preserve_case);

    if(pKwd && (pKwd->mask & mask))
    {
        flags = pKwd->mask;

        if(pKwd->no_ind & mask)
            flags &= ~MASK_INDENT;

        if(pKwd->no_auto & mask)
            flags &= ~MASK_AUTOUNINDENT;

        return 1;
    }

    flags = 0;

    return 0;
}

//-----------------------------------------
// Static initializer for dictionary of keywords
//-----------------------------------------

kwdPair keywords[] =
{
    { "AUTOLOAD"        , MASK_PL},
    { "BEGIN"           , MASK_PL},
    { "CHECK"           , MASK_PL},
    { "CORE"            , MASK_PL},
    { "DESTROY"         , MASK_PL},
    { "END"             , MASK_PL},
    { "EQ"              , MASK_PL},
    { "GE"              , MASK_PL},
    { "GT"              , MASK_PL},
    { "INIT"            , MASK_PL},
    { "LE"              , MASK_PL},
    { "LT"              , MASK_PL},
    { "NE"              , MASK_PL},
    { "NULL"            , MASK_PL},
    { "__DATA__"        , MASK_PL},
    { "__END__"         , MASK_PL},
    { "__FILE__"        , MASK_PL},
    { "__LINE__"        , MASK_PL},
    { "__PACKAGE__"     , MASK_PL},

    { "::method"        , MASK_REXX},
    { "::class"         , MASK_REXX},
    { "::requires"      , MASK_REXX},

    { "abs"             , MASK_PL   },
    { "absolute"        , MASK_PAS  },
    { "abstract"        , MASK_JAVA },
    { "accept"          , MASK_PL   },
    { "address"         , MASK_REXX },
    { "alarm"           , MASK_PL   },
    { "all"             , MASK_REXX },
    { "and"             , MASK_PAS | MASK_PL},
    { "arg"             , MASK_REXX },
    { "array"           , MASK_PAS  },
    { "asm"             , MASK_PAS  },
    { "assembler"       , MASK_PAS  },
    { "atan2"           , MASK_PL   },
    { "auto"            , MASK_CPP  },
    { "begin"           , MASK_INDENT | MASK_UNINDENT_C | MASK_PAS},
    { "bind"            , MASK_PL   },
    { "binmode"         , MASK_PL   },
    { "bless"           , MASK_PL   },
    { "bool"            , MASK_CPP  },
    { "boolean"         , MASK_JAVA },
    { "break"           , MASK_CPP | MASK_JAVA},
    { "by"              , MASK_REXX},
    { "byte"            , MASK_ASM | MASK_JAVA},
    { "call"            , MASK_ASM | MASK_REXX},
    { "caller"          , MASK_PL},
    { "case"            , MASK_INDENT | MASK_CPP | MASK_PAS | MASK_JAVA},
    { "catch"           , MASK_INDENT | MASK_CPP | MASK_JAVA},
    { "char"            , MASK_CPP },
    { "chdir"           , MASK_PL},
    { "chmod"           , MASK_PL},
    { "chomp"           , MASK_PL},
    { "chop"            , MASK_PL},
    { "chown"           , MASK_PL},
    { "chr"             , MASK_PL},
    { "chroot"          , MASK_PL},
    { "class"           , MASK_AUTOUNINDENT | MASK_INDENT | MASK_CPP | MASK_JAVA},
    { "close"           , MASK_PL},
    { "closedir"        , MASK_PL},
    { "cmp"             , MASK_PL},
    { "commands"        , MASK_REXX},
    { "connect"         , MASK_PL},
    { "const"           , MASK_CPP | MASK_PAS | MASK_JAVA},
    { "constructor"     , MASK_PAS },
    { "continue"        , MASK_CPP | MASK_JAVA | MASK_PL},
    { "cos"             , MASK_PL},
    { "crypt"           , MASK_PL},
    { "dbmclose"        , MASK_PL},
    { "dbmopen"         , MASK_PL},
    { "default"         , MASK_INDENT | MASK_CPP | MASK_JAVA},
    { "define"          , MASK_CPP },
    { "defined"         , MASK_PL},
    { "delete"          , MASK_CPP | MASK_PL},
    { "destructor"      , MASK_PAS },
    { "die"             , MASK_PL},
    { "digits"          , MASK_REXX},
    { "div"             , MASK_PAS },
    { "do"              , MASK_AUTOUNINDENT | MASK_INDENT | MASK_UNINDENT_C | MASK_REXX | MASK_CPP | MASK_PAS | MASK_JAVA | MASK_PL,
                          0, MASK_REXX | MASK_PAS, 0, MASK_CPP | MASK_JAVA | MASK_PL},
    { "double"          , MASK_CPP | MASK_JAVA},
    { "downto"          , MASK_PAS },
    { "drop"            , MASK_REXX},
    { "dump"            , MASK_PL},
    { "dword"           , MASK_ASM },
    { "each"            , MASK_PL},
    { "else"            , MASK_AUTOUNINDENT | MASK_INDENT | MASK_REXX | MASK_CPP | MASK_PAS | MASK_JAVA | MASK_PL},
    { "elsif"           , MASK_INDENT | MASK_PL},
    { "end"             , MASK_UNINDENT | MASK_REXX | MASK_ASM | MASK_PAS},
    { "endgrent"        , MASK_PL},
    { "endhostent"      , MASK_PL},
    { "endif"           , MASK_CPP },
    { "endnetent"       , MASK_PL},
    { "endp"            , MASK_ASM },
    { "endprotoent"     , MASK_PL},
    { "endpwent"        , MASK_PL},
    { "ends"            , MASK_ASM },
    { "endservent"      , MASK_PL},
    { "engineering"     , MASK_REXX},
    { "entry"           , MASK_CPP },
    { "enum"            , MASK_INDENT | MASK_CPP },
    { "eof"             , MASK_PL},
    { "eq"              , MASK_PL},
    { "error"           , MASK_REXX},
    { "errors"          , MASK_REXX},
    { "eval"            , MASK_PL},
    { "exec"            , MASK_PL},
    { "exists"          , MASK_PL},
    { "exit"            , MASK_REXX | MASK_PL},
    { "exp"             , MASK_PL},
    { "expose"          , MASK_REXX},
    { "extends"         , MASK_JAVA},
    { "extern"          , MASK_CPP },
    { "external"        , MASK_PAS },
    { "false"           , MASK_CPP | MASK_JAVA},
    { "far"             , MASK_PAS },
    { "fcntl"           , MASK_PL},
    { "file"            , MASK_PAS },
    { "fileno"          , MASK_PL},
    { "final"           , MASK_JAVA},
    { "finally"         , MASK_JAVA},
    { "float"           , MASK_CPP | MASK_JAVA},
    { "flock"           , MASK_PL},
    { "for"             , MASK_AUTOUNINDENT | MASK_INDENT | MASK_CPP | MASK_PAS | MASK_JAVA | MASK_PL},
    { "foreach"         , MASK_PL},
    { "forever"         , MASK_REXX},
    { "fork"            , MASK_PL},
    { "form"            , MASK_REXX},
    { "format"          , MASK_PL},
    { "formline"        , MASK_PL},
    { "forward"         , MASK_PAS },
    { "friend"          , MASK_CPP },
    { "function"        , MASK_PAS },
    { "fuzz"            , MASK_REXX},
    { "ge"              , MASK_PL},
    { "getc"            , MASK_PL},
    { "getgrent"        , MASK_PL},
    { "getgrgid"        , MASK_PL},
    { "getgrnam"        , MASK_PL},
    { "gethostbyaddr"   , MASK_PL},
    { "gethostbyname"   , MASK_PL},
    { "gethostent"      , MASK_PL},
    { "getlogin"        , MASK_PL},
    { "getnetbyaddr"    , MASK_PL},
    { "getnetbyname"    , MASK_PL},
    { "getnetent"       , MASK_PL},
    { "getpeername"     , MASK_PL},
    { "getpgrp"         , MASK_PL},
    { "getppid"         , MASK_PL},
    { "getpriority"     , MASK_PL},
    { "getprotobyname"  , MASK_PL},
    { "getprotobynumber", MASK_PL},
    { "getprotoent"     , MASK_PL},
    { "getpwent"        , MASK_PL},
    { "getpwnam"        , MASK_PL},
    { "getpwuid"        , MASK_PL},
    { "getservbyname"   , MASK_PL},
    { "getservbyport"   , MASK_PL},
    { "getservent"      , MASK_PL},
    { "getsockname"     , MASK_PL},
    { "getsockopt"      , MASK_PL},
    { "glob"            , MASK_PL},
    { "gmtime"          , MASK_PL},
    { "goto"            , MASK_CPP | MASK_PAS | MASK_JAVA | MASK_PL},
    { "grep"            , MASK_PL},
    { "gt"              , MASK_PL},
    { "halt"            , MASK_REXX},
    { "hex"             , MASK_PL},
    { "if"              , MASK_AUTOUNINDENT | MASK_INDENT | MASK_REXX | MASK_CPP | MASK_PAS | MASK_JAVA | MASK_PL},
    { "ifdef"           , MASK_CPP },
    { "ifndef"          , MASK_CPP },
    { "implementation"  , MASK_PAS },
    { "implements"      , MASK_JAVA},
    { "import"          , MASK_JAVA},
    { "in"              , MASK_PAS },
    { "include"         , MASK_CPP },
    { "index"           , MASK_PL},
    { "inherited"       , MASK_PAS },
    { "inline"          , MASK_CPP | MASK_PAS},
    { "instanceof"      , MASK_JAVA},
    { "int"             , MASK_CPP | MASK_JAVA | MASK_PL},
    { "interface"       , MASK_PAS | MASK_JAVA},
    { "intermediates"   , MASK_REXX},
    { "interpret"       , MASK_REXX},
    { "interrupt"       , MASK_PAS },
    { "ioctl"           , MASK_PL},
    { "iterate"         , MASK_REXX},
    { "join"            , MASK_PL},
    { "keys"            , MASK_PL},
    { "kill"            , MASK_PL},
    { "label"           , MASK_ASM | MASK_PAS},
    { "labels"          , MASK_REXX},
    { "last"            , MASK_PL},
    { "lc"              , MASK_PL},
    { "lcfirst"         , MASK_PL},
    { "le"              , MASK_PL},
    { "leave"           , MASK_REXX},
    { "length"          , MASK_PL},
    { "link"            , MASK_PL},
    { "listen"          , MASK_PL},
    { "local"           , MASK_PL},
    { "localtime"       , MASK_PL},
    { "lock"            , MASK_PL},
    { "log"             , MASK_PL},
    { "long"            , MASK_CPP | MASK_JAVA},
    { "lstat"           , MASK_PL},
    { "lt"              , MASK_PL},
    { "m"               , MASK_PL},
    { "map"             , MASK_PL},
    { "mkdir"           , MASK_PL},
    { "mod"             , MASK_PAS },
    { "msgctl"          , MASK_PL},
    { "msgget"          , MASK_PL},
    { "msgrcv"          , MASK_PL},
    { "msgsnd"          , MASK_PL},
    { "my"              , MASK_PL},
    { "native"          , MASK_JAVA},
    { "ne"              , MASK_PL},
    { "near"            , MASK_ASM | MASK_PAS},
    { "negative"        , MASK_REXX},
    { "new"             , MASK_CPP | MASK_JAVA},
    { "next"            , MASK_PL},
    { "nil"             , MASK_REXX | MASK_PAS },
    { "no"              , MASK_PL},
    { "nop"             , MASK_REXX},
    { "normal"          , MASK_REXX},
    { "not"             , MASK_PAS | MASK_PL},
    { "novalue"         , MASK_REXX},
    { "null"            , MASK_JAVA},
    { "numeric"         , MASK_REXX},
    { "object"          , MASK_PAS },
    { "oct"             , MASK_PL},
    { "of"              , MASK_PAS },
    { "off"             , MASK_REXX},
    { "offset"          , MASK_ASM },
    { "on"              , MASK_REXX},
    { "open"            , MASK_PL},
    { "opendir"         , MASK_PL},
    { "or"              , MASK_PAS | MASK_PL},
    { "ord"             , MASK_PL},
    { "otherwise"       , MASK_INDENT | MASK_REXX},
    { "our"             , MASK_PL},
    { "pack"            , MASK_PL},
    { "package"         , MASK_JAVA | MASK_PL},
    { "packed"          , MASK_PAS },
    { "parse"           , MASK_REXX},
    { "pipe"            , MASK_PL},
    { "pop"             , MASK_PL},
    { "pos"             , MASK_PL},
    { "print"           , MASK_PL},
    { "printf"          , MASK_PL},
    { "private"         , MASK_INDENT | MASK_CPP | MASK_PAS | MASK_JAVA},
    { "proc"            , MASK_INDENT | MASK_ASM },
    { "procedure"       , MASK_INDENT | MASK_AUTOUNINDENT | MASK_REXX | MASK_PAS,
                          0, MASK_REXX},
    { "program"         , MASK_PAS },
    { "protected"       , MASK_INDENT | MASK_CPP | MASK_JAVA},
    { "prototype"       , MASK_PL},
    { "ptr"             , MASK_ASM },
    { "public"          , MASK_INDENT | MASK_CPP | MASK_PAS | MASK_JAVA},
    { "pull"            , MASK_REXX},
    { "push"            , MASK_REXX | MASK_PL},
    { "q"               , MASK_PL},
    { "qq"              , MASK_PL},
    { "qr"              , MASK_PL},
    { "queue"           , MASK_REXX},
    { "quotemeta"       , MASK_PL},
    { "qw"              , MASK_PL},
    { "qword"           , MASK_ASM },
    { "qx"              , MASK_PL},
    { "rand"            , MASK_PL},
    { "read"            , MASK_PL},
    { "readdir"         , MASK_PL},
    { "readline"        , MASK_PL},
    { "readlink"        , MASK_PL},
    { "readpipe"        , MASK_PL},
    { "record"          , MASK_PAS },
    { "recv"            , MASK_PL},
    { "redo"            , MASK_PL},
    { "ref"             , MASK_PL},
    { "register"        , MASK_CPP },
    { "rename"          , MASK_PL},
    { "repeat"          , MASK_PAS },
    { "require"         , MASK_PL},
    { "reset"           , MASK_PL},
    { "results"         , MASK_REXX},
    { "return"          , MASK_REXX | MASK_CPP | MASK_JAVA | MASK_PL},
    { "reverse"         , MASK_PL},
    { "rewinddir"       , MASK_PL},
    { "rindex"          , MASK_PL},
    { "rmdir"           , MASK_PL},
    { "s"               , MASK_PL},
    { "say"             , MASK_REXX},
    { "scalar"          , MASK_PL},
    { "scan"            , MASK_REXX},
    { "scientific"      , MASK_REXX},
    { "seek"            , MASK_PL},
    { "seekdir"         , MASK_PL},
    { "segment"         , MASK_ASM },
    { "select"          , MASK_INDENT | MASK_REXX | MASK_PL},
    { "self"            , MASK_REXX | MASK_PAS},
    { "semctl"          , MASK_PL},
    { "semget"          , MASK_PL},
    { "semop"           , MASK_PL},
    { "send"            , MASK_PL},
    { "set"             , MASK_PAS },
    { "setgrent"        , MASK_PL},
    { "sethostent"      , MASK_PL},
    { "setnetent"       , MASK_PL},
    { "setpgrp"         , MASK_PL},
    { "setpriority"     , MASK_PL},
    { "setprotoent"     , MASK_PL},
    { "setpwent"        , MASK_PL},
    { "setservent"      , MASK_PL},
    { "setsockopt"      , MASK_PL},
    { "shift"           , MASK_PL},
    { "shl"             , MASK_PAS },
    { "shmctl"          , MASK_PL},
    { "shmget"          , MASK_PL},
    { "shmread"         , MASK_PL},
    { "shmwrite"        , MASK_PL},
    { "short"           , MASK_CPP | MASK_JAVA},
    { "shr"             , MASK_PAS },
    { "shutdown"        , MASK_PL},
    { "sigl"            , MASK_REXX},
    { "signal"          , MASK_REXX},
    { "signed"          , MASK_CPP },
    { "sin"             , MASK_PL},
    { "sizeof"          , MASK_CPP },
    { "sleep"           , MASK_PL},
    { "socket"          , MASK_PL},
    { "socketpair"      , MASK_PL},
    { "sort"            , MASK_PL},
    { "splice"          , MASK_PL},
    { "split"           , MASK_PL},
    { "sprintf"         , MASK_PL},
    { "sqrt"            , MASK_PL},
    { "srand"           , MASK_PL},
    { "stat"            , MASK_PL},
    { "static"          , MASK_CPP | MASK_JAVA},
    { "string"          , MASK_PAS },
    { "struc"           , MASK_INDENT | MASK_ASM },
    { "struct"          , MASK_CPP },
    { "study"           , MASK_PL},
    { "sub"             , MASK_PL},
    { "substr"          , MASK_PL},
    { "super"           , MASK_JAVA},
    { "switch"          , MASK_AUTOUNINDENT | MASK_INDENT | MASK_CPP | MASK_JAVA},
    { "symlink"         , MASK_PL},
    { "synchronized"    , MASK_JAVA},
    { "syntax"          , MASK_REXX},
    { "syscall"         , MASK_PL},
    { "sysopen"         , MASK_PL},
    { "sysread"         , MASK_PL},
    { "sysseek"         , MASK_PL},
    { "system"          , MASK_PL},
    { "syswrite"        , MASK_PL},
    { "tell"            , MASK_PL},
    { "telldir"         , MASK_PL},
//    { "then"            , MASK_AUTOUNINDENT | MASK_INDENT | MASK_REXX | MASK_PAS},
    { "then"            , MASK_AUTOUNINDENT | MASK_INDENT | MASK_UNINDENT | MASK_REXX | MASK_PAS},
    { "this"            , MASK_CPP | MASK_JAVA},
    { "throw"           , MASK_CPP | MASK_JAVA},
    { "throws"          , MASK_JAVA},
    { "tie"             , MASK_PL},
    { "tied"            , MASK_PL},
    { "time"            , MASK_PL},
    { "times"           , MASK_PL},
    { "to"              , MASK_REXX | MASK_PAS},
    { "tr"              , MASK_PL},
    { "trace"           , MASK_REXX},
    { "transient"       , MASK_JAVA},
    { "true"            , MASK_CPP | MASK_JAVA},
    { "truncate"        , MASK_PL},
    { "try"             , MASK_INDENT | MASK_CPP | MASK_JAVA},
    { "type"            , MASK_PAS },
    { "typedef"         , MASK_CPP },
    { "uc"              , MASK_PL},
    { "ucfirst"         , MASK_PL},
    { "umask"           , MASK_PL},
    { "undef"           , MASK_PL},
    { "union"           , MASK_CPP },
    { "unit"            , MASK_PAS },
    { "unless"          , MASK_PL},
    { "unlink"          , MASK_PL},
    { "unpack"          , MASK_PL},
    { "unshift"         , MASK_PL},
    { "unsigned"        , MASK_CPP },
    { "untie"           , MASK_PL},
    { "until"           , MASK_REXX | MASK_PAS | MASK_PL},
    { "upper"           , MASK_REXX},
    { "use"             , MASK_REXX | MASK_PL},
    { "uses"            , MASK_PAS },
    { "utime"           , MASK_PL},
    { "value"           , MASK_REXX},
    { "values"          , MASK_PL},
    { "var"             , MASK_REXX | MASK_PAS},
    { "vec"             , MASK_PL},
    { "virtual"         , MASK_CPP | MASK_PAS },
    { "void"            , MASK_CPP | MASK_JAVA},
    { "volatile"        , MASK_CPP | MASK_JAVA},
    { "wait"            , MASK_PL},
    { "waitpid"         , MASK_PL},
    { "wantarray"       , MASK_PL},
    { "warn"            , MASK_PL},
    { "when"            , MASK_REXX},
    { "while"           , MASK_AUTOUNINDENT | MASK_INDENT | MASK_REXX | MASK_CPP | MASK_PAS | MASK_JAVA | MASK_PL,
                          MASK_REXX, MASK_REXX},
    { "with"            , MASK_REXX | MASK_PAS},
    { "word"            , MASK_ASM },
    { "write"           , MASK_PL},
    { "x"               , MASK_PL},
    { "xor"             , MASK_PAS | MASK_PL},
    { "y"               , MASK_PL},
    {0,0}
};

