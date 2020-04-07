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

namespace api {

/// \brief upgrade objs meta data. saved in the front of persistent storage
typedef struct upg_obj_stash_meta_s {
    uint32_t        obj_id;       ///< upgrade obj id
    uint32_t        obj_count;    ///< number of objects saved for this object
    uint32_t        offset;       ///< pointer to the first object
} __PACK__ upg_obj_stash_meta_t;

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
    sdk_ret_t init(bool create);

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
