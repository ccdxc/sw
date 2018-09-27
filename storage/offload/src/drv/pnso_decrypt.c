/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_xts.h"
#include "pnso_xts_cmn.h"

/*
 * TODO-xts:
 *	- revisit ... only skeletons are added
 *
 */
static void __attribute__((unused))
fill_decrypt_desc(struct xts_desc *desc, void *in_aol, void *out_aol)
{
	memset(desc, 0, sizeof(*desc));

	desc->xd_in_aol = (uint64_t) osal_virt_to_phy(in_aol);
	desc->xd_out_aol = (uint64_t) osal_virt_to_phy(out_aol);

	desc->xd_cmd.xc_enable_crc = 0;
	desc->xd_cmd.xc_bypass_aes = 0;
	desc->xd_cmd.xc_is_decrypt = 0;
	desc->xd_cmd.xc_token_3 = 0;
	desc->xd_cmd.xc_token_4 = 0;

	desc->xd_key_desc_idx = 0;

	desc->xd_iv_addr = 0;
	desc->xd_auth_tag = 0;
	desc->xd_hdr_size = 0;
	desc->xd_status_addr = 0;

	desc->xd_otag = 0;
	desc->xd_otag_on = 0;

	desc->xd_sector_size = 0;
	desc->xd_app_tag = 0;
	desc->xd_sector_num = 0;
	desc->xd_db_addr = 0;
	desc->xd_db_data = 0;

	XTS_PPRINT_DESC(desc);
}

static pnso_error_t
decrypt_setup(struct service_info *svc_info,
		const struct service_params *svc_params)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decrypt_chain(struct chain_entry *centry)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decrypt_schedule(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decrypt_poll(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decrypt_read_status(const struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static pnso_error_t
decrypt_write_result(struct service_info *svc_info)
{
	return EOPNOTSUPP;
}

static void
decrypt_teardown(const struct service_info *svc_info)
{
	/* EOPNOTSUPP */
}

struct service_ops decrypt_ops = {
	.setup = decrypt_setup,
	.chain = decrypt_chain,
	.schedule = decrypt_schedule,
	.poll = decrypt_poll,
	.read_status = decrypt_read_status,
	.write_result = decrypt_write_result,
	.teardown = decrypt_teardown
};
