/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PMCLIENT_H__
#define __PMCLIENT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "pmmsg.h"

int pciemgrc_open(const char *myname, const char *addrstr, const int receiver);
void pciemgrc_close(void);
int pciemgrc_msgalloc(pmmsg_t **m, size_t len);
void pciemgrc_msgfree(pmmsg_t *m);
int pciemgrc_msgsend(pmmsg_t *m);
int pciemgrc_msgrecv(pmmsg_t **m);

#ifdef __cplusplus
}
#endif

#endif /* __PMCLIENT_H__ */
