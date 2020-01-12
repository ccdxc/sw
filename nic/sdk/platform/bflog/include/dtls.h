
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __DTLS_H__
#define __DTLS_H__

// main.c
enum {
    CM_CAT,
    CM_DUMP,
    CM_ERASE,
    CM_STATS,
    CM_TAIL,
    CM_WRITE,
};

struct opts {
    char *message;
    int mark;
    int expand;
    int cmd;
    int readonly;
};

struct ctx {
    int pass;
    int l_mark;
    int emitting;
    int output_done;
};

// flash.c
int flash_init(int readonly);
int flash_erase_sector(int i);
int flash_erase_all(void);
void flash_dump(void);

#endif
