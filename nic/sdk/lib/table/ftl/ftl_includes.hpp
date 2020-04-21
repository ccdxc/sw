//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <cmath>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "include/sdk/base.hpp"
#include "include/sdk/lock.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/common/table.hpp"
#include "lib/utils/crc_fast.hpp"
#include "lib/p4/p4_api.hpp"
#include "asic/pd/pd.hpp"
#include "lib/pal/pal.hpp"
#include "platform/pal/include/pal_mem.h"
#include "include/sdk/platform.hpp"
#ifdef ELBA
#include "platform/elba/elba_tbl_rw.hpp"
#else
#include "platform/capri/capri_tbl_rw.hpp"
#endif
#include "lib/utils/time_profile.hpp"

#ifdef USE_ARM64_SIMD
#include <arm_neon.h>
#endif

#ifndef __FTL_INCLUDES_HPP__
#define __FTL_INCLUDES_HPP__

namespace sdk {
namespace table {

class ftl_base;

namespace ftlint {
class indexer;
class apistats;
class tablestats;
}

namespace internal {

class Apictx;
class base_table;
class main_table;
class hint_table;
class Bucket;

} // namespace internal
} // namespace table
} // namespace sdk

using namespace std;
using namespace sdk::lib;
#ifdef ELBA
using namespace sdk::platform::elba;
#else
using namespace sdk::platform::capri;
#endif

using sdk::table::ftl_base;
using sdk::table::internal::main_table;
using sdk::table::internal::hint_table;
using sdk::table::internal::base_table;
using sdk::table::internal::Apictx;
using sdk::table::internal::Bucket;

using sdk::table::ftlint::apistats;
using sdk::table::ftlint::tablestats;
using ftlindexer = sdk::table::ftlint::indexer;

using sdk::table::sdk_table_factory_params_t;
using sdk::table::sdk_table_api_params_t;

#include "ftl_utils.hpp"
#include "ftl_indexer.hpp"
#include "ftl_stats.hpp"
#include "ftl_table.hpp"
#include "ftl_apictx.hpp"
#include "ftl_bucket.hpp"
#include "ftl_platform.hpp"

#endif
