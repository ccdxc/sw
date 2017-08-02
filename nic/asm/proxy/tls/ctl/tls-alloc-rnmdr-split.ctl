
#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"
#include "tls-constants.h"
	
 /* d is the data returned by lookup result */
struct d_struct {
	odesc				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				 : 32 ;
	RNMDR_alloc_idx			 : RING_INDEX_WIDTH ;
	write_serq			 : 1		   ;


        tls_hdr_type                     : 8;
        tls_hdr_version_major            : 8;
        tls_hdr_version_minor            : 8;
        tls_hdr_len                      : 16;

	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;

	desc                             : ADDRESS_WIDTH ;
	desc_aol0_addr                   : 30 ;
	desc_aol0_offset                 : 16 ;
	desc_aol0_len                    : 16 ;

	desc_aol1_addr                   : 30 ;
	desc_aol1_offset                 : 16 ;
	desc_aol1_len                    : 16 ;
	
	enc_flow                         : 1 ;

};
struct p_struct p;
struct k_struct k;
struct d_struct d;

 /* d is the data returned by lookup result */
d = {
	odesc				 = 0xB0    ;
};

/* Readonly Parsed packet header info for the current packet */
k = {
	fid				 = 150 ;
	RNMDR_alloc_idx			 = 123 ;
	write_serq			 = 1		   ;


	pending_rx_brq                   = 0		 ;
	pending_rx_serq                  = 1		 ;

	desc                             = 0xAB;
        desc_aol0_addr                   = 0xA0000;
        desc_aol0_offset                 = 64;
        desc_aol0_len                    = 9000;

        desc_aol1_addr                   = 0;
        desc_aol1_offset                 = 0;
        desc_aol1_len                    = 0;

	enc_flow                         = 0 ;

};
