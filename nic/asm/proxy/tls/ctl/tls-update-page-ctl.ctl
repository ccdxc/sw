#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	flags				 : 12 ;
	size				 : 4  ;
	refcnt				 : 8  ;
	rsvd				 : 8  ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	page_addr                        : ADDRESS_WIDTH ;

	pending_rx_brq                   : 1		 ;
	pending_rx_serq                  : 1		 ;
	enc_flow                         : 1		 ;
	
};
struct p_struct p;
struct k_struct k;
struct d_struct d;

 /* d is the data returned by lookup result */
d =  {
	flags				 = 0x0 ;
	size				 = 0xF ;
	refcnt				 = 1  ;
	rsvd				 = 0  ;
};

/* Readonly Parsed packet header info for the current packet */
k =  {
	page_addr                        = 0xA0 ;

	pending_rx_brq                   = 0		 ;
	pending_rx_serq                  = 1		 ;
	enc_flow                         = 0		 ;
	
};
