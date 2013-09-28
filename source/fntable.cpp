/*
** Module   :FNTABLE.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Mon  08/03/2004	Created
**
*/

#include <string.h>

#include <boxcoll.h>
#include <version.h>

//-----------------------------------------
// Main dispatcher table
//-----------------------------------------

DispatchEntry dispatch_list[]=
{
    {"abort"            , EditBoxCollection::doAbort           },
    {"bind"             , EditBoxCollection::doLoadProfile     },
    {"bksp"             , EditBoxCollection::doBksp            },
    {"close"            , EditBoxCollection::doClose           },
    {"copy"             , EditBoxCollection::doCopy            },
    {"copyright"        , EditBoxCollection::doCopyright2      },
    {"cut"              , EditBoxCollection::doCut             },
    {"del"              , EditBoxCollection::doDel             },
    {"delline"          , EditBoxCollection::doDelLine         },
    {"deltoeol"         , EditBoxCollection::doDelToEOL        },
    {"delwordleft"      , EditBoxCollection::doDelWordLeft     },
    {"delwordright"     , EditBoxCollection::doDelWordRight    },
    {"down"             , EditBoxCollection::doDown            },
    {"downmark"         , EditBoxCollection::doDownMark        },
    {"dupline"          , EditBoxCollection::doDupLine         },
    {"end"              , EditBoxCollection::doEnd             },
    {"endmark"          , EditBoxCollection::doEndMark         },
    {"exit"             , EditBoxCollection::doExit            },
    {"filebegin"        , EditBoxCollection::doFileBegin       },
    {"filebeginmark"    , EditBoxCollection::doFileBeginMark   },
    {"fileend"          , EditBoxCollection::doFileEnd         },
    {"fileendmark"      , EditBoxCollection::doFileEndMark     },
    {"filelist"         , EditBoxCollection::doFileList        },
    {"filestat"         , EditBoxCollection::doFileStat        },
    {"flipautoindent"   , EditBoxCollection::doFlipAutoindent  },
    {"flipblockmode"    , EditBoxCollection::doFlipBlockMode   },
    {"flipcompletion"   , EditBoxCollection::doFlipCompletion  },
    {"fliphiliting"     , EditBoxCollection::doFlipHiliting    },
    {"fliptype"         , EditBoxCollection::doFlipType        },
    {"flipwordwrap"     , EditBoxCollection::doFlipWordWrap    },
    {"flipwordwrapmerge", EditBoxCollection::doFlipWWMerge     },
    {"flipwordwrappara" , EditBoxCollection::doFlipWWLong      },
    {"helpscreen"       , EditBoxCollection::doHelpScreen      },
    {"hiliteselect"     , EditBoxCollection::doHilitingChoice  },
    {"home"             , EditBoxCollection::doHome            },
    {"homemark"         , EditBoxCollection::doHomeMark        },
    {"indent"           , EditBoxCollection::doIndent          },
    {"ins"              , EditBoxCollection::doIns             },
    {"insdate"          , EditBoxCollection::doInsDate         },
    {"insfilename"      , EditBoxCollection::doInsFileName     },
    {"insfilenameshort" , EditBoxCollection::doInsFileNameShort},
    {"jumpcol"          , EditBoxCollection::doJumpCol         },
    {"jumpline"         , EditBoxCollection::doJumpLine        },
    {"lastfile"         , EditBoxCollection::doLastFile        },
    {"left"             , EditBoxCollection::doLeft            },
    {"leftmark"         , EditBoxCollection::doLeftMark        },
    {"load"             , EditBoxCollection::doLoad            },
    {"lower"            , EditBoxCollection::doLower           },
    {"macrorecend"      , EditBoxCollection::doMacroRecEnd     },
    {"macrorecstart"    , EditBoxCollection::doMacroRecStart   },
    {"markgo0"          , EditBoxCollection::doGotoMark        },
    {"markgo1"          , EditBoxCollection::doGotoMark        },
    {"markgo2"          , EditBoxCollection::doGotoMark        },
    {"markgo3"          , EditBoxCollection::doGotoMark        },
    {"markgo4"          , EditBoxCollection::doGotoMark        },
    {"markgo5"          , EditBoxCollection::doGotoMark        },
    {"markgo6"          , EditBoxCollection::doGotoMark        },
    {"markgo7"          , EditBoxCollection::doGotoMark        },
    {"markgo8"          , EditBoxCollection::doGotoMark        },
    {"markgo9"          , EditBoxCollection::doGotoMark        },
    {"markset0"         , EditBoxCollection::doSetMark         },
    {"markset1"         , EditBoxCollection::doSetMark         },
    {"markset2"         , EditBoxCollection::doSetMark         },
    {"markset3"         , EditBoxCollection::doSetMark         },
    {"markset4"         , EditBoxCollection::doSetMark         },
    {"markset5"         , EditBoxCollection::doSetMark         },
    {"markset6"         , EditBoxCollection::doSetMark         },
    {"markset7"         , EditBoxCollection::doSetMark         },
    {"markset8"         , EditBoxCollection::doSetMark         },
    {"markset9"         , EditBoxCollection::doSetMark         },
    {"markword"         , EditBoxCollection::doMarkWord        },
    {"matchbracket"     , EditBoxCollection::doMatchBracket    },
    {"matchbracketmark" , EditBoxCollection::doMatchBracketMark},
    {"new"              , EditBoxCollection::doNew             },
    {"nextcompletion"   , EditBoxCollection::doNextCompletion  },
    {"nextfile"         , EditBoxCollection::doNextFile        },
    {"openjumplist0"    , EditBoxCollection::doJumpList        },
    {"openjumplist1"    , EditBoxCollection::doJumpList        },
    {"openjumplist2"    , EditBoxCollection::doJumpList        },
    {"openjumplist3"    , EditBoxCollection::doJumpList        },
    {"openjumplist4"    , EditBoxCollection::doJumpList        },
    {"openjumplist5"    , EditBoxCollection::doJumpList        },
    {"openjumplist6"    , EditBoxCollection::doJumpList        },
    {"openjumplist7"    , EditBoxCollection::doJumpList        },
    {"openjumplist8"    , EditBoxCollection::doJumpList        },
    {"openjumplist9"    , EditBoxCollection::doJumpList        },
    {"paste"            , EditBoxCollection::doPaste           },
    {"pgdn"             , EditBoxCollection::doPgDn            },
    {"pgdnmark"         , EditBoxCollection::doPgDnMark        },
    {"pgup"             , EditBoxCollection::doPgUp            },
    {"pgupmark"         , EditBoxCollection::doPgUpMark        },
    {"prevfile"         , EditBoxCollection::doPrevFile        },
    {"right"            , EditBoxCollection::doRight           },
    {"rightmark"        , EditBoxCollection::doRightMark       },
    {"save"             , EditBoxCollection::doSave            },
    {"saveall"          , EditBoxCollection::doSaveAll         },
    {"saveas"           , EditBoxCollection::doSaveAs          },
    {"search"           , EditBoxCollection::doSearch          },
    {"searchagain"      , EditBoxCollection::doSearchAgain     },
    {"selectcompletion" , EditBoxCollection::doSelectCompletion},
    {"setcp"            , EditBoxCollection::doSetXlat         },
    {"showstartupscreen", EditBoxCollection::doStartupScreen   },
    {"sort"             , EditBoxCollection::doSort            },
    {"touchall"         , EditBoxCollection::doTouchAll        },
    {"undo"             , EditBoxCollection::doUndo            },
    {"unindent"         , EditBoxCollection::doUnindent        },
    {"up"               , EditBoxCollection::doUp              },
    {"upmark"           , EditBoxCollection::doUpMark          },
    {"upper"            , EditBoxCollection::doUpper           },
    {"wordleft"         , EditBoxCollection::doWordLeft        },
    {"wordleftmark"     , EditBoxCollection::doWordLeftMark    },
    {"wordright"        , EditBoxCollection::doWordRight       },
    {"wordrightmark"    , EditBoxCollection::doWordRightMark   },

    {"-"                , EditBoxCollection::doRexx            },

    {0}
};

//-----------------------------------------
// Function disctionary constructor
//-----------------------------------------

void FunctionDictionary::fill_functions(FunctionDictionary* p)
{
    for(int i = 0; dispatch_list[i].key; i++)
        p->Add(&dispatch_list[i]);
}

char* FunctionDictionary::locate_name(void* ptr)
{
	for(int i = 0; dispatch_list[i].key; i++)
	{
		if(!memcmp(ptr, &dispatch_list[i].proc, sizeof(PEditProc)))
			return dispatch_list[i].key;
	}

	return 0;
}

