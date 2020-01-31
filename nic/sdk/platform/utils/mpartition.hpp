/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mpartition.hpp
 *
 * @brief   This module defines memory partition interface
 */
#ifndef __SDK_PLATFORM_MPARTITION_HPP__
#define __SDK_PLATFORM_MPARTITION_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "lib/shmmgr/shmmgr.hpp"
#include "platform/utils/mcache_pipe.hpp"

namespace sdk {
namespace platform {
namespace utils {

#define MEM_REG_NAME_MAX_LEN    80
#define INVALID_MEM_ADDRESS     0xFFFFFFFFFFFFFFFF

// Common memory regions irrespective of the pipeline
#define MEM_REGION_QOS_FIFO_NAME        "qos-hbm-fifo"
#define MEM_REGION_TIMERS_NAME          "timers"
#define MEM_REGION_P4_PROGRAM_NAME      "p4_program"
#define MEM_REGION_TX_SCHEDULER_NAME    "tx-scheduler"
#define MEM_REGION_MPU_TRACE_NAME       "mpu-trace"
#define MEM_REGION_QOS_DSCP_COS_MAP     "qos-dscp-cos-map"


typedef enum mpartition_region_kind_e {
    MEM_REGION_KIND_CFGTBL,
    MEM_REGION_KIND_OPERTBL,
    MEM_REGION_KIND_STATIC,
    MEM_REGION_KIND_UPGRADE
} mpartition_region_kind_t;

/**
 * @brief Memory mpartition region
 */
typedef struct mpartition_region_s {
    char            mem_reg_name[MEM_REG_NAME_MAX_LEN];   /**< Name */
    uint64_t        size;           /**< Size */
    uint64_t        block_size;     /**< Size of each block, if exists */
    uint32_t        max_elements;   /**< max. no. of elements, if exists */
    mem_addr_t      start_offset;   /**< Start address offset */
    cache_pipe_t    cache_pipe;     /**< Cached pipe */
    bool            reset;          /**< True to bzero this region during init */
    int32_t         alias_index;    /**< region index if alias for another region */
    mpartition_region_kind_t kind;  /**< Region kind */
    bool            upgrade_check_done; /**< For upgrade scenarios */
} mpartition_region_t;

class mpartition {
public:

    /**
     * @brief Singleton Get instance method
     *
     * @return #mpartition pointer on success, NULL on error
     */
    static mpartition *get_instance(void);

    /**
     * @brief Factory method
     *
     * @param[in] mpartition_json_file Memory regions info
     * @param[in] mmgr Pointer to the memory manager - optional
     *
     * @return #mpartition pointer on success, NULL on error
     */
    static mpartition *factory(const char *mpart_json_file,
                               shmmgr *mmgr = NULL);
    /**
     * @brief Destroy method
     *
     * @param[in] mpartition  Pointer to the mpartition instance
     *
     */
    static void destroy(mpartition *mpartition);

    /**
     * @brief Get memory base address
     *
     * @return #Memory base address
     */
    mem_addr_t base(void) { return base_addr_; }

    /**
     * @brief Convert memory offset to address
     *
     * @param[in] offset Memory offset
     *
     * @return #Memory address
     */
    mem_addr_t addr(mem_addr_t offset) { return base_addr_ + offset; }

    /**
     * @brief Get memory partition region start offset
     *
     * @param[in] name Name of memory partition region
     *
     * @return #Start offset of memory partition region
     */
    mem_addr_t start_offset(const char *name);

    /**
     * @brief Get memory partition region start address
     *
     * @param[in] name Name of memory partition region
     *
     * @return #Start address of memory partition region
     */
    mem_addr_t start_addr(const char *name);

    /**
     * @brief Get memory partition region size
     *
     * @param[in] name Name of memory partition region
     *
     * @return #Memory mpartition region size in bytes
     */
    uint64_t size(const char *name);

    /**
     * @brief Get size of each block in mpartition region
     *
     * @param[in] name Name of memory partition region
     *
     * @return #Memory size of each block in mpartition region
     */
    uint64_t block_size(const char *name);

    /**
     * @brief Get number of elements in mpartition region
     *
     * @param[in] name Name of memory partition region
     *
     * @return number of elements in each block of mpartition region
     */
    uint32_t max_elements(const char *name);

    /**
     * @brief Get memory partition region info
     *
     * @param[in] name Name of memory partition region
     *
     * @return #Memory mpartition region, NULL on error
     */
    mpartition_region_t *region(const char *name);

    /**
     * @brief Get memory partition region info by address
     *
     * @param[in] addr Memory address
     *
     * @return #Memory mpartition region, NULL on error
     */
    mpartition_region_t* region_by_address(mem_addr_t addr);

    /**
     * @brief Get memory partition region info by index
     *
     * @param[in] i Index
     *
     * @return #Memory mpartition region, NULL on error
     */
    mpartition_region_t *region(int i) {
        return (i >= 0) && (i < num_regions_) ? &regions_[i] : NULL;
    }

    /*
     * @brief Get number of memory partition regions
     *
     * @return #Number of memory partition regions
     */
    int num_regions(void) { return num_regions_; }

    /*
     * @brief Dump the memory partition regions to a file
     * Should be called when a bootup/upgrade is complete
     * Otherwise it will overwrite the existing configuration
     *
     * @return SDK_RET_OK on success, failure code on error
     */
    sdk_ret_t dump_regions_info();
    /*
     * @brief Offsets the memory partition regions during upgrade
     *
     * @param[in] regions_fname output file generated using dump_region_info
     * @param[in] oper_table_persist flag to indicate the operational table regions
     *
     * @return SDK_RET_OK on success, failure code on error
     */
    sdk_ret_t upg_regions(const char *regions_fname, bool oper_table_persist);

private:
    static mpartition *instance_;
    mpartition_region_t *regions_;
    shmmgr              *mmgr_;
    int                 num_regions_;
    mem_addr_t          base_addr_;

private:
    mpartition() = default;
    ~mpartition();
    mpartition(const mpartition &) = delete;
    mpartition &operator=(const mpartition &) = delete;
    static mpartition *init(const char *mpart_json_file, shmmgr *mmgr);
    sdk_ret_t region_init(const char *mpart_json_file, shmmgr *mmgr);
};

// functions to check the cache_pipe type
static inline bool
is_region_cache_pipe_none(mpartition_region_t *reg)
{
    return reg->cache_pipe == cache_pipe_t::MEM_REGION_CACHE_PIPE_NONE;
}

static inline bool
is_region_cache_pipe_p4_ig(mpartition_region_t *reg)
{
    return reg->cache_pipe & cache_pipe_t::MEM_REGION_CACHE_PIPE_P4IG;
}

static inline bool
is_region_cache_pipe_p4_eg(mpartition_region_t *reg)
{
    return reg->cache_pipe & cache_pipe_t::MEM_REGION_CACHE_PIPE_P4EG;
}

static inline bool
is_region_cache_pipe_p4plus_txdma(mpartition_region_t *reg)
{
    return reg->cache_pipe & cache_pipe_t::MEM_REGION_CACHE_PIPE_P4PLUS_TXDMA;
}

static inline bool
is_region_cache_pipe_p4plus_rxdma(mpartition_region_t *reg)
{
    return reg->cache_pipe & cache_pipe_t::MEM_REGION_CACHE_PIPE_P4PLUS_RXDMA;
}

static inline bool
is_region_cache_pipe_p4plus_pciedb(mpartition_region_t *reg)
{
    return reg->cache_pipe & cache_pipe_t::MEM_REGION_CACHE_PIPE_P4PLUS_PCIE_DB;
}

static inline bool
is_region_cache_pipe_p4plus_all(mpartition_region_t *reg)
{
    return (reg->cache_pipe & cache_pipe_t::MEM_REGION_CACHE_PIPE_P4PLUS_ALL) ==
           cache_pipe_t::MEM_REGION_CACHE_PIPE_P4PLUS_ALL;
}

static inline bool
is_region_an_alias(mpartition_region_t *reg)
{
    return reg->alias_index >= 0;
}

}    // namespace utils
}    // namespace platform
}    // namespace sdk

using sdk::platform::utils::mpartition;
using sdk::platform::utils::mpartition_region_t;

#endif    // __SDK_PLATFORM_MPARTITION_HPP__
