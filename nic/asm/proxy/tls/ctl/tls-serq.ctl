#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
	
/* d is the data returned by lookup result */
struct d_struct {
        seqe_fid                         : 16;
        seqe_desc                        : ADDRESS_WIDTH;
};

/* SERQ consumer index */
struct k_struct {
	serq_consumer_idx		: RING_INDEX_WIDTH ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

d = {
  seqe_fid		= 150;
  seqe_desc		= 0xAB;
};

k = {
  serq_consumer_idx	= 17;
};

r4 = 0xC0;