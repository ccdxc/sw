#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
	
/* d is the data returned by lookup result */
struct d_struct {
	TLS_SHARED_STATE
};

struct k_struct {
	sesq_pidx			 : RING_INDEX_WIDTH ;
        desc_scratch                     :  28;
        desc_num_entries                 :  2;
        desc_data_len                    :  18;
        desc_head_idx                    :  2;
        desc_tail_idx                    :  2;
        desc_offset                      :  8;

        desc_aol0_scratch                :  64;
        desc_aol0_free_pending           :  1;
        desc_aol0_valid                  :  1;
        desc_aol0_addr                   :  30;
        desc_aol0_offset                 :  16;
        desc_aol0_len                    :  16;

        desc_aol1_scratch                :  64;
        desc_aol1_free_pending           :  1;
        desc_aol1_valid                  :  1;
        desc_aol1_addr                   :  30;
        desc_aol1_offset                 :  16;
        desc_aol1_len                    :  16;

	fid                              :  16;
	desc	                         :  ADDRESS_WIDTH ;
	next_tls_hdr_offset              :  16		  ;
	enc_flow                         :  1		  ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

d = {
        cipher_type                     = 51;
        ver_major			= 0x03;
	ver_minor			= 0x03;
	key_addr		        = 0xF0;
	iv_addr		                = 0xF1;
	qhead			        = 0x0;
	qtail			        = 0x0;
	una_desc			= 0x0;
	una_desc_idx			= 0x0;
	una_data_offset			= 0x0;
	una_data_len			= 0x0;
	nxt_desc			= 0x0;
	nxt_desc_idx			= 0x0;
	nxt_data_offset			= 0x0;
	nxt_data_len			= 0x0;
       	next_tls_hdr_offset		= 0x0;
       	cur_tls_data_len		= 0x0;
        ofid		                = 200;
};

k =  {
	sesq_pidx			= 0;
        desc_scratch			= 0;
	desc_num_entries		= 1;
        desc_data_len                   = 9000;
        desc_head_idx                   = 0;
        desc_tail_idx                   = 0;
        desc_offset                     = 0;

        desc_aol0_scratch               = 0;
        desc_aol0_free_pending          = 0;
        desc_aol0_valid                 = 1;
        desc_aol0_addr                  = 0xA0000;
        desc_aol0_offset                = 64;
        desc_aol0_len                   = 9000;

        desc_aol1_scratch               = 0;
        desc_aol1_free_pending          = 0;
        desc_aol1_valid                 = 1;
        desc_aol1_addr                  = 0;
        desc_aol1_offset                = 0;
        desc_aol1_len                   = 0;

	fid                             = 150;
	desc	                        = 0xCD;
	next_tls_hdr_offset             = 0;
	enc_flow                        = 0;
};
