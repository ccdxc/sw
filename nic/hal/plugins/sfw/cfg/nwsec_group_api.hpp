/* 
 * ----------------------------------------------------------------------------
 *
 * nwsec_group_api.hpp
 *
 * Nwsec APIs exported by PI to PD.
 *
 * ----------------------------------------------------------------------------
 */
#ifndef __NWSEC_GROUP_API_HPP__
#define __NWSEC_GROUP_API_HPP__


namespace hal {

typedef struct nwsec_group_s nwsec_group_t;
typedef struct nwsec_policy_rules_s  nwsec_policy_rules_t;
typedef struct nwsec_policy_cfg_s    nwsec_policy_cfg_t;
typedef struct nwsec_policy_svc_s    nwsec_policy_svc_t;
typedef struct nwsec_policy_appid_s nwsec_policy_appid_t;
struct ipv4_rule_t;

using hal::nwsec_policy_svc_t;
using hal::nwsec_policy_appid_t;
using hal::ipv4_rule_t;


void *
nwsec_policy_cfg_get_key_func (void *entry);

uint32_t
nwsec_policy_cfg_compute_hash_func (void *key, uint32_t ht_size);

bool
nwsec_policy_cfg_compare_key_func (void *key1, void *key2);

nwsec_group_t *
nwsec_group_lookup_key_or_handle(const kh::SecurityGroupKeyHandle& key_or_handle);
dllist_ctxt_t *
get_ep_list_for_security_group(uint32_t sg_id);
hal_ret_t
add_ep_to_security_group(uint32_t sg_id, hal_handle_t ep_handle);
hal_ret_t
del_ep_from_security_group(uint32_t sg_id, hal_handle_t ep_handle);

hal_ret_t
add_nw_to_security_group(uint32_t sg_id, hal_handle_t nw_handle);
hal_ret_t
del_nw_from_security_group(uint32_t sg_id, hal_handle_t nw_handle);
dllist_ctxt_t *
get_nw_list_for_security_group(uint32_t sg_id);

} // namespace hal
#endif // __NWSEC_GROUP_API_HPP__
