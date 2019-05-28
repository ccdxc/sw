
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdint.h>
#include "dtls.h"

void
panic(const char *s)
{
    uart_write("\npanic: ");
    uart_write(s);
    uart_write("\n");
    for (;;);
}
