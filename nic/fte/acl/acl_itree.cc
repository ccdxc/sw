#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/acl_itree.hpp"

namespace acl {

void *
acl_itree_create(const acl_config_t *cfg, uint8_t fid)
{
    return itree_t::create();
}

const void *
acl_itree_clone(const acl_config_t *cfg, uint8_t fid, const void *tree)
{
    return ((itree_t*)tree)->clone();
}

void
acl_itree_deref(const acl_config_t *cfg, uint8_t fid, const void *tree)
{
    ((const itree_t*)tree)->deref();
}

uint32_t
acl_itree_cost(const acl_config_t *cfg, uint8_t fid, const void *tree,
              const acl_rule_t *rule)
{
    uint32_t low, high;

    if (acl_field_interval(cfg, fid, rule, &low, &high) == false) {
        return ~0;
    }

    return ((const itree_t*)tree)->cost(low, high);
}

hal_ret_t acl_itree_add_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **treep, const  acl_rule_t *rule)
{
    auto match = [](const void *cb_arg, const ref_t *entry) {
        const acl_rule_t *rule = (const acl_rule_t *) cb_arg;
        return acl_rule_from_ref(entry)->data.priority >= rule->data.priority; 
    };

    uint32_t low, high;

    if (acl_field_interval(cfg, fid, rule, &low, &high) == false) {
        return HAL_RET_INVALID_ARG;
    }

    return itree_t::insert((const itree_t **)treep, low, high,
                           acl_rule_to_ref(rule), rule, match);
}

hal_ret_t acl_itree_del_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **treep,  const acl_rule_t *rule)
{
    uint32_t low, high;

    if (acl_field_interval(cfg, fid, rule, &low, &high) == false) {
        return HAL_RET_INVALID_ARG;
    }

    struct cb_arg_t {
        const acl_config_t *cfg;
        const acl_rule_t *rule;
    } arg = { cfg, rule };

    auto match = [](const void* cb_arg, const ref_t *entry) {
        const cb_arg_t *arg = (const cb_arg_t *) cb_arg;
        return acl_rule_compare(arg->rule, arg->cfg, acl_rule_from_ref(entry));
    };

    return itree_t::remove((const itree_t**)treep, low, high, &arg, match);
}

hal_ret_t acl_itree_classify(const acl_config_t *cfg, uint8_t fid,
                            const void *tree, const uint8_t *key,
                            const acl_rule_t *rules[],
                            uint32_t categories)
{
    struct cb_arg_t {
        const acl_config_t *cfg;
        const uint8_t *key;
        const acl_rule_t **rules;
        uint32_t categories;
    } arg = { cfg, key, rules, categories};

    auto cb = [](const void* cb_arg, const ref_t *entry) {
        const cb_arg_t *arg = (const cb_arg_t *) cb_arg;
        const acl_rule_t *rule = acl_rule_from_ref(entry);
        uint32_t category = 0;  // TODO get it from rule (can be more than one)

        if (arg->rules[category] &&
            arg->rules[category]->data.priority < rule->data.priority) {
            // TODO(goli) need to support multiple catagories
            return false; // abort walk
        }
        
        if (acl_rule_match(arg->cfg, rule, arg->key)) {
            arg->rules[category] = rule;
            // TODO(goli) need to support multiple catagories
            return false; // found match, abort walk -
        }

        return true; // continue walk
    };

    uint32_t key_val = acl_field_key(cfg, fid, key);

    ((const itree_t*)tree)->walk(key_val, key_val, &arg, cb);

    return HAL_RET_OK;
}

} // namespace acl

