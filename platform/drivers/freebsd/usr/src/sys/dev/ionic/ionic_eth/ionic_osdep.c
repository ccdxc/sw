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

int
ionic_dma_alloc(struct ionic* ionic, bus_size_t size,
		struct ionic_dma_info *dma, int mapflags)
{
	device_t dev = ionic->dev->bsddev;
	int             r;

	r = bus_dma_tag_create(bus_get_dma_tag(dev),	/* parent */
			       128, 		            /* XXX: ??? Alignment */
				   0,	/* alignment, bounds */
			       BUS_SPACE_MAXADDR,	/* lowaddr */
			       BUS_SPACE_MAXADDR,	/* highaddr */
			       NULL, NULL,	/* filter, filterarg */
			       size,	/* maxsize */
			       1,	/* nsegments */
			       size,	/* maxsegsize */
			       BUS_DMA_ALLOCNOW,	/* flags */
			       NULL,	/* lockfunc */
			       NULL,	/* lockfuncarg */
			       &dma->dma_tag);
	if (r != 0) {
		IONIC_DEV_ERROR(ionic->dev, "bus_dma_tag_create failed, error: %d\n", r);
		goto fail_0;
	}
	r = bus_dmamem_alloc(dma->dma_tag, (void **)&dma->dma_vaddr,
			     BUS_DMA_NOWAIT, &dma->dma_map);
	if (r != 0) {
		IONIC_DEV_ERROR(ionic->dev,"bus_dmamem_alloc failed, error: %d\n", r);
		goto fail_1;
	}
	r = bus_dmamap_load(dma->dma_tag, dma->dma_map, dma->dma_vaddr,
			    size,
			    ionic_dmamap_cb,
			    &dma->dma_paddr,
			    mapflags | BUS_DMA_NOWAIT);
	if (r != 0) {
		IONIC_DEV_ERROR(ionic->dev, "bus_dmamap_load failed, error: %d\n", r);
		goto fail_2;
	}
	dma->dma_size = size;
	return (0);

fail_2:
	bus_dmamem_free(dma->dma_tag, dma->dma_vaddr, dma->dma_map);
fail_1:
	bus_dma_tag_destroy(dma->dma_tag);
fail_0:
	dma->dma_tag = NULL;
	return (r);
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