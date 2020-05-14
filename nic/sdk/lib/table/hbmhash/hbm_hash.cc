//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <cmath>
#include <string.h>

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "hbm_hash.hpp"
#include "hbm_hash_entry.hpp"
#include "hbm_hash_table_entry.hpp"
#include "hbm_hash_spine_entry.hpp"
#include "hbm_hash_hint_group.hpp"
#include "hbm_hash_mem_types.hpp"

using sdk::table::HbmHash;
using sdk::table::HbmHashTableEntry;
using sdk::table::HbmHashEntry;

void
hbm_hash_print_bytes(void *data, uint32_t len)
{
    uint8_t *tmp = (uint8_t *)data;

    SDK_TRACE_DEBUG("Key:");
    for (uint32_t i = 0; i < len; i++) {
        SDK_TRACE_PRINT("%#x ", tmp[i]);
    }
    SDK_TRACE_PRINT("\n");
}


//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
HbmHash *
HbmHash::factory(std::string table_name, uint32_t table_id,
                 uint32_t collision_table_id,
                 uint32_t hash_capacity,
                 uint32_t coll_capacity,
                 uint32_t key_len,
                 uint32_t data_len,
                 uint32_t num_hints_per_entry,
                 uint32_t max_recircs,
                 HbmHash::HashPoly hash_poly,
                 uint32_t mtrack_id,
                 bool entry_trace_en,
                 table_health_monitor_func_t health_monitor_func)
{
    void    *mem = NULL;
    HbmHash *hbmhash = NULL;
    sdk_ret_t ret;

    mem = SDK_CALLOC(mtrack_id, sizeof(HbmHash));
    if (!mem) {
        return NULL;
    }

    hbmhash = new (mem) HbmHash(table_name, table_id,
                                collision_table_id, hash_capacity,
                                coll_capacity, key_len,
                                data_len, num_hints_per_entry, max_recircs,
                                hash_poly, entry_trace_en,
                                health_monitor_func);

    ret = hbmhash->init();
    if (ret != SDK_RET_OK) {
        hbmhash->~HbmHash();
        SDK_FREE(mtrack_id, hbmhash);
        return NULL;
    }

    return hbmhash;
}

sdk_ret_t
HbmHash::initialize_slabs()
{
    hbm_hash_entry_slab_ = slab::factory("hbm_hash_entry",
                                         sdk::lib::SDK_SLAB_ID_HBM_HASH_ENTRY,
                                         sizeof(HbmHashEntry),
                                         256, true, true, true);
    if (!hbm_hash_entry_slab_) {
        return SDK_RET_OOM;
    }

    hbm_sw_key_slab_ = slab::factory("hbm_hash_sw_key",
                                     sdk::lib::SDK_SLAB_ID_HBM_HASH_SW_KEY,
                                     key_len_,
                                     256, true, true, true);
    if (!hbm_sw_key_slab_) {
        return SDK_RET_OOM;
    }

    hbm_hw_key_slab_ = slab::factory("hbm_hash_hw_key",
                                     sdk::lib::SDK_SLAB_ID_HBM_HASH_HW_KEY,
                                     hwkey_len_,
                                     256, true, true, true);
    if (!hbm_hw_key_slab_) {
        return SDK_RET_OOM;
    }

    hbm_sw_data_slab_ = slab::factory("hbm_hash_sw_data",
                                     sdk::lib::SDK_SLAB_ID_HBM_HASH_SW_DATA,
                                     8, /* data_len_ is 5B */
                                     256, true, true, true);
    if (!hbm_sw_data_slab_) {
        return SDK_RET_OOM;
    }

    hbm_hash_hint_group_slab_ = slab::factory("hbm_hash_hint_group",
                                              sdk::lib::SDK_SLAB_ID_HBM_HASH_HINT_GROUP,
                                              sizeof(HbmHashHintGroup),
                                              256, true, true, true);
    if (!hbm_hash_hint_group_slab_) {
        return SDK_RET_OOM;
    }

    hbm_hash_table_entry_slab_ = slab::factory("hbm_hash_table_entry",
                                               sdk::lib::SDK_SLAB_ID_HBM_HASH_TABLE_ENTRY,
                                               sizeof(HbmHashTableEntry),
                                               256, true, true, true);
    if (!hbm_hash_table_entry_slab_) {
        return SDK_RET_OOM;
    }

    hbm_hash_spine_entry_slab_ = slab::factory("hbm_hash_spine_entry",
                                               sdk::lib::SDK_SLAB_ID_HBM_HASH_SPINE_ENTRY,
                                               sizeof(HbmHashSpineEntry),
                                               256, true, true, true);
    if (!hbm_hash_spine_entry_slab_) {
        return SDK_RET_OOM;
    }

    return SDK_RET_OK;
}

sdk_ret_t
HbmHash::destroy_slabs()
{
    if (hbm_hash_entry_slab_) slab::destroy(hbm_hash_entry_slab_);
    if (hbm_sw_key_slab_) slab::destroy(hbm_sw_key_slab_);
    if (hbm_hw_key_slab_) slab::destroy(hbm_hw_key_slab_);
    if (hbm_sw_data_slab_) slab::destroy(hbm_sw_data_slab_);
    if (hbm_hash_hint_group_slab_) slab::destroy(hbm_hash_hint_group_slab_);
    if (hbm_hash_table_entry_slab_) slab::destroy(hbm_hash_table_entry_slab_);
    if (hbm_hash_spine_entry_slab_) slab::destroy(hbm_hash_spine_entry_slab_);

    return SDK_RET_OK;
}

sdk_ret_t
HbmHash::init()
{
    initialize_slabs();

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
HbmHash::destroy(HbmHash *re, uint32_t mtrack_id)
{
    if (re) {
        re->destroy_slabs();
        re->~HbmHash();
        SDK_FREE(mtrack_id, re);
    }
}

// ---------------------------------------------------------------------------
// Constructor - HbmHash
// ---------------------------------------------------------------------------
HbmHash::HbmHash(std::string table_name,
                 uint32_t table_id,
                 uint32_t collision_table_id,
                 uint32_t hash_capacity,             // 2M
                 uint32_t coll_capacity,             // 16k
                 uint32_t key_len,
                 uint32_t data_len,
                 uint32_t num_hints_per_entry,
                 uint32_t max_recircs,
                 HbmHash::HashPoly hash_poly,
                 bool entry_trace_en,
                 table_health_monitor_func_t health_monitor_func)
{
    table_name_                 = table_name;
    table_id_                   = table_id;
    collision_table_id_         = collision_table_id;
    key_len_                    = key_len;
    data_len_                   = data_len;
    num_hints_per_entry_        = num_hints_per_entry;
    max_recircs_                = max_recircs;
    hwkey_len_                  = 0;
    hwdata_len_                 = 0;
    hash_capacity_              = 0;
    hash_poly_                  = hash_poly;
    entry_trace_en_             = entry_trace_en;
    health_monitor_func_        = health_monitor_func;

    pre_process_sizes_(hash_capacity, coll_capacity);

    hint_len_ = 32/*CRC 32*/ - hash_tbl_key_len_;

    // Allocate indexer for Collision Table, skip zero
    coll_indexer_ = indexer::factory(coll_capacity_, true, true);

    // Assumption: Max. number of entries will be hash table cap.
    entry_indexer_ = indexer::factory(hash_capacity_, true, true);

    // Assumption: Delayed Delete is disabled.
    enable_delayed_del_ = FALSE;

    p4pd_hwentry_query(table_id_, &hwkey_len_, NULL, &hwdata_len_);

    // round off to higher byte
    hwkey_len_ = (hwkey_len_ >> 3) + ((hwkey_len_ & 0x7) ? 1 : 0);
    if (hwkey_len_ * 8 > 512) {
        if (hwkey_len_ % 64) {
            hwkey_len_ +=  (64 - (hwkey_len_ % 64));
        }
    }

    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);

    // Len in sw data structure. Its being generated as multiples of 16.
    hint_mem_len_B_ = (((hash_coll_tbl_key_len_ / 16) + 1) * 16) / 8;
    entire_data_len_ = 1 +                              /* action id */
                       1 +                              /* entry_valid */
                       data_len_ +                      /* data len */
                       num_hints_per_entry_ * 2 +  /* hash len */
                       num_hints_per_entry_  * hint_mem_len_B_ + /* coll idx */
                       1 +                              /* more_hashs */
                       hint_mem_len_B_;                 /* coll idx */

    // Initialize for Stats
    stats_ = (uint64_t *)SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_STATS,
                                    sizeof(uint64_t) * STATS_MAX);

    hbm_hash_table_ = (HbmHashTableEntry **)
                                SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_BUCKETS,
                                           sizeof(HbmHashTableEntry *) * hash_capacity_);

    entry_map_ = (HbmHashEntry **)SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_ENTRIES,
                                      sizeof(HbmHashEntry *) * hash_capacity_);

    hbm_hash_entry_slab_ = NULL;
    hbm_sw_key_slab_ = NULL;
    hbm_hw_key_slab_ = NULL;
    hbm_sw_data_slab_ = NULL;
    hbm_hash_hint_group_slab_ = NULL;
    hbm_hash_table_entry_slab_ = NULL;
    hbm_hash_spine_entry_slab_ = NULL;
    health_state_ = TABLE_HEALTH_GREEN;
    entry_count_ = 0;

    SDK_TRACE_DEBUG("HbmHashName:%s, key_len_:%dB, data_len_:%dB, entire_data_len:%dB, "
                    "hwkey_len:%dB, hwdata_len:%dB, "
                    "hash_tbl_key_len_:%db, hash_coll_tbl_key_len_:%db"
                    "hint_len:%db, hint_mem_len_B_:%d, hash_capacity_:%d "
                    "coll_capacity_:%d",
                    table_name_.c_str(), key_len_, data_len_, entire_data_len_,
                    hwkey_len_, hwdata_len_,
                    hash_tbl_key_len_, hash_coll_tbl_key_len_,
                    hint_len_, hint_mem_len_B_,
                    hash_capacity_, coll_capacity);

}

// ---------------------------------------------------------------------------
// Destructor - HbmHash
// ---------------------------------------------------------------------------
HbmHash::~HbmHash()
{
    indexer::destroy(coll_indexer_);
    indexer::destroy(entry_indexer_);
    SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_ENTRIES, entry_map_);
    SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_BUCKETS, hbm_hash_table_);
    SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_STATS, stats_);
}

// ---------------------------------------------------------------------------
// pre_process_sizes_
//
// The sizes for Hash Table and Collision table may not be power of 2.
// To get the key lengths, they have to be power of 2. If they are not
// we go for the lower power of 2 to get key lengths.
// ---------------------------------------------------------------------------
void
HbmHash::pre_process_sizes_(uint32_t hash_capacity,
                            uint32_t coll_capacity)
{
    // Normalize entries to power of 2
    hash_tbl_key_len_ = get_num_bits_from_size_(hash_capacity);
    if (hash_capacity & (hash_capacity-1)) { // power of 2 ?
        hash_capacity_ = hash_capacity;
    } else {
        hash_capacity_ = pow(2, hash_tbl_key_len_);
    }

    // Normalize Hash Collision entries to power of 2
    hash_coll_tbl_key_len_ =
        get_num_bits_from_size_(coll_capacity);
    if (coll_capacity & (coll_capacity-1)) {
        coll_capacity_ = coll_capacity;
    } else {
        coll_capacity_ = pow(2, hash_coll_tbl_key_len_);
    }

}

// ---------------------------------------------------------------------------
// - Gets number of bits needed in the key to support the size of a table.
//   If size is not a power of 2, we support lower bound.
//   For eg. size of 10 will support size of 8 with key size of 3 bits.
//   8 => 3
//   16 => 4
// ---------------------------------------------------------------------------
uint32_t
HbmHash::get_num_bits_from_size_(uint32_t size)
{
    uint32_t i = 0;

    while (size > 1) {
        size = size >> 1;
        i++;
    }
    return i;
}

uint32_t
HbmHash::calc_hash_(void *key, void *data)
{
    sdk_ret_t                       rs = SDK_RET_OK;
    HbmHashEntry                    *entry = NULL;
    uint32_t                        hash_val = 0;
    void                            *hwkey = NULL;

    // create entry
    entry = HbmHashEntry::factory(key, key_len_, data, data_len_,
                                  hwkey_len_, false, this);

    // hwkey = SDK_CALLOC(SDK_MEM_ALLOC_HBM_HASH_HW_KEY, hwkey_len_);
    hwkey = hbm_hw_key_alloc();

    rs = entry->form_hw_key(table_id_, hwkey);
    if (rs != SDK_RET_OK) SDK_ASSERT(0);

    // cal. hash
    hash_val = generate_hash_(hwkey, hwkey_len_, false);
    // SDK_FREE(SDK_MEM_ALLOC_HBM_HASH_HW_KEY, hwkey);
    hbm_hw_key_free(hwkey);

    // delete entry;
    HbmHashEntry::destroy(entry, this);

    return hash_val;
}

// ---------------------------------------------------------------------------
// Trigger Health monitor.
// ---------------------------------------------------------------------------
void
HbmHash::trigger_health_monitor()
{
    if (health_monitor_func_) {
        health_monitor_func_(collision_table_id_, (char *)table_name_.c_str(), health_state_,
                             coll_capacity_,
                             coll_table_num_entries_in_use(),
                             &health_state_);
    }
}

// ---------------------------------------------------------------------------
// Insert
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::insert (void *key, void *data, uint32_t *index)
{
    sdk_ret_t                       rs = SDK_RET_OK;
    sdk_ret_t                       rs1 = SDK_RET_OK;
    HbmHashEntry                    *entry = NULL;
    uint32_t                        hash_val = 0;
    uint32_t                        bucket_index = 0, fe_idx = 0;
    void                            *hwkey = NULL;
    HbmHashTableEntry               *bucket = NULL;


    SDK_TRACE_DEBUG("---------- Insert ---------");

    rs = alloc_entry_index_(&fe_idx);
    if (rs != SDK_RET_OK) goto end;

    //hbm_hash_print_bytes(key, key_len_);

    SDK_TRACE_DEBUG("Insert entry_pi_idx_:%d for tbl_id:%d",
                    fe_idx, table_id_);

    // create a entry
    entry = HbmHashEntry::factory(key, key_len_, data, data_len_,
                               hwkey_len_, true, this);

    hwkey = entry->get_hwkey();

    rs = entry->form_hw_key(table_id_, hwkey);
    if (rs != SDK_RET_OK) goto end;

    // cal. hash
    hash_val = generate_hash_(hwkey, hwkey_len_);

    entry->set_hash_val(hash_val);

    // check if table entry exists
    bucket_index = fetch_hbm_hash_table_bits_(hash_val);
    bucket = retrieve_bucket(bucket_index);
    SDK_TRACE_DEBUG("hash_val:%#x, hbm_hash_table_index:%#x",
                    hash_val, bucket_index);
    if (bucket) {
        // entry already exists
        SDK_TRACE_DEBUG("Bucket exists ...");
        rs = bucket->insert(entry);
        // TODO: No need to send coll return status
        if (rs == SDK_RET_OK) {
            SDK_TRACE_DEBUG("Setting collision return code");
            rs = SDK_RET_COLLISION;
        }

    } else {
        // entry doesnt exist
        SDK_TRACE_DEBUG("New FT Entry ...");
        bucket = HbmHashTableEntry::factory(bucket_index, this);
        rs = bucket->insert(entry);

        // If insert is SUCCESS, put bucket into the table
        if (rs == SDK_RET_OK) {
            insert_bucket(bucket_index, bucket);
        } else {
            HbmHashTableEntry::destroy(bucket);
        }
    }

    if (rs == SDK_RET_OK || rs == SDK_RET_COLLISION) {
        // insert into entry indexer map ... For retrieval
        // entry_map_[fe_idx] = entry;
        insert_entry(fe_idx, entry);
        entry->set_global_index(fe_idx);
        *index = fe_idx;

        //SDK_TRACE_ERR("#Recircs: %d", entry->get_recircs());
        if (entry->get_recircs() >= 2) {
            char            buff[4096] = {0};
            entry->entry_to_str(buff, 4096);
            SDK_TRACE_ERR("Recirc count >= 2, flow key : %s", buff);
        }
    } else {
        // insert failed
        SDK_TRACE_DEBUG("Insert FAIL ...rs: %d", rs);

        // delete entry
        HbmHashEntry::destroy(entry, this);

        // free index alloced
        rs1 = free_hbm_hash_entry_index_(fe_idx);
        SDK_ASSERT(rs1 == SDK_RET_OK);
    }

end:

    // Uncomment for debugging
    // print_hbm_hash();
    //SDK_TRACE_DEBUG("ret:{}", rs);
    stats_update(INSERT, rs);
    trigger_health_monitor();
    return rs;
}


// ---------------------------------------------------------------------------
// Insert with Hash
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::insert_with_hash(void *key, void *data, uint32_t *index, uint32_t hash_val)
{
    sdk_ret_t                       rs = SDK_RET_OK, rs1 = SDK_RET_OK;
    HbmHashEntry                    *entry = NULL;
    HbmHashTableEntry               *bucket = NULL;
    uint32_t                        bucket_index = 0, fe_idx = 0;
    void                            *hwkey = NULL;

    SDK_TRACE_DEBUG("---------- HbmHash Table Insert With Hash ---------");

    rs = alloc_entry_index_(&fe_idx);
    if (rs != SDK_RET_OK) goto end;

    //hbm_hash_print_bytes(key, key_len_);

    SDK_TRACE_DEBUG("Insert entry_pi_idx:%d for tbl_id:%d",
                    fe_idx, table_id_);

    // create a flow entry
    entry = HbmHashEntry::factory(key, key_len_, data, data_len_,
                                  hwkey_len_, true, this);

    hwkey = entry->get_hwkey();

    rs = entry->form_hw_key(table_id_, hwkey);
    if (rs != SDK_RET_OK) goto end;

    entry->set_hash_val(hash_val);

    // check if flow table entry exists
    bucket_index = fetch_hbm_hash_table_bits_(hash_val);
    SDK_TRACE_DEBUG("hash_val:%#x, hbm_hash_table_index:%#x",
                    hash_val, bucket_index);
    bucket = retrieve_bucket(bucket_index);
    if (bucket) {
        // flow table entry already exists
        SDK_TRACE_DEBUG("Bucket exists ...");
        rs = bucket->insert(entry);
        // TODO: No need to send flow coll return status
        if (rs == SDK_RET_OK) {
            SDK_TRACE_DEBUG("Setting collision return code");
            rs = SDK_RET_COLLISION;
        }

    } else {
        // flow table entry doesnt exist
        SDK_TRACE_DEBUG("New FT Entry ...");
        bucket = HbmHashTableEntry::factory(bucket_index, this);
        rs = bucket->insert(entry);

        // If insert is SUCCESS, put ht_entry into the map
        if (rs == SDK_RET_OK) {
            insert_bucket(bucket_index, bucket);
        } else {
            // delete ht_entry;
            HbmHashTableEntry::destroy(bucket);
        }
    }

    if (rs == SDK_RET_OK || rs == SDK_RET_COLLISION) {
        // insert into flow entry indexer map ... For retrieval
        // entry_map_[fe_idx] = entry;
        insert_entry(fe_idx, entry);
        entry->set_global_index(fe_idx);
        *index = fe_idx;

        if (entry->get_recircs() >= 2) {
            char            buff[4096] = {0};
            entry->entry_to_str(buff, 4096);
            SDK_TRACE_ERR("Recirc count >= 2, flow key : %s", buff);
        }
    } else {
        // insert failed
        SDK_TRACE_DEBUG("Insert FAIL ...rs: %d", rs);

        // delete flow entry
        // delete entry;
        HbmHashEntry::destroy(entry, this);

        // free index alloced
        rs1 = free_hbm_hash_entry_index_(fe_idx);
        SDK_ASSERT(rs1 == SDK_RET_OK);
    }

end:
    // Uncomment for debugging
    // print_flow();
    //SDK_TRACE_DEBUG("ret:%d", rs);
    stats_update(INSERT, rs);
    trigger_health_monitor();
    return rs;
}



// ---------------------------------------------------------------------------
// Updates the entry. Returns error, if its not present
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::update(uint32_t index, void *data)
{
    sdk_ret_t                   rs = SDK_RET_OK;
    HbmHashEntry                *entry = NULL;

    SDK_TRACE_DEBUG("Update Entry %d", index);
    // check if entry exists.
    entry = retrieve_entry(index);
    if (entry) {
        // get the entry and call update on HbmHashEntry.
        rs = entry->update(data);
        // ideally this should not fail as there is no alloc. of resources
        SDK_ASSERT(rs == SDK_RET_OK);
    } else {
        // entry doesn't exist
        SDK_TRACE_DEBUG("Error: Not Present %d ...", index);
        rs = SDK_RET_ENTRY_NOT_FOUND;
    }

    //print_hbm_hash();
    stats_update(UPDATE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Removes the entry. -- TODO: Revisit. Untested Code
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::remove(uint32_t index)
{
    sdk_ret_t                   rs = SDK_RET_OK;
    HbmHashEntry                *entry = NULL;
    HbmHashTableEntry           *bucket = NULL;

    // Check if entry exists.
    entry = retrieve_entry(index);
    if (entry) {
        // Store the Bucket before entry cleanup
        bucket = entry->get_bucket();

        // Call remove
        rs = bucket->remove(entry);
        if (rs == SDK_RET_OK) {
            HbmHashEntry::destroy(entry, this);
            // Remove it from entry map.
            assert(entry == remove_entry(index));
            // Free the index in indexer
            free_hbm_hash_entry_index_(index);

            // Check if we have to remove the FT entry
            if (!bucket->get_num_hbm_hash_hgs() && // No HGs
                !bucket->get_spine_entry()) { // No Spine Entries
                // Remove from FTE map
                remove_bucket(bucket->get_bucket_index());
                // Free up the Table Entry.
                HbmHashTableEntry::destroy(bucket);
            }
        }
    } else {
        // Entry doesn't exist
        rs = SDK_RET_ENTRY_NOT_FOUND;
    }

    //print_hbm_hash();
    stats_update(REMOVE, rs);
    trigger_health_monitor();
    return rs;
}

// ---------------------------------------------------------------------------
// Generate Hash from Key
// ---------------------------------------------------------------------------
uint32_t
HbmHash::generate_hash_(void *key, uint32_t key_len, bool log)
{
    return sdk::utils::crc32((uint8_t *)key, key_len, hash_poly_);
}

// ---------------------------------------------------------------------------
// Fetch Hash Table Bits
// ---------------------------------------------------------------------------
uint32_t
HbmHash::fetch_hbm_hash_table_bits_(uint32_t hash_val)
{
    // Get hash_tbl_key_len_ number of LSB bits
    return (hash_val & ((1 << hash_tbl_key_len_) - 1));
}

// ---------------------------------------------------------------------------
// Fetch Hint Bits
// ---------------------------------------------------------------------------
uint32_t
HbmHash::fetch_hint_bits_(uint32_t hash_val)
{
    // Get hash_tbl_key_len_ number of LSB bits
    return ((hash_val & ~((1 << hash_tbl_key_len_) - 1)) >> hash_tbl_key_len_);
}

// ---------------------------------------------------------------------------
// Get Delayed Delete Enable
// ---------------------------------------------------------------------------
bool
HbmHash::get_delayed_del_en()
{
    return enable_delayed_del_;
}

// ---------------------------------------------------------------------------
// Set Delayed Delete Enable
// ---------------------------------------------------------------------------
void
HbmHash::set_delayed_del_en(bool en)
{
    enable_delayed_del_ = en;
}


// ---------------------------------------------------------------------------
// Get number of hints per entry
// ---------------------------------------------------------------------------
uint32_t
HbmHash::get_num_hints_per_entry()
{
    return num_hints_per_entry_;
}


// ---------------------------------------------------------------------------
// Add Entry to the Global Map
// ---------------------------------------------------------------------------
void
HbmHash::add_hbm_hash_entry_global_map(HbmHashEntry *fe, uint32_t index)
{
    entry_map_[index] = fe;
}


// ---------------------------------------------------------------------------
// Get Collision indexer
// ---------------------------------------------------------------------------
indexer *
HbmHash::get_coll_indexer()
{
    return coll_indexer_;
}


// ---------------------------------------------------------------------------
// Push into FE delayed delete queue
// ---------------------------------------------------------------------------
void
HbmHash::push_fe_delete_q(HbmHashEntry *fe)
{
    hbm_hash_entry_del_q_.push(fe);
}

// ---------------------------------------------------------------------------
// Push into HG delayed delete queue
// ---------------------------------------------------------------------------
void
HbmHash::push_hg_delete_q(HbmHashHintGroup *hg)
{
    hbm_hash_hg_del_q_.push(hg);
}


// ----------------------------------------------------------------------------
// Allocate a entry index
// ----------------------------------------------------------------------------
sdk_ret_t
HbmHash::alloc_entry_index_(uint32_t *idx)
{
    sdk_ret_t   rs = SDK_RET_OK;

    // Allocate an index in repl. table
    indexer::status irs = entry_indexer_->alloc((uint32_t *)idx);
    if (irs != indexer::SUCCESS) {
        SDK_TRACE_DEBUG("HbmHash Entry Capacity reached : %u",
                        entry_indexer_->get_size());
        return SDK_RET_NO_RESOURCE;
    }
    //SDK_TRACE_DEBUG("Alloc entry_index : %u", *idx);
    return rs;
}

// ----------------------------------------------------------------------------
// Free entry index
// ----------------------------------------------------------------------------
sdk_ret_t
HbmHash::free_hbm_hash_entry_index_(uint32_t idx)
{
    sdk_ret_t   rs = SDK_RET_OK;

    indexer::status irs = entry_indexer_->free((uint32_t)idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return SDK_RET_DUPLICATE_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return SDK_RET_ERR;
    }
    //SDK_TRACE_DEBUG("Free entry_index : %u", idx);

    return rs;
}

// ---------------------------------------------------------------------------
// Alloc a free index in Collision indexer
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::alloc_collision_index(uint32_t *idx)
{
    sdk_ret_t   rs = SDK_RET_OK;

    // Allocate an index in Coll. table
    indexer::status irs = coll_indexer_->alloc(idx);
    if (irs != indexer::SUCCESS) {
        return SDK_RET_NO_RESOURCE;
    }
    //SDK_TRACE_DEBUG("alloc_coll_indexer : %u", *idx);
    return rs;
}

// ---------------------------------------------------------------------------
// Free up an index in Collision indexer
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::free_collision_index(uint32_t idx)
{
    sdk_ret_t   rs = SDK_RET_OK;

    indexer::status irs = coll_indexer_->free((uint32_t)idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return SDK_RET_DUPLICATE_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return SDK_RET_ERR;
    }

    //SDK_TRACE_DEBUG("free_coll_indexer : %u", idx);
    return rs;
}

// ----------------------------------------------------------------------------
// hbm hash action data offsets
//  {
//      action id;      (1 byte)
//      entry_valid;    (1 byte)
//      data            (data_len_)
//      <hash           (2 bytes)
//       hint>          (2 bytes)
//      more_hashs      (1 byte)
//      more_hints      (2 bytes)
//  }
// ----------------------------------------------------------------------------
#define HBM_HASH_DATA_ACTION_ID_LEN 1
#define HBM_HASH_DATA_ENTRY_VALID_LEN 1
#define HBM_HASH_DATA_HASH_LEN 2
// #define HBM_HASH_DATA_HINT_LEN 2
#define HBM_HASH_DATA_MORE_HASHS_LEN 1
// #define HBM_HASH_DATA_MORE_HINTS_LEN 2
sdk_ret_t
HbmHash::hbm_hash_action_data_offsets(void *action_data,
                                      uint8_t **action_id,
                                      uint8_t **entry_valid,
                                      void **data,
                                      void **first_hash_hint,
                                      uint8_t **more_hashs,
                                      void **more_hints)
{
    sdk_ret_t   ret = SDK_RET_OK;
    uint8_t     *seek = NULL;

    seek = (uint8_t *)action_data;

    *action_id       = seek;
    *entry_valid     = seek + HBM_HASH_DATA_ACTION_ID_LEN;
    *data            = seek + HBM_HASH_DATA_ACTION_ID_LEN +
                       HBM_HASH_DATA_ENTRY_VALID_LEN;
    *first_hash_hint = (void *)(seek + HBM_HASH_DATA_ACTION_ID_LEN +
                                     HBM_HASH_DATA_ENTRY_VALID_LEN + data_len_);
    *more_hashs      = seek + HBM_HASH_DATA_ACTION_ID_LEN +
                              HBM_HASH_DATA_ENTRY_VALID_LEN +
                              data_len_ +
                              num_hints_per_entry_ * (HBM_HASH_DATA_HASH_LEN +
                                                           hint_mem_len_B_);
    *more_hints      = (void *)(seek + HBM_HASH_DATA_ACTION_ID_LEN +
                                    HBM_HASH_DATA_ENTRY_VALID_LEN + data_len_ +
                                    num_hints_per_entry_ *
                                    (HBM_HASH_DATA_HASH_LEN + hint_mem_len_B_) +
                                    HBM_HASH_DATA_MORE_HASHS_LEN);

    return ret;
}



// ----------------------------------------------------------------------------
// Increment Stats
// ----------------------------------------------------------------------------
void
HbmHash::stats_incr(stats stat)
{
    SDK_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
HbmHash::stats_decr(stats stat)
{
    SDK_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
HbmHash::stats_update(HbmHash::api ap, sdk_ret_t rs)
{
    switch (ap) {
        case INSERT:
            if(rs == SDK_RET_OK) stats_incr(STATS_INS_SUCCESS);
            else if (rs == SDK_RET_COLLISION) stats_incr(STATS_INS_HBM_HASH_COLL);
            else if (rs == SDK_RET_ENTRY_EXISTS) stats_incr(STATS_INS_FAIL_DUP_INS);
            else if(rs == SDK_RET_HW_PROGRAM_ERR) stats_incr(STATS_INS_FAIL_HW);
            else if(rs == SDK_RET_NO_RESOURCE) stats_incr(STATS_INS_FAIL_NO_RES);
            else SDK_ASSERT(0);
            break;
        case UPDATE:
            if(rs == SDK_RET_OK) stats_incr(STATS_UPD_SUCCESS);
            else if(rs == SDK_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
            else SDK_ASSERT(0);
            break;
        case REMOVE:
            if (rs == SDK_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if (rs == SDK_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if (rs == SDK_RET_HW_PROGRAM_ERR) stats_incr(STATS_REM_FAIL_HW);
            else SDK_ASSERT(0);
            break;
        default:
            SDK_ASSERT(0);
    }
}

// ----------------------------------------------------------------------------
// Number of entries in use.
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_entries_in_use(void)
{
    return entry_count();
}

// ----------------------------------------------------------------------------
// Number of collision entries in use.
// ----------------------------------------------------------------------------
uint32_t
HbmHash::coll_table_num_entries_in_use(void)
{
    return coll_indexer_->num_indices_allocated();
}

// ----------------------------------------------------------------------------
// Number of insert operations attempted
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_inserts(void)
{
    return stats_[STATS_INS_SUCCESS] + stats_[STATS_INS_HBM_HASH_COLL] +
        stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed insert operations
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_insert_errors(void)
{
    return stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of update operations attempted
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_updates(void)
{
    return stats_[STATS_UPD_SUCCESS] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND];
}

// ----------------------------------------------------------------------------
// Number of failed update operations
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_update_errors(void)
{
    return stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND];
}

// ----------------------------------------------------------------------------
// Number of delete operations attempted
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_deletes(void)
{
    return stats_[STATS_REM_SUCCESS] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed delete operations
// ----------------------------------------------------------------------------
uint32_t
HbmHash::table_num_delete_errors(void)
{
    return stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

void
HbmHash::insert_bucket(uint32_t index, HbmHashTableEntry *data)
{
    hbm_hash_table_[index] = data;
    hbm_hash_table_count_++;
}

HbmHashTableEntry*
HbmHash::remove_bucket(uint32_t index)
{
    HbmHashTableEntry   *data = NULL;
    data = hbm_hash_table_[index];
    hbm_hash_table_[index] = NULL;
    return data;
}

void
HbmHash::insert_entry(uint32_t index, HbmHashEntry *data)
{
    entry_map_[index] = data;
    entry_count_++;
}

HbmHashEntry*
HbmHash::remove_entry(uint32_t index)
{
    HbmHashEntry   *data = NULL;
    data = entry_map_[index];
    entry_map_[index] = NULL;
    entry_count_--;
    return data;
}

void *
HbmHash::hbm_hash_entry_alloc()
{
    return hbm_hash_entry_slab_->alloc();
}

void
HbmHash::hbm_hash_entry_free(void *entry)
{
    hbm_hash_entry_slab_->free(entry);
}

void *
HbmHash::hbm_hw_key_alloc()
{
    return hbm_hw_key_slab_->alloc();
}

void
HbmHash::hbm_hw_key_free(void *entry)
{
    return hbm_hw_key_slab_->free(entry);
}

void *
HbmHash::hbm_sw_key_alloc()
{
    return hbm_sw_key_slab_->alloc();
}

void
HbmHash::hbm_sw_key_free(void *entry)
{
    return hbm_sw_key_slab_->free(entry);
}

void *
HbmHash::hbm_sw_data_alloc()
{
    return hbm_sw_data_slab_->alloc();
}

void
HbmHash::hbm_sw_data_free(void *entry)
{
    return hbm_sw_data_slab_->free(entry);
}

void *
HbmHash::hbm_hash_hint_group_alloc()
{
    return hbm_hash_hint_group_slab_->alloc();
}

void
HbmHash::hbm_hash_hint_group_free(void *entry)
{
    return hbm_hash_hint_group_slab_->free(entry);
}

void *
HbmHash::hbm_hash_table_entry_alloc()
{
    return hbm_hash_table_entry_slab_->alloc();
}

void
HbmHash::hbm_hash_table_entry_free(void *entry)
{
    return hbm_hash_table_entry_slab_->free(entry);
}

void *
HbmHash::hbm_hash_spine_entry_alloc()
{
    return hbm_hash_spine_entry_slab_->alloc();
}

void
HbmHash::hbm_hash_spine_entry_free(void *entry)
{
    return hbm_hash_spine_entry_slab_->free(entry);
}

// ---------------------------------------------------------------------------
// Print Tables
// ---------------------------------------------------------------------------
sdk_ret_t
HbmHash::print_hbm_hash()
{
    sdk_ret_t           ret = SDK_RET_OK;
    uint32_t            bucket_index = 0, entry_index = 0;
    HbmHashTableEntry   *bucket = NULL;
    HbmHashEntry        *entry = NULL;

    SDK_TRACE_DEBUG("Printing Tables:");
    SDK_TRACE_DEBUG("-------- ---- -------");
    SDK_TRACE_DEBUG("Total Num_FTEs:%d\n", bucket_count());
    for (bucket_index = 0; bucket_index < table_capacity(); bucket_index++) {
        bucket = retrieve_bucket(bucket_index);
        if (bucket) {
            SDK_TRACE_DEBUG("BucketIndex:%#x", bucket_index);
            bucket->print_hbm_hash_table_entries();
        }
    }

    SDK_TRACE_DEBUG("Total Num_FEs:%d", entry_count());
    for (entry_index = 0; entry_index < table_capacity(); entry_index++) {
        entry = retrieve_entry(entry_index);
        if (entry) {
            SDK_TRACE_DEBUG(" EntryIndex:%#x", entry_index);
            entry->print_fe();
        }
    }

    SDK_ASSERT(entry_count() == entry_indexer_->num_indices_allocated());

    return ret;
}

sdk_ret_t
HbmHash::entry_to_str(uint32_t entry_index, char *buff, uint32_t buff_size)
{
    sdk_ret_t ret = SDK_RET_OK;
    HbmHashEntry *entry = NULL;
    uint32_t bucket_index = 0, hint_bits = 0;
    char entry_buff[4096] = {0};
    char inter_spine_buff[2048] = {0};
    char inter_hg_buff[2048] = {0};
    char num_recircs_str[32] = {0};
    uint32_t num_recircs = 0;

    entry = retrieve_entry(entry_index);
    if (entry) {
         bucket_index = fetch_hbm_hash_table_bits_(entry->get_hash_val());
         hint_bits = entry->get_fh_group()->get_hint_bits();

         // Assumption: if its anchor its only the first spine entry
         if (entry->get_is_anchor_entry()) {
             // Spine entry
             sprintf(inter_spine_buff, "Spine Entries(Anchor): %s:0x%x ", "FT",
                     entry->get_eff_spine_entry()->get_ht_entry()->get_bucket_index());
             entry->get_eff_spine_entry()->entry_to_str(entry_buff, sizeof(entry_buff));
         } else {
             // Collision table entry
             // - List of Spine entries and hint list entries
             strcat(inter_spine_buff, "Spine Entries: ");
             entry->get_eff_spine_entry()->get_ht_entry()->
                 inter_spine_str(entry->get_eff_spine_entry(),
                                 inter_spine_buff, sizeof(inter_spine_buff),
                                 &num_recircs);

             // - List of HGs
             strcat(inter_hg_buff, "Hint List: ");
             entry->get_fh_group()->inter_hg_str(entry, inter_hg_buff, sizeof(inter_hg_buff),
                                                   entry_buff, sizeof(entry_buff),
                                                   &num_recircs);
         }

         sprintf(num_recircs_str, "#Recircs: %d", num_recircs);

         sprintf(buff, "Hash Value: 0x%x Hash Bits: 0x%x, Hint Bits: 0x%x, %s\n%s\n%s\n%s",
                 entry->get_hash_val(),
                 bucket_index, hint_bits, num_recircs_str,
                 inter_spine_buff, inter_hg_buff, entry_buff);
     }


     return ret;
 }

sdk_ret_t
HbmHash::iterate(hbm_hash_iterate_func_t func, const void *cb_data)
{
    HbmHashEntry    *entry = NULL;
    uint32_t        entry_index = 0;

    for (entry_index = 0; entry_index < table_capacity(); entry_index++) {
        entry = retrieve_entry(entry_index);
        if (entry) {
            func(entry_index, cb_data);
        }
    }

    return SDK_RET_OK;
}
