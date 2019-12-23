// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <cinttypes>
#include <sys/stat.h>
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
#define JKEY_REGION_KIND    "kind"

#define JKEYVAL_REGION_KIND_STATIC          "static"
#define JKEYVAL_REGION_KIND_UPGRADE         "upgrade"
#define JKEYVAL_REGION_KIND_OPERTBL         "opertbl"
#define JKEYVAL_REGION_KIND_CFGTBL          "cfgtbl"

#define MPART_INFO_FILE_NAME "mem_regions_info.json"

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

static inline std::string
region_kind_to_str (mpartition_region_kind_t kind)
{
    switch(kind) {
    case MEM_REGION_KIND_STATIC : return JKEYVAL_REGION_KIND_STATIC;
    case MEM_REGION_KIND_UPGRADE : return JKEYVAL_REGION_KIND_UPGRADE;
    case MEM_REGION_KIND_OPERTBL : return JKEYVAL_REGION_KIND_OPERTBL;
    default: return JKEYVAL_REGION_KIND_CFGTBL;
    }
}

sdk_ret_t
mpartition::dump_regions_info (void) {
    std::string         gen_dir = std::string(getenv("HAL_CONFIG_PATH")) + "/gen";
    pt::ptree           root, regions, entry;
    mpartition_region_t *reg;
    char                numbuf[64];
    struct stat         st = { 0 };

    // check if the gen dir exists
    if (stat(gen_dir.c_str(), &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(gen_dir.c_str(), 0755) < 0) {
            SDK_TRACE_ERR("Gen directory %s/ doesn't exist, failed to create one\n",
                          gen_dir.c_str());
            return SDK_RET_ERR;
        }
    }

    // write usage
    for (int i = 0; i < num_regions_; i++) {
        reg = &regions_[i];
        entry.put("name", reg->mem_reg_name);
        snprintf(numbuf, sizeof(numbuf), "0x%lx", reg->start_offset);
        entry.put("offset", numbuf);
        snprintf(numbuf, sizeof(numbuf), "0x%lx", reg->size);
        entry.put("size", numbuf);
        entry.put("kind", region_kind_to_str(reg->kind).c_str());
        regions.push_back(std::make_pair("", entry));
        entry.clear();

    }
    root.add_child("regions", regions);
    pt::write_json(gen_dir + "/" + MPART_INFO_FILE_NAME, root);
    return SDK_RET_OK;
}

// This function offsets the config/oper table regions to addresses
// not used by the currently running process.
// Upgrade manager sends NEW request to process A(to be upgraded) which
// will dump its memory configuration.
// The below function reads that find out the free area based on its
// memory configuration(hbm_mem.json) and update its tables.
// It also validates the static offsets which should be consistent
// across upgrades.
sdk_ret_t
mpartition::upg_regions (const char *regions_fname, bool oper_table_persist) {
    std::string         file = std::string(regions_fname);
    pt::ptree           json_pt;
    mpartition_region_t *mreg;
    std::vector<std::pair <uint64_t, uint64_t> > free_mem; // start address, end address
    uint64_t offset, eoffset, size;

    std::ifstream region_json(file);
    // Read the region info from the jSON
    read_json(region_json, json_pt);
    boost::optional<pt::ptree &> reg_pt =
        json_pt.get_child_optional("regions");
    if (!reg_pt) {
        return SDK_RET_ERR;
    }

    // initialize free memory
    free_mem.push_back(std::make_pair(regions_[0].start_offset,
                                       regions_[num_regions_ - 1].start_offset +
                                       regions_[num_regions_ - 1].size - 1));
    SDK_TRACE_DEBUG("Memory config startoffset %lx endoffset %lx",
                    free_mem[0].first, free_mem[0].second);

    BOOST_FOREACH (pt::ptree::value_type &reg,
                   json_pt.get_child("regions")) {
        std::string reg_name = reg.second.get<std::string>("name");
        std::string reg_kind = reg.second.get<std::string>("kind");
        offset = strtoull((reg.second.get<std::string>("offset")).c_str(), NULL, 16);
        size = strtoull((reg.second.get<std::string>("size")).c_str(), NULL, 16);
        eoffset = offset + size - 1;

        // remove inuse memory from free list by reading running mem region info
        if (reg_kind != JKEYVAL_REGION_KIND_UPGRADE) {
            SDK_TRACE_DEBUG("Inuse memory name %s, startoffset %lx, size %lx",
                            reg_name.c_str(), offset, size);
            for (uint32_t j = 0; j < free_mem.size();  j++) {
                if (free_mem[j].first <= offset && free_mem[j].second >= eoffset) {
                    if (free_mem[j].first == offset && free_mem[j].second == eoffset) { // exact match
                        free_mem.erase(free_mem.begin() + j);
                    } else {
                        if (free_mem[j].first == offset) { // front match
                            free_mem[j].first = eoffset + 1;
                        } else if (free_mem[j].second == eoffset) { // tail match
                            free_mem[j].second = offset - 1;
                        } else { // split
                            free_mem.insert(free_mem.begin() + 1, std::make_pair(eoffset + 1, free_mem[j].second));
                            free_mem[j].second = offset - 1; // update existing one
                        }
                    }
                    break;
                }
            }
        }

        // static entries in the existing should present in new
        // and should match in offset and size
        if (reg_kind == JKEYVAL_REGION_KIND_STATIC) {
            mreg = region(reg_name.c_str());
            SDK_ASSERT(mreg != NULL);
            SDK_ASSERT(mreg->start_offset == offset);
            SDK_ASSERT(mreg->size == size);
            mreg->upgrade_check_done = true;
        } else if (reg_kind == JKEYVAL_REGION_KIND_OPERTBL) {
            // if no oper table copy, oper table in the existing should
            // present in new. preserve the offset and size
            if (oper_table_persist) {
                mreg = region(reg_name.c_str());
                SDK_ASSERT(mreg != NULL);
                mreg->start_offset = offset;
                mreg->size = size;
                mreg->upgrade_check_done = true;
            }
        }
    }

    for (uint32_t j = 0; j < free_mem.size();  j++) {
        SDK_TRACE_DEBUG("Free memory index %u, startoffset %lx, endoffset %lx",
                        j, free_mem[j].first, free_mem[j].second);
    }

    // assign offsets and size for tables from free memory
    for (int i = 0; i < num_regions_; i++) {
        offset = INVALID_MEM_ADDRESS;
        mreg = &regions_[i];
        if (mreg->upgrade_check_done) {
            continue;
        }
        if (mreg->kind == MEM_REGION_KIND_UPGRADE) {
            continue;
        }
        if (is_region_an_alias(mreg)) {
            offset = regions_[mreg->alias_index].start_offset;
        } else {
            for (uint32_t j = 0; j < free_mem.size();  j++) {
                size = free_mem[j].second - free_mem[j].first + 1;
                if (size >= mreg->size) {
                    offset = free_mem[j].first;
                    if (size > mreg->size) {
                        free_mem[j].first += mreg->size;
                    } else {
                        free_mem.erase(free_mem.begin() + j);
                    }
                    break;
                }
            }
        }
        SDK_TRACE_DEBUG("Setting region %s offset from  0x%" PRIx64 " to  0x%" PRIx64 "",
                        mreg->mem_reg_name, mreg->start_offset, offset);
        mreg->start_offset = offset;
        SDK_ASSERT(mreg->start_offset != INVALID_MEM_ADDRESS);
        mreg->upgrade_check_done = true;
    }

    return SDK_RET_OK;
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
    reg->kind = MEM_REGION_KIND_STATIC;

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
        std::string kind = p4_tbl.second.get<std::string>(JKEY_REGION_KIND, "null");

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

        if (kind == JKEYVAL_REGION_KIND_STATIC) {
            reg->kind = MEM_REGION_KIND_STATIC;
        } else if (kind == JKEYVAL_REGION_KIND_UPGRADE) {
            reg->kind = MEM_REGION_KIND_UPGRADE;
        } else if (kind == JKEYVAL_REGION_KIND_OPERTBL) {
            reg->kind = MEM_REGION_KIND_OPERTBL;
        } else {
            reg->kind = MEM_REGION_KIND_CFGTBL;
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
            reg->alias_index = base - &regions_[0] ;
            reg->start_offset = base->start_offset;
        } else {
            reg->alias_index = -1;
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

uint64_t
mpartition::size(const char *name)
{
    mpartition_region_t *reg = region(name);
    return reg ? reg->size : 0;
}

uint64_t
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

