/*
** Module   :VERSION.H
** Abstract :FED Version
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Thu  26/03/1998     Created
*/

#ifndef __VERSION_H
#define __VERSION_H

#define VER_MAJ         0
#define VER_MID         2
#define VER_MIN         31

#define MK_S(a)         #a
#define MK_STR(a,b,c)   MK_S(a)"."MK_S(b)"."MK_S(c)
#define VERSION         "Version "MK_STR(VER_MAJ,VER_MID,VER_MIN)" ("__DATE__" "__TIME__")"

#ifndef RELEASE
#undef TRACE_ENABLED
#define TRACE_ENABLED 0
#endif

#endif  /* __VERSION_H */

