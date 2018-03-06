#include <gtest/gtest.h>
#include <vector>

#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/list.hpp"

using namespace acl;
using namespace std;

struct test_key_t {
    uint8_t prefix8;
    uint16_t prefix16;
    uint32_t prefix32;
    uint8_t range8;
    uint16_t range16;
    uint32_t range32;
    uint8_t exact8;
    uint16_t exact16;
    uint32_t exact32;
};

enum {
    FLD_PREFIX8 = 0, FLD_PREFIX16, FLD_PREFIX32,
    FLD_RANGE8, FLD_RANGE16, FLD_RANGE32,
    FLD_EXACT8, FLD_EXACT16, FLD_EXACT32,
    FLD_NUM_FIELDS
};

#define FLD_DEF(typ, struct_name, fld_name)      \
    {typ, sizeof(((struct_name*)0)->fld_name),   \
            offsetof(struct_name, fld_name) }

static acl_config_t acl_cfg = {
 name: "test.rules",
 num_categories: 1,
 num_fields: FLD_NUM_FIELDS,
 defs: {
        FLD_DEF(ACL_FIELD_TYPE_PREFIX, test_key_t, prefix8),
        FLD_DEF(ACL_FIELD_TYPE_PREFIX, test_key_t, prefix16),
        FLD_DEF(ACL_FIELD_TYPE_PREFIX, test_key_t, prefix32),
        FLD_DEF(ACL_FIELD_TYPE_RANGE, test_key_t, range8),
        FLD_DEF(ACL_FIELD_TYPE_RANGE, test_key_t, range16),
        FLD_DEF(ACL_FIELD_TYPE_RANGE, test_key_t, range32),
        FLD_DEF(ACL_FIELD_TYPE_EXACT, test_key_t, exact8),
        FLD_DEF(ACL_FIELD_TYPE_EXACT, test_key_t, exact16),
        FLD_DEF(ACL_FIELD_TYPE_EXACT, test_key_t, exact32),
 }
};

ACL_RULE_DEF(test_rule_t, FLD_NUM_FIELDS);

static slab* rule_slab_ =
    slab::factory("test_rule", 0, sizeof(test_rule_t), 1024, true, true, true);

static test_rule_t *test_rule_alloc(uint32_t priority, const void *userdata) {
    test_rule_t *rule = (test_rule_t*)rule_slab_->alloc();
    rule->data.priority = priority;
    rule->data.userdata = (void *)userdata;
    rule->data.category_mask = 0x01;
    ref_init(&rule->ref_count, [](const ref_t *ref_count) {
            rule_slab_->free((void*)acl_rule_from_ref(ref_count));
        });
    return rule;
}

class acl_test : public ::testing::Test {
public:

protected:
    acl_test(){}

    virtual ~acl_test(){}

    virtual void SetUp() {
    }

    virtual void TearDown() {
        EXPECT_EQ(rule_slab_->num_in_use(), 0);
        EXPECT_EQ(acl_ctx_t::num_ctx_in_use(), 0);
        EXPECT_EQ(list_t::num_lists_in_use(), 0);
        EXPECT_EQ(list_t::num_items_in_use(), 0);
    }
};


TEST_F(acl_test, acl_create) {
    const acl_ctx_t *ctx = lib_acl_create(&acl_cfg);

    acl_deref(ctx);

    ctx = acl_get(acl_cfg.name);
    EXPECT_NE(ctx, nullptr);

    lib_acl_delete(ctx);

    ctx = acl_get(acl_cfg.name);
    EXPECT_EQ(ctx, nullptr);
}

#define ADD_RULE(ctx, prio, name, fld_num, typ, val1, val2) {   \
        test_rule_t *rule = test_rule_alloc(prio, name);        \
        rule->field[fld_num].value.typ = val1;                  \
        rule->field[fld_num].mask_range.typ = val2;             \
        EXPECT_EQ(acl_add_rule(&ctx, (const acl_rule_t*)rule),  \
                  HAL_RET_OK);                                  \
    }                                                           \

#define DEL_RULE(ctx, prio, name, fld_num, typ, val1, val2) {   \
        test_rule_t *rule = test_rule_alloc(prio, name);        \
        rule->field[fld_num].value.typ = val1;                  \
        rule->field[fld_num].mask_range.typ = val2;             \
        EXPECT_EQ(acl_del_rule(&ctx, (const acl_rule_t *)rule), \
                  HAL_RET_OK);                                  \
        acl_rule_deref((const acl_rule_t *)rule);               \
    }

#define CHECK_VALID_RULE(ctx, name, fld, vals)                          \
    for (auto val : vals) {                                             \
        test_key_t key = {};                                            \
        const acl_rule_t *rule = NULL;                                  \
        key.fld = val;                                                  \
        acl_classify(ctx, (const uint8_t *)&key, &rule, 0x01);          \
        EXPECT_NE(rule, nullptr);                                       \
        EXPECT_STREQ((const char*)rule->data.userdata, name);           \
    }                                                                   \
    
#define CHECK_INVALID_RULE(ctx, fld, vals)                              \
    for (auto val : vals) {                                             \
        test_key_t key = {};                                            \
        const acl_rule_t *rule = NULL;                                  \
        key.fld = val;                                                  \
        acl_classify(ctx, (const uint8_t *)&key, &rule, 0x01);          \
        EXPECT_EQ(rule, nullptr);                                       \
    }

#define CHECK_RULE(ctx, name, fld, valid, invalid)  \
    CHECK_VALID_RULE(ctx, name, fld, valid)          \
    CHECK_INVALID_RULE(ctx, fld, invalid) 

TEST_F(acl_test, acl_add_rule) {
    const acl_ctx_t *ctx = lib_acl_create(&acl_cfg);

    ADD_RULE(ctx, 0, "exact8", FLD_EXACT8, u8, 0xA0, 0xF0);
    ADD_RULE(ctx, 0, "exact16", FLD_EXACT16, u16, 0xAB00, 0xFF00);
    ADD_RULE(ctx, 0, "exact32", FLD_EXACT32, u32, 0xABCD0000, 0xFFFF0000);

    ADD_RULE(ctx, 0, "range8", FLD_RANGE8, u8, 100, 200);
    ADD_RULE(ctx, 0, "range16", FLD_RANGE16, u16, 1000, 2000);
    ADD_RULE(ctx, 0, "range32", FLD_RANGE32, u32, 100000, 200000);

    ADD_RULE(ctx, 0, "prefix8", FLD_PREFIX8, u8, 0xA0, 4);
    ADD_RULE(ctx, 0, "prefix16", FLD_PREFIX16, u16, 0xAB00, 8);
    ADD_RULE(ctx, 0, "prefix32", FLD_PREFIX32, u32, 0xABCD0000, 16);

    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);

    CHECK_RULE(ctx, "exact8", exact8,
               (vector<uint8_t>{0xA0, 0xA9, 0xAF}),
               (vector<uint8_t>{0x90, 0xF0}));
    CHECK_RULE(ctx, "exact16", exact16,
               (vector<uint16_t>{0xAB00, 0xAB99, 0xABFF}),
               (vector<uint16_t>{0x9900, 0xF0F0}));
    CHECK_RULE(ctx, "exact32", exact32,
               (vector<uint32_t>{0xABCD0000, 0xABCD9999, 0xABCDFFFF}),
               (vector<uint32_t>{0xABC00000, 0x0BCD0000}));

    CHECK_RULE(ctx, "range8", range8,
               (vector<uint8_t>{100, 150, 200}),
               (vector<uint8_t>{99, 201}));
    CHECK_RULE(ctx, "range16", range16,
               (vector<uint16_t>{1000, 1500, 2000}),
               (vector<uint16_t>{990, 2900}));
    CHECK_RULE(ctx, "range32", range32,
               (vector<uint32_t>{100000, 150000, 200000}),
               (vector<uint32_t>{99000, 290000}));

    CHECK_RULE(ctx, "prefix8", prefix8,
               (vector<uint8_t>{0xA0, 0xA9, 0xAF}),
               (vector<uint8_t>{0x90, 0xF0}));
    CHECK_RULE(ctx, "prefix16", prefix16,
               (vector<uint16_t>{0xAB00, 0xAB99, 0xABFF}),
               (vector<uint16_t>{0x9900, 0xF0F0}));
    CHECK_RULE(ctx, "prefix32", prefix32,
               (vector<uint32_t>{0xABCD0000, 0xABCD9999, 0xABCDFFFF}),
               (vector<uint32_t>{0xABC00000, 0x0BCD0000}));

    lib_acl_delete(ctx);
}

TEST_F(acl_test, acl_add_muti_field_rule) {
    const acl_ctx_t *ctx = lib_acl_create(&acl_cfg);

    test_rule_t *rule;

    rule = test_rule_alloc(1, "rule1"); 
    rule->field[FLD_EXACT8].value.u8 = 100;
    rule->field[FLD_EXACT8].mask_range.u8 = 0xFF;
    rule->field[FLD_RANGE16].value.u16 = 1000;
    rule->field[FLD_RANGE16].mask_range.u16 = 2000;
    rule->field[FLD_PREFIX32].value.u32 = 0xAABBCC00;
    rule->field[FLD_PREFIX32].mask_range.u32 = 24;

    EXPECT_EQ(acl_add_rule(&ctx, (const acl_rule_t *)rule), HAL_RET_OK);

    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);

    test_key_t key;
    key.exact8 = 100;
    key.range16 = 1005;
    key.prefix32 = 0xAABBCCDD;
    acl_classify(ctx, (const uint8_t*)&key, (const acl_rule_t **)&rule, 0x01);
    EXPECT_STREQ((const char*)rule->data.userdata, "rule1");

    key.exact8 = 99;
    key.range16 = 1005;
    key.prefix32 = 0xAABBCCDD;
    acl_classify(ctx, (const uint8_t*)&key, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);

    key.exact8 = 100;
    key.range16 = 3000;
    key.prefix32 = 0xAABBCCDD;
    acl_classify(ctx, (const uint8_t*)&key, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);

    key.exact8 = 100;
    key.range16 = 1005;
    key.prefix32 = 0xAABB0000;
    acl_classify(ctx, (const uint8_t*)&key, (const acl_rule_t **)&rule, 0x01);
    EXPECT_EQ(rule, nullptr);

    lib_acl_delete(ctx);
}

TEST_F(acl_test, acl_rule_priority) {
    const acl_ctx_t *ctx = lib_acl_create(&acl_cfg);

    ADD_RULE(ctx, 0, "rule0", FLD_PREFIX32, u32, 0xAABBCCDD, 32);
    ADD_RULE(ctx, 1, "rule1", FLD_PREFIX32, u32, 0xAABBCCDD, 24);
    ADD_RULE(ctx, 2, "rule2", FLD_PREFIX32, u32, 0xAABBCCDD, 16);
    ADD_RULE(ctx, 3, "rule3", FLD_PREFIX32, u32, 0xAABBCCDD, 8);
    ADD_RULE(ctx, 0, "rule4", FLD_RANGE16, u16, 100, 100);
    ADD_RULE(ctx, 1, "rule5", FLD_RANGE16, u16, 10, 500);
    ADD_RULE(ctx, 2, "rule6", FLD_RANGE16, u16, 1, 0xFFFF);
    ADD_RULE(ctx, 0, "rule7", FLD_EXACT16, u16, 0xAABB, 0xFFFF);
    ADD_RULE(ctx, 1, "rule8", FLD_EXACT16, u16, 0xAABB, 0xFF00);
    ADD_RULE(ctx, 2, "rule9", FLD_EXACT16, u16, 0xAABB, 0x00FF);

    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);

    CHECK_VALID_RULE(ctx, "rule0", prefix32, (vector<uint32_t>{0xAABBCCDD}));
    CHECK_VALID_RULE(ctx, "rule1", prefix32, (vector<uint32_t>{0xAABBCCFF}));
    CHECK_VALID_RULE(ctx, "rule2", prefix32, (vector<uint32_t>{0xAABBFFFF}));
    CHECK_VALID_RULE(ctx, "rule3", prefix32, (vector<uint32_t>{0xAAFFFFFF}));
    CHECK_VALID_RULE(ctx, "rule4", range16, (vector<uint16_t>{100}));
    CHECK_VALID_RULE(ctx, "rule5", range16, (vector<uint16_t>{200}));
    CHECK_VALID_RULE(ctx, "rule6", range16, (vector<uint16_t>{1000}));
    CHECK_VALID_RULE(ctx, "rule7", exact16, (vector<uint16_t>{0xAABB}));
    CHECK_VALID_RULE(ctx, "rule8", exact16, (vector<uint16_t>{0xAAFF}));
    CHECK_VALID_RULE(ctx, "rule9", exact16, (vector<uint16_t>{0xFFBB}));

    lib_acl_delete(ctx);
}

TEST_F(acl_test, acl_rule_update) {
    const acl_ctx_t *ctx = lib_acl_create(&acl_cfg);

    ADD_RULE(ctx, 0, "rule0", FLD_PREFIX32, u32, 0xAABBCCDD, 32);
    ADD_RULE(ctx, 1, "rule1", FLD_PREFIX32, u32, 0xAABBCCDD, 24);
    ADD_RULE(ctx, 2, "rule2", FLD_PREFIX32, u32, 0xAABBCCDD, 16);
    ADD_RULE(ctx, 3, "rule3", FLD_PREFIX32, u32, 0xAABBCCDD, 8);
    ADD_RULE(ctx, 0, "rule4", FLD_RANGE16, u16, 100, 100);
    ADD_RULE(ctx, 1, "rule5", FLD_RANGE16, u16, 10, 500);
    ADD_RULE(ctx, 2, "rule6", FLD_RANGE16, u16, 1, 0xFFFF);
    ADD_RULE(ctx, 0, "rule7", FLD_EXACT16, u16, 0xAABB, 0xFFFF);
    ADD_RULE(ctx, 1, "rule8", FLD_EXACT16, u16, 0xAABB, 0xFF00);
    ADD_RULE(ctx, 2, "rule9", FLD_EXACT16, u16, 0xAABB, 0x00FF);

    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);

    const acl_ctx_t *old = acl_get(acl_cfg.name);

    DEL_RULE(ctx, 1, "rule1", FLD_PREFIX32, u32, 0xAABBCCDD, 24);
    ADD_RULE(ctx, 1, "rule1", FLD_PREFIX32, u32, 0xAABBCC88, 32);
    DEL_RULE(ctx, 1, "rule5", FLD_RANGE16, u16, 10, 500);
    ADD_RULE(ctx, 1, "rule5", FLD_RANGE16, u16, 800, 900);
    DEL_RULE(ctx, 1, "rule8", FLD_EXACT16, u16, 0xAABB, 0xFF00);
    ADD_RULE(ctx, 1, "rule8", FLD_EXACT16, u16, 0xAACC, 0xFFFF);


    CHECK_VALID_RULE(ctx, "rule0", prefix32, (vector<uint32_t>{0xAABBCCDD}));
    CHECK_VALID_RULE(ctx, "rule1", prefix32, (vector<uint32_t>{0xAABBCC88}));
    CHECK_VALID_RULE(ctx, "rule2", prefix32, (vector<uint32_t>{0xAABBCCFF,0xAABBFFFF}));
    CHECK_VALID_RULE(ctx, "rule3", prefix32, (vector<uint32_t>{0xAAFFFFFF}));
    CHECK_VALID_RULE(ctx, "rule4", range16, (vector<uint16_t>{100}));
    CHECK_VALID_RULE(ctx, "rule5", range16, (vector<uint16_t>{800}));
    CHECK_VALID_RULE(ctx, "rule6", range16, (vector<uint16_t>{200, 1000}));
    CHECK_VALID_RULE(ctx, "rule7", exact16, (vector<uint16_t>{0xAABB}));
    CHECK_RULE(ctx, "rule8", exact16, (vector<uint16_t>{0xAACC}), (vector<uint16_t>{0xAAFF}));
    CHECK_VALID_RULE(ctx, "rule9", exact16, (vector<uint16_t>{0xFFBB}));

    lib_acl_delete(ctx);

    // check old ref is still valid and has old rules
    CHECK_VALID_RULE(old, "rule0", prefix32, (vector<uint32_t>{0xAABBCCDD}));
    CHECK_VALID_RULE(old, "rule1", prefix32, (vector<uint32_t>{0xAABBCCFF}));
    CHECK_VALID_RULE(old, "rule2", prefix32, (vector<uint32_t>{0xAABBFFFF}));
    CHECK_VALID_RULE(old, "rule3", prefix32, (vector<uint32_t>{0xAAFFFFFF}));
    CHECK_VALID_RULE(old, "rule4", range16, (vector<uint16_t>{100}));
    CHECK_VALID_RULE(old, "rule5", range16, (vector<uint16_t>{200}));
    CHECK_VALID_RULE(old, "rule6", range16, (vector<uint16_t>{1000}));
    CHECK_VALID_RULE(old, "rule7", exact16, (vector<uint16_t>{0xAABB}));
    CHECK_VALID_RULE(old, "rule8", exact16, (vector<uint16_t>{0xAAFF}));
    CHECK_VALID_RULE(old, "rule9", exact16, (vector<uint16_t>{0xFFBB}));

    acl_deref(old);
}



