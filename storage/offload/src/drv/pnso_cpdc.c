/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_chain.h"
#include "pnso_cpdc.h"

#if 0
struct desc_pool *cpdc_dpool;
struct desc_pool *cpdc_sgl_dpool;
struct desc_pool *cpdc_status_nosha_dpool;
struct desc_pool *cpdc_status_sha256_dpool;
struct desc_pool *cpdc_status_sha512_dpool;

static void
__pprint_cp_cmd(struct cpdc_cmd *cmd)
{
	if (!cmd)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_enabled", cmd->cc_enabled);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_header_present",
			cmd->cc_header_present);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_insert_header",
			cmd->cc_insert_header);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_db_on", cmd->cc_db_on);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_otag_on", cmd->cc_otag_on);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_src_is_list",
			cmd->cc_src_is_list);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_dst_is_list",
			cmd->cc_dst_is_list);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_chksum_verify_enabled",
			cmd->cc_chksum_verify_enabled);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_chksum_adler",
			cmd->cc_chksum_adler);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_hash_enabled",
			cmd->cc_hash_enabled);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_hash_type",
			cmd->cc_hash_type);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_integrity_src",
			cmd->cc_integrity_src);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cc_integrity_type",
			cmd->cc_integrity_type);
}

void
cpdc_pprint_desc(struct cpdc_desc *desc)
{
	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cpdc_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cd_src", desc->cd_src);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cd_dst", desc->cd_dst);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== cpdc_cmd", &desc->u.cd_bits);
	__pprint_cp_cmd(&desc->u.cd_bits);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cd_datain_len",
			desc->cd_datain_len);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cd_extended_len",
			desc->cd_extended_len);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cd_threshold_len",
			desc->cd_threshold_len);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cd_status_addr",
			desc->cd_status_addr);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cd_db_addr",
			desc->cd_db_addr);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cd_db_data",
			desc->cd_db_data);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cd_otag_addr",
			desc->cd_otag_addr);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cd_otag_data",
			desc->cd_otag_data);

	/* TODO: group near cd_status_addr */
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cd_status_data",
			desc->cd_status_data);
}

void
cpdc_pprint_status_nosha(struct cpdc_status_nosha *status_nosha)
{
	if (!status_nosha)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== status_nosha", status_nosha);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "csn_err",
			status_nosha->csn_err);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "csn_valid",
			status_nosha->csn_valid);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "csn_output_data_len",
			status_nosha->csn_output_data_len);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "csn_partial_data",
			status_nosha->csn_partial_data);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "csn_integrity_data",
			status_nosha->csn_integrity_data);
}

void
cpdc_pprint_status_sha256(struct cpdc_status_sha256 *status_sha256)
{
	if (!status_sha256)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== status_sha256", status_sha256);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css256_err",
			status_sha256->css256_err);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css256_valid",
			status_sha256->css256_valid);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css256_output_data_len",
			status_sha256->css256_output_data_len);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css256_partial_data",
			status_sha256->css256_partial_data);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css256_integrity_data",
			status_sha256->css256_integrity_data);

	/* TODO: print SHA */
}

void
cpdc_pprint_status_sha512(struct cpdc_status_sha512 *status_sha512)
{
	if (!status_sha512)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== status_sha512", status_sha512);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css512_err",
			status_sha512->css512_err);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css512_valid",
			status_sha512->css512_valid);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css512_output_data_len",
			status_sha512->css512_output_data_len);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css512_partial_data",
			status_sha512->css512_partial_data);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "css512_integrity_data",
			status_sha512->css512_integrity_data);

	/* TODO: print SHA */
}

void
cpdc_release_sgl(struct cpdc_sgl *sgl)
{
	pnso_error_t err;
	struct cpdc_sgl *sgl_next;

	if (!sgl)
		return;

	while (sgl) {
		sgl_next = (struct cpdc_sgl *) sgl->cs_next;
		sgl->cs_next = 0;

		err = dpool_put_desc(cpdc_sgl_dpool, sgl);
		if (err) {
			PNSO_LOG_ERROR(err, "failed to return cp sgl desc to pool!");
			assert(0);
		}

		sgl = sgl_next;
	}
}

static struct cpdc_sgl *
__populate_sgl(struct pnso_buffer_list *buf_list)
{
	pnso_error_t err;
	struct cpdc_sgl *sgl_head = NULL;
	struct cpdc_sgl *sgl_prev = NULL;
	struct cpdc_sgl *sgl = NULL;
	uint32_t count;
	void *ptr;
	uint32_t i;

	count = buf_list->count;
	while (count) {
		sgl = (struct cpdc_sgl *) dpool_get_desc(cpdc_sgl_dpool);
		if (!sgl) {
			err = ENOMEM;
			PNSO_LOG_ERROR(err, "cannot obtain cp sgl desc from pool!");
			goto out;
		}
		memset(sgl, 0, sizeof(struct cpdc_sgl));

		if (!sgl_head)
			sgl_head = sgl;
		else
			sgl_prev->cs_next = (uint64_t) sgl;

		i = 0;
		ptr = (void *) buf_list->buffers[i].buf;
		sgl->cs_addr_0 = (uint64_t) pnso_virt_to_phys(ptr);
		sgl->cs_len_0 = buf_list->buffers[i].len;
		i++;
		count--;

		if (count == 0) {
			sgl->cs_next = 0;
			break;
		}

		if (count) {
			ptr = (void *) buf_list->buffers[i].buf;
			sgl->cs_addr_1 = (uint64_t) pnso_virt_to_phys(ptr);
			sgl->cs_len_1 = buf_list->buffers[i].len;
			i++;
			count--;
		} else {
			sgl->cs_next = 0;
			break;
		}

		if (count) {
			ptr = (void *) buf_list->buffers[i].buf;
			sgl->cs_addr_2 = (uint64_t) pnso_virt_to_phys(ptr);
			sgl->cs_len_2 = buf_list->buffers[i].len;
			i++;
			count--;
		} else {
			sgl->cs_next = 0;
			break;
		}

		sgl_prev = sgl;
	}

	return sgl_head;

out:
	cpdc_release_sgl(sgl_head);
	return NULL;
}

pnso_error_t
cpdc_convert_buffer_list_to_sgl(struct service_info *svc_info,
		struct pnso_buffer_list *src_buf,
		struct pnso_buffer_list *dst_buf)
{
	pnso_error_t err;
	bool is_sgl;

	is_sgl = pbuf_is_buffer_list_sgl(src_buf);
	if (!is_sgl) {
		err = EINVAL;
		// goto out;	/* TODO */
	}
	svc_info->si_src_sgl = __populate_sgl(src_buf);

	is_sgl = pbuf_is_buffer_list_sgl(dst_buf);
	if (!is_sgl) {
		err = EINVAL;
		// goto out;	/* TODO */
	}
	svc_info->si_dst_sgl = __populate_sgl(dst_buf);

	err = PNSO_OK;
// out:
	return err;
}

void
cpdc_populate_buffer_list(struct cpdc_sgl *sgl,
		struct pnso_buffer_list *buf_list)
{
	struct cpdc_sgl *sgl_head = NULL;
	uint32_t i;

	if (!sgl_head || !buf_list)
		return;

	i = 0;
	sgl_head = sgl;
	while (sgl) {
		if (sgl->cs_len_0 > 0) {
			buf_list->buffers[i].buf = sgl->cs_addr_0;
			buf_list->buffers[i].len = sgl->cs_len_0;
			i++;
		}

		if (sgl->cs_len_1 > 0) {
			buf_list->buffers[i].buf = sgl->cs_addr_1;
			buf_list->buffers[i].len = sgl->cs_len_1;
			i++;
		}

		if (sgl->cs_len_2 > 0) {
			buf_list->buffers[i].buf = sgl->cs_addr_2;
			buf_list->buffers[i].len = sgl->cs_len_2;
			i++;
		}

		sgl = (struct cpdc_sgl *) sgl->cs_next;
	}
	buf_list->count = i;

	// TODO:  will be taken care in teardown
	// cpdc_release_sgl(sgl_head);
}


pnso_error_t
cpdc_common_chain(struct chain_entry *centry)
{
	pnso_error_t err;
	struct chain_entry *centry_next;
	struct service_info *svc_info;

	/* NULL denotes terminal centry or valid error */
	if (!centry)
		return PNSO_OK;

	err = PNSO_OK;
	PNSO_LOG_INFO(PNSO_OK, "get and set sequencer info");

	centry_next = centry->ce_next;
	if (!centry_next) {
		err = PNSO_OK;
		PNSO_LOG_INFO(err, "nothing more to chain");
		goto out;
	}

	/* get next in list to chain */
	svc_info = &centry_next->ce_sinfo;
	err = svc_info->si_ops.chain(centry_next);
	if (err) {
		PNSO_LOG_INFO(err, "failed to chain");
		goto out;
	}

out:
	return err;
}

void
cpdc_common_teardown(void *desc)
{
	pnso_error_t err;
	struct cpdc_desc *cp_desc;

	if (!desc)
		return;

	cp_desc = (struct cpdc_desc *) desc;
	err = dpool_put_desc(cpdc_dpool, cp_desc);
	if (err)
		assert(0);

	/* TODO: do other clean-up stuff */
}

pnso_error_t
cpdc_start_accelerator(const struct cpdc_init_params *init_params)
{
	pnso_error_t err;

	PNSO_LOG_INFO(PNSO_OK, "enter ...");

	/* TODO: do the resource discovery */

	err = dpool_create(DPOOL_TYPE_CPDC, PNSO_UT_MAX_DBUFS, &cpdc_dpool);
	if (err) {
		PNSO_LOG_INFO(err, "failed to create CPDC descriptor pool");
		goto out;
	}

	err = dpool_create(DPOOL_TYPE_CPDC_SGL, PNSO_UT_MAX_DBUFS,
			&cpdc_sgl_dpool);
	if (err) {
		PNSO_LOG_INFO(err, "failed to create CPDC sgl descriptor pool");
		goto out_free_cpdc;
	}

	err = dpool_create(DPOOL_TYPE_CPDC_STATUS_NOSHA, PNSO_UT_MAX_DBUFS,
			&cpdc_status_nosha_dpool);
	if (err) {
		PNSO_LOG_INFO(err, "failed to create CPDC no sha status pool");
		goto out_free_cpdc_sgl;
	}

	err = dpool_create(DPOOL_TYPE_CPDC_STATUS_SHA256, PNSO_UT_MAX_DBUFS,
			&cpdc_status_sha256_dpool);
	if (err) {
		PNSO_LOG_INFO(err, "failed to create CPDC sha256 status pool");
		goto out_free_nosha;
	}

	err = dpool_create(DPOOL_TYPE_CPDC_STATUS_SHA512, PNSO_UT_MAX_DBUFS,
			&cpdc_status_sha512_dpool);
	if (err) {
		PNSO_LOG_INFO(err, "failed to create CPDC sha512 status pool");
		goto out_free_sha256;
	}

	dpool_pprint(cpdc_dpool);
	dpool_pprint(cpdc_sgl_dpool);
	dpool_pprint(cpdc_status_nosha_dpool);
	dpool_pprint(cpdc_status_sha256_dpool);
	dpool_pprint(cpdc_status_sha512_dpool);

	return err;

out_free_sha256:
	dpool_destroy(&cpdc_status_sha256_dpool);
out_free_nosha:
	dpool_destroy(&cpdc_status_nosha_dpool);
out_free_cpdc_sgl:
	dpool_destroy(&cpdc_sgl_dpool);
out_free_cpdc:
	dpool_destroy(&cpdc_dpool);
out:
	PNSO_LOG_INFO(PNSO_OK, "exit!");
	return err;
}

pnso_error_t
cpdc_stop_accelerator(void)
{
	/* TODO: add quiesce queue API */

	dpool_pprint(cpdc_dpool);
	dpool_pprint(cpdc_sgl_dpool);
	dpool_pprint(cpdc_status_nosha_dpool);
	dpool_pprint(cpdc_status_sha256_dpool);
	dpool_pprint(cpdc_status_sha512_dpool);

	dpool_destroy(&cpdc_status_nosha_dpool);
	dpool_destroy(&cpdc_status_sha256_dpool);
	dpool_destroy(&cpdc_status_sha512_dpool);
	dpool_destroy(&cpdc_sgl_dpool);
	dpool_destroy(&cpdc_dpool);

	return PNSO_OK;
}

pnso_error_t
cpdc_convert_desc_error(int error)
{
	/* TODO: ... */
	return PNSO_OK;
}
#endif

pnso_error_t
cpdc_start_accelerator(const struct cpdc_init_params *init_params)
{
	return EOPNOTSUPP;
}

void
cpdc_stop_accelerator(void)
{
	/* EOPNOTSUPP */
}
