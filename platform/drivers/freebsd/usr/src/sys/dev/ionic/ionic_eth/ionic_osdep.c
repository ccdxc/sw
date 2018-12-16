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

#include <sys/types.h>
#include <sys/bus.h>

#include <dev/pci/pcivar.h>

#include <machine/bus.h>

#include <linux/types.h> // for types u8, u16 etc.
#include <linux/bitops.h>

#include "ionic_osdep.h"
#include "ionic.h"

#ifdef notyet
inline uint8_t
ionic_read_reg_byte(struct ionic* ionic, uint32_t offset)
{
	uint32_t val;

	val = bus_space_read_1(ionic->ctrl_reg_tag, ionic->ctrl_reg_handle, offset);

#ifdef IONIC_DBG_REG
	device_printf(ionic->dev, "[REG_READ BYTE]offste: 0x%lx 0x%x @ 0x%x\n", ionic->ctrl_reg_handle, val, offset);
#endif

	return (val);
}

inline uint32_t
ionic_read_reg(struct ionic* ionic, uint32_t offset)
{
	uint32_t val;

	val = bus_space_read_4(ionic->ctrl_reg_tag, ionic->ctrl_reg_handle, offset);

#ifdef IONIC_DBG_REG
	device_printf(ionic->dev, "[REG_READ] offset: 0x%lx 0x%x @ 0x%x\n", ionic->ctrl_reg_handle, val, offset);
#endif

	return (val);
}

inline void
ionic_write_reg(struct ionic* ionic, uint32_t offset, uint32_t val)
{
#ifdef IONIC_DBG_REG
	device_printf(ionic->dev, "[REG_WRITE] offset: 0x%lx 0x%x @ 0x%x\n", ionic->ctrl_reg_handle, val, offset);
#endif
	bus_space_write_4(ionic->ctrl_reg_tag, ionic->ctrl_reg_handle, offset, val);
}

/*
 * Write to doorbell register in BAR1
 */
void
ionic_write_doorbell(struct ionic* ionic,int pid, int qid,  uint64_t val)
{
	uint32_t offset = pid * PAGE_SIZE;

	/* Jump to Interrupt control block area. */
	offset += qid * sizeof(struct doorbell);
	dev_info(ionic->dev, "[DOORBELL_WRITE] pid: %d qis: %d offset: 0x%x val: 0x%x\n",
		pid, qid, offset, val);

	bus_space_write_8(ionic->doorbells_tag, ionic->doorbells_handle, offset, val);
}
#endif

/* DMA related functions. */
static void
ionic_dmamap_cb(void *arg, bus_dma_segment_t *segs, int nsegs, int error)
{

	if (error == 0)
		*(bus_addr_t *)arg = segs->ds_addr;
}

/*
 * DMA ring allocations.
 */
int
ionic_dma_alloc(struct ionic* ionic, bus_size_t size,
		struct ionic_dma_info *dma, int mapflags)
{
	device_t dev = ionic->dev->bsddev;
	int error;

	error = bus_dma_tag_create(bus_get_dma_tag(dev),/* parent */
			       1, 		            /* Alignment */
				   0,					/* Bounds */
			       BUS_SPACE_MAXADDR,	/* lowaddr */
			       BUS_SPACE_MAXADDR,	/* highaddr */
			       NULL, NULL,			/* filter, filterarg */
			       size,				/* maxsize */
			       1,					/* nsegments */
			       size,				/* maxsegsize */
			       BUS_DMA_ALLOCNOW,	/* flags */
			       NULL,				/* lockfunc */
			       NULL,				/* lockfuncarg */
			       &dma->dma_tag);
	if (error != 0) {
		IONIC_DEV_ERROR(ionic->dev, "bus_dma_tag_create failed, error: %d\n", error);
		dma->dma_tag = NULL;
		return (error);
	}

	error = bus_dmamem_alloc(dma->dma_tag, (void **)&dma->dma_vaddr,
			     BUS_DMA_NOWAIT, &dma->dma_map);
	if (error != 0) {
		IONIC_DEV_ERROR(ionic->dev,"bus_dmamem_alloc failed, error: %d\n", error);
		goto alloc_failed;
	}

	error = bus_dmamap_load(dma->dma_tag, dma->dma_map, dma->dma_vaddr,
			    size,
			    ionic_dmamap_cb,
			    &dma->dma_paddr,
			    mapflags | BUS_DMA_NOWAIT);
	if (error != 0) {
		IONIC_DEV_ERROR(ionic->dev, "bus_dmamap_load failed, error: %d\n", error);
		goto map_failed;
	}
	dma->dma_size = size;
	return (0);

map_failed:
	bus_dmamem_free(dma->dma_tag, dma->dma_vaddr, dma->dma_map);
alloc_failed:
	bus_dma_tag_destroy(dma->dma_tag);
	dma->dma_tag = NULL;

	return (error);
}

void
ionic_dma_free(struct ionic* ionic, struct ionic_dma_info *dma)
{
	bus_dmamap_sync(dma->dma_tag, dma->dma_map,
	    BUS_DMASYNC_POSTREAD | BUS_DMASYNC_POSTWRITE);
	bus_dmamap_unload(dma->dma_tag, dma->dma_map);
	bus_dmamem_free(dma->dma_tag, dma->dma_vaddr, dma->dma_map);
	bus_dma_tag_destroy(dma->dma_tag);
}