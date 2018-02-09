#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/acl_list.hpp"
#include "sdk/slab.hpp"
#include "nic/include/hal_mem.hpp"

namespace acl {

using sdk::lib::slab;

// field tyep specific table operations 
typedef struct fld_type_ops_s {
    bool (*fld_match)(const uint8_t *data, uint8_t size, const acl_field_t *fld);
    void* (*create)(const acl_config_t *cfg, uint8_t fid);
    const void* (*clone)(const acl_config_t *cfg, uint8_t fid, const void *table);
    void (*deref)(const acl_config_t *cfg, uint8_t fid, const void *table);
    uint32_t (*cost)(const acl_config_t *cfg, uint8_t fid, const void *table, const acl_rule_t *rule);
    hal_ret_t (*add_rule)(const acl_config_t *cfg, uint8_t fid, const void **table, const acl_rule_t *rule);
    hal_ret_t (*del_rule)(const acl_config_t *cfg, uint8_t fid, const void **table, const acl_rule_t *rule);
    hal_ret_t (*classify)(const acl_config_t *cfg, uint8_t fid, const void *table, const uint8_t *key,
                        const acl_rule_t *results[],  uint32_t categories);
} fld_type_ops_t;



#define FLD_COMPARE(val, fld1, fld2)                \
    (fld1->value.val == fld2->value.val &&          \
     fld1->mask_range.val == fld2->mask_range.val ) 
    
static inline bool
acl_fld_compare(uint8_t size, const acl_field_t *fld1, const acl_field_t *fld2)
{
    switch(size) {
    case sizeof(uint8_t):
        return FLD_COMPARE(u8, fld1, fld2);
    case sizeof(uint16_t):
        return FLD_COMPARE(u16, fld1, fld2);
    case sizeof(uint32_t):
        return FLD_COMPARE(u32, fld1, fld2);
    case sizeof(uint64_t):
        return FLD_COMPARE(u64, fld1, fld2);
    default:
        SDK_ASSERT_RETURN(true, false);
    }

    return false;
}

#define PREFIX_MATCH(data, fld, typ, val, nbits)            \
    ((fld->value.val >> (nbits - fld->mask_range.val)) ==   \
     (*(typ *)data) >> (nbits - fld->mask_range.val))

static inline bool
acl_fld_match_prefix(const uint8_t *data, uint8_t size, const acl_field_t *fld)
{
    switch(size) {
    case sizeof(uint8_t):
        return PREFIX_MATCH(data, fld, uint8_t, u8, 8);
    case sizeof(uint16_t):
        return PREFIX_MATCH(data, fld, uint16_t, u16, 16);
    case sizeof(uint32_t):
        return PREFIX_MATCH(data, fld, uint32_t, u32, 32);
    case sizeof(uint64_t):
        return PREFIX_MATCH(data, fld, uint64_t, u64, 64);
    default:
        SDK_ASSERT_RETURN(true, false);
    }
    
    return false;
}

#define RANGE_MATCH(data, fld, typ, val)                                \
    ((*(typ *)data >= fld->value.val) && (*(typ *)data <= fld->mask_range.val))

static inline bool
acl_fld_match_range(const uint8_t *data, uint8_t size, const acl_field_t *fld)
{
    switch(size) {
    case sizeof(uint8_t):
        return RANGE_MATCH(data, fld, uint8_t, u8);
    case sizeof(uint16_t):
        return RANGE_MATCH(data, fld, uint16_t, u16);
    case sizeof(uint32_t):
        return RANGE_MATCH(data, fld, uint32_t, u32);
    case sizeof(uint64_t):
        return RANGE_MATCH(data, fld, uint64_t, u64);
    default:
        SDK_ASSERT_RETURN(true, false);
    }

    return false;
}

#define EXACT_MATCH(data, fld, typ, val)                            \
    (((fld->value.val) & (fld->mask_range.val)) == ((*(typ *)data) & (fld->mask_range.val)))

static inline bool
acl_fld_match_exact(const uint8_t *data, uint8_t size, const acl_field_t *fld)
{
    switch(size) {
    case sizeof(uint8_t):
        return EXACT_MATCH(data, fld, uint8_t, u8);
    case sizeof(uint16_t):
        return EXACT_MATCH(data, fld, uint16_t, u16);
    case sizeof(uint32_t):
        return EXACT_MATCH(data, fld, uint32_t, u32);
    case sizeof(uint64_t):
        return EXACT_MATCH(data, fld, uint64_t, u64);
    default:
        SDK_ASSERT_RETURN(true, false);
    }

    return false;
}

static fld_type_ops_t fld_type_ops_[] = {
    /* ACL_FIELD_TYPE_PREFIX */
    {
        fld_match: acl_fld_match_prefix,
        create: acl_list_create,
        clone: acl_list_clone,
        deref: acl_list_deref,
        cost: acl_list_cost,
        add_rule: acl_list_add_rule,
        del_rule: acl_list_del_rule,
        classify:acl_list_classify,
    },
    /* ACL_FIELD_TYPE_RANGE */
    {
        fld_match: acl_fld_match_range,
        create: acl_list_create,
        clone: acl_list_clone,
        deref: acl_list_deref,
        cost: acl_list_cost,
        add_rule: acl_list_add_rule,
        del_rule: acl_list_del_rule,
        classify:acl_list_classify,
    },
    /* ACL_FIELD_TYPE_EXACT */
    {
        fld_match: acl_fld_match_exact,
        create: acl_list_create,
        clone: acl_list_clone,
        deref: acl_list_deref,
        cost: acl_list_cost,
        add_rule: acl_list_add_rule,
        del_rule: acl_list_del_rule,
        classify:acl_list_classify,
    },
};

#define ACL_FLD_OPS(cfg, fid) \
    fld_type_ops_[(cfg)->defs[fid].type]

#define ACL_FLD_MATCH(cfg, fid, key, rule ) \
    ACL_FLD_OPS(cfg, fid).fld_match(key+(cfg)->defs[fid].offset,    \
                                    (cfg)->defs[fid].size,          \
                                    &(rule)->field[fid])
#define ACL_TABLE_CREATE(cfg, fid)              \
    ACL_FLD_OPS(cfg, fid).create(cfg, fid)

#define ACL_TABLE_CLONE(cfg, fid, table)        \
    ACL_FLD_OPS(cfg, fid).clone(cfg, fid, table)

#define ACL_TABLE_DEREF(cfg, fid, table)        \
    ACL_FLD_OPS(cfg, fid).deref(cfg, fid, table)

#define ACL_TABLE_COST(cfg, fid, table, rule)       \
    ACL_FLD_OPS(cfg, fid).cost(cfg, fid, table, rule)

#define ACL_TABLE_ADD_RULE(cfg, fid, table, rule)           \
    ACL_FLD_OPS(cfg, fid).add_rule(cfg, fid, table, rule)

#define ACL_TABLE_DEL_RULE(cfg, fid, table, rule)           \
    ACL_FLD_OPS(cfg, fid).del_rule(cfg, fid, table, rule)

#define ACL_TABLE_CLASSIFY(cfg, fid, table, key, rules, categories)    \
    ACL_FLD_OPS(cfg, fid).classify(cfg, fid, table, key, rules, categories)

bool
acl_rule_match(const acl_config_t *cfg, const acl_rule_t *rule, const uint8_t*key)
{
    for (uint8_t fid = 0; fid < cfg->num_fields; fid++) {
        // skip if field is not defined
        if (rule->field[fid].value.u64 == 0 && rule->field[fid].mask_range.u64 == 0) {
            continue;
        }

        if (!ACL_FLD_MATCH(cfg, fid, key, rule)) {
            return false;
        }
    }
    return true;
}

bool
acl_rule_compare(const acl_rule_t *rule,
                 const acl_config_t *cfg, const acl_rule_t *other)
{
    if (rule->data.category_mask != other->data.category_mask ||
        rule->data.priority != other->data.priority ||
        rule->data.userdata != other->data.userdata) {
        return false;
    }

    for (uint8_t fid = 0; fid < cfg->num_fields; fid++) {
        if (!acl_fld_compare(cfg->defs[fid].size, &rule->field[fid], &other->field[fid])) {
            return false;
        }
    }

    return true;
}

// static initializers
slab *acl_ctx_t::ctx_slab_ = slab::factory("acl_ctx", hal::HAL_SLAB_ACL_CTX,
                                           sizeof(acl_ctx_t), 8, true, true, true);
acl_ctx_t *
acl_ctx_t::factory(const acl_config_t *cfg)
{
    acl_ctx_t *ctx = (acl_ctx_t *)ctx_slab_->alloc();

    // TODO (goli) validate config
    ctx->cfg_ = *cfg;

    ref_init(&ctx->ref_count_, [](const ref_t *ref) {
            acl_ctx_t *ctx = container_of(ref, acl_ctx_t, ref_count_);
            // free tables
            for (uint8_t fid = 0; fid < ctx->cfg_.num_fields; fid++) {
                ACL_TABLE_DEREF(&ctx->cfg_, fid, ctx->tables_[fid]);
            }
            ctx_slab_->free(ctx);
        });

    return ctx;
}


const acl_ctx_t *
acl_ctx_t::create(const acl_config_t *cfg)
{
    acl_ctx_t *ctx = acl_ctx_t::factory(cfg);

    // create sub-tables
    for (uint8_t fid = 0; fid < ctx->cfg_.num_fields; fid++) {
        ctx->tables_[fid] = ACL_TABLE_CREATE(&ctx->cfg_, fid);
    }

    return ctx;
}

acl_ctx_t *
acl_ctx_t::copy() const
{
    if (!ref_is_shared(&ref_count_)) {
        return (acl_ctx_t *)this;
    }

    acl_ctx_t *ctx = acl_ctx_t::factory(&cfg_);

    // clone sub-tables
    for (uint8_t fid = 0; fid < cfg_.num_fields; fid++) {
        ctx->tables_[fid] = ACL_TABLE_CLONE(&cfg_, fid, tables_[fid]);
    }

    this->deref();

    return ctx;
}
 
hal_ret_t
acl_ctx_t::add_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule)
{
    uint32_t min_cost = 0xffffffff;
    uint8_t  min_cost_fid = 0;

    acl_ctx_t *ctx = (*ctxp)->copy();
    *ctxp = ctx;

    for (uint8_t fid = 0; fid < ctx->cfg_.num_fields && min_cost > 0; fid ++) {
        uint32_t cost = ACL_TABLE_COST(&ctx->cfg_, fid, ctx->tables_[fid], rule);
        if (cost < min_cost) {
            min_cost = cost;
            min_cost_fid = fid;
        }
    }

    return ACL_TABLE_ADD_RULE(&ctx->cfg_, min_cost_fid,
                              &ctx->tables_[min_cost_fid], rule);
}

hal_ret_t
acl_ctx_t::del_rule(const acl_ctx_t **ctxp, const acl_rule_t *rule)
{
    hal_ret_t ret;

    acl_ctx_t *ctx = (*ctxp)->copy();
    *ctxp = ctx;

    // delete from sub-table
    for (uint8_t fid = 0; fid < ctx->cfg_.num_fields; fid ++) {
        ret = ACL_TABLE_DEL_RULE(&ctx->cfg_, fid, &ctx->tables_[fid], rule);
        if (ret == HAL_RET_OK) {
            break;
        }
    }

    return ret;
}
 
hal_ret_t
acl_ctx_t::classify(const uint8_t *key, const acl_rule_t *rules[],
                    uint32_t categories) const
{
    // TODO(goli) - handle multiple categories
    rules[0] = NULL;

    for (uint8_t fid = 0; fid < cfg_.num_fields; fid ++) {
        ACL_TABLE_CLASSIFY(&cfg_, fid, tables_[fid], key, rules, categories);
    }

    return HAL_RET_OK;
}

} // namespace acl
