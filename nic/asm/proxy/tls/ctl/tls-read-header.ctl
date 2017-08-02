#include "tls-constants.h"
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

struct d_struct {
        tls_hdr_type                     : 8;
        tls_hdr_version_major            : 8;
        tls_hdr_version_minor            : 8;
        tls_hdr_len                      : 16;
};

struct k_struct {
	tls_hdr_addr			 : ADDRESS_WIDTH ;
	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;
	enc_flow                         : 1		 ;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;

d = {
        tls_hdr_type                     = NTLS_RECORD_DATA;
        tls_hdr_version_major            = NTLS_TLS_1_2_MAJOR;
        tls_hdr_version_minor            = NTLS_TLS_1_2_MINOR;
        tls_hdr_len                      = 8995;
};

k = {
	tls_hdr_addr			 = 0xA0;
	pending_rx_brq                   = 0;
	pending_rx_serq                  = 1;
	enc_flow                         = 0;

};
