/*
 *	Get an index and auto increment it.
 *      This stage will be used to get
	      - BRQ producer idx
 */

#include "tls-phv.h"
#include "tls-shared-state.h"
#include "tls-macros.h"
#include "tls-table.h"

 /* d is the data returned by lookup result */
struct d_struct {
	brq_pidx			: 8    ;
};

/* Readonly Parsed packet header info for the current packet */
struct k_struct {
	semaphore_addr			: ADDRESS_WIDTH ;
	fid                             : 16		;
};

struct p_struct p	;
struct k_struct k	;
struct d_struct d	;
	
d = {
        brq_pidx                        = 135;
};

k = {
        fid                             = 150;
};

