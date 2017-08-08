/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMDR alloc idx
	      - RNMPR alloc idx
	      - TNMDR alloc idx
	      - SERQ producer idx
 */

#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tcp_rx_read_serq_read_serq_d d;
	
%%
	
tcp_rx_read_serq_pidx_stage2_start:
	phvwr.e		p.to_s6_serq_pidx, d.serq_pidx
	nop.e
