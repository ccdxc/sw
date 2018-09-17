#ifndef __PNSO_CPDC_CMN_H__
#define __PNSO_CPDC_CMN_H__

#include "pnso_chain.h"

struct pnso_compression_header {
	uint32_t chksum;
	uint16_t data_len;
	uint16_t version;
};

#define MAX_CPDC_SRC_BUF_LEN	(1 << 16)
#define MAX_CPDC_DST_BUF_LEN	MAX_CPDC_SRC_BUF_LEN

/* status reported by hardware */
#define CP_STATUS_SUCCESS		0
#define CP_STATUS_AXI_TIMEOUT		1
#define CP_STATUS_AXI_DATA_ERROR	2
#define CP_STATUS_AXI_ADDR_ERROR	3
#define CP_STATUS_COMPRESSION_FAILED	4
#define CP_STATUS_DATA_TOO_LONG		5
#define CP_STATUS_CHECKSUM_FAILED	6
#define CP_STATUS_SGL_DESC_ERROR	7

/* HW to update 'partial_data' in status descriptor */
#define CPDC_CP_STATUS_DATA		1234
#define CPDC_DC_STATUS_DATA		2345
#define CPDC_HASH_STATUS_DATA		3456
#define CPDC_CHKSUM_STATUS_DATA		4567

/* CPDC common/utility functions */
pnso_error_t cpdc_common_chain(struct chain_entry *centry);

void cpdc_common_teardown(void *desc);

pnso_error_t cpdc_common_read_status(struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc);

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_update_service_info_sgl(struct service_info *svc_info,
		const struct service_params *svc_params);

pnso_error_t cpdc_update_service_info_sgls(struct service_info *svc_info,
		const struct service_params *svc_params);

void cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list);

void cpdc_release_sgl(struct cpdc_sgl *sgl);

struct cpdc_desc *
cpdc_get_desc(struct per_core_resource *pc_res, bool per_block);

pnso_error_t
cpdc_put_desc(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_desc *desc);

struct cpdc_status_desc *
cpdc_get_status_desc(struct per_core_resource *pc_res, bool per_block);

pnso_error_t
cpdc_put_status_desc(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_status_desc *desc);

void cpdc_get_desc_size(uint32_t *object_size, uint32_t *pad_size);

void cpdc_get_status_desc_size(uint32_t *object_size, uint32_t *pad_size);

typedef void (*fill_desc_fn_t) (uint32_t algo_type,
		uint32_t buf_len, bool flat_buf, void *src_buf,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc);

uint32_t cpdc_fill_per_block_desc(uint32_t algo_type, uint32_t block_size,
		uint32_t src_buf_len, struct cpdc_sgl *src_sgl,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc,
		fill_desc_fn_t fill_desc_fn);

pnso_error_t cpdc_convert_desc_error(int error);

#endif /* __PNSO_CPDC_CMN_H__ */
