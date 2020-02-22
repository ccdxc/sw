/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    utils.hpp
 *
 * @brief   helper functions
 */

#if !defined (__UTILS_HPP__)
#define __UTILS_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**
 * @brief    given two epochs, pick invalid one or the lowest numbered epoch so
 *           we can overwrite its contents, and return its index
 * @param[in] epoch1    epoch 1 value
 * @param[in] epoch2    epoch 2 value
 * @return index of lowered numbered epoch or 1st invalid one
 */
// TODO: we will handle the case when epoch rollsover later !!
static inline uint8_t
pick_older_epoch_idx (uint32_t epoch1, uint32_t epoch2)
{
    if (epoch1 == PDS_EPOCH_INVALID) {
        return 0;
    } else if ((epoch2 == PDS_EPOCH_INVALID) || (epoch2 < epoch1)) {
        return 1;
    }
    return 0;
}

// construct a 'sticky' uuid given an integer so that same uuid is generated
// even across reboots i.e., same input gives same uuid everytime
#define PDS_UUID_MAGIC_BYTE           0x42
#define PDS_UUID_MAGIC_BYTE_OFFSET    8
#define PDS_UUID_SYSTEM_MAC_OFFSET    10
static inline pds_obj_key_t
uuid_from_objid (uint32_t id)
{
    pds_obj_key_t key = { 0 };

    auto mac = api::g_pds_state.system_mac();
    memcpy(&key.id[0], &id, sizeof(id));
    memset(&key.id[8], 0x42, 2);
    memcpy(&key.id[10], mac, ETH_ADDR_LEN);
    return key;
}

// extract integer id from given 'sticky' uuid
static inline uint32_t
objid_from_uuid (const pds_obj_key_t& key) {
    char *buf;
    char id_buf[4][9];
    static thread_local uint8_t next_buf = 0;

    buf = id_buf[next_buf++ & 0x3];
    memcpy(buf, key.id, 8);
    buf[8] = '\0';
    return *(uint32_t *)buf;
}

}    // namespace api

#endif    /** __UTILS_HPP__ */
