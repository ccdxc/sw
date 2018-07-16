#ifndef __PNSO_XTS_CMN_H__
#define __PNSO_XTS_CMN_H__

#include "pnso_chain.h"

/* XTS common/utility functions */
pnso_error_t xts_common_chain(struct chain_entry *centry);

void xts_common_teardown(void *desc);

void xts_pprint_desc(const struct xts_desc *desc);

#endif /* __PNSO_XTS_CMN_H__ */
