#ifndef __PNSO_CPDC_CMN_H__
#define __PNSO_CPDC_CMN_H__

#include "pnso_chain.h"

struct pnso_compression_header {
	uint32_t chksum;
	uint16_t data_len;
	uint16_t version;
};

extern struct mem_pool *cpdc_mpool;
extern struct mem_pool *cpdc_sgl_mpool;
extern struct mem_pool *cpdc_status_mpool;

extern struct mem_pool *cpdc_bulk_mpool;
extern struct mem_pool *cpdc_status_bulk_mpool;

#define MAX_CP_THRESHOLD_LEN	(1 << 16)

/* status reported by hardware */
#define CP_STATUS_SUCCESS		0
#define CP_STATUS_AXI_TIMEOUT		1
#define CP_STATUS_AXI_DATA_ERROR	2
#define CP_STATUS_AXI_ADDR_ERROR	3
#define CP_STATUS_COMPRESSION_FAILED	4
#define CP_STATUS_DATA_TOO_LONG		5
#define CP_STATUS_CHECKSUM_FAILED	6
#define CP_STATUS_SGL_DESC_ERROR	7

/* CPDC common/utility functions */
pnso_error_t cpdc_common_chain(struct chain_entry *centry);

void cpdc_common_teardown(void *desc);

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_convert_buffer_list_to_sgl(struct service_info *svc_info,
		const struct pnso_buffer_list *src_buf,
		const struct pnso_buffer_list *dst_buf);

struct cpdc_sgl	*cpdc_convert_buffer_list_to_sgl_ex(
		const struct pnso_buffer_list *buf_list);

void cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list);

void cpdc_release_sgl(struct cpdc_sgl *sgl);

pnso_error_t cpdc_convert_desc_error(int error);

#endif /* __PNSO_CPDC_CMN_H__ */
