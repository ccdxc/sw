/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mpartition.hpp
 *
 * @brief   This module defines memory partition interface
 */
#ifndef __SDK_PLATFORM_MPARTITION_HPP__
#define __SDK_PLATFORM_MPARTITION_HPP__

#include "nic/include/base.hpp"
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/shmmgr.hpp"

namespace sdk {
namespace platform {
namespace utils {

#define MEM_REG_NAME_MAX_LEN    80

#define INVALID_MEM_ADDRESS     0xFFFFFFFFFFFFFFFF

#define JKEY_REGIONS            "regions"
#define JKEY_REGION_NAME        "name"
#define JKEY_SIZE_KB            "size_kb"
#define JKEY_CACHE_PIPE         "cache"
#define JKEY_RESET_REGION       "reset"
#define JKEY_START_OFF          "start_offset"

/**
 * @brief Memory mpartition cache pipes
 */
typedef enum mpartition_cache_pipe_s {
    // Note: Values are used in bitmap
    MPARTITION_CACHE_PIPE_NONE           = 0,    /**< None */
    MPARTITION_CACHE_PIPE_P4IG           = 1,    /**< P4 ingress */
    MPARTITION_CACHE_PIPE_P4EG           = 2,    /**< P4 egress */
    MPARTITION_CACHE_PIPE_P4IG_P4EG      = 3,    /**< P4 ingress and egress */
    MPARTITION_CACHE_PIPE_P4PLUS_TXDMA   = 4,    /**< P4plus txdma */
    MPARTITION_CACHE_PIPE_P4PLUS_RXDMA   = 8,    /**< P4plus rxdma */
    MPARTITION_CACHE_PIPE_P4PLUS_PCIE_DB = 16,   /**< P4plus pcie */
    MPARTITION_CACHE_PIPE_P4PLUS_ALL     = 28,   /**< TxDMA + RxDMA + PCIE + DB blocks */
} mpartition_cache_pipe_t;

/**
 * @brief Memory mpartition region 
 */
typedef struct mpartition_region_s {
    char                    mem_reg_name[MEM_REG_NAME_MAX_LEN];   /**< Name */
    uint32_t                size_kb;        /**< Size */
    mem_addr_t              start_offset;   /**< Start address offset */
    mpartition_cache_pipe_t cache_pipe;     /**< Cached pipe */
    bool                    reset;          /**< True to bzero this region during init */
} mpartition_region_t;

class mpartition {
public:
    /** 
     * @brief Factory method
     *
     * @param[in] mpartition_json_file Location of the memory partition json
     * @param[in] base_addr Base address of the memory
     *
     * @return #mpartition pointer on success, NULL on error 
     */
    static mpartition *factory(const char *mpartition_json_file, 
                               mem_addr_t base_addr,
                               shmmgr *mmgr = NULL);
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
     * @param[in] reg_name Name of memory partition region
     *
     * @return #Start offset of memory partition region
     */
    mem_addr_t start_offset(const char *reg_name);
    /** 
     * @brief Get memory partition region start address 
     *
     * @param[in] reg_name Name of memory partition region
     *
     * @return #Start address of memory partition region
     */
    mem_addr_t start_addr(const char *reg_name);
    /** 
     * @brief Get memory partition region size 
     *
     * @param[in] reg_name Name of memory partition region
     *
     * @return #Memory mpartition region size in kilobytes
     */
    uint32_t size_kb(const char *reg_name);

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

private:
    mpartition_region_t *regions_;
    shmmgr              *mmgr_;
    int                 num_regions_;
    mem_addr_t          base_addr_;

private:
    mpartition() {};
    ~mpartition();
    sdk_ret_t init(const char *mpartition_json_file, mem_addr_t base_addr,
                   shmmgr *mmgr = NULL);
};

}   // namespace utils
}   // namespace platform
}   // namespace sdk

#endif    // __SDK_PLATFORM_MPARTITION_HPP__
