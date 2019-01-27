/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    capri_impl.hpp
 *
 * @brief   CAPRI asic implementation
 */
#if !defined (__CAPRI_IMPL_HPP__)
#define __CAPRI_IMPL_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/framework/asic_impl_base.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"

namespace impl {

/**
 * @defgroup OCI_ASIC_IMPL - asic wrapper implementation
 * @ingroup OCI_ASIC
 * @{
 */

/**
 * @brief    asic implementation
 */
class capri_impl : public asic_impl_base {
public:
    /**
     * @brief    factory method to asic impl instance
     * @param[in] asic_cfg    asic configuration information
     * @return    new instance of capri asic impl or NULL, in case of error
     */
    static capri_impl *factory(asic_cfg_t *asic_cfg);

    /**
     * @brief    init routine to initialize the asic
     * @param[in] asic_cfg    asic configuration information
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t asic_init(asic_cfg_t *asic_cfg) override;

    /**
     * @brief    dump all the debug information to given file
     * @param[in] fp    file handle
     */
    virtual void debug_dump(FILE *fp);

private:
    capri_impl() {}

    ~capri_impl() {}

    /*
     * @brief    initialize an instance of capri impl class
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t init_(void);

    /**
     * @brief    dump per TM port stats
     * @param[in] fp       file handle
     * @param[in] stats    pointer to the stats
     */
    void dump_tm_debug_stats_(FILE *fp, tm_pb_debug_stats_t *debug_stats);

private:
    asic_cfg_t           asic_cfg_;
};

}    // namespace impl

#endif    /** __CAPRI_IMPL_HPP__ */
