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
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/pal/include/pal_mem.h"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/lib/utils/time_profile.hpp"

#ifdef USE_ARM64_SIMD
#include <arm_neon.h>
#endif

#ifndef __FTL_INCLUDES_HPP__
#define __FTL_INCLUDES_HPP__

#include "ftlaf.hpp"

namespace sdk {
namespace table {

class FTL_AFPFX();

namespace ftlint {
class indexer;
class apistats;
class tablestats;
}

namespace FTL_MAKE_AFTYPE(internal) {

class FTL_MAKE_AFTYPE(apictx);
class FTL_MAKE_AFTYPE(base_table);
class FTL_MAKE_AFTYPE(main_table);
class FTL_MAKE_AFTYPE(hint_table);
class FTL_MAKE_AFTYPE(bucket);

} // namespace FTL_MAKE_AFTYPE(int)
} // namespace table
} // namespace sdk

using namespace std;
using namespace sdk::lib;
using namespace sdk::platform::capri;

using sdk::table::FTL_AFPFX();
using sdk::table::FTL_MAKE_AFTYPE(internal)::FTL_MAKE_AFTYPE(main_table);
using sdk::table::FTL_MAKE_AFTYPE(internal)::FTL_MAKE_AFTYPE(hint_table);
using sdk::table::FTL_MAKE_AFTYPE(internal)::FTL_MAKE_AFTYPE(base_table);
using sdk::table::FTL_MAKE_AFTYPE(internal)::FTL_MAKE_AFTYPE(apictx);
using sdk::table::FTL_MAKE_AFTYPE(internal)::FTL_MAKE_AFTYPE(bucket);

using sdk::table::ftlint::apistats;
using sdk::table::ftlint::tablestats;
using ftlindexer = sdk::table::ftlint::indexer;

using sdk::table::sdk_table_factory_params_t;
using sdk::table::sdk_table_api_params_t;

#include "ftl_utils.hpp"
// #include "ftl_structs.hpp"
#include "ftl_indexer.hpp"
#include "ftl_stats.hpp"
#include "ftl_table.hpp"
#include "ftl_apictx.hpp"
#include "ftl_bucket.hpp"
#include "ftl_platform.hpp"

#endif
