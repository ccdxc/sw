#ifndef __VERBS_H__
#define __VERBS_H__

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

#include <infiniband/driver.h>
#include <infiniband/verbs.h>

int ionic_query_device(struct ibv_context *uctx,
			 struct ibv_device_attr *attr);
int ionic_query_port(struct ibv_context *uctx, uint8_t port,
		       struct ibv_port_attr *attr);
struct ibv_pd *ionic_alloc_pd(struct ibv_context *uctx);
int ionic_free_pd(struct ibv_pd *ibvpd);
struct ibv_mr *ionic_reg_mr(struct ibv_pd *ibvpd, void *buf, size_t len,
			      int ibv_access_flags);
int ionic_dereg_mr(struct ibv_mr *ibvmr);

struct ibv_cq *ionic_create_cq(struct ibv_context *uctx, int ncqe,
				 struct ibv_comp_channel *ch, int vec);
int ionic_resize_cq(struct ibv_cq *ibvcq, int ncqe);
int ionic_destroy_cq(struct ibv_cq *ibvcq);
int ionic_poll_cq(struct ibv_cq *ibvcq, int nwc, struct ibv_wc *wc);
void ionic_cq_event(struct ibv_cq *ibvcq);
int ionic_arm_cq(struct ibv_cq *ibvcq, int flags);

struct ibv_qp *ionic_create_qp(struct ibv_pd *ibvpd,
				 struct ibv_qp_init_attr *attr);
int ionic_modify_qp(struct ibv_qp *ibvqp, struct ibv_qp_attr *attr,
		      int ibv_qp_attr_mask);
int ionic_query_qp(struct ibv_qp *ibvqp, struct ibv_qp_attr *attr,
		     int attr_mask, struct ibv_qp_init_attr *init_attr);
int ionic_destroy_qp(struct ibv_qp *ibvqp);
int ionic_post_send(struct ibv_qp *ibvqp, struct ibv_send_wr *wr,
		      struct ibv_send_wr **bad);
int ionic_post_recv(struct ibv_qp *ibvqp, struct ibv_recv_wr *wr,
		      struct ibv_recv_wr **bad);

struct ibv_srq *ionic_create_srq(struct ibv_pd *ibvpd,
				   struct ibv_srq_init_attr *attr);
int ionic_modify_srq(struct ibv_srq *ibvsrq,
		       struct ibv_srq_attr *attr, int mask);
int ionic_destroy_srq(struct ibv_srq *ibvsrq);
int ionic_query_srq(struct ibv_srq *ibvsrq, struct ibv_srq_attr *attr);
int ionic_post_srq_recv(struct ibv_srq *ibvsrq, struct ibv_recv_wr *wr,
			  struct ibv_recv_wr **bad);

struct ibv_ah *ionic_create_ah(struct ibv_pd *ibvpd,
				 struct ibv_ah_attr *attr);
int ionic_destroy_ah(struct ibv_ah *ibvah);

#endif /* __IONIC_VERBS_H__ */
