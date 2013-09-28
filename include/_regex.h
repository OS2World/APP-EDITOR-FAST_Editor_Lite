#ifdef __EMX__
	#include <xregex.h>
#elif defined(__WATCOMC__) || defined(__IBMC__)
	#include <pcreposix.h>
#else
	#include <regex.h>
#endif

#ifndef __REG_SUBEXP_MAX
#define __REG_SUBEXP_MAX    9
#endif
