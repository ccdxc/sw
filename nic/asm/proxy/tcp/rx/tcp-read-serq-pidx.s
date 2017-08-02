/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - RNMDR alloc idx
	      - RNMPR alloc idx
	      - TNMDR alloc idx
	      - SERQ producer idx
 */

#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	serq_pidx			: 8    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	semaphore_addr			: ADDRESS_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
%%
	
flow_read_serq_pidx:
	phvwr.e		p.serq_pidx, d.serq_pidx
	nop.e
