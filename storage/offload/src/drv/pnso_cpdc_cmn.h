#ifndef __PNSO_CPDC_CMN_H__
#define __PNSO_CPDC_CMN_H__

#include "pnso_chain.h"

struct pnso_compression_header {
	uint32_t chksum;
	uint16_t data_len;
	uint16_t version;
};

/* CPDC common/utility functions */
pnso_error_t cpdc_common_chain(struct chain_entry *centry);

void cpdc_common_teardown(void *desc);

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_convert_buffer_list_to_sgl(struct service_info *svc_info,
		struct pnso_buffer_list *src_buf,
		struct pnso_buffer_list *dst_buf);

void cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list);

void cpdc_release_sgl(struct cpdc_sgl *sgl);

pnso_error_t cpdc_convert_desc_error(int error);

#endif /* __PNSO_CPDC_CMN_H__ */
