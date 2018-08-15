#include <linux/kernel.h>
#include <linux/types.h>

#include "sonic.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"
#include "osal_logger.h"

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

#define DBG_CHK_RING_ID(accel_ring_id, ret) \
do {\
	if(accel_ring_id == ACCEL_RING_ID_FIRST || accel_ring_id >= ACCEL_RING_ID_MAX) \
		return ret; \
} while(0)

accel_ring_t* sonic_get_accel_ring(uint32_t accel_ring_id)
{
	identity_t *ident = sonic_get_identity();

	if(NULL == ident)
		return NULL;
	DBG_CHK_RING_ID(accel_ring_id, NULL);
	
	return &ident->dev.accel_ring_tbl[accel_ring_id];
}

int sonic_get_accel_ring_base_pa(uint32_t accel_ring_id, uint64_t *ring_base)
{
	identity_t *ident = sonic_get_identity();

	if(NULL == ident)
		return -EINVAL;
	DBG_CHK_RING_ID(accel_ring_id, -EINVAL);
	
	*ring_base = ident->dev.accel_ring_tbl[accel_ring_id].ring_base_pa;
	return 0;
}

int sonic_get_accel_ring_pndx_pa(uint32_t accel_ring_id, uint64_t *ring_pndx)
{
	identity_t *ident = sonic_get_identity();

	if(NULL == ident)
		return -EINVAL;
	DBG_CHK_RING_ID(accel_ring_id, -EINVAL);
	
	*ring_pndx = ident->dev.accel_ring_tbl[accel_ring_id].ring_pndx_pa;
	return 0;
}

int sonic_get_accel_ring_shadow_pndx_pa(uint32_t accel_ring_id, uint64_t *shadow_pndx)
{
	identity_t *ident = sonic_get_identity();

	if(NULL == ident)
		return -EINVAL;
	DBG_CHK_RING_ID(accel_ring_id, -EINVAL);
	
	*shadow_pndx = ident->dev.accel_ring_tbl[accel_ring_id].ring_shadow_pndx_pa;
	return 0;
}

int sonic_get_accel_ring_size(uint32_t accel_ring_id, uint32_t *ring_size)
{
	identity_t *ident = sonic_get_identity();

	if(NULL == ident)
		return -EINVAL;
	DBG_CHK_RING_ID(accel_ring_id, -EINVAL);
	
	*ring_size = ident->dev.accel_ring_tbl[accel_ring_id].ring_size;
	return 0;
}
