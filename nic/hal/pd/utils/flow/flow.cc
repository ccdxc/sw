#include <cmath>
#include "p4pd_api.hpp"
#include "flow.hpp"
#include "flow_entry.hpp"
#include "flow_table_entry.hpp"

using hal::pd::utils::Flow;

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
           Flow::HashPoly hash_poly)
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

    pre_process_sizes_(flow_hash_capacity, flow_coll_capacity);

    hint_len_ = 32/*CRC 32*/ - hash_tbl_key_len_;

    HAL_TRACE_DEBUG("Flow:{}: key_len_: {}, data_len_: {}, "
            " hash_tbl_key_len_: {}, hash_coll_tbl_key_len_: {} "
            "hint_len: {} flow_hash_capacity_: {} flow_coll_capacity_: {}", 
            table_name_.c_str(), key_len_, data_len_, 
            hash_tbl_key_len_, hash_coll_tbl_key_len_, 
            hint_len_, flow_hash_capacity_, flow_coll_capacity);

    // Allocate indexer for Flow Collision Table
    flow_coll_indexer_ = new indexer(flow_coll_capacity_);

    // Assumption: Max. number of flow entries will be hash table cap.
    flow_entry_indexer_ = new indexer(flow_hash_capacity_);

    // Assumption: Delayed Delete is disabled.
    enable_delayed_del_ = FALSE;

    p4pd_hwentry_query(table_id_, &hwkey_len_, NULL, &hwdata_len_);

    hwkey_len_ = (hwkey_len_ >> 3) + ((hwkey_len_ & 0x7) ? 1 : 0);
    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);

    HAL_TRACE_DEBUG("Flow:{}: hwkey_len: {}, hwdata_len: {}",
            __FUNCTION__, hwkey_len_, hwdata_len_);

	// TODO: Remove this once p4pd apis are implemented ... just to gen diff
    //       hash values
	// hwkey_len_ = key_len_;
	// hwdata_len_ = data_len_;

	// TODO: Remove this once p4 apis are implemented. Setting the seed
	// srand(time(0));
	// hwkey_is_swkey_ = (rand() %< 2 == 0);
    hwkey_is_swkey_ = 0;

	HAL_TRACE_DEBUG("Flow:{}: hwkey_is_swkey_: {}", 
			__FUNCTION__, hwkey_is_swkey_);
}
           
// ---------------------------------------------------------------------------
// Destructor - Flow
// ---------------------------------------------------------------------------
Flow::~Flow()
{
    delete flow_coll_indexer_;
    delete flow_entry_indexer_;
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

    HAL_TRACE_DEBUG("Flow::{}: Insert {} ", __FUNCTION__, fe_idx);

    // create a flow entry
    entry = new FlowEntry(key, key_len_, data, data_len_);

    // call P4 API to get hw key
    hwkey = ::operator new(hwkey_len_);
	memset(hwkey, 0, hwkey_len_);
	// TODO: Remove this after P4 API is implemented.
	if (hwkey_is_swkey_) {
		memcpy(hwkey, key, key_len_);
	}

    rs = entry->form_hw_key(table_id_, hwkey);
	if (rs != HAL_RET_OK) goto end;

    // cal. hash
    hash_val = generate_hash_(hwkey, hwkey_len_);
    ::operator delete(hwkey);

    entry->set_hash_val(hash_val);

    // check if flow table entry exists
    ft_bits = fetch_flow_table_bits_(hash_val);
    itr = flow_table_.find(ft_bits);
    HAL_TRACE_DEBUG("Flow::{}: hash_val: {}, ft_bits: {}", 
                    __FUNCTION__, hash_val, ft_bits);
    if (itr != flow_table_.end()) {
        // flow table entry already exists
        HAL_TRACE_DEBUG("Flow::{}: FT Entry exist ...", __FUNCTION__);
        ft_entry = itr->second;
        rs = ft_entry->insert(entry);

    } else {
        // flow table entry doesnt exist
        HAL_TRACE_DEBUG("Flow::{}: New FT Entry ...", __FUNCTION__);
        ft_entry = new FlowTableEntry(ft_bits, this);
        rs = ft_entry->insert(entry);

        // If insert is SUCCESS, put ft_entry into the map
        if (rs == HAL_RET_OK) {
            flow_table_[ft_bits] = ft_entry;
        } else {
            delete ft_entry;
        }
    }

    if (rs == HAL_RET_OK) {
        HAL_TRACE_DEBUG("Flow::{}: Insert SUCCESS ...", __FUNCTION__);

        // insert into flow entry indexer map ... For retrieval
        flow_entry_map_[fe_idx] = entry;
        entry->set_global_index(fe_idx);
        *index = fe_idx;
    } else {
        // insert failed
        HAL_TRACE_DEBUG("Flow::{}: Insert FAIL ...", __FUNCTION__);

        // delete flow entry
        delete entry;

        // free index alloced
		rs1 = free_flow_entry_index_(fe_idx);
		HAL_ASSERT(rs1 == HAL_RET_OK);
    }
end:
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

    HAL_TRACE_DEBUG("Flow::{}: Update {} ...", __FUNCTION__, index);
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
        HAL_TRACE_DEBUG("Flow::{}: Error: Not Present {} ...", __FUNCTION__, index);
        rs = HAL_RET_ENTRY_NOT_FOUND;
    }

    return rs;
}

// ---------------------------------------------------------------------------
// Removes the entry. -- TODO: Revisit. Untested Code
// ---------------------------------------------------------------------------
hal_ret_t
Flow::remove(uint32_t index)
{
    hal_ret_t               rs = HAL_RET_OK;
    FlowEntry               *f_entry          = NULL;
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
            delete f_entry;
            // Remove it from Flow entry map.
            flow_entry_map_.erase(index);
            // Free the index in indexer
            flow_entry_indexer_->free(index);

            // Check if we have to remove the FT entry
            if (!ft_entry->get_num_flow_hgs() && // No HGs
                    !ft_entry->get_spine_entry()) { // No Spine Entries
                // Free up the Flow Table Entry.
                delete ft_entry;
            }

        }
    } else {
        // Entry doesn't exist
        rs = HAL_RET_ENTRY_NOT_FOUND;
    }

    return rs;
}


// ---------------------------------------------------------------------------
// Generate Hash from Key
// ---------------------------------------------------------------------------
#define HAL_INTERNAL_MCAST_CRC32_HASH_SEED 0x33335555
uint32_t
Flow::generate_hash_(void *key, uint32_t key_len)
{
    // TODO - Replace this with whatever hardware implements
    // return crc32((uint32_t)HAL_INTERNAL_MCAST_CRC32_HASH_SEED, (const void *)key, 
    //         (uint32_t)key_len) % flow_hash_capacity_;
	uint32_t hash_val = 0;
	uint32_t crc_init_val = 0x00000000;
	boost::crc_basic<32> *crc_hash;
    // TODO - Replace this with whatever hardware implements
    // return crc32((uint32_t)HAL_INTERNAL_MCAST_CRC32_HASH_SEED, (const void *)key, 
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
			HAL_ASSERT_GOTO(0, end);
	}

end:
	delete crc_hash;
    HAL_TRACE_DEBUG("Flow::{}: flow_hash: {}, ft_capacity: {}", 
                    __FUNCTION__, hash_val, flow_hash_capacity_);
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
    indexer::status irs = flow_entry_indexer_->alloc(idx);
    if (irs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    return rs;
}

// ----------------------------------------------------------------------------
// Free a flow entry index
// ----------------------------------------------------------------------------
hal_ret_t
Flow::free_flow_entry_index_(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    indexer::status irs = flow_entry_indexer_->free(idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return HAL_RET_DUP_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return HAL_RET_ERR;
    }

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
    HAL_TRACE_DEBUG("Flow::{}: alloc_coll_indexer: {}", 
                    __FUNCTION__, *idx);

    return rs;
}

// ---------------------------------------------------------------------------
// Free up an index in  Flow Collision indexer
// ---------------------------------------------------------------------------
hal_ret_t
Flow::free_fhct_index(uint32_t idx)
{
    hal_ret_t   rs = HAL_RET_OK;

    indexer::status irs = flow_coll_indexer_->free(idx);
    if (irs == indexer::DUPLICATE_FREE) {
        return HAL_RET_DUP_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return HAL_RET_ERR;
    }

    HAL_TRACE_DEBUG("Flow::{}: free_coll_indexer: {}", 
                    __FUNCTION__, idx);
     return rs;
}

