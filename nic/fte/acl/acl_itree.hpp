#pragma once

#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/itree.hpp"

namespace acl {

void *acl_itree_create(const acl_config_t *cfg, uint8_t fid);
const void *acl_itree_clone(const acl_config_t *cfg, uint8_t fid,
                           const void *tree);
void acl_itree_deref(const acl_config_t *cfg, uint8_t fid,
                    const void *tree);
uint32_t acl_itree_cost(const acl_config_t *cfg,  uint8_t fid,
                       const void *tree, const acl_rule_t *rule);
hal_ret_t acl_itree_add_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **treep, const acl_rule_t *rule);
hal_ret_t acl_itree_del_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **treep, const acl_rule_t *rule);
hal_ret_t acl_itree_classify(const acl_config_t *cfg, uint8_t fid,
                            const void *tree, const uint8_t *key,
                            const acl_rule_t *rules[],  uint32_t categories);
} // namespace acl
