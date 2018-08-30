#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>

#include "sonic.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"
#include "osal_logger.h"

/*
 * Physical host address bit manipulation
 */
#define SONIC_PHYS_ADDR_HOST_POS        63
#define SONIC_PHYS_ADDR_HOST_MASK       0x1
#define SONIC_PHYS_ADDR_LIF_POS         52
#define SONIC_PHYS_ADDR_LIF_MASK        0x7ff

#define SONIC_PHYS_ADDR_FIELD_VAL(pos, mask, val)	\
	(((uint64_t)((val) & (mask))) << (pos))
    
#define SONIC_PHYS_ADDR_HOST_VAL()   \
	SONIC_PHYS_ADDR_FIELD_VAL(SONIC_PHYS_ADDR_HOST_POS, SONIC_PHYS_ADDR_HOST_MASK, 1)
#define SONIC_PHYS_ADDR_LIF_VAL(lif) \
	SONIC_PHYS_ADDR_FIELD_VAL(SONIC_PHYS_ADDR_LIF_POS, SONIC_PHYS_ADDR_LIF_MASK, lif)
    
static int sonic_api_get_rmem(struct lif *lif, uint32_t *pgid, uint64_t *pgaddr, int order)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	int ret;
	uint64_t cm_base_pa = lif->sonic->ident->dev.cm_base_pa;

	spin_lock(&idev->hbm_inuse_lock);
	ret = bitmap_find_free_region(idev->hbm_inuse, idev->hbm_npages, order);
	spin_unlock(&idev->hbm_inuse_lock);

	if (ret < 0)
	{
		OSAL_LOG_ERROR("rmem bitmap_find_free_region failed ret: %d", ret);
		return ret;
	}

	*pgid = (u32)ret;
	*pgaddr = cm_base_pa + ret * PAGE_SIZE;

	return 0;
}

uint64_t sonic_rmem_alloc(size_t size)
{
	int ret = 0;
	int order = size/PAGE_SIZE;
	uint32_t pgid;
	uint64_t addr = 0;
	struct lif* lif = sonic_get_lif();

	ret = sonic_api_get_rmem(lif, &pgid, &addr, order);
	if (ret < 0)
	{
		OSAL_LOG_ERROR("sonic_api_get_rmem failed ret: %d", ret);
		return 0;
	}

	return addr;
}

static void sonic_api_put_rmem(struct lif *lif, uint32_t pgid, int order)
{
	struct sonic_dev *idev = &lif->sonic->idev;

	spin_lock(&idev->hbm_inuse_lock);
	bitmap_release_region(idev->hbm_inuse, pgid, order);
	spin_unlock(&idev->hbm_inuse_lock);
}

void sonic_rmem_free(uint64_t ptr, size_t size)
{
	uint32_t pgid;
	struct lif* lif = sonic_get_lif();
	int order = 0;
	uint64_t cm_base_pa = lif->sonic->ident->dev.cm_base_pa;

	pgid = (ptr - cm_base_pa)/PAGE_SIZE;
	order = size/PAGE_SIZE; 
	sonic_api_put_rmem(lif, pgid, order);
}

static identity_t* sonic_get_identity(void)
{
	struct lif* lif = sonic_get_lif();

	if(NULL == lif)
		return NULL;
	return lif->sonic->ident;
}

#ifdef NDEBUG
#define DBG_CHK_RING_ID(r)	PNSO_OK
#else
#define DBG_CHK_RING_ID(r)	dbg_check_ring_id(r)
#endif

static inline int
dbg_check_ring_id(uint32_t accel_ring_id)
{
	return (accel_ring_id >= ACCEL_RING_ID_MAX) ? -EINVAL : PNSO_OK;
}

accel_ring_t* sonic_get_accel_ring(uint32_t accel_ring_id)
{
	int err;
	identity_t *ident;

	err = DBG_CHK_RING_ID(accel_ring_id);
	if (err)
		return NULL;
	
	ident = sonic_get_identity();
	if(!ident)
		return NULL;

	return &ident->dev.accel_ring_tbl[accel_ring_id];
}

uint64_t sonic_hostpa_to_devpa(uint64_t hostpa)
{
	identity_t *ident = sonic_get_identity();

	return hostpa | SONIC_PHYS_ADDR_HOST_VAL() | SONIC_PHYS_ADDR_LIF_VAL(ident->dev.hw_lif_id_tbl[0]);
}

uint64_t sonic_devpa_to_hostpa(uint64_t devpa)
{
	return devpa & ~(SONIC_PHYS_ADDR_HOST_VAL() | SONIC_PHYS_ADDR_LIF_VAL(SONIC_PHYS_ADDR_LIF_MASK));
}
