#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/bitmap.h>
#include <linux/pci.h>
#include <rdma/ib_verbs.h>
#include <rdma/ib_user_verbs.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_addr.h>

#include "ionic_ib.h"

int ionic_page_dir_init(struct ionic_ib_dev *dev, struct ionic_page_dir *pdir,
                      u64 npages)
{

	if (npages > IONIC_PAGE_DIR_MAX_PAGES)
		return -EINVAL;

	memset(pdir, 0, sizeof(*pdir));

    pdir->phy_npages = IONIC_PAGES_NEEDED(npages);
    pdir->npages = npages;

    pdir->table = dma_alloc_coherent(&dev->pdev->dev, pdir->phy_npages*PAGE_SIZE,
                                     (dma_addr_t *)&pdir->tbl_dma,
                                     GFP_KERNEL);
    if (!pdir->table) {
        return -ENOMEM;
    }
    
	return 0;
}

void ionic_page_dir_cleanup(struct ionic_ib_dev *dev,
                          struct ionic_page_dir *pdir)
{
	if (pdir->tbl_dma) {
        dma_free_coherent(&dev->pdev->dev, pdir->phy_npages*PAGE_SIZE,
                          pdir->table, pdir->tbl_dma);
    }
    pdir->tbl_dma = 0;
}

int ionic_page_dir_insert_dma(struct ionic_page_dir *pdir, u64 idx,
                            dma_addr_t daddr)
{
	if (idx >= pdir->npages)
		return -EINVAL;

    pdir->table[idx] = daddr;
    
	return 0;
}

int ionic_page_dir_insert_umem(struct ionic_page_dir *pdir,
                             struct ib_umem *umem, u64 offset)
{
	u64 i = offset;
	int j, entry;
	int ret = 0, len = 0;
	struct scatterlist *sg;

	if (offset >= pdir->npages)
		return -EINVAL;

	for_each_sg(umem->sg_head.sgl, sg, umem->nmap, entry) {
		len = sg_dma_len(sg) >> PAGE_SHIFT;
		for (j = 0; j < len; j++) {
			dma_addr_t addr = sg_dma_address(sg) +
                (j << umem->page_shift);

            /*
             * TODO: Model expects bit 63 to be set
             */
			ret = ionic_page_dir_insert_dma(pdir, i, (addr | (0x1L<<63)));
			if (ret)
				goto exit;

			i++;
		}
	}

exit:
	return ret;
}

void ionic_page_dir_print (struct ionic_page_dir *pdir)
{
    int i;
    for (i = 0 ; i < pdir->npages; i++) {
        pr_info("\nPage table entry %d : %llx", i, pdir->table[i]);
    }
}
