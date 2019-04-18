//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __ACCEL_HPP__
#define __ACCEL_HPP__

#include "devapi_object.hpp"
#include "devapi_types.hpp"

namespace iris {
class devapi_accel : public devapi_object {
private:
    static devapi_accel *accel_;

private:
    sdk_ret_t init_();
    devapi_accel() {}
    ~devapi_accel() {}

public:
    static devapi_accel *factory();
    static void destroy(devapi_accel *accel);

    static devapi_accel *get_accel() {return accel_; };
    static devapi_accel *find_or_create();

    sdk_ret_t accel_rgroup_add(string name,
                               uint64_t metrics_mem_addr,
                               uint32_t metrics_mem_size);
    sdk_ret_t accel_rgroup_del(string name);
    sdk_ret_t accel_rgroup_ring_add(string name,
                                    std::vector<std::pair<const std::string,uint32_t>>& ring_vec);
    sdk_ret_t accel_rgroup_ring_del(string name,
                                    std::vector<std::pair<const std::string,uint32_t>>& ring_vec);
    sdk_ret_t accel_rgroup_reset_set(string name, uint32_t sub_ring,
                                     bool reset_sense);
    sdk_ret_t accel_rgroup_enable_set(string name, uint32_t sub_ring,
                                      bool enable_sense);
    sdk_ret_t accel_rgroup_pndx_set(string name, uint32_t sub_ring,
                                    uint32_t val, bool conditional);
    sdk_ret_t accel_rgroup_info_get(string name, uint32_t sub_ring,
                                    accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                                    void *user_ctx,
                                    uint32_t *ret_num_entries);
    sdk_ret_t accel_rgroup_indices_get(string name, uint32_t sub_ring,
                                       accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                                       void *user_ctx,
                                       uint32_t *ret_num_entries);
    sdk_ret_t accel_rgroup_metrics_get(string name, uint32_t sub_ring,
                                       accel_rgroup_rmetrics_rsp_cb_t rsp_cb_func,
                                       void *user_ctx,
                                       uint32_t *ret_num_entries);
    sdk_ret_t accel_rgroup_misc_get(string name, uint32_t sub_ring,
                                    accel_rgroup_rmisc_rsp_cb_t rsp_cb_func,
                                    void *user_ctx,
                                    uint32_t *ret_num_entries);
    sdk_ret_t crypto_key_index_upd(uint32_t key_index,
                                   crypto_key_type_t type,
                                   void *key, uint32_t key_size);
};

}    // namespace iris

using iris::devapi_accel;

#endif /* __ACCEL_HPP__ */
