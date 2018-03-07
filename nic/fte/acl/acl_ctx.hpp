#pragma once

#include "nic/fte/acl/acl.hpp"
#include "nic/include/hal_mem.hpp"
#include "sdk/slab.hpp"

namespace acl {

using sdk::lib::slab;

class acl_ctx_t {
public:
    static const acl_ctx_t* create(const acl_config_t *cfg);
    const acl_ctx_t* clone() const {
        ref_inc(&ref_count_);
        return this;
    }
    void deref() const {
        ref_dec(&ref_count_);
    }

    hal_ret_t classify(const uint8_t *key, const acl_rule_t *rules[],
                       uint32_t categories) const;
    static hal_ret_t add_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule);
    static hal_ret_t del_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule);

    // debug counters
    static uint32_t num_ctx_in_use() { return ctx_slab_->num_in_use();}
private:
    static slab *ctx_slab_; // slab for acl ctx 
    static acl_ctx_t* factory(const acl_config_t *cfg);
    acl_ctx_t *copy() const;
    acl_config_t           cfg_;
    ref_t                  ref_count_;
    const void             *tables_[ACL_MAX_FIELDS];
    const void             *def_list_; // default catch all list

};


bool acl_rule_match(const acl_config_t *cfg, const acl_rule_t *rule,
                    const uint8_t*key);

uint32_t acl_field_key(const acl_config_t *cfg, uint8_t fid, const uint8_t *key);

bool acl_field_interval(const acl_config_t *cfg, uint8_t fid, const acl_rule_t *rule,
                        uint32_t *low, uint32_t *high);

bool acl_rule_compare(const acl_rule_t *rule,
                     const acl_config_t *cfg, const acl_rule_t *other);

} //namespace acl
