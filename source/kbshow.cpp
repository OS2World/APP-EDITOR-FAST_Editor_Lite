/*
** Module   :KBSHOW.CPP
** Abstract :
**
** Copyright (C) Sergey I. Yevtushenko
**
** Log: Tue  21/04/1998 Created
**
*/
#include <vio.h>
#include <keynames.h>
#include <stdio.h>

int main()
{
    KeyInfo k;

    vio_init();

    do
    {
        vio_read_key(&k);

        printf("Pressed key name: %s, pressed key code %04x\n",
               k.KeyName, k.skey);
    }
    while(k.skey != (kbEsc | shIsCtrl));
    return 0;
}
