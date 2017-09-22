/*
 * 	Implements the reading of SERQ to pick up the new descriptor to process
 */

#include "ingress.h"
#include "INGRESS_p.h"
#include "capri-macros.h"

struct phv_ p	;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_d d;
%%

	.param	        esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table
   	.param	        esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table	
	
esp_ipv4_tunnel_h2n_txdma_stage0:
     CAPRI_OPERAND_DEBUG(r7)
	.brbegin
	    brpri		r7[1:0], [0,1]
	    nop
	        .brcase 0
	            j esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table 
	            nop
	        .brcase 1
	            j esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table
	            nop
	        .brcase 2
	            j esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table 
	            nop
	.brend

	nop.e
    nop
