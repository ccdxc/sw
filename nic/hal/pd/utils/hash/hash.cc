#include <cstring>

#include "nic/hal/pd/utils/hash/hash_entry.hpp"
#include "nic/hal/pd/utils/hash/hash.hpp"
#include "nic/hal/pd/utils/tcam/tcam.hpp"
#include "nic/hal/pd/p4pd_api.hpp"
#include "nic/include/trace.hpp"

using hal::pd::utils::HashEntry;
using hal::pd::utils::Hash;

// ---------------------------------------------------------------------------
// Constructor - Hash
// ---------------------------------------------------------------------------
Hash::Hash(std::string table_name, 
           uint32_t dleft_table_id, uint32_t otcam_table_id,
           uint32_t dleft_capacity, uint32_t otcam_capacity,
           uint32_t swkey_len, uint32_t swdata_len,
           Hash::HashPoly hash_poly)
{
    table_name_         = table_name;
    table_id_           = dleft_table_id;
    dleft_capacity_     = dleft_capacity;
    swkey_len_          = swkey_len;
    swdata_len_         = swdata_len;
    hash_poly_          = hash_poly;

    
    // Initialize the Overflow Tcam
    otcam_ = NULL;
    if (otcam_capacity) {
        otcam_ = new Tcam(table_name + "_otcam", otcam_table_id, otcam_capacity,
                swkey_len_, swdata_len_);
        if (!otcam_) {
            HAL_ASSERT_RETURN_VOID(FALSE);
        }
    }

    p4pd_hwentry_query(table_id_, &hwkey_len_, NULL, &hwdata_len_);
    // TODO: Remove once P4-PD APIs are implemented
    // hwkey_len_ = swkey_len;
    // hwdata_len_ = swdata_len;
    uint32_t hwkeylen_bits = hwkey_len_;
    uint32_t hwdatalen_bits = hwdata_len_;
    hwkey_len_ = (hwkey_len_ >> 3) + ((hwkey_len_ & 0x7) ? 1 : 0);
    hwdata_len_ = (hwdata_len_ >> 3) + ((hwdata_len_ & 0x7) ? 1 : 0);
    HAL_TRACE_DEBUG("Hash::{:<30s}: dleft_tid: {:<3d} otcam_tid: {:<3d} swkey_len: {:<4d} "
                    "hwkey_len_: bits:{:<4d} bytes:{:<4d} "
                    "hwdata_len_: bits:{:<4d} bytes:{:<4d}",
                    table_name.c_str(), dleft_table_id, otcam_table_id, swkey_len_, 
                    hwkeylen_bits, hwkey_len_, hwdatalen_bits, hwdata_len_);

    // Initialize Stats
    stats_ = new uint64_t[STATS_MAX]();
}

// ---------------------------------------------------------------------------
// Destructor - Hash
// ---------------------------------------------------------------------------
Hash::~Hash() 
{
    // Freeing up OTcam
    if (otcam_) {
        delete otcam_;
    }
    delete[] stats_;
}

// ---------------------------------------------------------------------------
// Insert
//
// Return Code:
//      HAL_RET_OK              : Successfull
//      HAL_RET_DUP_INS_FAIL    : Duplicate Insert
//      HAL_RET_NO_RESOURCE     : Capacity reached
//
// ---------------------------------------------------------------------------
hal_ret_t 
Hash::insert(void *key, void *data, uint32_t *index, 
             bool direct_to_otcam)
{
    hal_ret_t rs            = HAL_RET_OK;
    p4pd_error_t pd_err     = P4PD_SUCCESS;
    HashEntry *he           = NULL;
    void *hwkey             = NULL;
    uint32_t dleft_index    = 0;
    uint32_t tcam_index     = 0;
    HashEntryMap::iterator itr;


    // build hw keys & masks
    // TODO: Uncomment once P4-PD APIs are implemented
    hwkey = ::operator new(hwkey_len_);
    memset(hwkey, 0, hwkey_len_);
    pd_err = p4pd_hwkey_hwmask_build(table_id_, key, NULL,
                                     (uint8_t *)hwkey, NULL);
    // HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);
    if (pd_err != P4PD_SUCCESS) {
        rs = HAL_RET_HW_FAIL;
        goto end;
    }
    // TODO: Remove once P4-PD APIs are implemented
    // hwkey = key;

    dleft_index = generate_hash_(hwkey, hwkey_len_);

    itr = hash_entry_map_.find(dleft_index);
    if (itr == hash_entry_map_.end()) {
        HAL_TRACE_DEBUG("Hash::{}: dleft Insert ", __FUNCTION__);
        he = new HashEntry(key, swkey_len_, data, swdata_len_, dleft_index);
        *index = form_hash_idx_from_dleft_id_(dleft_index);

        // program hw
        rs = program_table_(he, hwkey);
        
        if (rs == HAL_RET_OK) {
            // insert in sw DS
            hash_entry_map_[dleft_index] = he;
            stats_incr(STATS_NUM_HASH);
        } else {
            delete he;
        }

    } else {
        HAL_TRACE_DEBUG("Hash::{}: otcam Insert ", __FUNCTION__);
        he = itr->second;
        if (!std::memcmp(he->get_key(), key, swkey_len_)) {
            rs = HAL_RET_DUP_INS_FAIL;
            goto end;
        }

        if (has_otcam_()) {
            // initialize mask
            void *key_mask = ::operator new(swkey_len_);
            memset(key_mask, ~0, swkey_len_); 
            
            // otcam insert
            rs = otcam_->insert(key, key_mask, data, &tcam_index);
            ::operator delete(key_mask);
            if (rs == HAL_RET_OK) {
                *index = form_hash_idx_from_otcam_id_(tcam_index);
                stats_incr(STATS_NUM_TCAM);
            }
        } else {
            // No OTCAM
            rs = HAL_RET_NO_RESOURCE;
        }
    }

end:
    if (hwkey) {
        // TODO: Uncomment once P4-PD APIs are implemented
        ::operator delete(hwkey); 
    }

    // return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
    stats_update(INSERT, rs);
    return rs;
}

// ---------------------------------------------------------------------------
// Update
//
// Return Code:
//      HAL_RET_OK              : Successfull
//      HAL_RET_OOB             : Out of Bound Index
//      HAL_RET_ENTRY_NOT_FOUND : Entry not found
//      HAL_RET_INVALID_ARG     : Invalid index
//      HAL_RET_HW_FAIL         : Hw API failed
//
// ---------------------------------------------------------------------------
hal_ret_t 
Hash::update(uint32_t hash_idx, void *data)
{
    hal_ret_t rs            = HAL_RET_OK;
    p4pd_error_t pd_err     = P4PD_SUCCESS;
    void *hwkey             = NULL;
    uint32_t dleft_id       = 0;
    uint32_t otcam_id       = 0;
    HashEntryMap::iterator itr;

    if (is_dleft(hash_idx)) {
        dleft_id = get_dleft_id_from_hash_idx_(hash_idx);
        if (dleft_id >= dleft_capacity_) {
            rs = HAL_RET_OOB;
            goto end;
        }

        // check if entry exists
        itr = hash_entry_map_.find(dleft_id);
        if (itr == hash_entry_map_.end()) {
            rs = HAL_RET_ENTRY_NOT_FOUND;
            goto end;
        }

        // even if update to hw fails sw will be updated.
        itr->second->update_data(data);

        // build hw key & mask
        hwkey = ::operator new(hwkey_len_);
        memset(hwkey, 0, hwkey_len_);
        pd_err = p4pd_hwkey_hwmask_build(table_id_, itr->second->get_key(), 
                                         NULL, (uint8_t *)hwkey, NULL);
        // HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);
        if (pd_err != P4PD_SUCCESS) {
            rs = HAL_RET_HW_FAIL;
            HAL_ASSERT(0);
            goto end;
        }

        rs = program_table_(itr->second, hwkey);

    } else {
        if (otcam_) {
            otcam_id = get_otcam_id_from_hash_idx_(hash_idx);
            rs = otcam_->update(otcam_id, data);
        } else {
            // Invalid hash idx
            rs = HAL_RET_INVALID_ARG;
        }
    }

end:
   
    if (hwkey) ::operator delete(hwkey);
    // return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : rs;
    stats_update(UPDATE, rs);
    return rs;

}

// ---------------------------------------------------------------------------
// Remove
//
// Return Code:
//      HAL_RET_OK              : Successfull
//      HAL_RET_OOB             : Out of Bound Index
//      HAL_RET_ENTRY_NOT_FOUND : Entry not found
//      HAL_RET_INVALID_ARG     : Invalid index
//      HAL_RET_HW_FAIL         : Hw API failed
//
// ---------------------------------------------------------------------------
hal_ret_t 
Hash::remove(uint32_t hash_idx)
{
    hal_ret_t rs = HAL_RET_OK;
    HashEntryMap::iterator itr;
    uint32_t    dleft_id = -1;
    uint32_t    otcam_id = -1;

    if (is_dleft(hash_idx)) {
        // entry is hash
        dleft_id = get_dleft_id_from_hash_idx_(hash_idx);
        if (dleft_id >= dleft_capacity_) {
            rs = HAL_RET_OOB;
            goto end;
        }

        // check if entry exists
        itr = hash_entry_map_.find(dleft_id);
        if (itr == hash_entry_map_.end()) {
            rs = HAL_RET_ENTRY_NOT_FOUND;
            goto end;
        }

        // deprogram hw
        rs = deprogram_table_(itr->second);

        if (rs == HAL_RET_OK) {
            // free & remove from sw DS
            delete itr->second;
            hash_entry_map_.erase(itr);
        }

    } else {
        // entry is tcam
        if (has_otcam_()) {
            otcam_id = get_otcam_id_from_hash_idx_(hash_idx);
            rs = otcam_->remove(otcam_id);
        } else {
            rs = HAL_RET_INVALID_ARG;
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
//      HAL_RET_OK              : Successfull
//      HAL_RET_OOB             : Out of Bound Index
//      HAL_RET_ENTRY_NOT_FOUND : Entry not found
//      HAL_RET_INVALID_ARG     : Invalid index
//
// ---------------------------------------------------------------------------
hal_ret_t
Hash::retrieve(uint32_t hash_idx, void *key, void *data)
{
    hal_ret_t rs = HAL_RET_OK;
    HashEntryMap::iterator itr;
    HashEntry *he = NULL;
    uint32_t dleft_id = -1;
    uint32_t otcam_id = -1;

    if (is_dleft(hash_idx)) {
        // entry is hash
        dleft_id = get_dleft_id_from_hash_idx_(hash_idx);
        if (dleft_id >= dleft_capacity_) {
            rs = HAL_RET_OOB;
            goto end;
        }

        // check if entry exists
        itr = hash_entry_map_.find(dleft_id);
        if (itr == hash_entry_map_.end()) {
            rs = HAL_RET_ENTRY_NOT_FOUND;
            goto end;
        }

        he = itr->second;
        if (key) {
            memcpy(key, he->get_key(), he->get_key_len());
        }
        if (data) {
            memcpy(data, he->get_data(), he->get_data_len());
        }
    } else {
        // entry is tcam
        if (otcam_) {
            otcam_id = get_otcam_id_from_hash_idx_(hash_idx);
            rs = otcam_->retrieve(otcam_id, key, NULL, data);
        } else {
            rs = HAL_RET_INVALID_ARG;
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
Hash::otcam_iterate_(const void *key, const void *key_mask,
                     const void *data, 
                     uint32_t tcam_idx, const void *cb_data) 
{
    otcam_iterate_cb_t *otcam_cb_data = (otcam_iterate_cb_t *)cb_data;
    otcam_cb_data->cb(key, data, 
                     form_hash_idx_from_otcam_id_(tcam_idx), otcam_cb_data->cb_data); 

    return TRUE;

}


// ---------------------------------------------------------------------------
// Hash Iterate
// ---------------------------------------------------------------------------
hal_ret_t
Hash::iterate(hash_iterate_func_t cb, const void *cb_data, 
              Hash::EntryType type)
{
    hal_ret_t rs = HAL_RET_OK;
    HashEntryMap::iterator itr;
    HashEntry *he = NULL;


    if (type == Hash::DLEFT || type == Hash::BOTH) {

        for (itr = hash_entry_map_.begin(); itr != hash_entry_map_.end();
                ++itr) {
            he = itr->second;
            cb(he->get_key(), he->get_data(), 
                    form_hash_idx_from_dleft_id_(itr->first),
                    cb_data);
        }

    }
    if (type == Hash::OTCAM || type == Hash::BOTH) {
        if (otcam_) {
            otcam_iterate_cb_t otcam_cb;
            otcam_cb.cb = cb;
            otcam_cb.cb_data = cb_data;

            otcam_->iterate(Hash::otcam_iterate_, (const void *)&otcam_cb); 
        }
    }

    return rs;
}


// ---------------------------------------------------------------------------
// Checks if Hash Table has Overflow TCAM
// ---------------------------------------------------------------------------
bool
Hash::has_otcam_()
{
    return (otcam_ != NULL);
}

// ---------------------------------------------------------------------------
// Forms Hash Idx from Dleft Id
// ---------------------------------------------------------------------------
uint32_t 
Hash::form_hash_idx_from_dleft_id_(uint32_t dleft_id)
{
    return dleft_id;
}

// ---------------------------------------------------------------------------
// Forms Hash Idx from OTCAM Id
// ---------------------------------------------------------------------------
uint32_t 
Hash::form_hash_idx_from_otcam_id_(uint32_t otcam_id)
{
    return ((uint32_t)(1 << otcam_bit_) | otcam_id);
}

// ---------------------------------------------------------------------------
// Get Dleft Id from Hash Idx
// ---------------------------------------------------------------------------
uint32_t
Hash::get_dleft_id_from_hash_idx_(uint32_t hash_idx)
{
    return hash_idx;
}

// ---------------------------------------------------------------------------
// Get OTCAM Id from Hash Idx
// ---------------------------------------------------------------------------
uint32_t
Hash::get_otcam_id_from_hash_idx_(uint32_t hash_idx) 
{
    return (hash_idx & ~(uint32_t)(1 << otcam_bit_));
}

// ---------------------------------------------------------------------------
// Is Hash Idx Dleft or OTCAM
// ---------------------------------------------------------------------------
bool
Hash::is_dleft(uint32_t hash_idx)
{
    // For OTCAM, otcam_bit_ is set
    return ((hash_idx & (uint32_t)(1 << otcam_bit_)) ? FALSE : TRUE);
}


// ---------------------------------------------------------------------------
// Generate Hash from Key
// ---------------------------------------------------------------------------
#define HAL_INTERNAL_MCAST_CRC32_HASH_SEED 0x33335555
uint32_t
Hash::generate_hash_(void *key, uint32_t key_len)
{
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
    HAL_TRACE_DEBUG("HashVal: {}, Idx: {}\n", hash_val, hash_val % dleft_capacity_);
	return hash_val % dleft_capacity_;
}

// ----------------------------------------------------------------------------
// Program HW table
// ----------------------------------------------------------------------------
hal_ret_t
Hash::program_table_(HashEntry *he, void *hwkey) 
{
    p4pd_error_t pd_err = P4PD_SUCCESS;

    if (!he || !hwkey) {
        return HAL_RET_INVALID_ARG;
    }

    // Entry trace
    entry_trace_(he);

    // P4-API: Wrihe 
    pd_err = p4pd_entry_write(table_id_, he->get_index(), (uint8_t *)hwkey, 
                              NULL, he->get_data());
    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// DeProgram HW table
// ----------------------------------------------------------------------------
hal_ret_t
Hash::deprogram_table_(HashEntry *he) 
{
    p4pd_error_t pd_err = P4PD_SUCCESS;
    void *hwkey         = NULL;

    if (!he) {
        return HAL_RET_INVALID_ARG;
    }

    // Build Hw Keys/KeyMasks
    hwkey       = ::operator new(hwkey_len_);

    std::memset(hwkey, 0, hwkey_len_);
    std::memset(he->get_data(), 0, swdata_len_);

    // P4-API: Write 
    pd_err = p4pd_entry_write(table_id_, he->get_index(), (uint8_t *)hwkey, 
                              NULL, he->get_data());
    HAL_ASSERT_GOTO((pd_err == P4PD_SUCCESS), end);

end:
    ::operator delete(hwkey);

    return (pd_err != P4PD_SUCCESS) ? HAL_RET_HW_FAIL : HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Increment Stats
// ----------------------------------------------------------------------------
void
Hash::stats_incr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]++;
}

// ----------------------------------------------------------------------------
// Decrement Stats
// ----------------------------------------------------------------------------
void
Hash::stats_decr(stats stat)
{
    HAL_ASSERT_RETURN_VOID((stat < STATS_MAX));
    stats_[stat]--;
}

// ----------------------------------------------------------------------------
// Update stats
// ----------------------------------------------------------------------------
void
Hash::stats_update(Hash::api ap, hal_ret_t rs)
{
    switch (ap) {
        case INSERT:
            if(rs == HAL_RET_OK) stats_incr(STATS_INS_SUCCESS);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_INS_FAIL_HW);
            else if(rs == HAL_RET_NO_RESOURCE) stats_incr(STATS_INS_FAIL_NO_RES);
            else if(rs == HAL_RET_DUP_INS_FAIL) stats_incr(STATS_INS_FAIL_DUP_INS);
            else HAL_ASSERT(0);
            break;
        case UPDATE:
            if(rs == HAL_RET_OK) stats_incr(STATS_UPD_SUCCESS);
            else if(rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_UPD_FAIL_ENTRY_NOT_FOUND);
            else if(rs == HAL_RET_INVALID_ARG) stats_incr(STATS_UPD_FAIL_INV_ARG);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_UPD_FAIL_HW);
            else if(rs == HAL_RET_OOB) stats_incr(STATS_UPD_FAIL_OOB);
            else HAL_ASSERT(0);
            break;
        case REMOVE:
            if(rs == HAL_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if(rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if(rs == HAL_RET_INVALID_ARG) stats_incr(STATS_REM_FAIL_INV_ARG);
            else if(rs == HAL_RET_HW_FAIL) stats_incr(STATS_REM_FAIL_HW);
            else if(rs == HAL_RET_OOB) stats_incr(STATS_REM_FAIL_OOB);
            else HAL_ASSERT(0);
            break;
        case RETRIEVE:
            if(rs == HAL_RET_OK) stats_incr(STATS_REM_SUCCESS);
            else if(rs == HAL_RET_ENTRY_NOT_FOUND) 
                stats_incr(STATS_REM_FAIL_ENTRY_NOT_FOUND);
            else if(rs == HAL_RET_INVALID_ARG) stats_incr(STATS_REM_FAIL_INV_ARG);
            else if(rs == HAL_RET_OOB) stats_incr(STATS_REM_FAIL_OOB);
            else HAL_ASSERT(0);
            break;
        default:
            HAL_ASSERT(0);
    }
}

// ----------------------------------------------------------------------------
// Oflow tcam capacity
// ----------------------------------------------------------------------------
uint32_t 
Hash::oflow_table_capacity(void) 
{ 
    return otcam_ ? otcam_->table_capacity() : 0; 
} 

// ----------------------------------------------------------------------------
// Number of entries in use.
// ----------------------------------------------------------------------------
uint32_t
Hash::table_num_entries_in_use(void)
{
    return hash_entry_map_.size();
}

// ----------------------------------------------------------------------------
// Number of oflow entries in use.
// ----------------------------------------------------------------------------
uint32_t
Hash::oflow_table_num_entries_in_use(void)
{
    return otcam_ ? otcam_->table_num_entries_in_use() : 0;
}

// ----------------------------------------------------------------------------
// Number of insert operations attempted
// ----------------------------------------------------------------------------
uint32_t 
Hash::table_num_inserts(void)
{
    return stats_[STATS_INS_SUCCESS] + stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of failed insert operations
// ----------------------------------------------------------------------------
uint32_t 
Hash::table_num_insert_errors(void)
{
    return stats_[STATS_INS_FAIL_DUP_INS] +
        stats_[STATS_INS_FAIL_NO_RES] + stats_[STATS_INS_FAIL_HW];
}

// ----------------------------------------------------------------------------
// Number of delete operations attempted
// ----------------------------------------------------------------------------
uint32_t 
Hash::table_num_deletes(void)
{
    return stats_[STATS_REM_SUCCESS] + stats_[STATS_REM_FAIL_OOB] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW] +
        stats_[STATS_REM_FAIL_INV_ARG];
}

// ----------------------------------------------------------------------------
// Number of failed delete operations
// ----------------------------------------------------------------------------
uint32_t 
Hash::table_num_delete_errors(void)
{
    return stats_[STATS_REM_FAIL_OOB] +
        stats_[STATS_REM_FAIL_ENTRY_NOT_FOUND] + stats_[STATS_REM_FAIL_HW] +
        stats_[STATS_REM_FAIL_INV_ARG];
}

// ----------------------------------------------------------------------------
// Print entry
// ----------------------------------------------------------------------------
hal_ret_t
Hash::entry_trace_(HashEntry *he)
{
    char            buff[4096] = {0};
    p4pd_error_t    p4_err;

    p4_err = p4pd_table_ds_decoded_string_get(table_id_,
            he->get_key(), NULL, he->get_data(), buff, sizeof(buff));
    HAL_ASSERT(p4_err == P4PD_SUCCESS);

    HAL_TRACE_DEBUG("Index: {} \n {}", he->get_index(), buff);

    return HAL_RET_OK;
}
