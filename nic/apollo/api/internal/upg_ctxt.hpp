//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines upgrade context and meta data for backup and restore
///
//----------------------------------------------------------------------------

#ifndef __API_INTERNAL_UPGRADE_HPP__
#define __API_INTERNAL_UPGRADE_HPP__

#include "nic/sdk/include/sdk/base.hpp"

// total size of the below should not exceed PDS_UPG_SHM_SIZE
// api objects
#define PDS_UPGRADE_API_OBJ_STORE_NAME "pds_upg_api_objs_info"
#define PDS_UPGRADE_API_OBJ_STORE_SIZE (50 * 1024)
// nicmgr objects
#define PDS_UPGRADE_NICMGR_OBJ_STORE_NAME "pds_upg_nicmgr_objs_info"
#define PDS_UPGRADE_NICMGR_OBJ_STORE_SIZE (20 * 1024)

namespace api {

/// \brief upgrade objs meta data. saved in the front of persistent storage.
// ...WARNING.. this structure should be preserved across upgrades upgrades.
// so don't change the order. contains the information about the object and
// gives the offset where the objects are stored and also the number of objects
typedef struct upg_obj_stash_meta_s {
    uint32_t        obj_id;       ///< upgrade obj id
    uint32_t        obj_count;    ///< number of objects saved for this object
    uint32_t        offset;       ///< pointer to the first object
    uint64_t        pad;          ///< for future use
} __PACK__ upg_obj_stash_meta_t;

/// \brief upgrade backup obj info. This is specifically used for backup
/// contains info about the size left in persistent storage for each obj,
/// total size of stashed objs per class and number of objs stashed
typedef struct upg_backup_info_s {
    uint32_t size_left;             ///< size left in persistent storage per obj
    uint32_t total_size;            ///< total size of stashed objs per class
    uint32_t stashed_obj_count;     ///< number of objs stashed
} upg_backup_info_t;

/// \brief upgrade obj info. will be used per class during backup/restore
/// to point/fetch data in/from persistent storage. 
//  it helps objs to navigate to its correct position inside persistent storage
typedef struct upg_obj_info_s {
    char *mem;                      ///< reference into persistent storage
    uint32_t obj_id ;               ///< obj id
    size_t   size;                  ///< bytes written/read
    upg_backup_info_t backup;       ///< backup specific data
} upg_obj_info_t;

/// \brief obj tlv. abstraction used to read/write from/at specified location
/// in persistent storage
// ...WARNING.. this structure should be preserved across upgrades
typedef struct upg_obj_tlv_s {
    uint32_t len;         ///< length of read/write from/to persistent stoarge
    char obj[0];          ///< location in persistent stoarge for read/write
} upg_obj_tlv_t;

class upg_ctxt {
public:
    /// \brief factory method to initialize
    static upg_ctxt *factory(void);

    /// \brief destroy the upg ctxt
    static void destroy(upg_ctxt *);

    /// \brief get obj mem offset
    uint32_t obj_offset(void) const { return obj_offset_; }

    /// \brief increment offset for obj mem by given value
    void incr_obj_offset(uint32_t val) { obj_offset_ += val; }

    /// \brief set offset for obj mem to given value
    void set_obj_offset(uint32_t offset) { obj_offset_ = offset; }

    /// \brief get obj mem size
    uint32_t obj_size(void) const { return obj_size_; };

    /// \brief get mapped memory start reference
    char *mem(void) const { return mem_; }

    /// \brief get segment from shared memory
    sdk_ret_t init(const char *obj_store_name, size_t obj_store_size,
                   bool obj_store_create);

private:
    /// \brief constructor
    upg_ctxt(){};
    /// \brief destructor
    virtual ~upg_ctxt(){};

private:
    char                *mem_;              ///< mapped memory start
    uint32_t            obj_offset_;        ///< object memory offset
    uint32_t            obj_size_;          ///< max object memory size
};

}    // namespace api
#endif    // __API_INTERNAL_UPGRADE_HPP__
