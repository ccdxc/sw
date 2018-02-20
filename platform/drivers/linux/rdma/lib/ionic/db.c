#include <util/udma_barrier.h>
#include "main.h"
#include "ionic-abi.h"

static void ionic_ring_db(struct ionic_dpi *dpi,
                          struct ionic_doorbell *db,
                          uint8_t qtype)
{
	pthread_spin_lock(&dpi->db_lock);
	udma_to_device_barrier();
	iowrite64((__u64 *)(dpi->dbpage+qtype), (__u64 *)db);
	pthread_spin_unlock(&dpi->db_lock);
}

static void ionic_init_db_hdr(struct ionic_doorbell *db,
                            uint32_t                 index,
                            uint32_t                 qid)
{
    db->qid_lo = qid & 0xFF;
    db->qid_hi = (qid >> 8) & 0xFFFF;
    db->ring = 0;
    db->rsvd = 0;
    db->p_index = index;
    db->rsvd2 = 0;
}

void ionic_ring_rq_db(struct ionic_qp *qp)
{
	struct ionic_doorbell hdr;

	ionic_init_db_hdr(&hdr, qp->rqq->tail, qp->qpid);
	ionic_ring_db(qp->udpi, &hdr, qp->rq_qtype);
}

void ionic_ring_sq_db(struct ionic_qp *qp)
{
	struct ionic_doorbell hdr;
    
	ionic_init_db_hdr(&hdr, qp->sqq->tail, qp->qpid);
	ionic_ring_db(qp->udpi, &hdr, qp->sq_qtype);
}

void ionic_ring_cq_db(struct ionic_cq *cq)
{
	struct ionic_doorbell hdr;
    
	ionic_init_db_hdr(&hdr, cq->cqq.head, cq->cqid);
	ionic_ring_db(cq->udpi, &hdr, cq->qtype);
}

