//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Overlay Bridge Domain store
//---------------------------------------------------------------

#include "pdsa_bd_store.hpp"

namespace pdsa_stub {
template<> sdk::lib::slab* slab_obj_t<bd_obj_t>::slab_ = nullptr;
}

