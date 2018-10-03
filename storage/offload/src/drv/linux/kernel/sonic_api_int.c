#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>

#include "sonic.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_api_int.h"
#include "osal_logger.h"
#include "osal_assert.h"

static identity_t *sonic_get_identity(void);

static inline uint64_t
sonic_rmem_base_pa(void)
{
	identity_t *ident = sonic_get_identity();
	return ident->dev.cm_base_pa;
}

static inline void __iomem *
sonic_rmem_base_iomem(void)
{
	struct sonic_dev *idev = sonic_get_idev();
	OSAL_ASSERT(idev->hbm_iomem_vaddr);
	return idev->hbm_iomem_vaddr;
}

static inline uint64_t
sonic_rmem_pgid_to_offset(uint32_t pgid)
{
	return (uint64_t)pgid * PAGE_SIZE;
}

static inline uint32_t
sonic_rmem_offset_to_pgid(uint64_t offset)
{
	return offset / PAGE_SIZE;
}

static inline int
sonic_rmem_page_order(size_t size)
{
	return size / PAGE_SIZE;
}

static inline uint64_t
sonic_rmem_pgid_to_pgaddr(uint32_t pgid)
{
	uint64_t offset = sonic_rmem_pgid_to_offset(pgid);
	OSAL_ASSERT((sonic_rmem_base_pa() & (offset + PAGE_SIZE - 1)) == 0);
	return sonic_rmem_base_pa() + offset;
}

static inline void __iomem *
sonic_rmem_pgid_to_iomem(uint32_t pgid)
{
	uint64_t offset = sonic_rmem_pgid_to_offset(pgid);
	OSAL_ASSERT(((uint64_t)sonic_rmem_base_iomem() &
		(offset + PAGE_SIZE - 1)) == 0);
	return sonic_rmem_base_iomem() + offset;
}

static inline uint32_t
sonic_rmem_pgaddr_to_pgid(uint64_t pgaddr)
{
	OSAL_ASSERT(pgaddr >= sonic_rmem_base_pa());
	return sonic_rmem_offset_to_pgid(pgaddr - sonic_rmem_base_pa());
}

static inline void __iomem *
sonic_rmem_pgaddr_to_iomem(uint64_t pgaddr)
{
	return sonic_rmem_pgid_to_iomem(sonic_rmem_pgaddr_to_pgid(pgaddr));
}

static uint64_t sonic_api_get_rmem(int order)
{
	struct sonic_dev *idev = sonic_get_idev();
	int ret;

	spin_lock(&idev->hbm_inuse_lock);
	ret = bitmap_find_free_region(idev->hbm_inuse, idev->hbm_npages, order);
	spin_unlock(&idev->hbm_inuse_lock);

	if (ret < 0) {
		OSAL_LOG_ERROR("rmem bitmap_find_free_region failed ret: %d",
				ret);
		return 0;
	}

	return sonic_rmem_pgid_to_pgaddr((uint32_t)ret);
}

static void sonic_api_put_rmem(uint32_t pgid, int order)
{
	struct sonic_dev *idev = sonic_get_idev();

	spin_lock(&idev->hbm_inuse_lock);
	bitmap_release_region(idev->hbm_inuse, pgid, order);
	spin_unlock(&idev->hbm_inuse_lock);
}

uint64_t sonic_rmem_alloc(size_t size)
{
	return sonic_api_get_rmem(sonic_rmem_page_order(size));
}

uint64_t sonic_rmem_calloc(size_t size)
{
	uint64_t pgaddr = sonic_rmem_alloc(size);

	if (pgaddr) {
		sonic_rmem_set(pgaddr, 0, size);
	}
	return pgaddr;
}

void sonic_rmem_free(uint64_t pgaddr, size_t size)
{
	sonic_api_put_rmem(sonic_rmem_pgaddr_to_pgid(pgaddr), sonic_rmem_page_order(size));
}

void sonic_rmem_set(uint64_t pgaddr, uint8_t val, size_t size)
{
	memset_io(sonic_rmem_pgaddr_to_iomem(pgaddr), val, size);
}

void sonic_rmem_read(void *dst, uint64_t pgaddr, size_t size)
{
	OSAL_ASSERT(size <= PAGE_SIZE);
	memcpy_fromio(dst, sonic_rmem_pgaddr_to_iomem(pgaddr), size);
}

void sonic_rmem_write(uint64_t pgaddr, const void *src, size_t size)
{
	OSAL_ASSERT(size <= PAGE_SIZE);
	memcpy_toio(sonic_rmem_pgaddr_to_iomem(pgaddr), src, size);
}

static identity_t *sonic_get_identity(void)
{
	struct lif *lif = sonic_get_lif();

	if (lif == NULL)
		return NULL;
	return lif->sonic->ident;
}

uint16_t
sonic_get_lif_id(void)
{
	identity_t *ident = sonic_get_identity();
	return ident->dev.lif_tbl[0].hw_lif_id;
}

uint64_t
sonic_get_lif_local_dbaddr(void)
{
	identity_t *ident = sonic_get_identity();
	return ident->dev.lif_tbl[0].hw_lif_local_dbaddr;
}

uint32_t
sonic_get_crypto_key_idx(uint32_t user_key_idx)
{
	identity_t *ident = sonic_get_identity();
	return ident->dev.lif_tbl[0].hw_key_idx_base + user_key_idx;
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

accel_ring_t *sonic_get_accel_ring(uint32_t accel_ring_id)
{
	int err;
	identity_t *ident;

	err = DBG_CHK_RING_ID(accel_ring_id);
	if (err)
		return NULL;

	ident = sonic_get_identity();
	if (!ident)
		return NULL;

	return &ident->dev.accel_ring_tbl[accel_ring_id];
}

uint64_t sonic_hostpa_to_devpa(uint64_t hostpa)
{
	identity_t *ident = sonic_get_identity();

	OSAL_ASSERT((hostpa & ident->dev.lif_tbl[0].hw_host_mask) == 0);
	return hostpa | ident->dev.lif_tbl[0].hw_host_prefix;
}

uint64_t sonic_devpa_to_hostpa(uint64_t devpa)
{
	identity_t *ident = sonic_get_identity();

	return devpa & ~ident->dev.lif_tbl[0].hw_host_mask;
}
