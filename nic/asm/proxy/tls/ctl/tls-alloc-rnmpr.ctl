#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	opage				: ADDRESS_WIDTH    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	fid				: 32 ;
	RNMPR_alloc_idx			: RING_INDEX_WIDTH ;
	write_serq			: 1		   ;
	desc				: ADDRESS_WIDTH	   ;
};
struct p_struct p;
struct k_struct k;
struct d_struct d;

d = {
	opage				= 0xFA;
};

/* Readonly Parsed packet header info for the current packet */
k = {
	fid				= 150;
	RNMPR_alloc_idx			= 234;
	write_serq			= 1;
	desc				= 0xD0;
};
