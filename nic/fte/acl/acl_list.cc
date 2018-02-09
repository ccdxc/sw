#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/acl_list.hpp"

namespace acl {

void *
acl_list_create(const acl_config_t *cfg, uint8_t fid)
{
    return list_t::create();
}

const void *
acl_list_clone(const acl_config_t *cfg, uint8_t fid, const void *list)
{
    return ((list_t*)list)->clone();
}

void
acl_list_deref(const acl_config_t *cfg, uint8_t fid, const void *list)
{
    ((const list_t*)list)->deref();
}

uint32_t
acl_list_cost(const acl_config_t *cfg, uint8_t fid, const void *list,
              const acl_rule_t *rule)
{
    return ((const list_t*)list)->size();
}

hal_ret_t acl_list_add_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **listp, const  acl_rule_t *rule)
{
    auto match = [](const void *cb_arg, const ref_t *entry) {
        const acl_rule_t *rule = (const acl_rule_t *) cb_arg;
        return acl_rule_from_ref(entry)->data.priority >= rule->data.priority; 
    };

    return list_t::insert((const list_t **)listp, acl_rule_to_ref(rule),
                          rule, match);
}

hal_ret_t acl_list_del_rule(const acl_config_t *cfg, uint8_t fid,
                            const void **listp,  const acl_rule_t *rule)
{
    struct cb_arg_t {
        const acl_config_t *cfg;
        const acl_rule_t *rule;
    } arg = { cfg, rule };

    auto match = [](const void* cb_arg, const ref_t *entry) {
        const cb_arg_t *arg = (const cb_arg_t *) cb_arg;
        return acl_rule_compare(arg->rule, arg->cfg, acl_rule_from_ref(entry));
    };

    return list_t::remove((const list_t**)listp, &arg, match);
}

hal_ret_t acl_list_classify(const acl_config_t *cfg, uint8_t fid,
                            const void *list, const uint8_t *key,
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

    ((const list_t*)list)->walk(&arg, cb);

    return HAL_RET_OK;
}

} // namespace acl

