//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Base for all PDSA objects
//---------------------------------------------------------------

#ifndef __PDSA_SLAB_OBJECT_HPP__
#define __PDSA_SLAB_OBJECT_HPP__

// Somewhere in slab.hpp __FNAME__ is used that result in build failure 
#define __FNAME__ __FUNCTION__

#include "sdk/lib/slab/slab.hpp"
#include <new>

// NBASE mempool
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

namespace pdsa_stub {

class slab_destroy_t {
public:
    void operator()(sdk::lib::slab* ptr) {
#if 0 // TODO: Need HAL slab library linkage
        ptr->destroy (ptr);
#endif
    }
};

//-----------------------------------------------------------------------------------------
// All PDSA stub objects that need to be allocated from the Slab needs to derive from this.  
// They should also prove a slab() static method to obtain the slab pointer for the object
//----------------------------------------------------------------------------------------- 
template <typename DERIVED_OBJ>
class slab_obj_t
{
public :
    void* operator new(size_t size) {
        SDK_ASSERT (slab()->elem_sz() > size);
        auto ptr = slab()->alloc(); 
        if (ptr == nullptr) {throw std::bad_alloc();}
        return ptr;
    }

    void operator delete(void* ptr) {
#if 0 // TODO: Requires SDK library linkage
        slab()->free(ptr); 
#else
        ptr= ptr;
#endif
    }

    static sdk::lib::slab* slab(void) {return slab_;}
    static void set_slab(sdk::lib::slab* slb) {slab_ = slb;}

private:
    static sdk::lib::slab* slab_;
};

}

#endif
