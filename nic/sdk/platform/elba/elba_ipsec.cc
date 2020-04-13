// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

/*
 * elba_ipsec.cc
 */
#ifndef ELEKTRA
#include "nic/hal/pd/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

#include "nic/hal/pd/iris/internal/ipseccb_pd.hpp"
#include "nic/hal/src/internal/ipseccb_def.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/hal/pd/iris/internal/system_pd.hpp"

using namespace hal::pd;

namespace sdk {
namespace platform {
namespace elba {

bool
elba_ipsec_inline_capable (void)
{
    return TRUE;
}

sdk_ret_t
elba_ipsec_inline_encrypt_tunnel_info_table_program (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_inline_tunnel_info_actiondata_t d;

    memset(&d, 0, sizeof(d));
    for (int i = 0; i < ETH_ADDR_LEN; i++) {
        d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.mac_sa[(ETH_ADDR_LEN-1)-i] = ipseccb->smac[i];
        d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.mac_da[(ETH_ADDR_LEN-1)-i] = ipseccb->dmac[i];
    }
    memcpy(&d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.iv, &ipseccb->iv, 8);
    memcpy(&d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.iv_salt, &ipseccb->iv_salt, 4);
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.l3_src_addr = ipseccb->tunnel_sip4;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.l3_dst_addr = ipseccb->tunnel_dip4;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.vlan_id = ipseccb->vrf_vlan;
    SDK_TRACE_DEBUG("encap vlan %d", d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.vlan_id);
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.vlan_valid = 1;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.spi_hi = ipseccb->spi >> 16;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.spi_lo = ipseccb->spi & 0xFFFF;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.seqno = ipseccb->esn_lo;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.ipsec_mode = IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.ipsec_protocol = IPPROTO_ESP;

    d.action_id = IPSEC_INLINE_TUNNEL_INFO_IPSEC_INLINE_TUNNEL_INFO_ENCRYPT_ID;

    SDK_ASSERT(ipseccb_pd->ipsec_inline_tunnel_info_tbl != NULL);
    sdk_ret =
        ipseccb_pd->ipsec_inline_tunnel_info_tbl->insert_withid(&d,
                                                                ipseccb->cb_id);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_encrypt_tunnel_info_table_update (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_inline_tunnel_info_actiondata_t d;

    memset(&d, 0, sizeof(d));
    for (int i = 0; i < ETH_ADDR_LEN; i++) {
        d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.mac_sa[(ETH_ADDR_LEN-1)-i] = ipseccb->smac[i];
        d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.mac_da[(ETH_ADDR_LEN-1)-i] = ipseccb->dmac[i];
    }
    memcpy(&d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.iv, &ipseccb->iv, 8);
    memcpy(&d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.iv_salt, &ipseccb->iv_salt, 4);
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.l3_src_addr = ipseccb->tunnel_sip4;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.l3_dst_addr = ipseccb->tunnel_dip4;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.vlan_id = ipseccb->vrf_vlan;
    SDK_TRACE_DEBUG("encap vlan %d", d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.vlan_id);
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.vlan_valid = 1;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.spi_hi = ipseccb->spi >> 16;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.spi_lo = ipseccb->spi & 0xFFFF;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.seqno = ipseccb->esn_lo;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.ipsec_mode = IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_encrypt.ipsec_protocol = IPPROTO_ESP;

    d.action_id = IPSEC_INLINE_TUNNEL_INFO_IPSEC_INLINE_TUNNEL_INFO_ENCRYPT_ID;

    SDK_ASSERT(ipseccb_pd->ipsec_inline_tunnel_info_tbl != NULL);
    sdk_ret =
        ipseccb_pd->ipsec_inline_tunnel_info_tbl->update(ipseccb->cb_id, &d);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_encrypt_ipsec_info_table_program (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_info_inline_actiondata_t d;

    memset(&d, 0, sizeof(d));

    d.action_id =  IPSEC_INFO_INLINE_IPSEC_INFO_INLINE_ENCRYPT_ID;

    if (ipseccb->key_size == IPSEC_KEYSIZE_128) {
        d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.key_size = 0;
    } else {
        d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.key_size = 1;
    }

    memcpy(d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.key,
           ipseccb->key, IPSEC_MAX_KEY_SIZE);
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.esn_hi =
        ipseccb->esn_hi;
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.esn_hi_offset =
        IPSEC_INLINE_ESP_ESN_HI_OFFSET;
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.mode =
        IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.protocol =
        IPPROTO_ESP;

    SDK_ASSERT(ipseccb_pd->ipsec_inline_ipsec_info_tbl != NULL);
    sdk_ret =
        ipseccb_pd->ipsec_inline_ipsec_info_tbl->insert_withid(&d,
                                                               ipseccb->cb_id);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_encrypt_ipsec_info_table_update (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_info_inline_actiondata_t d;

    memset(&d, 0, sizeof(d));

    d.action_id =  IPSEC_INFO_INLINE_IPSEC_INFO_INLINE_ENCRYPT_ID;

    if (ipseccb->key_size == IPSEC_KEYSIZE_128) {
        d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.key_size = 0;
    } else {
        d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.key_size = 1;
    }

    memcpy(d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.key,
           ipseccb->key, IPSEC_MAX_KEY_SIZE);
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.esn_hi =
        ipseccb->esn_hi;
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.esn_hi_offset =
        IPSEC_INLINE_ESP_ESN_HI_OFFSET;
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.mode =
        IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_info_inline_ipsec_info_inline_encrypt.protocol =
        IPPROTO_ESP;

    SDK_ASSERT(ipseccb_pd->ipsec_inline_ipsec_info_tbl != NULL);
    sdk_ret =
        ipseccb_pd->ipsec_inline_ipsec_info_tbl->update(ipseccb->cb_id, &d);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_decrypt_tunnel_info_table_program (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_inline_tunnel_info_actiondata_t d;
    uint32_t salt  = htonl(ipseccb->iv_salt) & 0xFFFFFFFF;
    salt = htonl(salt);

    memset(&d, 0, sizeof(d));

    d.action_id = IPSEC_INLINE_TUNNEL_INFO_IPSEC_INLINE_TUNNEL_INFO_DECRYPT_ID;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.expected_seqno = 1;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.ipsec_mode = IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.ipsec_protocol = IPPROTO_ESP;
    //memcpy(d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.iv_salt, &salt, sizeof(salt)); ;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.iv_salt = salt;
    SDK_TRACE_DEBUG("salt = 0x%lx", ipseccb->iv_salt);
    SDK_ASSERT(ipseccb_pd->ipsec_inline_tunnel_info_tbl != NULL);
    sdk_ret = ipseccb_pd->ipsec_inline_tunnel_info_tbl->insert_withid(&d, ipseccb->cb_id+1024);
    SDK_TRACE_DEBUG("insert with id %d", ipseccb->cb_id+1024);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_decrypt_ipsec_info_table_program (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_info_inline_actiondata_t d;

    memset(&d, 0, sizeof(d));

    d.action_id =  IPSEC_INFO_INLINE_IPSEC_INFO_INLINE_DECRYPT_ID;
    if (ipseccb->key_size == IPSEC_KEYSIZE_128) {
        d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.key_size = 0;
    } else {
        d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.key_size = 1;
    }

    memcpy(d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.key,
           ipseccb->key, IPSEC_MAX_KEY_SIZE);
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.esn_hi =
        ipseccb->esn_hi;
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.esn_hi_offset =
        IPSEC_INLINE_ESP_ESN_HI_OFFSET;
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.mode =
        IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.protocol =
        IPPROTO_ESP;

    SDK_ASSERT(ipseccb_pd->ipsec_inline_ipsec_info_tbl != NULL);
    sdk_ret =
        ipseccb_pd->ipsec_inline_ipsec_info_tbl->insert_withid(&d,
                                                               ipseccb->cb_id+1024);
    SDK_TRACE_DEBUG("insert with id {}", ipseccb->cb_id+1024);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_decrypt_tunnel_info_table_update (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_inline_tunnel_info_actiondata_t d;
    uint32_t salt  = htonl(ipseccb->iv_salt);

    salt = htonl(salt);
    memset(&d, 0, sizeof(d));

    d.action_id = IPSEC_INLINE_TUNNEL_INFO_IPSEC_INLINE_TUNNEL_INFO_DECRYPT_ID;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.expected_seqno = 1;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.ipsec_mode = IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.ipsec_protocol = IPPROTO_ESP;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.iv_salt = salt;
    //d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.vid = ipseccb->vrf_vlan;
    d.action_u.ipsec_inline_tunnel_info_ipsec_inline_tunnel_info_decrypt.vid = 4;

    SDK_TRACE_DEBUG("salt = %lx", ipseccb->iv_salt);
    SDK_ASSERT(ipseccb_pd->ipsec_inline_tunnel_info_tbl != NULL);
    sdk_ret = ipseccb_pd->ipsec_inline_tunnel_info_tbl->update(ipseccb->cb_id+1024, &d);

    return sdk_ret;
}

sdk_ret_t
elba_ipsec_inline_decrypt_ipsec_info_table_update (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    hal::ipseccb_t *ipseccb = ipseccb_pd->ipseccb;
    ipsec_info_inline_actiondata_t d;

    memset(&d, 0, sizeof(d));

    d.action_id =  IPSEC_INFO_INLINE_IPSEC_INFO_INLINE_DECRYPT_ID;
    if (ipseccb->key_size == IPSEC_KEYSIZE_128) {
        d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.key_size = 0;
    } else {
        d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.key_size = 1;
    }

    memcpy(d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.key,
           ipseccb->key, IPSEC_MAX_KEY_SIZE);
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.esn_hi =
        ipseccb->esn_hi;
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.esn_hi_offset =
        IPSEC_INLINE_ESP_ESN_HI_OFFSET;
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.mode =
        IPSEC_INLINE_MODE_TUNNEL;
    d.action_u.ipsec_info_inline_ipsec_info_inline_decrypt.protocol =
        IPPROTO_ESP;

    SDK_ASSERT(ipseccb_pd->ipsec_inline_ipsec_info_tbl != NULL);
    sdk_ret =
        ipseccb_pd->ipsec_inline_ipsec_info_tbl->update(ipseccb->cb_id+1024,
                                                        &d);

    return sdk_ret;
}

//ELBA ASIC PD APIs
sdk_ret_t
elba_ipsec_encrypt_create (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;
    ipseccb_pd->ipseccb->ipsec_inline = TRUE;

    if (ipseccb_pd && ipseccb_pd->ipseccb &&
        (ipseccb_pd->ipseccb->is_v6 == 0) ) {
        // Program tunnel info table
        sdk_ret =
            elba_ipsec_inline_encrypt_tunnel_info_table_program(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb encrypt create error {}", sdk_ret);
            return sdk_ret;
        }

        // Program ipsec info table
        sdk_ret =
            elba_ipsec_inline_encrypt_ipsec_info_table_program(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb encrypt create error {}", sdk_ret);
            return sdk_ret;
        }
    }

    HAL_TRACE_DEBUG("cb_id: {}", ipseccb_pd->ipseccb->cb_id);
    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_encrypt_update (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;

    if (ipseccb_pd && ipseccb_pd->ipseccb &&
        (ipseccb_pd->ipseccb->is_v6 == 0) ) {
        // Program tunnel info table
        sdk_ret =
            elba_ipsec_inline_encrypt_tunnel_info_table_update(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb encrypt update error {}", sdk_ret);
            return sdk_ret;
        }

        // Program ipsec info table
        sdk_ret = elba_ipsec_inline_encrypt_ipsec_info_table_update(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb encrypt update error {}", sdk_ret);
            return sdk_ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_encrypt_delete (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_encrypt_get (pd_ipseccb_encrypt_t *ipseccb_pd)
{
    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_decrypt_create (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;

    if (ipseccb_pd && ipseccb_pd->ipseccb &&
        (ipseccb_pd->ipseccb->is_v6 == 0) ) {
        // Program tunnel info table
        sdk_ret =
            elba_ipsec_inline_decrypt_tunnel_info_table_program(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb decrypt create error {}", sdk_ret);
            return sdk_ret;
        }

        // Program ipsec info table
        sdk_ret =
            elba_ipsec_inline_decrypt_ipsec_info_table_program(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb decrypt create error {}", sdk_ret);
            return sdk_ret;
        }
    }

    HAL_TRACE_DEBUG("cb_id: {}", ipseccb_pd->ipseccb->cb_id);
    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_decrypt_update (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    sdk_ret_t  sdk_ret = SDK_RET_OK;

    if (ipseccb_pd && ipseccb_pd->ipseccb &&
        (ipseccb_pd->ipseccb->is_v6 == 0) ) {
        // Program tunnel info table
        sdk_ret =
            elba_ipsec_inline_decrypt_tunnel_info_table_update(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb decrypt update error {}", sdk_ret);
            return sdk_ret;
        }

        // Program ipsec info table
        sdk_ret = elba_ipsec_inline_decrypt_ipsec_info_table_update(ipseccb_pd);
        if (sdk_ret != SDK_RET_OK) {
            SDK_TRACE_ERR("asic_pd:ipseccb decrypt update error {}", sdk_ret);
            return sdk_ret;
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_decrypt_delete (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    return SDK_RET_OK;
}

sdk_ret_t
elba_ipsec_decrypt_get (pd_ipseccb_decrypt_t *ipseccb_pd)
{
    return SDK_RET_OK;
}

} //elba
} //platform
} //sdk

#endif // ELEKTRA
