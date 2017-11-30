/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHSYS_H__
#define __PCIEHSYS_H__

typedef struct pciehsys_ops {
} pciehsys_ops_t;

void pciehsys_init(void);

void *pciehsys_zalloc(const size_t size);
void pciehsys_free(void *p);
void *pciehsys_realloc(void *p, const size_t size);

void pciehsys_log(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pciehsys_error(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));

#endif /* __PCIEHSYS_H__ */
