//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// key-value store base API implementation
///
//----------------------------------------------------------------------------

#include "lib/kvstore/kvstore.hpp"
#include "lib/kvstore/kvstore_lmdb.hpp"

namespace sdk {
namespace lib {

kvstore *
kvstore::factory(std::string dbpath, size_t size) {
    return kvstore_lmdb::factory(dbpath, size);
}

void
kvstore::destroy(kvstore *kvs) {
    kvstore_lmdb::destroy(kvs);
}

}    // namespace lib
}    // namespace sdk

