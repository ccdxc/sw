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

#ifdef NDEBUG
#define CPDC_PPRINT_DESC(d)
#define CPDC_PPRINT_STATUS_DESC(d)
#else
#define CPDC_PPRINT_DESC(d)						       \
	do {								\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
		cpdc_pprint_desc(d);					\
		OSAL_LOG_DEBUG("%.*s", 30, "=========================================");\
	} while (0)
#define CPDC_PPRINT_STATUS_DESC(d)	cpdc_pprint_status_desc(d)
#endif

/* CPDC common/utility functions */
pnso_error_t cpdc_common_chain(struct chain_entry *centry);

pnso_error_t cpdc_poll(const struct service_info *svc_info);

pnso_error_t cpdc_common_read_status(struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc);

void cpdc_pprint_desc(const struct cpdc_desc *desc);

void cpdc_pprint_status_desc(const struct cpdc_status_desc *status_desc);

pnso_error_t cpdc_update_service_info_sgl(struct service_info *svc_info);
pnso_error_t cpdc_update_service_info_sgls(struct service_info *svc_info);

struct cpdc_desc *cpdc_get_desc(struct per_core_resource *pc_res,
		bool per_block);

pnso_error_t cpdc_put_desc(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_desc *desc);

struct cpdc_status_desc *cpdc_get_status_desc(struct per_core_resource *pc_res,
		bool per_block);

pnso_error_t cpdc_put_status_desc(struct per_core_resource *pc_res,
		bool per_block, struct cpdc_status_desc *desc);

struct cpdc_sgl *cpdc_get_sgl(struct per_core_resource *pc_res, bool per_block);

pnso_error_t cpdc_put_sgl(struct per_core_resource *pc_res, bool per_block,
		struct cpdc_sgl *sgl);

/* TODO-batch: Fix the naming */
struct cpdc_desc *cpdc_get_batch_bulk_desc(struct mem_pool *mpool);

pnso_error_t cpdc_put_batch_bulk_desc(struct mem_pool *mpool,
		struct cpdc_desc *desc);

struct cpdc_desc *cpdc_get_batch_desc(struct service_info *svc_info);

pnso_error_t cpdc_put_batch_desc(const struct service_info *svc_info,
		struct cpdc_desc *desc);

struct cpdc_desc *cpdc_get_desc_ex(struct service_info *svc_info,
		bool per_block);

pnso_error_t cpdc_put_desc_ex(const struct service_info *svc_info,
		bool per_block, struct cpdc_desc *desc);

uint32_t cpdc_get_desc_size(void);

uint32_t cpdc_get_status_desc_size(void);

uint32_t cpdc_get_sgl_size(void);

struct cpdc_status_desc *cpdc_get_next_status_desc(
		struct cpdc_status_desc *desc, uint32_t object_size);

typedef void (*fill_desc_fn_t) (uint32_t algo_type,
		uint32_t buf_len, bool flat_buf, void *src_buf,
		struct cpdc_desc *desc, struct cpdc_status_desc *status_desc);

uint32_t cpdc_fill_per_block_desc(uint32_t algo_type, uint32_t block_size,
		uint32_t src_buf_len, struct pnso_buffer_list *src_blist,
		struct cpdc_sgl *sgl, struct cpdc_desc *desc,
		struct cpdc_status_desc *status_desc,
		fill_desc_fn_t fill_desc_fn);

struct service_deps *cpdc_get_service_deps(const struct service_info *svc_info);

bool cpdc_is_service_in_batch(uint8_t flags);

pnso_error_t cpdc_setup_batch_desc(struct service_info *svc_info,
		struct cpdc_desc *desc);

pnso_error_t cpdc_convert_desc_error(int error);

#endif /* __PNSO_CPDC_CMN_H__ */
