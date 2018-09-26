/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_xts.h"
#include "pnso_xts_cmn.h"

/*
 * TODO-xts:
 *	- revisit ... only skeletons are added
 *
 */
static void __attribute__((unused))
pprint_aol(uint64_t aol_pa)
{
	const struct xts_aol *aol;

	aol = (const struct xts_aol *) osal_phy_to_virt(aol_pa);
	if (!aol)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "xa_addr_0", aol->xa_addr_0);
	OSAL_LOG_INFO("%30s: %d", "xa_off_0", aol->xa_off_0);
	OSAL_LOG_INFO("%30s: %d", "xa_len_0", aol->xa_len_0);

	OSAL_LOG_INFO("%30s: 0x%llx", "xa_addr_1", aol->xa_addr_1);
	OSAL_LOG_INFO("%30s: %d", "xa_off_1", aol->xa_off_1);
	OSAL_LOG_INFO("%30s: %d", "xa_len_1", aol->xa_len_1);

	OSAL_LOG_INFO("%30s: 0x%llx", "xa_addr_2", aol->xa_addr_2);
	OSAL_LOG_INFO("%30s: %d", "xa_off_2", aol->xa_off_2);
	OSAL_LOG_INFO("%30s: %d", "xa_len_2", aol->xa_len_2);

	OSAL_LOG_INFO("%30s: 0x%llx", "xa_next", aol->xa_next);
	OSAL_LOG_INFO("%30s: 0x%llx", "xa_rsvd", aol->xa_rsvd);
}

static void __attribute__((unused))
pprint_xts_cmd(const struct xts_cmd *cmd)
{
	if (!cmd)
		return;

	OSAL_LOG_INFO("%30s: %d", "xc_enable_crc", cmd->xc_enable_crc);
	OSAL_LOG_INFO("%30s: %d", "xc_bypass_aes", cmd->xc_bypass_aes);
	OSAL_LOG_INFO("%30s: %d", "xc_is_decrypt", cmd->xc_is_decrypt);
	OSAL_LOG_INFO("%30s: %d", "xc_token_3", cmd->xc_token_3);
	OSAL_LOG_INFO("%30s: %d", "xc_token_4", cmd->xc_token_4);
}

void __attribute__((unused))
xts_pprint_desc(const struct xts_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "xts_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: 0x%llx", "xd_src", desc->xd_in_aol);
	OSAL_LOG_INFO("%30s: 0x%llx", "xd_dst", desc->xd_out_aol);

	OSAL_LOG_INFO("%30s: %p", "=== xts_cmd", &desc->xd_cmd);
	pprint_xts_cmd(&desc->xd_cmd);

	OSAL_LOG_INFO("%30s: %d", "xd_key_desc_idx", desc->xd_key_desc_idx);
	OSAL_LOG_INFO("%30s: 0x%llx", "xd_iv_addr", desc->xd_iv_addr);

	OSAL_LOG_INFO("%30s: 0x%llx", "xd_auth_tag", desc->xd_auth_tag);

	OSAL_LOG_INFO("%30s: %d", "xd_hdr_size", desc->xd_hdr_size);
	OSAL_LOG_INFO("%30s: 0x%llx", "xd_status_addr", desc->xd_status_addr);

	OSAL_LOG_INFO("%30s: %d", "xd_otag", desc->xd_otag);
	OSAL_LOG_INFO("%30s: %d", "xd_otag_on", desc->xd_otag_on);

	OSAL_LOG_INFO("%30s: %d", "xd_sector_size", desc->xd_sector_size);

	OSAL_LOG_INFO("%30s: %d", "xd_app_tag", desc->xd_app_tag);
	OSAL_LOG_INFO("%30s: %d", "xd_sector_num", desc->xd_sector_num);

	OSAL_LOG_INFO("%30s: 0x%llx", "xd_db_addr", desc->xd_db_addr);
	OSAL_LOG_INFO("%30s: 0x%llx", "xd_db_data", desc->xd_db_data);
}
