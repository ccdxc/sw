#include <cmath>
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/hal/pd/utils/flow/flow.hpp"
#include "nic/hal/pd/utils/flow/flow_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_table_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_spine_entry.hpp"
#include "nic/hal/pd/utils/flow/flow_hint_group.hpp"

using hal::pd::utils::Flow;

thread_local boost::crc_basic<32> *g_crc32_hash_poly0 =
                                      new boost::crc_basic<32>(0x04C11DB7, 0, 0,
                                                               false, false);
thread_local boost::crc_basic<32> *g_crc32_hash_poly1 =
                                      new boost::crc_basic<32>(0x1EDC6F41, 0, 0,
                                                               false, false);
thread_local boost::crc_basic<32> *g_crc32_hash_poly2 =
                                      new boost::crc_basic<32>(0x741B8CD7, 0, 0,
                                                               false, false);
thread_local boost::crc_basic<32> *g_crc32_hash_poly3 =
                                      new boost::crc_basic<32>(0x814141AB, 0, 0,
                                                               false, false);

//---------------------------------------------------------------------------
// Factory method to instantiate the class
//---------------------------------------------------------------------------
Flow *
Flow::factory(std::string table_name, uint32_t table_id,
              uint32_t oflow_table_id,
              uint32_t flow_hash_capacity,             // 2M
              uint32_t flow_coll_capacity,             // 16k
              uint32_t key_len,
              uint32_t data_len,
              uint32_t num_hints_per_flow_entry,
              Flow::HashPoly hash_poly,
              uint32_t mtrack_id,
              bool entry_trace_en)
{
    void    *mem = NULL;
    Flow    *flow = NULL;

    mem = HAL_CALLOC(mtrack_id, sizeof(Flow));
    if (!mem) {
        return NULL;
    }

    flow = new (mem) Flow(table_name, table_id, oflow_table_id,
                          flow_hash_capacity, flow_coll_capacity, key_len,
                          data_len, num_hints_per_flow_entry, hash_poly,
                          entry_trace_en);
    return flow;
}

//---------------------------------------------------------------------------
// Method to free & delete the object
//---------------------------------------------------------------------------
void
Flow::destroy(Flow *re, uint32_t mtrack_id)
{
    if (re) {
        re->~Flow();
        HAL_FREE(mtrack_id, re);
    }
}

// ---------------------------------------------------------------------------
// Constructor - Flow
// ---------------------------------------------------------------------------
Flow::Flow(std::string table_name, uint32_t table_id,
           uint32_t oflow_table_id,
           uint32_t flow_hash_capacity,             // 2M
           uint32_t flow_coll_capacity,             // 16k
           uint32_t key_len,
           uint32_t data_len,
           uint32_t num_hints_per_flow_entry,
           Flow::HashPoly hash_poly, bool entry_trace_en)
{

    table_name_                 = table_name;
    table_id_                   = table_id;
    oflow_table_id_             = oflow_table_id;
    key_len_                    = key_len;
    data_len_                   = data_len;
    num_hints_per_flow_entry_   = num_hints_per_flow_entry;
    hwkey_len_                  = 0;
    hwdata_len_                 = 0;
    flow_hash_capacity_         = 0;
    hash_poly_                  = hash_poly;
    entry_trace_en_             = entry_trace_en;

    pre_process_sizes_(flow_hash_capacity, flow_coll_capacity);

    hint_len_ = 32/*CRC 32*/ - hash_tbl_key_len_;

    // Allocate indexer for Flow Collision Table, skip zero
    flow_coll_indexer_ = indexer::factory(flow_coll_capacity_, true,
                                          true);

    // Assumption: Max. number of flow entries will be hash table cap.
    flow_entry_indexer_ = indexer::factory(flow_hash_capacity_, true,
                                           true);

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
                       num_hints_per_flow_entry_ * 2 +  /* hash len */
                       num_hints_per_flow_entry_  * hint_mem_len_B_ + /* coll idx */
                       1 +                              /* more_hashs */
                       hint_mem_len_B_;                 /* coll idx */

    // Initialize for Stats
    // stats_ = new uint64_t[STATS_MAX]();
    stats_ = (uint64_t *)HAL_CALLOC(HAL_MEM_ALLOC_FLOW_STATS,
                                    sizeof(uint64_t) * STATS_MAX);

    HAL_TRACE_DEBUG("Flow:{}: key_len_: {} B, data_len_: {} B, entire_data_len: {} B, "
                    "hwkey_len: {} B, hwdata_len: {} B, "
                    " hash_tbl_key_len_: {} b, hash_coll_tbl_key_len_: {} b"
                    "hint_len: {} b, hint_mem_len_B_: {}, flow_hash_capacity_: {} "
                    "flow_coll_capacity_: {}",
                    table_name_.c_str(), key_len_, data_len_, entire_data_len_,
                    hwkey_len_, hwdata_len_,
                    hash_tbl_key_len_, hash_coll_tbl_key_len_,
                    hint_len_, hint_mem_len_B_,
                    flow_hash_capacity_, flow_coll_capacity);

}

// ---------------------------------------------------------------------------
// Destructor - Flow
// ---------------------------------------------------------------------------
Flow::~Flow()
{
    // delete flow_coll_indexer_;
    // delete flow_entry_indexer_;
    indexer::destroy(flow_coll_indexer_);
    indexer::destroy(flow_entry_indexer_);
}

// ---------------------------------------------------------------------------
// pre_process_sizes_
//
// The sizes for Flow Table and Flow Collision table may not be power of 2.
// To get the key lengths, they have to be power of 2. If they are not
// we go for the lower power of 2 to get key lengths.
// ---------------------------------------------------------------------------
void
Flow::pre_process_sizes_(uint32_t flow_hash_capacity,
                         uint32_t flow_coll_capacity)
{
    // Normalize Flow entries to power of 2
    hash_tbl_key_len_ = get_num_bits_from_size_(flow_hash_capacity);
    if (flow_hash_capacity & (flow_hash_capacity-1)) { // power of 2 ?
        flow_hash_capacity_ = flow_hash_capacity;
    } else {
        flow_hash_capacity_ = pow(2, hash_tbl_key_len_);
    }

    // Normalize Flow Hash Collision entries to power of 2
    hash_coll_tbl_key_len_ =
        get_num_bits_from_size_(flow_coll_capacity);
    if (flow_coll_capacity & (flow_coll_capacity-1)) {
        flow_coll_capacity_ = flow_coll_capacity;
    } else {
        flow_coll_capacity_ = pow(2, hash_coll_tbl_key_len_);
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
Flow::get_num_bits_from_size_(uint32_t size)
{
    uint32_t i = 0;

    while (size > 1) {
        size = size >> 1;
        i++;
    }
    return i;
}

uint32_t
Flow::calc_hash_(void *key, void *data)
{
    hal_ret_t                       rs = HAL_RET_OK;
    FlowEntry                       *entry = NULL;
    uint32_t                        hash_val = 0;
    void                            *hwkey = NULL;

    // create a flow entry
    // entry = new FlowEntry(key, key_len_, data, data_len_, hwkey_len_, false);
    entry = FlowEntry::factory(key, key_len_, data, data_len_,
                               hwkey_len_, false);

    // call P4 API to get hw key
    // hwkey = ::operator new(hwkey_len_);
    // memset(hwkey, 0, hwkey_len_);
    hwkey = HAL_CALLOC(HAL_MEM_ALLOC_FLOW_HW_KEY, hwkey_len_);

    rs = entry->form_hw_key(table_id_, hwkey);
    if (rs != HAL_RET_OK) HAL_ASSERT(0);

    // cal. hash
    hash_val = generate_hash_(hwkey, hwkey_len_, false);
    // ::operator delete(hwkey);
    HAL_FREE(HAL_MEM_ALLOC_FLOW_HW_KEY, hwkey);

    // delete entry;
    FlowEntry::destroy(entry);

    return hash_val;
}

// ---------------------------------------------------------------------------
// Insert
// ---------------------------------------------------------------------------
hal_ret_t
Flow::insert(void *key, void *data, uint32_t *index)
{
    hal_ret_t                       rs = HAL_RET_OK, rs1 = HAL_RET_OK;
    FlowEntry                       *entry = NULL;
    FlowTableEntry                  *ft_entry = NULL;
    uint32_t                        hash_val = 0;
    uint32_t                        ft_bits = 0, fe_idx = 0;
    void                            *hwkey = NULL;
    FlowTableEntryMap::iterator     itr;

    rs = alloc_flow_entry_index_(&fe_idx);
    if (rs != HAL_RET_OK) goto end;

    HAL_TRACE_DEBUG("Insert flow_entry_pi_idx: {} for tbl_id:{}",
                    fe_idx, table_id_);

    // create a flow entry
    // entry = new FlowEntry(key, key_len_, data, data_len_, hwkey_len_, true);
    entry = FlowEntry::factory(key, key_len_, data, data_len_,
                               hwkey_len_, true);

    // call P4 API to get hw key
    // hwkey = ::operator new(hwkey_len_);
    // memset(hwkey, 0, hwkey_len_);
    hwkey = HAL_CALLOC(HAL_MEM_ALLOC_FLOW_HW_KEY, hwkey_len_);

    rs = entry->form_hw_key(table_id_, hwkey);
    if (rs != HAL_RET_OK) goto end;

    // cal. hash
    hash_val = generate_hash_(hwkey, hwkey_len_);
    // ::operator delete(hwkey);
    HAL_FREE(HAL_MEM_ALLOC_FLOW_HW_KEY, hwkey);

    entry->set_hash_val(hash_val);

    // check if flow table entry exists
    ft_bits = fetch_flow_table_bits_(hash_val);
    itr = flow_table_.find(ft_bits);
    HAL_TRACE_DEBUG("hash_val: {:#x}, flow_table_index: {:#x}",
                    hash_val, ft_bits);
    if (itr != flow_table_.end()) {
        // flow table entry already exists
        HAL_TRACE_DEBUG("FT Entry exist ...");
        ft_entry = itr->second;
        rs = ft_entry->insert(entry);
        // TODO: No need to send flow coll return status
        if (rs == HAL_RET_OK) {
            HAL_TRACE_DEBUG("Setting collision return code");
            rs = HAL_RET_FLOW_COLL;
        }

    } else {
        // flow table entry doesnt exist
        HAL_TRACE_DEBUG("New FT Entry ...");
        // ft_entry = new FlowTableEntry(ft_bits, this);
        ft_entry = FlowTableEntry::factory(ft_bits, this);
        rs = ft_entry->insert(entry);

        // If insert is SUCCESS, put ft_entry into the map
        if (rs == HAL_RET_OK) {
            flow_table_[ft_bits] = ft_entry;
        } else {
            // delete ft_entry;
            FlowTableEntry::destroy(ft_entry);
        }
    }

    if (rs == HAL_RET_OK || rs == HAL_RET_FLOW_COLL) {
        // insert into flow entry indexer map ... For retrieval
        flow_entry_map_[fe_idx] = entry;
        entry->set_global_index(fe_idx);
        *index = fe_idx;
    } else {
        // insert failed
        HAL_TRACE_DEBUG("Insert FAIL ...");

        // delete flow entry
        // delete entry;
        FlowEntry::destroy(entry);

        // free index alloced
        rs1 = free_flow_entry_index_(fe_idx);
        HAL_ASSERT(rs1 == HAL_RET_OK);
    }

end:

    // Uncomment for debugging
    // print_flow();
    //HAL_TRACE_DEBUG("ret:{}", rs);
    stats_update(INSERT, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Updates the entry. Returns error, if its not present
// ---------------------------------------------------------------------------
hal_ret_t
Flow::update(uint32_t index, void *data)
{
    hal_ret_t               rs   = HAL_RET_OK;
    FlowEntry               *f_entry      = NULL;
    FlowEntryMap::iterator  itr;

    HAL_TRACE_DEBUG("Update {} ...", index);
    // check if entry exists.
    itr = flow_entry_map_.find(index);
    if (itr != flow_entry_map_.end()) {
        // get the entry and call update on FlowEntry.
        f_entry = itr->second;
        rs = f_entry->update(data);
        // ideally this should not fail as there is no alloc. of resources
        HAL_ASSERT(rs == HAL_RET_OK);
    } else {
        // entry doesn't exist
        HAL_TRACE_DEBUG("Error: Not Present {} ...", index);
        rs = HAL_RET_ENTRY_NOT_FOUND;
    }

    //print_flow();
    stats_update(UPDATE, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Removes the entry. -- TODO: Revisit. Untested Code
// ---------------------------------------------------------------------------
hal_ret_t
Flow::remove(uint32_t index)
{
    hal_ret_t               rs = HAL_RET_OK;
    FlowEntry               *f_entry     = NULL;
    FlowTableEntry          *ft_entry    = NULL;
    FlowEntryMap::iterator  itr;

    // Check if entry exists.
    itr = flow_entry_map_.find(index);
    if (itr != flow_entry_map_.end()) {
        // Get the entry and call update on FlowEntry.
        f_entry = itr->second;

        // Store the Flow Table Entry before entry cleanup
        ft_entry = f_entry->get_flow_table_entry();

        // Call remove
        rs = ft_entry->remove(f_entry);
        // rs = f_entry->remove();

        if (rs == HAL_RET_OK) {

            // Free the flow entry
            // delete f_entry;
            FlowEntry::destroy(f_entry);
            // Remove it from Flow entry map.
            flow_entry_map_.erase(index);
            // Free the index in indexer
            free_flow_entry_index_(index);

            // Check if we have to remove the FT entry
            if (!ft_entry->get_num_flow_hgs() && // No HGs
                    !ft_entry->get_spine_entry()) { // No Spine Entries
                // Remove from FTE map
                flow_table_.erase(ft_entry->get_ft_bits());
                // Free up the Flow Table Entry.
                // delete ft_entry;
                FlowTableEntry::destroy(ft_entry);
            }

        }
    } else {
        // Entry doesn't exist
        rs = HAL_RET_ENTRY_NOT_FOUND;
    }

    //print_flow();
    stats_update(REMOVE, rs);
    return rs;
}


// ---------------------------------------------------------------------------
// Generate Hash from Key
// ---------------------------------------------------------------------------
#define HAL_INTERNAL_MCAST_CRC32_HASH_SEED 0x33335555
uint32_t
Flow::generate_hash_(void *key, uint32_t key_len, bool log)
{
    uint32_t hash_val = 0;

    if (log) {
        uint8_t *tmp = (uint8_t *)key;
        fmt::MemoryWriter buf;

        for (uint32_t i = 0; i < key_len; i++, tmp++) {
            buf.write("{:#x} ", (uint8_t)*tmp);
        }
        HAL_TRACE_DEBUG("Key:");
        HAL_TRACE_DEBUG("{}", buf.c_str());
    }

    switch(hash_poly_) {
    case HASH_POLY0:
        g_crc32_hash_poly0->reset();
        g_crc32_hash_poly0->process_bytes(key, key_len);
        hash_val = g_crc32_hash_poly0->checksum();
        break;

    case HASH_POLY1:
        g_crc32_hash_poly1->reset();
        g_crc32_hash_poly1->process_bytes(key, key_len);
        hash_val = g_crc32_hash_poly1->checksum();
        break;

    case HASH_POLY2:
        g_crc32_hash_poly2->reset();
        g_crc32_hash_poly2->process_bytes(key, key_len);
        hash_val = g_crc32_hash_poly2->checksum();
        break;

    case HASH_POLY3:
        g_crc32_hash_poly3->reset();
        g_crc32_hash_poly3->process_bytes(key, key_len);
        hash_val = g_crc32_hash_poly3->checksum();
        break;

    default:
        HAL_ASSERT_GOTO(0, end);
    }

end:

#if 0
    HAL_TRACE_DEBUG("flow_hash: {}, ft_capacity: {}",
                    hash_val, flow_hash_capacity_);
#endif
    return hash_val;
}


// ---------------------------------------------------------------------------
// Fetch Flow Table Bits
// ---------------------------------------------------------------------------
uint32_t
Flow::fetch_flow_table_bits_(uint32_t hash_val)
{
    // Get hash_tbl_key_len_ number of LSB bits
    return (hash_val & ((1 << hash_tbl_key_len_) - 1));
}

// ---------------------------------------------------------------------------
// Fetch Hint Bits
// ---------------------------------------------------------------------------
uint32_t
Flow::fetch_hint_bits_(uint32_t hash_val)
{
    // Get hash_tbl_key_len_ number of LSB bits
    return ((hash_val & ~((1 << hash_tbl_key_len_) - 1)) >> hash_tbl_key_len_);
}



// ---------------------------------------------------------------------------
// Get Delayed Delete Enable
// ---------------------------------------------------------------------------
bool
Flow::get_delayed_del_en()
{
    return enable_delayed_del_;
}

// ---------------------------------------------------------------------------
// Set Delayed Delete Enable
// ---------------------------------------------------------------------------
void
Flow::set_delayed_del_en(bool en)
{
    enable_delayed_del_ = en;
}


// ---------------------------------------------------------------------------
// Get number of hints per flow entry
// ---------------------------------------------------------------------------
uint32_t
Flow::get_num_hints_per_flow_entry()
{
    return num_hints_per_flow_entry_;
}


// ---------------------------------------------------------------------------
// Add Flow Entry to the Global Map
// ---------------------------------------------------------------------------
void
Flow::add_flow_entry_global_map(FlowEntry *fe, uint32_t index)
{
    flow_entry_map_[index] = fe;
}


// ---------------------------------------------------------------------------
// Get Flow Collision indexer
// ---------------------------------------------------------------------------
indexer *
Flow::get_flow_coll_indexer()
{
    return flow_coll_indexer_;
}


// ---------------------------------------------------------------------------
// Push into FE delayed delete queue
// ---------------------------------------------------------------------------
void
Flow::push_fe_delete_q(FlowEntry *fe)
{
    flow_entry_del_q_.push(fe);
}

// ---------------------------------------------------------------------------
// Push into FHG delayed delete queue
// ---------------------------------------------------------------------------
void
Flow::push_fhg_delete_q(FlowHintGroup *fhg)
{
    flow_hg_del_q_.push(fhg);
}


// ----------------------------------------------------------------------------
// Allocate a flow entry index
// ----------------------------------------------------------------------------
hal_ret_t
Flow::alloc_flow_entry_index_(uint32_t *idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    // Allocate an index in repl. table
    indexer::status irs = flow_entry_indexer_->alloc((uint32_t *)idx);
    if (irs != indexer::SUCCESS) {
        HAL_TRACE_DEBUG("Flow Entry Capacity reached: {}",
                        flow_entry_indexer_->get_size());
        return HAL_RET_NO_RESOURCE;
    }

    HAL_TRACE_DEBUG("Alloc Flow_entry_index: {}", *idx);
    return rs;
}

// ----------------------------------------------------------------------------
// Free a flow entry index
// ----------------------------------------------------------------------------
hal_ret_t
Flow::free_flow_entry_index_(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    indexer::status irs = flow_entry_indexer_->free((uint32_t)idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return HAL_RET_DUP_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Free Flow_entry_index: {}", idx);

     return rs;
}

// ---------------------------------------------------------------------------
// Alloc a free index in  Flow Collision indexer
// ---------------------------------------------------------------------------
hal_ret_t
Flow::alloc_fhct_index(uint32_t *idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    // Allocate an index in Coll. table
    indexer::status irs = flow_coll_indexer_->alloc(idx);
    if (irs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("alloc_coll_indexer: {}", *idx);

    return rs;
}

// ---------------------------------------------------------------------------
// Free up an index in  Flow Collision indexer
// ---------------------------------------------------------------------------
hal_ret_t
Flow::free_fhct_index(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    indexer::status irs = flow_coll_indexer_->free((uint32_t)idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return HAL_RET_DUP_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return HAL_RET_ERR;
    }

    HAL_TRACE_DEBUG("free_coll_indexer: {}", idx);
     return rs;
}

// ----------------------------------------------------------------------------
// flow action data offsets
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
#define FLOW_DATA_ACTION_ID_LEN 1
#define FLOW_DATA_ENTRY_VALID_LEN 1
#define FLOW_DATA_HASH_LEN 2
// #define FLOW_DATA_HINT_LEN 2
#define FLOW_DATA_MORE_HASHS_LEN 1
// #define FLOW_DATA_MORE_HINTS_LEN 2
hal_ret_t
Flow::flow_action_data_offsets (void *action_data,
                                uint8_t **action_id,
                                uint8_t **entry_valid,
                                void **data,
                                void **first_hash_hint,
                                uint8_t **more_hashs,
                                void **more_hints)
{
    hal_ret_t   ret = HAL_RET_OK;
    uint8_t     *seek = NULL;

    seek = (uint8_t *)action_data;

    *action_id       = seek;
    *entry_valid     = seek + FLOW_DATA_ACTION_ID_LEN;
    *data            = seek + FLOW_DATA_ACTION_ID_LEN +
                       FLOW_DATA_ENTRY_VALID_LEN;
    *first_hash_hint = (void *)(seek + FLOW_DATA_ACTION_ID_LEN +
                                     FLOW_DATA_ENTRY_VALID_LEN + data_len_);
    *more_hashs      = seek + FLOW_DATA_ACTION_ID_LEN +
                              FLOW_DATA_ENTRY_VALID_LEN +
                              data_len_ +
                              num_hints_per_flow_entry_ * (FLOW_DATA_HASH_LEN +
                                                           hint_mem_len_B_);
    *more_hints      = (void *)(seek + FLOW_DATA_ACTION_ID_LEN +
                                    FLOW_DATA_ENTRY_VALID_LEN + data_len_ +
                                    num_hints_per_flow_entry_ *
                                    (FLOW_DATA_HASH_LEN + hint_mem_len_B_) +
                                    FLOW_DATA_MORE_HASHS_LEN);

    return ret;
}



// ----------------------------------------------------------------------------
// Increment Stats
// ----------------------------------------------------------------------------
void
Flow::stats_incr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
Flow::stats_decr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
Flow::stats_update(Flow::api ap, hal_ret_t rs)
{
    switch (ap) {
        case INSERT:
            if(rs == HAL_RET_OK) stats_incr(STATS_INS_SUCCESS);
            else if (rs == HAL_RET_FLOW_COLL) stats_incr(STATS_INS_FLOW_COLL);
            else if (rs == HAL_RET_DUP_INS_FAIL) stats_incr(STATS_INS_FAIL_DUP_INS);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_INS_FAIL_HW);
            else if(rs == HAL_RET_NO_RESOURCE) stats_incr(STATS_INS_FAIL_NO_RES);
            else HAL_ASSERT(0);
            break;
        case UPDATE:
            if(rs == HAL_RET_OK) stats_incr(STATS_UPD_SUCCESS);
            else if(rs == HAL_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
            else HAL_ASSERT(0);
            break;
        case REMOVE:
            if (rs == HAL_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if (rs == HAL_RET_ENTRY_NOT_FOUND)
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if (rs == HAL_RET_HW_FAIL) stats_incr(STATS_REM_FAIL_HW);
            else HAL_ASSERT(0);
            break;
        default:
            HAL_ASSERT(0);
    }
}

// ----------------------------------------------------------------------------
// Number of entries in use.
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_entries_in_use(void)
{
    return flow_entry_map_.size();
}

// ----------------------------------------------------------------------------
// Number of oflow entries in use.
// ----------------------------------------------------------------------------
uint32_t
Flow::oflow_table_num_entries_in_use(void)
{
    return flow_coll_indexer_->num_indices_allocated();
}

// ----------------------------------------------------------------------------
// Number of insert operations attempted
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_inserts(void)
{
    return stats_[STATS_INS_SUCCESS] + stats_[STATS_INS_FLOW_COLL] +
        stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed insert operations
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_insert_errors(void)
{
    return stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of update operations attempted
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_updates(void)
{
    return stats_[STATS_UPD_SUCCESS] +
        stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND];
}

// ----------------------------------------------------------------------------
// Number of failed update operations
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_update_errors(void)
{
    return stats_[STATS_UPD_FAIL_ENTRY_NOT_FOUND];
}

// ----------------------------------------------------------------------------
// Number of delete operations attempted
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_deletes(void)
{
    return stats_[STATS_REM_SUCCESS] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed delete operations
// ----------------------------------------------------------------------------
uint32_t
Flow::table_num_delete_errors(void)
{
    return stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW];
}

// ---------------------------------------------------------------------------
// Print Flow Table
// ---------------------------------------------------------------------------
hal_ret_t
Flow::print_flow()
{
    hal_ret_t        ret = HAL_RET_OK;
    uint32_t         flow_bits = 0, fe_idx = 0;
    FlowTableEntry  *fte = NULL;
    FlowEntry       *fe = NULL;

    HAL_TRACE_DEBUG("Printing Flow Tables:");
    HAL_TRACE_DEBUG("-------- ---- -------");
    HAL_TRACE_DEBUG("Total Num_FTEs: {}", flow_table_.size());
    for (FlowTableEntryMap::const_iterator it = flow_table_.begin();
         it != flow_table_.end(); ++it) {
        flow_bits = it->first;
        fte = it->second;
        HAL_TRACE_DEBUG("flow_bits: {:#x}", flow_bits);
        fte->print_fte();
    }

    HAL_TRACE_DEBUG("Total Num_FEs: {}", flow_entry_map_.size());
    for (FlowEntryMap::const_iterator it = flow_entry_map_.begin();
         it != flow_entry_map_.end(); ++it) {
        fe_idx = it->first;
        fe = it->second;
        HAL_TRACE_DEBUG("  fe_idx: {:#x}", fe_idx);
        fe->print_fe();
    }

    HAL_ASSERT(flow_entry_map_.size() == flow_entry_indexer_->num_indices_allocated());

    return ret;
}

hal_ret_t
 Flow::entry_to_str(uint32_t gl_index, char *buff, uint32_t buff_size)
 {
     hal_ret_t ret = HAL_RET_OK;
     FlowEntryMap::iterator  itr;
     FlowEntry *f_entry = NULL;
     uint32_t ft_bits = 0, hint_bits = 0;
     char entry_buff[4096] = {0};
     char inter_spine_buff[2048] = {0};
     char inter_hg_buff[2048] = {0};
     char num_recircs_str[32] = {0};
     uint32_t num_recircs = 0;



     itr = flow_entry_map_.find(gl_index);
     if (itr != flow_entry_map_.end()) {
         f_entry = itr->second;
         ft_bits = fetch_flow_table_bits_(f_entry->get_hash_val());
         hint_bits = f_entry->get_fh_group()->get_hint_bits();

         // Assumption: if its anchor its only the first spine entry
         if (f_entry->get_is_anchor_entry()) {
             // Spine entry
             sprintf(inter_spine_buff, "Spine Entries(Anchor): %s:0x%x ", "FT",
                     f_entry->get_eff_spine_entry()->get_ft_entry()->get_ft_bits());
             f_entry->get_eff_spine_entry()->entry_to_str(entry_buff, sizeof(entry_buff));
         } else {
             // Collision table entry
             // - List of Spine entries and hint list entries
             strcat(inter_spine_buff, "Spine Entries: ");
             f_entry->get_eff_spine_entry()->get_ft_entry()->
                 inter_spine_str(f_entry->get_eff_spine_entry(),
                                 inter_spine_buff, sizeof(inter_spine_buff),
                                 &num_recircs);

             // - List of FHGs
             strcat(inter_hg_buff, "Hint List: ");
             f_entry->get_fh_group()->inter_hg_str(f_entry, inter_hg_buff, sizeof(inter_hg_buff),
                                                   entry_buff, sizeof(entry_buff),
                                                   &num_recircs);
         }

         sprintf(num_recircs_str, "#Recircs: %d", num_recircs);

         sprintf(buff, "Hash Value: 0x%x Hash Bits: 0x%x, Hint Bits: 0x%x, %s\n%s\n%s\n%s",
                 f_entry->get_hash_val(),
                 ft_bits, hint_bits, num_recircs_str,
                 inter_spine_buff, inter_hg_buff, entry_buff);
     }


     return ret;
 }

hal_ret_t
Flow::iterate(flow_iterate_func_t func, const void *cb_data)
{
    // FlowEntry               *f_entry      = NULL;
    uint32_t                gl_index = 0;

    for (FlowEntryMap::iterator  itr = flow_entry_map_.begin();
         itr != flow_entry_map_.end(); itr++) {
        gl_index = itr->first;
        // f_entry = itr->second;
        func(gl_index, cb_data);
    }

    return HAL_RET_OK;
}
