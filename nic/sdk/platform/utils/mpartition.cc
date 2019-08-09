// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <cinttypes>
#include "include/sdk/mem.hpp"
#include "platform/utils/mpartition.hpp"
#include "platform/utils/mpart_rsvd.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace pt = boost::property_tree;

namespace sdk {
namespace platform {
namespace utils {

#define JKEY_REGIONS        "regions"
#define JKEY_REGION_NAME    "name"
#define JKEY_SIZE           "size"
#define JKEY_BLKSIZE        "block_size"
#define JKEY_MAX_ELEMENTS   "max_elements"
#define JKEY_CACHE_PIPE     "cache"
#define JKEY_RESET_REGION   "reset"
#define JKEY_BASE_REGION    "base_region"

mpartition *mpartition::instance_ = nullptr;

static uint64_t
extract_size (std::string str)
{
    char *             endptr = NULL;
    unsigned long long val    = strtoull(str.c_str(), &endptr, 10);
    int                off    = 0;
    if (endptr) {
        switch (*endptr) {
        case 'G':
            off = 30;
            break;
        case 'M':
            off = 20;
            break;
        case 'K':
            off = 10;
            break;
       case 'B':
            break;
        default:
            SDK_ASSERT(0);
        }
    }
    return val << off;
}

static void
insert_common_regions (mpartition_region_t *reg, const char *name,
                       uint64_t offset, uint32_t size, bool reset)
{
    reg->cache_pipe = MEM_REGION_CACHE_PIPE_NONE;
    strcpy(reg->mem_reg_name, name);
    reg->start_offset = offset;
    reg->size         = size;
    reg->reset        = reset;
    reg->block_size   = -1;
    reg->max_elements = -1;

    SDK_TRACE_DEBUG(
        "region : %s, size : %u, block size : %u, max elems : %u, "
        "reset : %u, cpipe : %s, start : 0x%" PRIx64 ", end : 0x%" PRIx64 "",
        reg->mem_reg_name, reg->size, reg->block_size, reg->max_elements,
        reg->reset, "none", MREGION_BASE_ADDR + offset,
        MREGION_BASE_ADDR + offset + reg->size);
}

sdk_ret_t
mpartition::region_init(const char *mpart_json_file, shmmgr *mmgr)
{
    pt::ptree json_pt;
    std::string full_path;
    mpartition_region_t *reg;
    uint64_t offset = 0ULL;

    std::ifstream mpartition_json(mpart_json_file);
    // Read the region info from the jSON
    read_json(mpartition_json, json_pt);
    boost::optional<pt::ptree &> reg_pt =
        json_pt.get_child_optional(JKEY_REGIONS);
    if (!reg_pt) {
        return SDK_RET_ERR;
    }
    num_regions_ = json_pt.get_child(JKEY_REGIONS).size();
    if (num_regions_ == 0) {
        return SDK_RET_ERR;
    }
    // TODO. We should call a asic function and get the address
    base_addr_ = MREGION_BASE_ADDR;
    mmgr_ = mmgr;
    num_regions_ += MREGIONS_RSVD_COUNT;
    reg = regions_ = (mpartition_region_t *)SDK_CALLOC(
        SDK_MEM_ALLOC_ID_PLATFORM_MEMORY,
        num_regions_ * sizeof(mpartition_region_t));
    if (!regions_) {
        return SDK_RET_OOM;
    }

    BOOST_FOREACH (pt::ptree::value_type &p4_tbl,
                   json_pt.get_child(JKEY_REGIONS)) {
        std::string reg_name = p4_tbl.second.get<std::string>(JKEY_REGION_NAME);
        if (reg_name == "reserved_common") {
            uint32_t size = extract_size(p4_tbl.second.get<std::string>(JKEY_SIZE));
            // Assert if the current offset excceeds the start offset
            SDK_ASSERT_TRACE_RETURN((offset <= MREGION_RSVD_START_OFFSET),
                                    SDK_RET_ERR, "Invalid MEM Config");
            offset = MREGION_RSVD_START_OFFSET;
            insert_common_regions(reg, MREGION_PCIEMGR_NAME,
                                  MREGION_PCIEMGR_START_OFFSET,
                                  MREGION_PCIEMGR_SIZE, 0);
            reg++;
            insert_common_regions(reg, MREGION_MPU_TRACE_NAME,
                                  MREGION_MPU_TRACE_START_OFFSET,
                                  MREGION_MPU_TRACE_SIZE, 0);
            reg++;
            insert_common_regions(reg, MREGION_MEMTUN_NAME,
                                  MREGION_MEMTUN_START_OFFSET,
                                  MREGION_MEMTUN_SIZE, 0);
            reg++;
            // This region is not used, reduce the count
            num_regions_--;
            offset += size;
            SDK_ASSERT_TRACE_RETURN((offset >= MREGION_RSVD_END_OFFSET),
                                    SDK_RET_ERR, "Reserved region overflow");
            continue;
        }
        std::string cache_pipe_name =
            p4_tbl.second.get<std::string>(JKEY_CACHE_PIPE, "null");
        reg->reset = p4_tbl.second.get<bool>(JKEY_RESET_REGION, false);

        if (cache_pipe_name == "p4ig") {
            reg->cache_pipe = MEM_REGION_CACHE_PIPE_P4IG;
        } else if (cache_pipe_name == "p4eg") {
            reg->cache_pipe = MEM_REGION_CACHE_PIPE_P4EG;
        } else if (cache_pipe_name == "p4plus-txdma") {
            reg->cache_pipe = MEM_REGION_CACHE_PIPE_P4PLUS_TXDMA;
        } else if (cache_pipe_name == "p4plus-rxdma") {
            reg->cache_pipe = MEM_REGION_CACHE_PIPE_P4PLUS_RXDMA;
        } else if (cache_pipe_name == "p4plus-all") {
            reg->cache_pipe = MEM_REGION_CACHE_PIPE_P4PLUS_ALL;
        } else {
            reg->cache_pipe = MEM_REGION_CACHE_PIPE_NONE;
        }
        strcpy(reg->mem_reg_name, reg_name.c_str());
        reg->size = extract_size(p4_tbl.second.get<std::string>(JKEY_SIZE));
        reg->block_size =
            extract_size(p4_tbl.second.get<std::string>(JKEY_BLKSIZE,
                                                        "4294967295B")); // -1 default
        reg->max_elements =
            extract_size(p4_tbl.second.get<std::string>(JKEY_MAX_ELEMENTS,
                                                        "4294967295B")); // -1 default

        // If this region is aliased to some other base region, then dont allocate memory
        // for this region, instead find that region's start offset and set for this region
        std::string base_region =
            p4_tbl.second.get<std::string>(JKEY_BASE_REGION, "null");
        if (base_region != "null") {
            mpartition_region_t *base = region(base_region.c_str());
            if (base == NULL) {
                SDK_TRACE_ERR("Unable to find base region %s for the region %s", base_region, reg->mem_reg_name);
                return SDK_RET_ERR;
            }
            reg->is_alias = true;
            reg->start_offset = base->start_offset;
        } else {
            reg->start_offset = offset;
            offset += reg->size;
        }

        SDK_TRACE_DEBUG("region : %s, size : %u, block size : %u, "
                        "max elements : %u, reset : %u, cpipe : %s, "
                        "start : 0x%" PRIx64 ", end : 0x%" PRIx64 "",
                        reg->mem_reg_name, reg->size, reg->block_size,
                        reg->max_elements, reg->reset, cache_pipe_name.c_str(),
                        addr(reg->start_offset),
                        addr(reg->start_offset + reg->size));
        reg++;
    }
    SDK_TRACE_DEBUG("Region Memory Usage %uM,%uK", offset >> 20,
                    (offset - ((offset >> 20) << 20)) >> 10);
    return SDK_RET_OK;
}

mpartition *
mpartition::init(const char *mpart_json_file, shmmgr *mmgr)
{
    void          *mem;
    mpartition    *new_mpartition;

    if (mmgr) {
        mem = mmgr->alloc(sizeof(mpartition), 4, true);
    } else {
        mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_PLATFORM, sizeof(mpartition));
    }
    if (mem == NULL) {
        SDK_TRACE_ERR("Failed to create mpartition instance");
        return NULL;
    }
    new_mpartition = new (mem) mpartition();
    if (new_mpartition->region_init(mpart_json_file, mmgr) != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to initialize program info");
        new_mpartition->~mpartition();
        if (mmgr) {
            mmgr->free(mem);
        } else {
            SDK_FREE(SDK_MEM_ALLOC_LIB_PLATFORM, mem);
        }
        return NULL;
    }
    return new_mpartition;
}

mpartition *
mpartition::factory(const char *mpart_json_file, shmmgr *mmgr)
{
    if (instance_ == nullptr) {
        instance_ = mpartition::init(mpart_json_file, mmgr);
    }
    return instance_;
}

mpartition *
mpartition::get_instance(void)
{
    // Factory should be called by the init/main functions.
    return instance_;
}

mpartition::~mpartition() {
    SDK_FREE(SDK_MEM_ALLOC_ID_PLATFORM_MEMORY, regions_);
}

void
mpartition::destroy(mpartition *mpartition)
{
    shmmgr    *mmgr;
    if (!mpartition) {
        return;
    }
    mmgr = mpartition->mmgr_;
    mpartition->~mpartition();
    if (mmgr) {
        mmgr->free(mpartition);
    } else {
        SDK_FREE(SDK_MEM_ALLOC_LIB_PLATFORM, mpartition);
    }
    instance_ = nullptr;
}

mem_addr_t
mpartition::start_offset(const char *name)
{
    mpartition_region_t *reg = region(name);
    return reg ? reg->start_offset : INVALID_MEM_ADDRESS;
}

mem_addr_t
mpartition::start_addr(const char *name)
{
    mem_addr_t start_offset = this->start_offset(name);
    if (start_offset == INVALID_MEM_ADDRESS) {
        return start_offset;
    }
    return addr(start_offset);
}

uint32_t
mpartition::size(const char *name)
{
    mpartition_region_t *reg = region(name);
    return reg ? reg->size : 0;
}

uint32_t
mpartition::block_size(const char *name)
{
    mpartition_region_t *reg = region(name);
    return reg ? reg->block_size : 0;
}

uint32_t
mpartition::max_elements(const char *name)
{
    mpartition_region_t *reg = region(name);
    return reg ? reg->max_elements : 0;
}

mpartition_region_t *
mpartition::region(const char *name)
{
    mpartition_region_t      *reg;

    for (int i = 0; i < num_regions_; i++) {
        reg = &regions_[i];
        if (!std::strcmp(reg->mem_reg_name, name)) {
            return reg;
        }
    }
    return NULL;
}

mpartition_region_t *
mpartition::region_by_address(uint64_t addr)
{
    mpartition_region_t      *reg;

    for (int i = 0; i < num_regions_; i++) {
        reg = &regions_[i];
        if ((addr >= this->addr(reg->start_offset)) &&
            (addr < this->addr(reg->start_offset + (reg->size)))) {
            return reg;
        }
    }
    return NULL;
}

}   // namespace utils
}   // namespace platform
}   // namespace sdk

