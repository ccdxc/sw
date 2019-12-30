//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Base for all PDS-MS objects
//---------------------------------------------------------------

#ifndef __PDS_MS_SLAB_OBJECT_HPP__
#define __PDS_MS_SLAB_OBJECT_HPP__

#include "nic/sdk/lib/slab/slab.hpp"
#include <new>
#include <memory>

// NBASE mempool for comparison
// nbb_mm_cb_data
// * operator new(size, usage = NBB_NORETRY_ACT) {
//      Control block type mempool
//      nbb_mem_alloc(size, usage, traits<CLASS>::type ); 
//}
// NBB_DEFINE_MEM_TRAITS(l2f::MacEntry, MEM_L2F_MAC_ENTRY);
//  class VlanIrbInterface : public IrbInterface,
//                           public nbb::Allocatable<VlanIrbInterface>
// NBB_DEFINE_MEM_TRAITS(lim::VlanSubinterface, MEM_LIM_VLAN_SUBINTERFACE);
//
//
// HAL mempool
// new_slab = slab::factory to create new slab
// new_slab->alloc
//

namespace pds_ms {

//----------------------------------------------------------------
// All PDS-MS stub objects that need to be allocated from the Slab 
// needs to derive from this. set_slab() needs to be called for
// each of them at init time.
//--------------------------------------------------------------- 
template <typename DERIVED_OBJ>
class slab_obj_t
{
public :
    void* operator new(size_t size) {
        SDK_ASSERT (slab()->elem_sz() >= size);
        auto ptr = slab()->alloc(); 
        if (ptr == nullptr) {throw std::bad_alloc();}
        return ptr;
    }

    void operator delete(void* ptr) {
        slab()->free(ptr); 
    }

    static sdk::lib::slab* slab(void) {return slab_;}
    static void set_slab(sdk::lib::slab* slb) {slab_ = slb;}
private:
    static sdk::lib::slab* slab_;
};

class slab_destroy_t {
public:
    void operator()(sdk::lib::slab* ptr) {
        ptr->destroy (ptr);
    }
};

// Unique pointer to entire slab
using slab_uptr_t = std::unique_ptr<sdk::lib::slab, slab_destroy_t>;
}

#endif
