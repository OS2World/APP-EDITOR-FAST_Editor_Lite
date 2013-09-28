                            FAST Editor Lite

                          Open Source Release

                             Version v2.xx

                Copyright (C) Sergey I. Yevtushenko, 1997-2004


  INTRODUCTION
   ‹‹‹‹‹‹‹‹‹‹‹‹€

    FED is VIO mode programmers editor. It's fast, small in size
    and flexible. Some FED features are listed below:

    - CUA style blocks
    - Full UNDO
    - Stream and rectangular blocks
    - Auto indent
    - Conversion to upper/lower case characters/blocks with NLS support
    - Bracket matching for ()/{}/[]/<> bracket pairs
    - Flexible key assignment
    - Transparent with PM clipboard
    - Block sort of rectangular blocks with NLS collate support
    - Keyboard macro recorder/player
    - REXX API
    - Saving of last cursor position and other data in Extended Attributes
    - Unix/DOS file formats supported
    - Up to 10 user accessible JumpLists supported
    - kbInit/kbDone/kbTimer/kbOpen pseudo keys
    - Word wraping
    - Named pipe interface
    - Optional opening of file in one instance of the editor
    - Configurable smart indent for C/C++/REXX/ASM/Pascal/Perl
    - Support for "flashing" matching bracket
    - Syntax highlighting for C/C++/Java/REXX/TeX/HTML/ASM/Makefile/
      Pascal/Perl/Mail/JavaScript
    - History for search strings and open files saved in main FED.INI EA's

    Regular expression support is provided by the PCRE library package,
    which is open source software, written by Philip Hazel <ph10@cam.ac.uk>,
    and copyright by the University of Cambridge, England.

    PCRE sources can be obtained at
    ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/

    Also, for the convenience complete PCRE sources ported to OS/2 is included
    into FED package.

  HISTORY
   ‹‹‹‹‹‹‹€

    I have started FAST Editor Lite project in April 1997.
    Lack of time does not allow me to continue it in more or less regular way.
    So, I'm adding features from time to time (if I feel need in it or some
    FED user request it), or fixing bugs (if FED user reports it).
    Primary goal of project was to create replacement for QEdit which was my
    primary editor for about five years. Of course, I did not want to make
    exact copy of QEdit, because I did not use all features of QEdit.
    From the other hand QEdit lacks some features which are important for me.
    Two most important were syntax highlighting and full undo.

    So, first step was to write fast editing engine with syntax highlighting.
    The goal was achieved in January of 1998 when I switched to FED as my
    everyday editor.

    Starting from version 0.2.0s, so called 'Jump Lists' were introduced.
    What is Jump List? This is a list of <file, row, col, comment> elements,
    which can be filled from REXX script assigned to key and later invoked
    by another (or the same) key. This allows to implement some features
    existing in 'large' IDE's. Provided FED.INI file contains example of
    filling Jump List with compiler messages. Then this JumpList is used for
    quick navigation between error locations (file, row, col) pointed by
    these messages. Note that file listed in JumpList not necessarily should
    be already opened to enable navigation - FED will open it for you if
    necessary.

    Version 0.2.16 intoduced word wrap feature.

    Version 0.2.17 beside some bugfixes introduced new important feature -
    pipe interface. First copy of FED opens a named pipe \PIPE\FED,
    reads and executes commands from that pipe. If copy of FED
    which holds pipe is closed and other copy of FED is running,
    then pipe will be reopened by that copy. If multiple copies
    of FED is running and copy holding the pipe is closed, then
    any copy may became a new pipe holder. There is no particular
    order in which ownership will be transferred.
    Each command begins with keyword followed by parameters. At
    present pipe interface supports 3 commands:

    - open          (syntax: open <file>)
    - cursor        (syntax: cursor <row>x<col>)
    - execute       (syntax: execute <FED macro>)

    Any command can be executed by sending command in pipe using
    CMD.EXE echo command:

	    echo open FED.INI>\pipe\fed
    	echo cursor 5x6>\pipe\fed
	    echo execute PgUp>\pipe\fed

    For long commands it's possible to store command in file and
    send file in pipe using 'type' or 'copy' commands.

    Now FED supports 'single instance' feature. In order to used it,
    just open files using FED and '-p' command line parameter as
    follows:

        FED -p <file>

    Note the whitespace between command line switch and file name.
    If there is no running copy of FED, then parameter is ignored and
    file is opened in current copy of FED. If there is other copy
    of FED is running, then file will be opened in that copy and
    current copy will exit.

    Version 0.2.20 introduced smart indent for a number of languages.

    Versions 0.2.28-0.2.30 are intermediate version created during
    major rework of the editor internals. Although they include a
    number of new features, main goal was to do cleanup and
    refactoring. This was necessary because updating and adding
    new features (especially new commands) became inconvenient.
    Now it's enough to write new static memeber for EditBoxCollection
    class, add a registration record in dispatch table and everything
    else is done automatically - editor starts recognize new command
    in INI file and in REXX macro.

  PACKAGE
   ‹‹‹‹‹‹‹€

    This package contains sources and binaries of FAST Editor Lite
    and accompanying utilities.

    NOTE: Configurable parameters have default values, but key
          bindings have not. This means that FED does not work
          without INI file.

    Base FED.INI file is included into package.
    You can use it as a base for your own configuration.
    FED.INI should be placed in the same directory as FED.EXE.

  LICENSE
   ‹‹‹‹‹‹‹€

 	Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistribution of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. Redistribution in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR OR CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.


  CREDITS
   ‹‹‹‹‹‹‹€

	Special thanks to Dmitry E. Melamud <balu@balu.ms.odessa.ua>
        who done hard work of cleaning up FED doc's.

	Denis Smirnov <mithraen@os2.ru> helped me a lot with testing
        and discussing PERL and PHP syntax highlighting and other things.

    Jacek Juskowiak <jatsek@hotmail.com> is an initiator of the
	    creation 'single instance' feature.

    Vadim Yegorov <vy@org.vrn.ru> helped me to test FED.

    Alexander Belyaev <isle@free.kursknet.ru> is author of the TeX
        highlighting module and he tracked down some bugs.

    Anton Malykh <qu2@mail.ru> gave me basic idea for automatic codepage
        detection (KOI8/CP1251).

    Denis Tazetdinov <deniska@ecomstation.ru> helped me a lot with
        testing of many recent FED features.

    Philip Hazel <ph10@cam.ac.uk> for great PCRE regular expression
	    library.

	Dmitry A. Kuminov <dmik@hugaida.com> for the very useful discussions
		which resulted to adding many new featured to FED (and extending
		TODO list :-) ).

    Sergey I. Yevtushenko

    e-mail: es@os2.ru
    WWW   : http://es.os2.ru/

