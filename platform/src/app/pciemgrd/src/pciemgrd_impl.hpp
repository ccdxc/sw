/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#ifndef __PCIEMGRD_IMPL_H__
#define __PCIEMGRD_IMPL_H__

struct pciemgrenv_s;
typedef struct pciemgrenv_s pciemgrenv_t;

int cli_loop(pciemgrenv_t *pme);
void logger_init(void);

#endif /* __PCIEMGRD_IMPL_H__ */
