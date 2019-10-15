//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/utils/ftlite/ftlite_table.hpp"
#include "nic/utils/ftlite/ftlite.hpp"
#include "lib/utils/crc_fast.hpp"

namespace ftlite {
namespace p4psim {

using namespace ftlite;
static state_t g_state;

sdk_ret_t
init(init_params_t *ips) {
    auto ret = g_state.init(ips);
    SDK_ASSERT(ret == SDK_RET_OK);

    return SDK_RET_OK;
}

template<class T> static uint32_t
calchash_(T* flow) {
    static uint8_t key[64];
    memset(key, 0, sizeof(key));
    memcpy(key, &flow->lentry, sizeof(T));
    ((T*)key)->lentry.swizzle();
    return sdk::utils::crc32(key, 64, 0);
}

template<class T> static void
traverse_(meta_t *meta, T *flow) {
    auto *mtab = &g_state.mtables[meta->ipv6];
    auto *htab = &g_state.htables[meta->ipv6];

    T tflow = {0};
    uint32_t eindex = meta->hash.index;

    auto ret = mtab->read(meta->ipv6, &tflow.lentry, eindex);
    SDK_ASSERT(ret == SDK_RET_OK);

    uint32_t slot = 0;
    while (tflow.lentry.entry_valid &&
           meta->level < (SDK_TABLE_MAX_RECIRC - 1)) {
        flow->pentry = tflow.lentry;
        meta->level++;
        meta->pindex = eindex;
        tflow.lentry.find_hint(meta->hash.msb, eindex, slot);
        if (!eindex) {
            meta->pslot = tflow.lentry.find_free_slot();
            break;
        }
        ret = htab->read(meta->ipv6, &tflow.lentry, eindex);
        SDK_ASSERT(ret == SDK_RET_OK);
        meta->ptype = 1;
    }
    return;
}

static void
lookup_flow_(meta_t* m, info_t* f, bool hash_valid) {
    if (m->ipv6) {
        if (!hash_valid) {
            m->hash.value = calchash_<v6info_t>(&f->v6);
        }
        traverse_<v6info_t>(m, &f->v6);
    } else {
        if (!hash_valid) {
            m->hash.value = calchash_<v4info_t>(&f->v4);
        }
        traverse_<v4info_t>(m, &f->v4);
    }
    return;
}
                            
static void
lookup_(insert_params_t *ips) {
    lookup_flow_(&ips->imeta, &ips->iflow, ips->cmeta.action);
    lookup_flow_(&ips->rmeta, &ips->rflow, ips->cmeta.action);
    return;
}

sdk_ret_t
insert(insert_params_t *ips) {
    lookup_(ips);
    return ftlite::insert(ips);
}

} // namespace ftlite
} // namespace p4psim
