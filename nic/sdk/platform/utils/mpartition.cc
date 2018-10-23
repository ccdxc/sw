#include <inttypes.h>
#include "include/sdk/platform/utils/mpartition.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "include/sdk/mem.hpp"

namespace pt = boost::property_tree;

namespace sdk {
namespace platform {
namespace utils {

sdk_ret_t
mpartition::init(const char *mpartition_json_file, mem_addr_t base_addr,
                 shmmgr *mmgr)
{
    pt::ptree           json_pt;
    std::string         full_path;
    mpartition_region_t *reg;

    base_addr_ = base_addr;
    mmgr_ = mmgr;

    std::ifstream mpartition_json(mpartition_json_file);

    // Reading from file
    read_json(mpartition_json, json_pt);

    boost::optional<pt::ptree&>reg_pt = json_pt.get_child_optional(JKEY_REGIONS);
    if (!reg_pt) {
        return SDK_RET_ERR;
    }

    num_regions_ = json_pt.get_child(JKEY_REGIONS).size();
    regions_ = (mpartition_region_t *)
        SDK_CALLOC(SDK_MEM_ALLOC_ID_PLATFORM_MEMORY, num_regions_ * sizeof(mpartition_region_t));
    if (!regions_) {
        return SDK_RET_OOM;
    }

    int idx = 0;
    uint64_t offset = 0;
    BOOST_FOREACH(pt::ptree::value_type &p4_tbl, json_pt.get_child(JKEY_REGIONS)) {
        reg = regions_ + idx;
        std::string reg_name = p4_tbl.second.get<std::string>(JKEY_REGION_NAME);
        std::string cache_pipe_name = p4_tbl.second.get<std::string>(JKEY_CACHE_PIPE, "null");
        reg->reset = p4_tbl.second.get<bool>(JKEY_RESET_REGION, false);
        if (cache_pipe_name == "p4ig") {
            reg->cache_pipe = MPARTITION_CACHE_PIPE_P4IG;
        } else if (cache_pipe_name == "p4eg") {
            reg->cache_pipe = MPARTITION_CACHE_PIPE_P4EG;
        } else if (cache_pipe_name == "p4plus-txdma") {
            reg->cache_pipe = MPARTITION_CACHE_PIPE_P4PLUS_TXDMA;
        } else if (cache_pipe_name == "p4plus-rxdma") {
            reg->cache_pipe = MPARTITION_CACHE_PIPE_P4PLUS_RXDMA;
        } else if (cache_pipe_name == "p4plus-all") {
            reg->cache_pipe = MPARTITION_CACHE_PIPE_P4PLUS_ALL;
        } else {
            reg->cache_pipe = MPARTITION_CACHE_PIPE_NONE;
        }
        strcpy(reg->mem_reg_name, reg_name.c_str());
        reg->size_kb = p4_tbl.second.get<int>(JKEY_SIZE_KB);
        reg->start_offset = offset;

        SDK_TRACE_DEBUG("region : %s, size : %dkb, reset : %d, "
                        "start : 0x%" PRIx64 ", end : 0x%" PRIx64 "",
                        reg->mem_reg_name, reg->size_kb, reg->reset,
                        addr(reg->start_offset),
                        addr(reg->start_offset + reg->size_kb * 1024));

        offset += reg->size_kb * 1024;
        idx++;
    }

    return SDK_RET_OK;
}

mpartition *
mpartition::factory(const char *mpartition_json_file, mem_addr_t base_addr,
                    shmmgr *mmgr)
{
    void          *mem;
    mpartition    *new_mpartition;

    if (mpartition_json_file == nullptr) {
        return NULL;
    }

    SDK_TRACE_DEBUG("Memory partition config file : %s", mpartition_json_file);
    if (access(mpartition_json_file, R_OK) < 0) {
        SDK_TRACE_ERR("File %s doesn't exist or not accessible",
                      mpartition_json_file);
        return NULL;
    }

    if (mmgr) {
        mem = mmgr->alloc(sizeof(mpartition), 4, true);
    } else {
        mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_PLATFORM, sizeof(mpartition));
    }
    if (mem == NULL) {
        SDK_TRACE_ERR("Failed to create mpartition instance for %s",
                      mpartition_json_file);
        return NULL;
    }
    new_mpartition = new (mem) mpartition();
    if (new_mpartition->init(mpartition_json_file, base_addr, mmgr) != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to initialize program info for %s",
                      mpartition_json_file);
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

mpartition::~mpartition()
{
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
}

mem_addr_t
mpartition::start_offset (const char *reg_name)
{
    mpartition_region_t *reg = region(reg_name);
    return reg ? reg->start_offset : INVALID_MEM_ADDRESS;
}

mem_addr_t
mpartition::start_addr (const char *reg_name)
{
    mem_addr_t start_offset = this->start_offset(reg_name);
    return start_offset != INVALID_MEM_ADDRESS ? addr(start_offset) : INVALID_MEM_ADDRESS;
}

uint32_t
mpartition::size_kb (const char *reg_name)
{
    mpartition_region_t *reg = region(reg_name);
    return reg ? reg->size_kb : 0;
}

mpartition_region_t *
mpartition::region (const char *reg_name)
{
    mpartition_region_t      *reg;

    for (int i = 0; i < num_regions_; i++) {
        reg = &regions_[i];
        if (!strcmp(reg->mem_reg_name, reg_name)) {
            return reg;
        }
    }
    return NULL;
}

mpartition_region_t *
mpartition::region_by_address (uint64_t addr)
{
    mpartition_region_t      *reg;

    for (int i = 0; i < num_regions_; i++) {
        reg = &regions_[i];

        /*
        SDK_TRACE_DEBUG("Region: {}, Size_KB: {}, Start:{:#x} End:{:#x}, addr: {:#x}",
                        reg->mem_reg_name, reg->size_kb,
                        offset(reg->start_offset),
                        offset(reg->start_offset + reg->size_kb * 1024), addr);
        */

        if ((addr >= this->addr(reg->start_offset)) &&
            (addr < this->addr(reg->start_offset + (reg->size_kb * 1024)))) {
            return reg;
        }
    }
    return NULL;
}

}   // namespace utils
}   // namespace platform
}   // namespace sdk

