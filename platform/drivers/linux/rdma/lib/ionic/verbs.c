
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <unistd.h>

#include <util/compiler.h>

#include "main.h"
#include "verbs.h"


int g_wrid = 0;
#define IONIC_DEBUG

#ifdef IONIC_DEBUG
#define IONIC_LOG(format, ...) \
    printf("\n%s(%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define IONIC_LOG(format, ...)
#endif

uint32_t g_msn = 0;

int ionic_query_device(struct ibv_context *ibvctx,
			 struct ibv_device_attr *dev_attr)
{
	struct ibv_query_device cmd;
	uint64_t fw_ver;
	int status;

    IONIC_LOG("");
	memset(dev_attr, 0, sizeof(struct ibv_device_attr));
	status = ibv_cmd_query_device(ibvctx, dev_attr, &fw_ver,
                                  &cmd, sizeof(cmd));
	return status;
}

int ionic_query_port(struct ibv_context *ibvctx, uint8_t port,
		       struct ibv_port_attr *port_attr)
{
	struct ibv_query_port cmd;

    IONIC_LOG("");
	memset(port_attr, 0, sizeof(struct ibv_port_attr));
	return ibv_cmd_query_port(ibvctx, port, port_attr, &cmd, sizeof(cmd));
}

struct ibv_pd *ionic_alloc_pd(struct ibv_context *ibvctx)
{
	struct ibv_alloc_pd cmd;
	struct ionic_pd_resp resp;
	struct ionic_pd *pd;

	pd = calloc(1, sizeof(*pd));
	if (!pd)
		return NULL;

    IONIC_LOG("");
	memset(&resp, 0, sizeof(resp));
	if (ibv_cmd_alloc_pd(ibvctx, &pd->ibvpd, &cmd, sizeof(cmd),
			     &resp.resp, sizeof(resp)))
		goto out;

	pd->pdid = resp.pdid;

	return &pd->ibvpd;
out:
	free(pd);
	return NULL;
}

int ionic_free_pd(struct ibv_pd *ibvpd)
{
	struct ionic_pd *pd = to_ionic_pd(ibvpd);
	int status;

    IONIC_LOG("");
	status = ibv_cmd_dealloc_pd(ibvpd);
	if (status)
		return status;
	/* DPI un-mapping will be during uninit_ucontext */
	free(pd);

	return 0;
}

struct ibv_mr *ionic_reg_mr(struct ibv_pd *ibvpd,
                            void *sva,
                            size_t len,
                            int access)
{
	struct ionic_mr *mr;
	struct ibv_reg_mr cmd;
	struct ionic_mr_resp resp;

    IONIC_LOG("");
	mr = calloc(1, sizeof(*mr));
	if (!mr)
		return NULL;

	if (ibv_cmd_reg_mr(ibvpd, sva, len, (uintptr_t)sva, access, &mr->ibvmr,
			   &cmd, sizeof(cmd), &resp.resp, sizeof(resp))) {
		free(mr);
		return NULL;
	}

	return &mr->ibvmr;
}

int ionic_dereg_mr(struct ibv_mr *ibvmr)
{
	struct ionic_mr *mr = (struct ionic_mr *)ibvmr;
	int status;

    IONIC_LOG("");
	status = ibv_cmd_dereg_mr(ibvmr);
	if (status)
		return status;
	free(mr);

	return 0;
}

struct ibv_cq *ionic_create_cq(struct ibv_context *ibvctx,
                             int ncqe,
                             struct ibv_comp_channel *channel,
                             int vec)
{
	struct ionic_cq *cq;
	struct ionic_cq_req cmd;
	struct ionic_cq_resp resp;

	struct ionic_context *cntx = to_ionic_context(ibvctx);
	struct ionic_dev *dev = to_ionic_dev(ibvctx->device);

    IONIC_LOG("");
	if (ncqe > dev->max_cq_depth)
		return NULL;

	cq = calloc(1, sizeof(*cq));
	if (!cq)
		return NULL;

	cq->cqq.depth = roundup_pow_of_two(ncqe + 1);
	if (cq->cqq.depth > dev->max_cq_depth)
		cq->cqq.depth = dev->max_cq_depth;
	cq->cqq.stride = dev->cqe_size;
    
	if (ionic_alloc_aligned(&cq->cqq, dev->pg_size))
		goto fail;

	pthread_spin_init(&cq->cqq.qlock, PTHREAD_PROCESS_PRIVATE);

	cmd.cq_va = (uintptr_t)cq->cqq.va;
	cmd.cq_bytes = cq->cqq.bytes;

	memset(&resp, 0, sizeof(resp));
	if (ibv_cmd_create_cq(ibvctx, ncqe, channel, vec,
			      &cq->ibvcq, &cmd.cmd, sizeof(cmd),
			      &resp.resp, sizeof(resp)))
		goto cmdfail;

	cq->cqid = resp.cqid;
    cq->qtype = resp.qtype;
	cq->udpi = &cntx->udpi; //TODO: Dont need this.
    cq->done_color = 1;
    cq->cntxt = cntx;
	return &cq->ibvcq;
cmdfail:
	ionic_free_aligned(&cq->cqq);
fail:
	free(cq);
	return NULL;
}

int ionic_resize_cq(struct ibv_cq *ibvcq, int ncqe)
{
	return -ENOSYS;
}

int ionic_destroy_cq(struct ibv_cq *ibvcq)
{
	int status;
	struct ionic_cq *cq = to_ionic_cq(ibvcq);

	status = ibv_cmd_destroy_cq(ibvcq);
	if (status)
		return status;

	ionic_free_aligned(&cq->cqq);
	free(cq);

	return 0;
}


static uint8_t *memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

static inline bool ionic_cq_has_data (struct ionic_cq *cq)
{
    struct ionic_queue *que = &cq->cqq;
    volatile struct cqwqe_t *wqe;
    struct cqwqe_t lwqe;
    
    wqe = (struct cqwqe_t *)(que->va + sizeof(struct cqwqe_t) * que->head);
    lwqe = *wqe;
    memrev((uint8_t *)&lwqe, sizeof(struct cqwqe_t));
           
    if (lwqe.color == cq->done_color) {
        IONIC_LOG("wrid 0x%lx color %d", be64toh(lwqe.id.wrid), lwqe.color);
        return 1;
    }
    return 0;
}

static struct cqwqe_be_t * get_cqe (struct ionic_cq *cq,
                                    int *has_data)
{
    struct cqwqe_be_t *cqe;
    
    *has_data = 0;
    cqe = (struct cqwqe_be_t *)(cq->cqq.va + sizeof(struct cqwqe_t) * cq->cqq.head);

    if ((cqe->color_flags >> COLOR_SHIFT) == cq->done_color) {
        *has_data = 1;
        udma_from_device_barrier();
        
        IONIC_LOG("wrid 0x%lx color %d", be64toh(cqe->id.wrid), cq->done_color);

#if 0
        unsigned int *ptr;
        ptr = (unsigned int *)cqe;
        for (int i = 0; i < 5; i++, ptr++) {
            IONIC_LOG(" 0x%x ", *ptr);
        }
#endif        
    }
    return cqe;
}

static inline void ionic_cq_incr (struct ionic_cq *cq)
{
    IONIC_LOG("cq head %d cq done_color %d cq depth %d",
              cq->cqq.head, cq->done_color, cq->cqq.depth);
    ionic_incr_head(&cq->cqq);
    if (cq->cqq.head == 0) {
        //Flip the color
        cq->done_color ^= 0x1;
        IONIC_LOG("Changed done_color: cq head %d cq done_color %d cq depth %d",
                  cq->cqq.head, cq->done_color, cq->cqq.depth);
    }
}

static int ionic_poll_one(struct ionic_cq *cq, 
                          struct ibv_wc *wc,
                          int *npolled)
{
	int has_data = 0;
    struct cqwqe_be_t *cqe;
    struct rqwqe_t *rqe;
    struct ionic_qp *qp;
    __u32 qp_num;

	cqe = get_cqe(cq, &has_data);
	if (has_data == 0) {
        return -EAGAIN;
	}

    if (cqe->status == 0) {
        //SUCCESS
        wc->status = IBV_WC_SUCCESS;        
    } else {
        wc->status = ionic_to_ibv_wc_status(cqe->status);        
    }

    wc->qp_num = qp_num = cqe->qp_hi | (be16toh(cqe->qp_lo) << 8);
    qp = cq->cntxt->qp_tbl[qp_num];

    // Need to figure out if it belongs to SQ or RQ
    if (cqe->op_type == OP_TYPE_SEND_RCVD ||
        cqe->op_type == OP_TYPE_RDMA_OPER_WITH_IMM) {
        // recv event
        wc->opcode = IBV_WC_RECV;
        wc->wc_flags = 0;
        if (cqe->color_flags & IMM_DATA_VLD_MASK) {
            wc->wc_flags |= IBV_WC_WITH_IMM;
        }
        if (cqe->op_type == OP_TYPE_RDMA_OPER_WITH_IMM) {
			wc->opcode = IBV_WC_RECV_RDMA_WITH_IMM;
            wc->imm_data = be32toh(cqe->imm_data);
        }
		wc->src_qp = cqe->src_qp_hi | (be16toh(cqe->src_qp_lo) << 8);
        //Let us try to copy wrid always from Recv WQE.
        rqe = (struct rqwqe_t *)(qp->rqq->va + sizeof(struct rqwqe_t) * qp->rqq->head);
        wc->wr_id = rqe->wrid;
        ionic_incr_head(qp->rqq);
        (*npolled)++;
        IONIC_LOG("CQ Poll Success: Recv CQEs %d\n", *npolled);

    } else {
        struct ionic_queue *sq = qp->sqq;
        struct sqwqe_t *sqe;
        //TODO: Need to handle many op types
        if (cqe->op_type == OP_TYPE_SEND) {
            __u32 msn = be32toh(cqe->id.msn);

            //We need to pop one or more sqes because of cq coalescing

            sqe = (struct sqwqe_t *)(sq->va + (sq->head * sq->stride));
            for (int i = 0; i < (msn-g_msn); i++) {
                wc->opcode = IBV_WC_SEND;
                wc->wc_flags = 0;
                wc->wr_id = sqe->base.wrid;
                wc->qp_num = qp_num;

                ionic_incr_head(sq);
                wc++;
                (*npolled)++;
            }
            g_msn = msn;
            IONIC_LOG("CQ Poll Success: MSN %d Send CQEs %d\n", msn, *npolled);
        } 
    }
    //TODO: What about error event.

	/* Update cq head */
    ionic_cq_incr(cq);

	return 0;
}

int ionic_poll_cq(struct ibv_cq *ibvcq, int nwc, struct ibv_wc *wc)
{
	struct ionic_cq *cq = to_ionic_cq(ibvcq);
	int npolled;

	if (nwc < 1 || wc == NULL)
		return 0;

	pthread_spin_lock(&cq->cqq.qlock);
    npolled = 0;
    while (npolled < nwc) { 
        if (ionic_poll_one(cq, (wc + npolled), &npolled)) {
            break;
        }
    }
	pthread_spin_unlock(&cq->cqq.qlock);

	return npolled;
}

void ionic_cq_event(struct ibv_cq *ibvcq)
{

}

int ionic_arm_cq(struct ibv_cq *ibvcq, int flags)
{
    IONIC_LOG("");
    return 0;
}

static int ionic_check_qp_limits(struct ionic_context *cntx,
				   struct ibv_qp_init_attr *attr)
{
	struct ibv_device_attr devattr;
	int ret;

    IONIC_LOG("");
	ret = ionic_query_device(&cntx->ibvctx, &devattr);
	if (ret)
		return ret;
	if (attr->cap.max_send_sge > devattr.max_sge)
		return EINVAL;
	if (attr->cap.max_recv_sge > devattr.max_sge)
		return EINVAL;
	if (attr->cap.max_inline_data > IONIC_MAX_INLINE_SIZE)
		return EINVAL;
	if (attr->cap.max_send_wr > devattr.max_qp_wr)
		attr->cap.max_send_wr = devattr.max_qp_wr;
	if (attr->cap.max_recv_wr > devattr.max_qp_wr)
		attr->cap.max_recv_wr = devattr.max_qp_wr;

	return 0;
}

static int ionic_alloc_queue_ptr(struct ionic_qp *qp,
				   struct ibv_qp_init_attr *attr)
{
    IONIC_LOG("");
	qp->sqq = calloc(1, sizeof(struct ionic_queue));
	if (!qp->sqq)
		return -ENOMEM;
	if (attr->srq) {
        // srq is not supported
		qp->srq = NULL;/*TODO: to_ionic_srq(attr->srq);*/
        return -EINVAL;
    } else {
		qp->rqq = calloc(1, sizeof(struct ionic_queue));
		if (!qp->rqq) {
			free(qp->sqq);
			return -ENOMEM;
		}
	}

	return 0;
}

static void ionic_free_queue_ptr(struct ionic_qp *qp)
{
    IONIC_LOG("");
	if (qp->rqq)
		free(qp->rqq);
	if (qp->sqq)
		free(qp->sqq);
}

static void ionic_free_queues(struct ionic_qp *qp)
{
    
    IONIC_LOG("");
	if (qp->rwrid)
		free(qp->rwrid);
	pthread_spin_destroy(&qp->rqq->qlock);
	ionic_free_aligned(qp->rqq);

	if (qp->swrid)
		free(qp->swrid);
	pthread_spin_destroy(&qp->sqq->qlock);
	ionic_free_aligned(qp->sqq);

    ionic_free_queue_ptr(qp);
}

static int ionic_alloc_queues(struct ionic_qp *qp,
                            struct ibv_qp_init_attr *attr,
                            uint32_t pg_size)
{
	struct ionic_queue *que;
	int ret;

    IONIC_LOG("");
	/* alloc queue pointers */
	if ((ret = ionic_alloc_queue_ptr(qp, attr))) {
		return ret;
    }
    
	que = qp->sqq;
	que->stride = ionic_get_sqe_size();

	que->depth = roundup_pow_of_two(attr->cap.max_send_wr + 1);

	if ((ret = ionic_alloc_aligned(qp->sqq, pg_size))) {
		goto fail1;
    }
    
	pthread_spin_init(&que->qlock, PTHREAD_PROCESS_PRIVATE);

	if (qp->rqq) {
		que = qp->rqq;
		que->stride = ionic_get_rqe_size();
		que->depth = roundup_pow_of_two(attr->cap.max_recv_wr + 1);
        
		if ((ret = ionic_alloc_aligned(qp->rqq, pg_size))) {
			goto fail2;
        }
		pthread_spin_init(&que->qlock, PTHREAD_PROCESS_PRIVATE);
	}

	return 0;
fail2:
	ionic_free_queues(qp);
fail1:
    ionic_free_queue_ptr(qp);
	return ret;
}

struct ibv_qp *ionic_create_qp(struct ibv_pd *ibvpd,
                             struct ibv_qp_init_attr *attr)
{
	struct ionic_qp *qp;
	struct ionic_qp_req req;
	struct ionic_qp_resp resp;
	struct ionic_qpcap *cap;

    IONIC_LOG("");
	struct ionic_context *cntx = to_ionic_context(ibvpd->context);
	struct ionic_dev *dev = to_ionic_dev(cntx->ibvctx.device);

	if (ionic_check_qp_limits(cntx, attr))
		return NULL;

	qp = calloc(1, sizeof(*qp));
	if (!qp)
		return NULL;

	/* alloc queues */
	if (ionic_alloc_queues(qp, attr, dev->pg_size))
		goto fail;

	/* Fill ibv_cmd */
	cap = &qp->cap;
	req.qpsva = (uintptr_t)qp->sqq->va;
    req.sq_bytes = qp->sqq->bytes;
	req.qprva = qp->rqq ? (uintptr_t)qp->rqq->va : 0;
    req.rq_bytes = qp->rqq ? qp->rqq->bytes : 0;
    req.sq_wqe_size = sizeof(struct sqwqe_t);
    req.rq_wqe_size = sizeof(struct rqwqe_t);
    
	if (ibv_cmd_create_qp(ibvpd, &qp->ibvqp, attr, &req.cmd, sizeof(req),
			      &resp.resp, sizeof(resp))) {
		goto failcmd;
	}

	qp->qpid = resp.qpid;
    qp->qptype = attr->qp_type;
	qp->sq_qtype = resp.sq_qtype;
    qp->rq_qtype = resp.rq_qtype;
	qp->qpst = IBV_QPS_RESET;
	qp->scq = to_ionic_cq(attr->send_cq);
	qp->rcq = to_ionic_cq(attr->recv_cq);
	qp->udpi = &cntx->udpi;
    
	/* Save/return the altered Caps. */
	cap->max_ssge = attr->cap.max_send_sge;
	cap->max_rsge = attr->cap.max_recv_sge;
	cap->max_inline = attr->cap.max_inline_data;
	cap->sqsig = attr->sq_sig_all;

    cntx->qp_tbl[qp->qpid] = qp;
    qp->cntxt = cntx;
    
	return &qp->ibvqp;
failcmd:
	ionic_free_queues(qp);
fail:
	free(qp);

	return NULL;
}

int ionic_modify_qp(struct ibv_qp *ibvqp,
                  struct ibv_qp_attr *attr,
                  int attr_mask)
{
	struct ibv_modify_qp cmd = {};
	struct ionic_qp *qp = to_ionic_qp(ibvqp);
	int rc;

    IONIC_LOG("");
	/* Sanity check */
	if (!attr_mask)
		return 0;

	rc = ibv_cmd_modify_qp(ibvqp, attr, attr_mask, &cmd, sizeof(cmd));
	if (!rc) {
		if (attr_mask & IBV_QP_STATE) {
			qp->qpst = attr->qp_state;
			/* transition to reset */
			if (qp->qpst == IBV_QPS_RESET) {
				qp->sqq->head = 0;
				qp->sqq->tail = 0;
				if (qp->rqq) {
					qp->rqq->head = 0;
					qp->rqq->tail = 0;
				}
			}
		}
	}

	return rc;
}

int ionic_query_qp(struct ibv_qp *ibvqp,
                 struct ibv_qp_attr *attr,
                 int attr_mask,
                 struct ibv_qp_init_attr *init_attr)
{
	struct ibv_query_qp cmd;
	struct ionic_qp *qp = to_ionic_qp(ibvqp);
	int rc;

    IONIC_LOG("");
	rc = ibv_cmd_query_qp(ibvqp, attr, attr_mask, init_attr,
                          &cmd, sizeof(cmd));
	if (!rc)
		qp->qpst = ibvqp->state;

	init_attr->cap.max_inline_data = IONIC_MAX_INLINE_SIZE;

	attr->cap = init_attr->cap;
    
	return rc;
}

int ionic_destroy_qp(struct ibv_qp *ibvqp)
{
	struct ionic_qp *qp = to_ionic_qp(ibvqp);
	int status;

    IONIC_LOG("");
	status = ibv_cmd_destroy_qp(ibvqp);
	if (status)
		return status;

#if TODO    
	ionic_cleanup_cq(qp, qp->rcq);
	ionic_cleanup_cq(qp, qp->scq);
#endif
    
	ionic_free_queues(qp);
	free(qp);

	return 0;
}

#if 0
static inline uint8_t ionic_set_hdr_flags(struct ionic_bsqe *hdr,
					    uint32_t send_flags, uint8_t sqsig)
{
	uint8_t is_inline = false;
	uint32_t hdrval = 0;

	if (send_flags & IBV_SEND_SIGNALED || sqsig)
		hdrval |= ((IONIC_WR_FLAGS_SIGNALED & IONIC_HDR_FLAGS_MASK)
			    << IONIC_HDR_FLAGS_SHIFT);
	if (send_flags & IBV_SEND_FENCE)
		/*TODO: See when RD fence can be used. */
		hdrval |= ((IONIC_WR_FLAGS_UC_FENCE & IONIC_HDR_FLAGS_MASK)
			    << IONIC_HDR_FLAGS_SHIFT);
	if (send_flags & IBV_SEND_SOLICITED)
		hdrval |= ((IONIC_WR_FLAGS_SE & IONIC_HDR_FLAGS_MASK)
			    << IONIC_HDR_FLAGS_SHIFT);

	if (send_flags & IBV_SEND_INLINE) {
		hdrval |= ((IONIC_WR_FLAGS_INLINE & IONIC_HDR_FLAGS_MASK)
			    << IONIC_HDR_FLAGS_SHIFT);
		is_inline = true;
	}
	hdr->rsv_ws_fl_wt = htole32(hdrval);

	return is_inline;
}
#endif

static int ionic_build_sge(struct sge_t *sge, struct ibv_sge *sg_list,
                         uint32_t num_sge, uint8_t is_inline) {
	int indx, length = 0;
	void *dst;

    IONIC_LOG("");
	if (!num_sge) {
		memset(sge, 0, sizeof(*sge));
		return 0;
	}

	if (is_inline) {
		dst = sge;
		for (indx = 0; indx < num_sge; indx++) {
			length += sg_list[indx].length;
			if (length > IONIC_MAX_INLINE_SIZE)
				return -ENOMEM;
			memcpy(dst, (void *)(uintptr_t)sg_list[indx].addr,
			       sg_list[indx].length);
			dst = dst + sg_list[indx].length;
		}
	} else {
		for (indx = 0; indx < num_sge; indx++) {
			sge[indx].va = htobe64(sg_list[indx].addr);
			sge[indx].lkey = htobe32(sg_list[indx].lkey);
			sge[indx].len = htobe32(sg_list[indx].length);
			length += sg_list[indx].length;
		}
	}

	return length;
}

#if TODO
static void ionic_fill_psns(struct ionic_qp *qp, struct ionic_psns *psns,
			      uint8_t opcode, uint32_t len)
{
	uint32_t pkt_cnt = 0, nxt_psn;

	memset(psns, 0, sizeof(*psns));
	if (qp->qptype == IBV_QPT_RC) {
		psns->opc_spsn = htole32(qp->sq_psn & IONIC_PSNS_SPSN_MASK);
		pkt_cnt = (len / qp->mtu);
		if (len % qp->mtu)
			pkt_cnt++;
		nxt_psn = ((qp->sq_psn + pkt_cnt) & IONIC_PSNS_NPSN_MASK);
		psns->flg_npsn = htole32(nxt_psn);
		qp->sq_psn = nxt_psn;
	}
	opcode = ionic_ibv_wr_to_wc_opcd(opcode);
	psns->opc_spsn |= htole32(((opcode & IONIC_PSNS_OPCD_MASK) <<
				    IONIC_PSNS_OPCD_SHIFT));
}
#endif

#if TODO
static void ionic_fill_wrid(struct ionic_wrid *wrid, struct ibv_send_wr *wr,
			      uint32_t len, uint8_t sqsig)
{
	wrid->wrid = wr->wr_id;
	wrid->bytes = len;
	wrid->sig = 0;
	if (wr->send_flags & IBV_SEND_SIGNALED || sqsig)
		wrid->sig = IBV_SEND_SIGNALED;
}
#endif

static int ionic_build_send_sqe(struct ionic_qp      *qp, 
                                struct sqwqe_t     *wqe,
                                struct ibv_send_wr *wr, 
                                uint8_t             is_inline)
{
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	int len;
	uint8_t opcode;

    IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0) {
        IONIC_LOG("build sge failed");
		return len;
    }

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL) {
        IONIC_LOG("invalid opcode %d", wr->opcode);
		return -EINVAL;
    }

    wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
        // TODO
        wqe->u.non_atomic.wqe.send.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
        wqe->u.non_atomic.wqe.send.length = htobe32(len);
	}

    ionic_set_ibv_send_flags(wr->send_flags, wqe);

    wqe->base.wrid = wr->wr_id;
	return len;
}

static int ionic_build_ud_sqe(struct ionic_qp      *qp, 
                            struct sqwqe_t     *wqe,
                            struct ibv_send_wr *wr, 
                            uint8_t             is_inline)
{
	struct ionic_ah *ah;
	int len;
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	uint8_t opcode;

    IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0)
		return len;

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL)
		return -EINVAL;

    wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
        // TODO
        wqe->u.non_atomic.wqe.ud_send.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
        wqe->u.non_atomic.wqe.ud_send.length = len;
	}

    ionic_set_ibv_send_flags(wr->send_flags, wqe);

    wqe->base.wrid = wr->wr_id;

	wqe->u.non_atomic.wqe.ud_send.q_key = wr->wr.ud.remote_qkey;
	wqe->u.non_atomic.wqe.ud_send.dst_qp = wr->wr.ud.remote_qpn;

	if (!wr->wr.ud.ah) {
		return (-EINVAL);
	}

	ah = to_ionic_ah(wr->wr.ud.ah);
	wqe->u.non_atomic.wqe.ud_send.ah_handle = ah->avid;

    return len;
}

static int ionic_build_rdma_write_sqe(struct ionic_qp      *qp, 
                                    struct sqwqe_t     *wqe,
                                    struct ibv_send_wr *wr, 
                                    uint8_t             is_inline)
{
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	int len;
	uint8_t opcode;

    IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0)
		return len;

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL)
		return -EINVAL;

    wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
        // TODO
        wqe->u.non_atomic.wqe.write.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
        wqe->u.non_atomic.wqe.write.length = len;
	}

    ionic_set_ibv_send_flags(wr->send_flags, wqe);

    wqe->base.wrid = ++g_wrid;

	wqe->u.non_atomic.wqe.write.va = wr->wr.rdma.remote_addr;
	wqe->u.non_atomic.wqe.write.r_key = wr->wr.rdma.rkey;

	return len;
}

static int ionic_build_rdma_read_sqe(struct ionic_qp      *qp, 
                                   struct sqwqe_t     *wqe,
                                   struct ibv_send_wr *wr, 
                                   uint8_t             is_inline)
{
	struct sge_t *sge = (struct sge_t *)&wqe->u.non_atomic.inline_data;
	int len;
	uint8_t opcode;

    IONIC_LOG("");
	len = ionic_build_sge(sge, wr->sg_list, wr->num_sge, is_inline);
	if (len < 0)
		return len;

	/* Fill Header */
	opcode = ibv_to_ionic_wr_opcd(wr->opcode);
	if (opcode == IONIC_WR_OPCD_INVAL) {
        IONIC_LOG("invlid opcode %d", wr->opcode);
		return -EINVAL;
    }

    wqe->base.op_type = opcode & IONIC_WR_OPCODE_MASK;

	if (is_inline) {
        // TODO
        wqe->u.non_atomic.wqe.read.length = len;
	} else {
		wqe->base.num_sges = wr->num_sge;
        wqe->u.non_atomic.wqe.read.length = len;
	}

    ionic_set_ibv_send_flags(wr->send_flags, wqe);

    wqe->base.wrid = ++g_wrid;

	wqe->u.non_atomic.wqe.read.va = wr->wr.rdma.remote_addr;
	wqe->u.non_atomic.wqe.read.r_key = wr->wr.rdma.rkey;

	return len;
}

/*
 * TODO:
 * Assumption is driver always run on host with Little Endian format (BE endian
 * driver support needs to come later) and HCA is Big Endian format. For now
 * driver is doing all the endian conversions as needed. Later we need to
 * optimize this to see capri can take WR entry data in little endian format
 * without compromising performance.
 */
int ionic_post_send(struct ibv_qp *ibvqp, 
                  struct ibv_send_wr *wr,
                  struct ibv_send_wr **bad)
{
	struct ionic_qp *qp = to_ionic_qp(ibvqp);
	struct ionic_queue *sq = qp->sqq;
	struct sqwqe_t *sqe;
	int ret = 0, bytes = 0, nreq = 0;
	uint8_t is_inline = false;

    IONIC_LOG("");
	pthread_spin_lock(&sq->qlock);
	while (wr) {
		if ((qp->qpst != IBV_QPS_RTS) && (qp->qpst != IBV_QPS_SQD)) {
			*bad = wr;
			ret = -EINVAL;
            IONIC_LOG("Bad QP State %d", qp->qpst);
            goto out;
		}

		if ((qp->qptype == IBV_QPT_UD) &&
		    (wr->opcode != IBV_WR_SEND &&
		     wr->opcode != IBV_WR_SEND_WITH_IMM)) {
			*bad = wr;
			ret = -EINVAL;
            IONIC_LOG("Bad QP type %d opcode %d", qp->qptype, wr->opcode);
            goto out;
		}

		if (ionic_is_que_full(sq) ||
		    wr->num_sge > qp->cap.max_ssge) {
            IONIC_LOG("Queue Full");
			*bad = wr;
			ret = -ENOMEM;
            goto out;
		}

		sqe = (struct sqwqe_t *)(sq->va + (sq->tail * sq->stride));
		memset(sqe, 0, sizeof(struct sqwqe_t));

        sqe->base.wrid = wr->wr_id;
        if (wr->send_flags & IBV_SEND_INLINE) {
            is_inline = true;
            sqe->base.inline_data_vld = 1;
        }

		switch (wr->opcode) {
		case IBV_WR_SEND_WITH_IMM:
			sqe->u.non_atomic.wqe.send.imm_data = wr->imm_data;
			SWITCH_FALLTHROUGH;
		case IBV_WR_SEND:
			if (qp->qptype == IBV_QPT_UD)
				bytes = ionic_build_ud_sqe(qp, sqe, wr,
                                         is_inline);
			else
				bytes = ionic_build_send_sqe(qp, sqe, wr,
                                           is_inline);
			break;
		case IBV_WR_RDMA_WRITE_WITH_IMM:
			sqe->u.non_atomic.wqe.write.imm_data = wr->imm_data;
			SWITCH_FALLTHROUGH;
		case IBV_WR_RDMA_WRITE:
			bytes = ionic_build_rdma_write_sqe(qp, sqe, wr, is_inline);
			break;
		case IBV_WR_RDMA_READ:
			bytes = ionic_build_rdma_read_sqe(qp, sqe, wr, false);
			break;

#ifdef LATER
            //TODO
		case IBV_WR_ATOMIC_CMP_AND_SWP:
			bytes = ionic_build_cns_sqe(qp, sqe, wr);
			break;
		case IBV_WR_ATOMIC_FETCH_AND_ADD:
			bytes = ionic_build_fna_sqe(qp, sqe, wr);
			break;
#endif
		default:
			bytes = -EINVAL;
			break;
		}

		if (bytes < 0) {
			ret = (bytes == -EINVAL) ? EINVAL : ENOMEM;
			*bad = wr;
            IONIC_LOG("Invalid Bytes built");
			break;
		}

		ionic_incr_tail(sq);
		qp->wqe_cnt++;
        nreq++;
		wr = wr->next;
	}

out:
    if (nreq) {
		ionic_ring_sq_db(qp);
    }
	pthread_spin_unlock(&sq->qlock);
	return ret;
}

int ionic_post_recv(struct ibv_qp *ibvqp, 
                  struct ibv_recv_wr *wr,
                  struct ibv_recv_wr **bad)
{
	struct ionic_qp *qp = to_ionic_qp(ibvqp);
	struct ionic_queue *rq = qp->rqq;
	struct rqwqe_t *rqe;
	int ret = 0, nreq = 0;
	struct sge_t *sge;

    IONIC_LOG("");
	pthread_spin_lock(&rq->qlock);
	while (wr) {
		/* check QP state, abort if it is ERR or RST */
		if (qp->qpst == IBV_QPS_RESET || qp->qpst == IBV_QPS_ERR) {
			*bad = wr;
			ret = -EINVAL;
            goto out;
		}

		if (ionic_is_que_full(rq) ||
		    wr->num_sge > qp->cap.max_rsge) {
			*bad = wr;
			ret = -ENOMEM;
            goto out;
		}

		rqe = (struct rqwqe_t *)(rq->va + (rq->tail * rq->stride));
		memset(rqe, 0, sizeof(struct rqwqe_t));

        sge = (struct sge_t *) rqe->sge_arr;
        ionic_build_sge(sge, wr->sg_list, wr->num_sge, false);
        
        rqe->wrid = wr->wr_id;
        rqe->num_sges = wr->num_sge;
		ionic_incr_tail(rq);
        qp->wqe_cnt++;
		wr = wr->next;
        nreq++;
	}
	pthread_spin_unlock(&rq->qlock);

out:
    if (nreq) {
        ionic_ring_rq_db(qp);
    }
    pthread_spin_unlock(&rq->qlock);
	return ret;
}

struct ibv_srq *ionic_create_srq(struct ibv_pd *ibvpd,
				   struct ibv_srq_init_attr *attr)
{
    IONIC_LOG("");
	errno = ENOSYS;
	return NULL;
}

int ionic_modify_srq(struct ibv_srq *ibvsrq, struct ibv_srq_attr *attr,
		       int init_attr)
{
    IONIC_LOG("");
	return -ENOSYS;
}

int ionic_destroy_srq(struct ibv_srq *ibvsrq)
{
    IONIC_LOG("");
	return -ENOSYS;
}

int ionic_query_srq(struct ibv_srq *ibvsrq, struct ibv_srq_attr *attr)
{
    IONIC_LOG("");
	return -ENOSYS;
}

int ionic_post_srq_recv(struct ibv_srq *ibvsrq, struct ibv_recv_wr *wr,
			  struct ibv_recv_wr **bad)
{
    IONIC_LOG("");
	return -ENOSYS;
}

struct ibv_ah *ionic_create_ah(struct ibv_pd *pd,
                                struct ibv_ah_attr *attr)
{
    IONIC_LOG("");
	errno = ENOSYS;
	return NULL;
}

int ionic_destroy_ah(struct ibv_ah *ah)
{
    IONIC_LOG("");
	errno = ENOSYS;
	return -1;
}

