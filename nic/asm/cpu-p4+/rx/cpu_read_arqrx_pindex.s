#include "INGRESS_p.h"
#include "ingress.h"
#include "capri-macros.h"
#include "cpu-table.h"

struct phv_ p;
struct cpu_rx_read_arqrx_k k;
struct cpu_rx_read_arqrx_d d;

%%
    .align
cpu_rx_read_arqrx_pindex_start:
    CAPRI_CLEAR_TABLE3_VALID
    
    // TODO: CPU-id should be calculated based on hash rather than 0 
    CPU_ARQ_PIDX_READ_INC(r4, 0, d, u.read_arqrx_d.pi_0)
    
    // pindex will be in r4
    phvwr   p.to_s3_arqrx_pindex, r4
    nop.e
    nop
