
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __DTLS_H__
#define __DTLS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

void flash_init(void);
void flash_erase_sec(int idx);
void flash_erase_all(void);
void flash_dump(void);

#endif
