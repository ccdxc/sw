/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

pnso_error_t
cpdc_common_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

void
cpdc_common_teardown(void *desc)
{
	/* EOPNOTSUPP */
}

void
cpdc_pprint_desc(const struct cpdc_desc *desc)
{
	/* EOPNOTSUPP */
}

void
cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc)
{
	/* EOPNOTSUPP */
}

pnso_error_t
cpdc_convert_buffer_list_to_sgl(struct service_info *svc_info,
		const struct pnso_buffer_list *src_buf,
		const struct pnso_buffer_list *dst_buf)
{
	return EOPNOTSUPP;
}

void
cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list)
{
	/* EOPNOTSUPP */
}

void
cpdc_release_sgl(struct cpdc_sgl *sgl)
{
	/* EOPNOTSUPP */
}

pnso_error_t
cpdc_convert_desc_error(int error)
{
	return EOPNOTSUPP;
}
