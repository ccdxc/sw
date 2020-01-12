
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "defs.h"

static int
fwname_to_id(const char *name)
{
    static const struct {
        const char *name;
        int id;
    } map[] = {
        { "mainfwa", FW_MAIN_A },
        { "mainfwb", FW_MAIN_B },
        { "goldfw",  FW_GOLD   },
        { "diagfw",  FW_DIAG   },
        { NULL }
    };
    for (int i = 0; map[i].name != NULL; i++) {
        if (strcmp(map[i].name, name) == 0) {
            return map[i].id;
        }
    }
    return -1;
}

int
get_pri_fw(void)
{
    intptr_t addr;
    char buf[16];
    char *p;
    int id;

    if (board_get_part("fwsel", &addr, NULL) < 0) {
        return FW_MAIN_A;
    }
    memcpy(buf, (void *)addr, sizeof (buf));
    buf[sizeof (buf) - 1] = '\0';
    p = strchr(buf, 0xff);
    if (p != NULL) {
        *p = '\0';
    }
    id = fwname_to_id(buf);
    if (id < 0) {
        id = FW_MAIN_A;
    }
    return id;
}
