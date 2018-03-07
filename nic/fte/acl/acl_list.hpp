#pragma once

#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/list.hpp"

namespace acl {

void *acl_list_create(const acl_config_t *cfg, uint8_t fid);
const void *acl_list_clone(const acl_config_t *cfg, uint8_t fid,
                           const void *list);
void acl_list_deref(const acl_config_t *cfg, uint8_t fid,
                    const void *list);
uint32_t acl_list_cost(const acl_config_t *cfg,  uint8_t fid,
                       const void *list, const acl_rule_t *rule);
hal_ret_t acl_list_add_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **listp, const acl_rule_t *rule);
hal_ret_t acl_list_del_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **listp, const acl_rule_t *rule);
hal_ret_t acl_list_classify(const acl_config_t *cfg, uint8_t fid,
                            const void *list, const uint8_t *key,
                            const acl_rule_t *rules[],  uint32_t categories);
} // namespace acl
