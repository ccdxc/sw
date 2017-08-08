/* This file is auto-generated. Changes will be overwritten! */
/* p4pd.i */
%module p4pd
%{
	#include "p4pd.h"
	typedef int p4pd_error_t;
	p4pd_error_t p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey, void *swkey_mask, void *actiondata);
%}

typedef int p4pd_error_t;
p4pd_error_t p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey, void *swkey_mask, void *actiondata);
