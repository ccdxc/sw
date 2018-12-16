/*
 * Copyright (c) 2017-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _IONIC_OS_H_
#define _IONIC_OS_H_


#include <sys/types.h>
#include <sys/bus.h>
#include <sys/systm.h>

#include "ionic.h"

/*
 * DMA allocation structure.
 */
struct ionic_dma_info {
	bus_addr_t			dma_paddr;
	caddr_t				dma_vaddr;
	bus_dma_tag_t		dma_tag;
	bus_dmamap_t		dma_map;
	bus_size_t			dma_size;
};

/* DMA related. */
int ionic_dma_alloc(struct ionic* ionic, bus_size_t size, /* XXX: alignment?? */
		struct ionic_dma_info *dma, int mapflags);
void ionic_dma_free(struct ionic* ionic, struct ionic_dma_info *dma);

#ifdef notyet
extern uint32_t ionic_read_reg(struct ionic* ionic, uint32_t offset);
extern uint8_t ionic_read_reg_byte(struct ionic* ionic, uint32_t offset);
extern void ionic_write_reg(struct ionic* ionic, uint32_t offset, uint32_t val);

void
ionic_write_doorbell(struct ionic* ionic,int pid, int qid,  uint32_t val);
#endif

#endif /* _IONIC_OS_H_ */