/*
 *      Read the TLSCB 2nd block for config data
 * Stage 3, Table 1
 */

#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tx_table_s3_t1_d d;
        
%%

tls_dec_aesgcm_read_tlscb_blk2:	

    //CAPRI_CLEAR_TABLE1_VALID

    phvwr p.iv_salt, d.u.read_tlscb_blk2_d.salt

    nop.e
    nop
