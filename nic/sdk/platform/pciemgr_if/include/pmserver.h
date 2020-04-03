/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PMSERVER_H__
#define __PMSERVER_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "pmmsg.h"

typedef void (pciemgrs_handler_t)(pmmsg_t *m);

int pciemgrs_open(const char *addr, pciemgrs_handler_t *msghand);
void pciemgrs_close(void);
int pciemgrs_add_receiver(pmmsg_t *m);
int pciemgrs_msgalloc(pmmsg_t **m, size_t len);
void pciemgrs_msgfree(pmmsg_t *m);
int pciemgrs_msgsend(pmmsg_t *m);

#ifdef __cplusplus
}
#endif

#endif /* __PMSERVER_H__ */
