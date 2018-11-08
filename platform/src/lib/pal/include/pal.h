/*
 * Copyright (c) 2017-18, Pensando Systems Inc.
 *
 * pal.h : This header file includes all abstraction headers.
 * 
 * PAL is supposed to be a high performance library to access 
 * Naples resources. It is NOT THREAD SAFE and does not support
 * Re-entrancy. The consumers of PAL are responsible for ensuring
 * synchronized access to PAL among their threads.
 * 
 * To add a new abstraction API, ensure a new header file 
 * for the component being abstracted and include it here.
 *
 * The definition for the abstraction APIs live in src directory.
 * Try to ensure there is symmetry in naming between the abstraction
 * header file and the file definint the abstraction.
 *
 * for. eg.
 * To create a new abstraction for component "foo" -
 *
 * 1. Create a header file under include which declare the APIs
 *    viz. pal_foo.h
 *
 * 2. Include the header "pal_foo.h" in pal.h
 *
 * 3. Define the functions declared in pal_foo.h in foo.c file
 *    in source
 *
 */

#ifndef __PAL_H__
#define __PAL_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include <sys/types.h>
#include "pal_types.h"
#include "pal_locks.h"
#include "pal_i2c.h"
#include "pal_cpld.h"
#include "pal_mem.h"
#include "pal_reg.h"
#include "pal_mm.h"

int pal_get_env(void);
int pal_is_asic(void);

void pal_init(char *application_name);

int pal_reg_trace_control(const int on);

int pal_mem_trace_control(const int on);

void pal_reg_trace(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pal_mem_trace(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));

#ifdef __cplusplus
}
#endif

#endif /* __PAL_H__ */

