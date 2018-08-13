#include <linux/kernel.h>
#include <linux/types.h>
#include "sonic_api_int.h"
#include "sonic.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "osal_logger.h"

static int sonic_api_get_rmem(struct lif *lif, uint32_t *pgid, phys_addr_t *pgaddr, int order)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	int ret;

	spin_lock(&idev->hbm_inuse_lock);
	ret = bitmap_find_free_region(idev->hbm_inuse, idev->hbm_npages, order);
	spin_unlock(&idev->hbm_inuse_lock);

	if (ret < 0)
	{
		OSAL_LOG_ERROR("rmem bitmap_find_free_region failed ret: %d", ret);
		return ret;
	}

	*pgid = (u32)ret;
	*pgaddr = idev->phy_hbm_pages + ret * PAGE_SIZE;

	return 0;
}

uint64_t sonic_rmem_alloc(size_t size)
{
	int ret = 0;
	int order = size/PAGE_SIZE;
	uint32_t pgid;
	phys_addr_t pgaddr;
	uint64_t addr = 0;
	struct lif* lif = sonic_get_lif();

	ret = sonic_api_get_rmem(lif, &pgid, &pgaddr, order);
	if(ret == 0)
		addr = (uint64_t) pgaddr;

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
	phys_addr_t addr;
	uint32_t pgid;
	struct lif* lif = sonic_get_lif();
	struct sonic_dev *idev = &lif->sonic->idev;
	int order = 0;

	addr = (phys_addr_t)ptr;
	pgid = (addr - idev->phy_hbm_pages)/PAGE_SIZE;
	order = size/PAGE_SIZE; 
	sonic_api_put_rmem(lif, pgid, order);
}

