//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA global states
//--------------------------------------------------------------

#ifndef __PDSA_STATE_HPP__
#define __PDSA_STATE_HPP__

// Somewhere in slab.hpp __FNAME__ is used that result in build failure 
#define __FNAME__ __FUNCTION__

#include "pdsa_util.hpp"
#include "pdsa_error.hpp"
#include "pdsa_tep_store.hpp"
#include "pdsa_bd_store.hpp"
#include "pdsa_if_store.hpp"
#include "sdk/lib/slab/slab.hpp"
#include <memory>

namespace pdsa_stub {

using pdsa_slab_id_t = uint32_t;

enum {
    PDSA_TEP_SLAB_ID = 1,
    PDSA_IF_SLAB_ID,
    PDSA_BD_SLAB_ID,
    PDSA_MAX_SLAB_ID
};

// Singleton that holds all global state for the PDSA stubs
class state_t {
public:
    static void create(void) { 
        SDK_ASSERT (g_state == nullptr);
        g_state = new state_t;
    }
    static void destroy(void) {delete(g_state); g_state = nullptr;}
    static state_t* state(void) {return g_state;}

public:
    tep_store_t& tep_store(void) {return tep_store_;}
    bd_store_t&  bd_store(void) {return bd_store_;}
    if_store_t&  if_store(void) {return if_store_;}

private:
    // Unique ptr helps to uninitialize cleanly in case of initialization errors
    using slab_uptr_t = std::unique_ptr<sdk::lib::slab, slab_destroy_t>;
    slab_uptr_t slabs_[PDSA_MAX_SLAB_ID];

    tep_store_t tep_store_;
    bd_store_t  bd_store_;
    if_store_t  if_store_;
    lif_store_t lif_store_;

    static state_t* g_state;

private:
    state_t(void);
};

}

#endif
