/*
 * Copyright 2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
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