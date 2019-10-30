//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA stub Interface Store
//---------------------------------------------------------------

#include "pdsa_if_store.hpp"

namespace pdsa_stub {
template<> sdk::lib::slab* slab_obj_t<if_obj_t>::slab_ = nullptr;
}

