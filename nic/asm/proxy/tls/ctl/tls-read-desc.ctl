#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

struct d_struct {
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
};

struct k_struct {
	fid				: 32 ;
	desc				: ADDRESS_WIDTH ;
	pending_rx_brq			: 1  ;
	pending_rx_serq			: 1  ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

d = {
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
};

k =  {
	fid				= 150;
	desc				= 0xCD;
	pending_rx_brq			= 0;
	pending_rx_serq			= 1;
};

r4 = 0xC0;