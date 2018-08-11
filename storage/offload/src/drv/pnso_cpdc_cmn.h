#ifndef __PNSO_CPDC_CMN_H__
#define __PNSO_CPDC_CMN_H__

#include "pnso_chain.h"

struct pnso_compression_header {
	uint32_t chksum;
	uint16_t data_len;
	uint16_t version;
};

#define MAX_CPDC_SRC_BUF_LEN	(1 << 16)

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

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_update_service_info_params(struct service_info *svc_info,
		const struct service_params *svc_params);

void cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list);

void cpdc_release_sgl(struct cpdc_sgl *sgl);

pnso_error_t cpdc_convert_desc_error(int error);

#endif /* __PNSO_CPDC_CMN_H__ */
