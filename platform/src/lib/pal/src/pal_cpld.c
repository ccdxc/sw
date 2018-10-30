
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pal.h"

#ifdef __x86_64__
int pal_is_qsfp_port_psnt(int port_no) {
    return 0;
}

#else
#include "pal_impl.h"
#include "cpld.h"

int pal_is_qsfp_port_psnt(int port_no) {
    int cpld_rd_data = 0;
    pal_data_t *pd = pal_get_data();

    if(0) {
	cpld_write(0x02, cpld_rd_data);
    }


    if (pd->cpldlckfd <= 0) {
        pd->cpldlckfd = pal_lock_init(CPLDLOCKFILE);
    }    

    if (!pal_rd_lock_int(pd->memlckfd)) {
        printf("Could not lock pal.lck\n");
        return 0;
    }

    cpld_rd_data = cpld_read(0x02);


    if (!pal_rd_unlock_int(pd->memlckfd)) {
        printf("Failed to unlock.\n");
    }

    if(port_no == 1) { 
        return ((cpld_rd_data & 0x10) != 0);
    } else if (port_no == 2) {
        return ((cpld_rd_data & 0x20) != 0);
    }

    return 0;
}
#endif
