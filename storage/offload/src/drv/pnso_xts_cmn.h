#ifndef __PNSO_XTS_CMN_H__
#define __PNSO_XTS_CMN_H__

#include "pnso_chain.h"

#ifdef NDEBUG
#define XTS_PPRINT_DESC(d)
#else
#define XTS_PPRINT_DESC(d)						       \
	do {								\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
		xts_pprint_desc(d);					\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
	} while (0)
#endif

/* XTS common/utility functions */
void xts_pprint_desc(const struct xts_desc *desc);

#endif /* __PNSO_XTS_CMN_H__ */
