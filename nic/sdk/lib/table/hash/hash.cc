//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <cstring>
#include "sdk/tcam.hpp"
#include "lib/p4pd/p4pd_api.hpp"
#include "hash.hpp"

namespace sdk {
namespace table {

typedef struct hash_iter_cb_s {
    hash_iterate_func_t func;
    const void          *iter_cb_data;
} hash_iter_cb_t;

//---------------------------------------------------------------------------
// factory method to instantiate the class
//---------------------------------------------------------------------------
hash *
hash::factory(char *name, uint32_t dleft_table_id,
              uint32_t otcam_table_id, uint32_t dleft_capacity,
              uint32_t otcam_capacity, uint32_t swkey_len,
              uint32_t swdata_len, hash::HashPoly hash_poly,
              bool entry_trace_en)
{
    void   *mem = NULL;
    hash   *h   = NULL;

    mem = SDK_CALLOC(SDK_MEM_ALLOC_ID_HASH, sizeof(hash));
    if (!mem) {
        return NULL;
    }

    h = new (mem) hash(name, dleft_table_id, otcam_table_id,
                       dleft_capacity, otcam_capacity, swkey_len,
                       swdata_len, hash_poly, entry_trace_en);

    h->entry_ht_ = ht::factory(dleft_capacity,
                               hash_entry_get_key_func,
                               hash_entry_compute_hash_func,
                               hash_entry_compare_key_func);

    h->name_ = (char *)SDK_CALLOC(SDK_MEM_ALLOC_ID_HASH_NAME,
                                  strlen(name) + 1);
    memcpy(h->name_, name, strlen(name) + 1);
    // Initialize Stats
    h->stats_ = (uint64_t *)SDK_CALLOC(SDK_MEM_ALLOC_HASH_STATS,
                                    sizeof(uint64_t) * STATS_MAX);

    SDK_TRACE_DEBUG("hash::%-30s: dleft_tid: %-3d otcam_tid: %-3d "
                    "swkey_len: %-4d hwkey_len_: %-4d "
                    "hwdata_len_: %-4d",
                    h->name_, dleft_table_id, otcam_table_id, h->swkey_len_,
                    h->hwkey_len_, h->hwdata_len_);

    return h;
}

//---------------------------------------------------------------------------
// method to free & delete the object
//---------------------------------------------------------------------------
void
hash::destroy(hash *hash)
{
    if (hash) {
        hash->~hash();
        SDK_FREE(SDK_MEM_ALLOC_ID_HASH, hash);
    }
}

//---------------------------------------------------------------------------
// constructor - hash
//---------------------------------------------------------------------------
hash::hash(char *name, uint32_t dleft_table_id, uint32_t otcam_table_id,
           uint32_t dleft_capacity, uint32_t otcam_capacity,
           uint32_t swkey_len, uint32_t swdata_len,
           hash::HashPoly hash_poly, bool entry_trace_en)
{
    name_           = name;
    id_             = dleft_table_id;
    dleft_capacity_ = dleft_capacity;
    swkey_len_      = swkey_len;
    swdata_len_     = swdata_len;
    hash_poly_      = hash_poly;
    entry_trace_en_ = entry_trace_en;


    // Initialize the Overflow tcam
    otcam_ = NULL;
    if (otcam_capacity) {
        otcam_ = tcam::factory(name, otcam_table_id,
                               otcam_capacity, swkey_len_, swdata_len_, false,
                               entry_trace_en);
        if (!otcam_) {
            SDK_ASSERT_RETURN_VOID(FALSE);
        }
    }

    p4pd_hwentry_query(id_, &hwkey_len_, NULL, &hwdata_len_);
    // TODO: Remove once P4-PD APIs are implemented
    // hwkey_len_ = swkey_len;
    // hwdata_len_ = swdata_len;
    // uint32_t hwkeylen_bits = hwkey_len_;
    // uint32_t hwdatalen_bits = hwdata_len_;
    hwkey_len_ = (hwkey_len_ >> 3) + ((hwkey_len_ & 0x7) ? 1 : 0);
    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);
}

// ---------------------------------------------------------------------------
// destructor - hash
// ---------------------------------------------------------------------------
hash::~hash()
{
    // freeing up OTcam
    if (otcam_) {
        // delete otcam_;
        tcam::destroy(otcam_);
    }
    // delete[] stats_;
    SDK_FREE(SDK_MEM_ALLOC_HASH_STATS, stats_);
}

// ---------------------------------------------------------------------------
// Insert
//
// Return Code:
//      SDK_RET_OK              : Successfull
//      SDK_RET_DUPLICATE_INS   : Duplicate Insert
//      SDK_RET_NO_RESOURCE     : Capacity reached
//
// ---------------------------------------------------------------------------
sdk_ret_t
hash::insert(void *key, void *data, uint32_t *index, void *key_mask,
             bool direct_to_otcam)
{
    sdk_ret_t rs                = SDK_RET_OK;
    p4pd_error_t pd_err         = P4PD_SUCCESS;
    hash_entry_t *he            = NULL;
    void *hwkey                 = NULL;
    uint32_t dleft_index        = 0;
    uint32_t tcam_index         = 0;
    bool key_mask_free          = false;
    // HashEntryMap::iterator itr;

    hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HASH_HW_KEY_INS, hwkey_len_);
    pd_err = p4pd_hwkey_hwmask_build(id_, key, NULL,
                                     (uint8_t *)hwkey, NULL);
    if (pd_err != P4PD_SUCCESS) {
        rs = SDK_RET_HW_PROGRAM_ERR;
        goto end;
    }

    dleft_index = generate_hash_(hwkey, hwkey_len_);

    he = (hash_entry_t *)entry_ht_->lookup(&dleft_index);

    // itr = hash_entry_map_.find(dleft_index);
    // if (itr == hash_entry_map_.end() && !direct_to_otcam) {
    if (he == NULL && !direct_to_otcam) {
        SDK_TRACE_DEBUG("hash::%s: dleft Insert ", __FUNCTION__);
        // he = hash_entry::factory(key, swkey_len_, data,
        //                         swdata_len_, dleft_index);
        he = hash_entry_create(key, swkey_len_, data, swdata_len_,
                               dleft_index);
        *index = form_hash_idx_from_dleft_id_(dleft_index);

        // program hw
        rs = program_table_(he, hwkey);

        if (rs == SDK_RET_OK) {
            // insert in sw DS
            // hash_entry_map_[dleft_index] = he;
            rs = entry_ht_->insert_with_key(&dleft_index, he, &he->ht_ctxt);
            stats_incr(STATS_NUM_HASH);
        } else {
            // delete he;
            // hash_entry::destroy(he);
            hash_entry_delete(he);
        }

    } else {
        SDK_TRACE_DEBUG("hash::%s: otcam Insert ", __FUNCTION__);
        if (he != NULL && !std::memcmp(he->key, key, swkey_len_)) {
            rs = SDK_RET_DUPLICATE_INS;
            goto end;
        }
#if 0
        he = itr->second;
        if (itr != hash_entry_map_.end() &&
            !std::memcmp(he->get_key(), key, swkey_len_)) {
            rs = SDK_RET_DUPLICATE_INS;
            goto end;
        }
#endif

        if (has_otcam_()) {
            if (key_mask == NULL) {
                key_mask = SDK_MALLOC(SDK_MEM_ALLOC_HASH_SW_KEY_MASK_INS,
                                      swkey_len_);
                memset(key_mask, ~0, swkey_len_);
                key_mask_free = true;
            }

            // otcam insert
            rs = otcam_->insert(key, key_mask, data, &tcam_index);
            if (key_mask_free) {
                SDK_FREE(SDK_MEM_ALLOC_HASH_SW_KEY_MASK_INS, key_mask);
            }
            if (rs == SDK_RET_OK) {
                *index = form_hash_idx_from_otcam_id_(tcam_index);
                stats_incr(STATS_NUM_TCAM);
            }
        } else {
            // No OTCAM
            rs = SDK_RET_NO_RESOURCE;
        }
    }

end:
    if (hwkey) {
        SDK_FREE(SDK_MEM_ALLOC_HASH_HW_KEY_INS, hwkey);
    }

    stats_update(INSERT, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Insert WithId
//
// Return Code:
//      SDK_RET_OK              : Successfull
//      SDK_RET_DUPLICATE_INS   : Duplicate Insert
//
// ---------------------------------------------------------------------------
sdk_ret_t
hash::insert_withid(void *key, void *data, uint32_t index, void *key_mask)
{
    sdk_ret_t       rs            = SDK_RET_OK;
    p4pd_error_t    pd_err        = P4PD_SUCCESS;
    void            *hwkey        = NULL;
    uint32_t        dleft_id      = 0, dleft_index_key = 0;
    uint32_t        otcam_id      = 0;
    hash_entry_t    *he           = NULL;
    bool            key_mask_free = false;

    if (is_dleft(index)) {
        // get hash dleft table index from index
        dleft_id = get_dleft_id_from_hash_idx_(index);
        if (dleft_id >= dleft_capacity_) {
            rs = SDK_RET_OOB;
            goto end;
        }

        SDK_TRACE_DEBUG("dleft insert at: {}", dleft_id);

        // check if entry exists
        he = (hash_entry_t *)entry_ht_->lookup(&dleft_id);
        if (he != NULL) {
            rs = SDK_RET_DUPLICATE_INS;
            goto end;
        }

        // check if hash is matching the index passed
        hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HASH_HW_KEY_INS, hwkey_len_);
        pd_err = p4pd_hwkey_hwmask_build(id_, key, NULL,
                                         (uint8_t *)hwkey, NULL);
        if (pd_err != P4PD_SUCCESS) {
            rs = SDK_RET_HW_PROGRAM_ERR;
            goto end;
        }
        dleft_index_key = generate_hash_(hwkey, hwkey_len_);
        if (dleft_index_key != dleft_id) {
            rs = SDK_RET_INVALID_ARG;
            goto end;
        }

        // allocate hash entry
        he = hash_entry_create(key, swkey_len_, data, swdata_len_,
                               dleft_id);

        // program hw
        rs = program_table_(he, hwkey);

        if (rs == SDK_RET_OK) {
            // insert in sw DS
            rs = entry_ht_->insert_with_key(&dleft_id, he, &he->ht_ctxt);
            stats_incr(STATS_NUM_HASH);
        } else {
            // delete he;
            hash_entry_delete(he);
        }
    } else {
        if (otcam_) {
            otcam_id = get_otcam_id_from_hash_idx_(index);
            SDK_TRACE_DEBUG("otcam insert at: {}", otcam_id);
            if (key_mask == NULL) {
                key_mask = SDK_MALLOC(SDK_MEM_ALLOC_HASH_SW_KEY_MASK_INS,
                                      swkey_len_);
                memset(key_mask, ~0, swkey_len_);
                key_mask_free = true;
            }
            rs = otcam_->insert_withid(key, key_mask, data, otcam_id);
            if (key_mask_free) {
                SDK_FREE(SDK_MEM_ALLOC_HASH_SW_KEY_MASK_INS, key_mask);
            }
            if (rs == SDK_RET_OK) {
                stats_incr(STATS_NUM_TCAM);
            }
        } else {
            // invalid index. table doesn't have otcam.
            rs = SDK_RET_INVALID_ARG;
        }
    }
end:
    if (hwkey) {
        SDK_FREE(SDK_MEM_ALLOC_HASH_HW_KEY_INS, hwkey);
    }
    stats_update(INSERT, rs);
    return rs;

}


// ---------------------------------------------------------------------------
// Update
//
// Return Code:
//      SDK_RET_OK              : Successfull
//      SDK_RET_OOB             : Out of Bound Index
//      SDK_RET_ENTRY_NOT_FOUND : Entry not found
//      SDK_RET_INVALID_ARG     : Invalid index
//      SDK_RET_HW_PROGRAM_ERR         : Hw API failed
//
// ---------------------------------------------------------------------------
sdk_ret_t
hash::update(uint32_t hash_idx, void *data)
{
    sdk_ret_t       rs       = SDK_RET_OK;
    p4pd_error_t    pd_err   = P4PD_SUCCESS;
    void            *hwkey   = NULL;
    uint32_t        dleft_id = 0;
    uint32_t        otcam_id = 0;
    hash_entry_t    *he      = NULL;

    if (is_dleft(hash_idx)) {
        dleft_id = get_dleft_id_from_hash_idx_(hash_idx);
        if (dleft_id >= dleft_capacity_) {
            rs = SDK_RET_OOB;
            goto end;
        }

        // check if entry exists
        he = (hash_entry_t *)entry_ht_->lookup(&dleft_id);
        if (he == NULL) {
            rs = SDK_RET_ENTRY_NOT_FOUND;
            goto end;
        }
#if 0
        itr = hash_entry_map_.find(dleft_id);
        if (itr == hash_entry_map_.end()) {
            rs = SDK_RET_ENTRY_NOT_FOUND;
            goto end;
        }
#endif

        // even if update to hw fails sw will be updated.
        hash_entry_update_data(he, data);

        // build hw key & mask
        hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HASH_HW_KEY_UPD, hwkey_len_);
        pd_err = p4pd_hwkey_hwmask_build(id_, he->key,
                                         NULL, (uint8_t *)hwkey, NULL);
        if (pd_err != P4PD_SUCCESS) {
            rs = SDK_RET_HW_PROGRAM_ERR;
            SDK_ASSERT(0);
            goto end;
        }

        rs = program_table_(he, hwkey);

    } else {
        if (otcam_) {
            otcam_id = get_otcam_id_from_hash_idx_(hash_idx);
            rs = otcam_->update(otcam_id, data);
        } else {
            // Invalid hash idx
            rs = SDK_RET_INVALID_ARG;
        }
    }

end:

    if (hwkey) SDK_FREE(SDK_MEM_ALLOC_HASH_HW_KEY_UPD, hwkey);
    stats_update(UPDATE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Remove
//
// Return Code:
//      SDK_RET_OK              : Successfull
//      SDK_RET_OOB             : Out of Bound Index
//      SDK_RET_ENTRY_NOT_FOUND : Entry not found
//      SDK_RET_INVALID_ARG     : Invalid index
//      SDK_RET_HW_PROGRAM_ERR         : Hw API failed
//
// ---------------------------------------------------------------------------
sdk_ret_t
hash::remove(uint32_t hash_idx)
{
    sdk_ret_t       rs       = SDK_RET_OK;
    uint32_t        dleft_id = -1;
    uint32_t        otcam_id = -1;
    hash_entry_t    *he      = NULL;

    if (is_dleft(hash_idx)) {
        // entry is hash
        dleft_id = get_dleft_id_from_hash_idx_(hash_idx);
        if (dleft_id >= dleft_capacity_) {
            rs = SDK_RET_OOB;
            goto end;
        }

        // check if entry exists
        he = (hash_entry_t *)entry_ht_->lookup(&dleft_id);
        if (he == NULL) {
            rs = SDK_RET_ENTRY_NOT_FOUND;
            goto end;
        }

        // deprogram hw
        rs = deprogram_table_(he);

        if (rs == SDK_RET_OK) {
            // free & remove from sw DS
            // delete itr->second;
            // hash_entry::destroy(itr->second);
            // hash_entry_map_.erase(itr);
            he = (hash_entry_t *)entry_ht_->remove(&dleft_id);
            hash_entry_delete(he);
        }

    } else {
        // entry is tcam
        if (has_otcam_()) {
            otcam_id = get_otcam_id_from_hash_idx_(hash_idx);
            rs = otcam_->remove(otcam_id);
        } else {
            rs = SDK_RET_INVALID_ARG;
        }
    }

end:
    stats_update(REMOVE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Retrieve
//
// Return Code:
//      SDK_RET_OK              : Successfull
//      SDK_RET_OOB             : Out of Bound Index
//      SDK_RET_ENTRY_NOT_FOUND : Entry not found
//      SDK_RET_INVALID_ARG     : Invalid index
//
// ---------------------------------------------------------------------------
sdk_ret_t
hash::retrieve(uint32_t hash_idx, void *key, void *data)
{
    sdk_ret_t       rs       = SDK_RET_OK;
    hash_entry_t    *he      = NULL;
    uint32_t        dleft_id = -1;
    uint32_t        otcam_id = -1;

    if (is_dleft(hash_idx)) {
        // entry is hash
        dleft_id = get_dleft_id_from_hash_idx_(hash_idx);
        if (dleft_id >= dleft_capacity_) {
            rs = SDK_RET_OOB;
            goto end;
        }

        // check if entry exists
        he = (hash_entry_t *)entry_ht_->lookup(&dleft_id);
        if (he == NULL) {
            rs = SDK_RET_ENTRY_NOT_FOUND;
            goto end;
        }

        if (key) {
            memcpy(key, he->key, he->key_len);
        }
        if (data) {
            memcpy(data, he->data, he->data_len);
        }
    } else {
        // entry is tcam
        if (otcam_) {
            otcam_id = get_otcam_id_from_hash_idx_(hash_idx);
            rs = otcam_->retrieve(otcam_id, key, NULL, data);
        } else {
            rs = SDK_RET_INVALID_ARG;
        }
    }

end:
    stats_update(RETRIEVE, rs);
    return rs;
}


// ---------------------------------------------------------------------------
// call back func. for otcam iterate
// ---------------------------------------------------------------------------
bool
hash::otcam_iterate_(void *key, void *key_mask,
                     void *data,
                     uint32_t tcam_idx, const void *cb_data)
{
    otcam_iterate_cb_t *otcam_cb_data = (otcam_iterate_cb_t *)cb_data;
    otcam_cb_data->cb(key, key_mask, data,
                     form_hash_idx_from_otcam_id_(tcam_idx),
                     otcam_cb_data->cb_data);

    return TRUE;
}

bool hash_iter_walk_cb(void *entry, void *ctxt)
{
    hash_entry_t *he = (hash_entry_t *)entry;
    hash_iter_cb_t *he_cb = (hash_iter_cb_t *)ctxt;

    he_cb->func(he->key, NULL, he->data, he->index, he_cb->iter_cb_data);
    return false;

}

// ---------------------------------------------------------------------------
// hash iterate
// ---------------------------------------------------------------------------
sdk_ret_t
hash::iterate(hash_iterate_func_t cb, const void *cb_data,
              hash::EntryType type)
{
    sdk_ret_t       rs = SDK_RET_OK;
    // hash_entry_t    *he = NULL;
    hash_iter_cb_t  he_cb  = {0};


    if (type == hash::DLEFT || type == hash::BOTH) {


    he_cb.func = cb;
    he_cb.iter_cb_data = cb_data;

    entry_ht_->walk(hash_iter_walk_cb, &he_cb);
#if 0
        for (itr = hash_entry_map_.begin(); itr != hash_entry_map_.end();
                ++itr) {
            he = itr->second;
            cb(he->get_key(), he->get_data(),
                    form_hash_idx_from_dleft_id_(itr->first),
                    cb_data);
        }
#endif

    }
    if (type == hash::OTCAM || type == hash::BOTH) {
        if (otcam_) {
            otcam_iterate_cb_t otcam_cb;
            otcam_cb.cb = cb;
            otcam_cb.cb_data = cb_data;

            otcam_->iterate(hash::otcam_iterate_, (const void *)&otcam_cb);
        }
    }

    return rs;
}

// ---------------------------------------------------------------------------
// checks if hash table has Overflow TCAM
// ---------------------------------------------------------------------------
bool
hash::has_otcam_()
{
    return (otcam_ != NULL);
}

// ---------------------------------------------------------------------------
// forms hash idx from Dleft Id
// ---------------------------------------------------------------------------
uint32_t
hash::form_hash_idx_from_dleft_id_(uint32_t dleft_id)
{
    return dleft_id;
}

// ---------------------------------------------------------------------------
// forms hash idx from OTCAM Id
// ---------------------------------------------------------------------------
uint32_t
hash::form_hash_idx_from_otcam_id_(uint32_t otcam_id)
{
    return ((uint32_t)(1 << otcam_bit_) | otcam_id);
}

// ---------------------------------------------------------------------------
// get dleft id from hash Idx
// ---------------------------------------------------------------------------
uint32_t
hash::get_dleft_id_from_hash_idx_(uint32_t hash_idx)
{
    return hash_idx;
}

// ---------------------------------------------------------------------------
// get OTCAM Id from hash Idx
// ---------------------------------------------------------------------------
uint32_t
hash::get_otcam_id_from_hash_idx_(uint32_t hash_idx)
{
    return (hash_idx & ~(uint32_t)(1 << otcam_bit_));
}

// ---------------------------------------------------------------------------
// is hash Idx Dleft or OTCAM
// ---------------------------------------------------------------------------
bool
hash::is_dleft(uint32_t hash_idx)
{
    // For OTCAM, otcam_bit_ is set
    return ((hash_idx & (uint32_t)(1 << otcam_bit_)) ? FALSE : TRUE);
}


// ---------------------------------------------------------------------------
// Generate hash from Key
// ---------------------------------------------------------------------------
#define SDK_INTERNAL_MCAST_CRC32_HASH_SEED 0x33335555
uint32_t
hash::generate_hash_(void *key, uint32_t key_len)
{
    uint32_t hash_val = 0;
    uint32_t crc_init_val = 0x00000000;
    boost::crc_basic<32> *crc_hash;
    // TODO - Replace this with whatever hardware implements
    // return crc32((uint32_t)SDK_INTERNAL_MCAST_CRC32_HASH_SEED, (const void *)key,
    //        (uint32_t)key_len) % dleft_capacity_;

    switch(hash_poly_) {
        case HASH_POLY0:
            crc_hash = new boost::crc_basic<32>(0x04C11DB7, crc_init_val,
                                                0x00000000, false, false);
            crc_hash->process_bytes(key, key_len);
            hash_val = crc_hash->checksum();
            break;
        case HASH_POLY1:
            crc_hash = new boost::crc_basic<32>(0x1EDC6F41, crc_init_val,
                                                0x00000000, false, false);
            crc_hash->process_bytes(key, key_len);
            hash_val = crc_hash->checksum();
            break;
        case HASH_POLY2:
            crc_hash = new boost::crc_basic<32>(0x741B8CD7, crc_init_val,
                                                0x00000000, false, false);
            crc_hash->process_bytes(key, key_len);
            hash_val = crc_hash->checksum();
            break;
        case HASH_POLY3:
            crc_hash = new boost::crc_basic<32>(0x814141AB, crc_init_val,
                                                0x00000000, false, false);
            crc_hash->process_bytes(key, key_len);
            hash_val = crc_hash->checksum();
            break;
        default:
            SDK_ASSERT_GOTO(0, end);
    }

end:
    delete crc_hash;
    SDK_TRACE_DEBUG("hashVal: 0x%x, Idx: %d\n", hash_val, hash_val % dleft_capacity_);
    return hash_val % dleft_capacity_;
}

// ----------------------------------------------------------------------------
// program hw table
// ----------------------------------------------------------------------------
sdk_ret_t
hash::program_table_(hash_entry_t *he, void *hwkey)
{
    p4pd_error_t pd_err = P4PD_SUCCESS;

    if (!he || !hwkey) {
        return SDK_RET_INVALID_ARG;
    }

    // Entry trace
    if (entry_trace_en_) {
        entry_trace_(he);
    }

    // P4-API: Wrihe
    pd_err = p4pd_entry_write(id_, he->index, (uint8_t *)hwkey,
                              NULL, he->data);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : SDK_RET_OK;
}

// ----------------------------------------------------------------------------
// deprogram HW table
// ----------------------------------------------------------------------------
sdk_ret_t
hash::deprogram_table_(hash_entry_t *he)
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;

    if (!he) {
        return SDK_RET_INVALID_ARG;
    }

    // Build Hw Keys/KeyMasks
    hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HASH_HW_KEY_DEPGM, hwkey_len_);
    std::memset(he->data, 0, swdata_len_);

    // P4-API: Write
    pd_err = p4pd_entry_write(id_, he->index, (uint8_t *)hwkey,
                              NULL, he->data);
    SDK_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

    SDK_TRACE_DEBUG("%s: Index: %d de-programmed\n", name_, he->index);
end:
    SDK_FREE(SDK_MEM_ALLOC_HASH_HW_KEY_DEPGM, hwkey);

    return (pd_err != P4PD_SUCCESS) ? SDK_RET_HW_PROGRAM_ERR : SDK_RET_OK;
}

// ----------------------------------------------------------------------------
// Increment Stats
// ----------------------------------------------------------------------------
void
hash::stats_incr(stats stat)
{
    SDK_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
hash::stats_decr(stats stat)
{
    SDK_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
hash::stats_update(hash::api ap, sdk_ret_t rs)
{
    switch (ap) {
        case INSERT:
            if(rs == SDK_RET_OK) stats_incr(STATS_INS_SUCCESS);
            else if(rs == SDK_RET_HW_PROGRAM_ERR) stats_incr(STATS_INS_FAIL_HW);
            else if(rs == SDK_RET_NO_RESOURCE) stats_incr(STATS_INS_FAIL_NO_RES);
            else if(rs == SDK_RET_DUPLICATE_INS) stats_incr(STATS_INS_FAIL_DUP_INS);
            else SDK_ASSERT(0);
            break;
        case INSERT_WITHID:
            if(rs == SDK_RET_OK) stats_incr(STATS_INS_WITHID_SUCCESS);
            else if(rs == SDK_RET_HW_PROGRAM_ERR) stats_incr(STATS_INS_WITHID_FAIL_HW);
            else if(rs == SDK_RET_DUPLICATE_INS) stats_incr(STATS_INS_WITHID_FAIL_DUP_INS);
            else if(rs == SDK_RET_INVALID_ARG) stats_incr(STATS_INS_WITHID_FAIL_INV_ARG);
            else SDK_ASSERT(0);
        case UPDATE:
            if(rs == SDK_RET_OK) stats_incr(STATS_UPD_SUCCESS);
            else if(rs == SDK_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
            else if(rs == SDK_RET_INVALID_ARG) stats_incr(STATS_UPD_FAIL_INV_ARG);
            else if(rs == SDK_RET_HW_PROGRAM_ERR) stats_incr(STATS_UPD_FAIL_HW);
            else if(rs == SDK_RET_OOB) stats_incr(STATS_UPD_FAIL_OOB);
            else SDK_ASSERT(0);
            break;
        case REMOVE:
            if(rs == SDK_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if(rs == SDK_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if(rs == SDK_RET_INVALID_ARG) stats_incr(STATS_REM_FAIL_INV_ARG);
            else if(rs == SDK_RET_HW_PROGRAM_ERR) stats_incr(STATS_REM_FAIL_HW);
            else if(rs == SDK_RET_OOB) stats_incr(STATS_REM_FAIL_OOB);
            else SDK_ASSERT(0);
            break;
        case RETRIEVE:
            if(rs == SDK_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if(rs == SDK_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if(rs == SDK_RET_INVALID_ARG) stats_incr(STATS_REM_FAIL_INV_ARG);
            else if(rs == SDK_RET_OOB) stats_incr(STATS_REM_FAIL_OOB);
            else SDK_ASSERT(0);
            break;
        default:
            SDK_ASSERT(0);
    }
}

// ----------------------------------------------------------------------------
// Oflow tcam capacity
// ----------------------------------------------------------------------------
uint32_t
hash::oflow_capacity(void)
{
    return otcam_ ? otcam_->capacity() : 0;
}

// ----------------------------------------------------------------------------
// number of entries in use.
// ----------------------------------------------------------------------------
uint32_t
hash::num_entries_in_use(void)
{
    return entry_ht_->num_entries();
}

// ----------------------------------------------------------------------------
// number of oflow entries in use.
// ----------------------------------------------------------------------------
uint32_t
hash::oflow_num_entries_in_use(void)
{
    return otcam_ ? otcam_->num_entries_in_use() : 0;
}

// ----------------------------------------------------------------------------
// number of insert operations attempted
// ----------------------------------------------------------------------------
uint32_t
hash::num_inserts(void)
{
    return stats_[STATS_INS_SUCCESS] + stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// number of failed insert operations
// ----------------------------------------------------------------------------
uint32_t
hash::num_insert_errors(void)
{
    return stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// number of update operations attempted
// ----------------------------------------------------------------------------
uint32_t
hash::num_updates(void) const
{
    return stats_[STATS_UPD_SUCCESS] + stats_[STATS_UPD_FAIL_OOB] +
        stats_[STATS_UPD_FAIL_INV_ARG] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_UPD_FAIL_HW];
}

// ----------------------------------------------------------------------------
// number of failed update operations
// ----------------------------------------------------------------------------
uint32_t
hash::num_update_errors(void) const
{
    return stats_[STATS_UPD_FAIL_INV_ARG] + stats_[STATS_UPD_FAIL_OOB] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_UPD_FAIL_HW];
}
// ----------------------------------------------------------------------------
// number of delete operations attempted
// ----------------------------------------------------------------------------
uint32_t
hash::num_deletes(void)
{
    return stats_[STATS_REM_SUCCESS] + stats_[STATS_REM_FAIL_OOB] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW] +
        stats_[STATS_REM_FAIL_INV_ARG];
}

// ----------------------------------------------------------------------------
// number of failed delete operations
// ----------------------------------------------------------------------------
uint32_t
hash::num_delete_errors(void)
{
    return stats_[STATS_REM_FAIL_OOB] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW] +
        stats_[STATS_REM_FAIL_INV_ARG];
}

// ----------------------------------------------------------------------------
// print entry
// ----------------------------------------------------------------------------
sdk_ret_t
hash::entry_trace_(hash_entry_t *he)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(id_, he->index,
            he->key, NULL, he->data, buff, sizeof(buff));
    SDK_ASSERT(p4_err == P4PD_SUCCESS);

    SDK_TRACE_DEBUG("%s: Index: %d \n %s", name_, he->index, buff);

    return SDK_RET_OK;
}

// ----------------------------------------------------------------------------
// Returns string of the entry
// ----------------------------------------------------------------------------
sdk_ret_t
hash::entry_to_str(void *key, void *key_mask, void *data, uint32_t index,
                   char *buff, uint32_t buff_size)
{
    p4pd_error_t    p4_err;

    if (hash::is_dleft(index)) {
        p4_err = p4pd_global_table_ds_decoded_string_get(id_, index,
                                                         key, key_mask, data,
                                                         buff, buff_size);
        SDK_ASSERT(p4_err == P4PD_SUCCESS);
    } else {
        otcam_->entry_to_str(key, key_mask, data,
                             get_otcam_id_from_hash_idx_(index),
                             buff, buff_size);
    }

    SDK_TRACE_DEBUG("%s: Index: %d \n %s", name_, index, buff);

    return SDK_RET_OK;
}

}    // namespace table
}    // namespace sdk

