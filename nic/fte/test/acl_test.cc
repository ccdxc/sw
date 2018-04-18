#include <gtest/gtest.h>
#include <vector>

#include "nic/fte/acl/acl.hpp"
#include "nic/fte/acl/acl_ctx.hpp"
#include "nic/fte/acl/list.hpp"
#include "nic/fte/acl/itree.hpp"

using namespace acl;
using namespace std;

void timeit(const std::string&, int count, std::function<void()> fn);

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
    slab::factory("test_rule", 0, sizeof(test_rule_t), 10 * 1024, true, true, true);

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

// Five tuple key
struct flow_key_t {
    uint32_t tid;
    uint32_t sip;
    uint32_t dip;
    uint16_t proto;
    uint16_t sport;
    uint16_t dport;
};

enum {
    FLD_TID = 0, FLD_SIP, FLD_DIP, FLD_PROTO, FLD_SPORT, FLD_DPORT, FLD_MAX_FLDS
};

static acl_config_t acl_flow_cfg = {
 num_categories: 1,
 num_fields: FLD_MAX_FLDS,
 defs: {
        FLD_DEF(ACL_FIELD_TYPE_EXACT, flow_key_t, tid),
        FLD_DEF(ACL_FIELD_TYPE_PREFIX, flow_key_t, sip),
        FLD_DEF(ACL_FIELD_TYPE_PREFIX, flow_key_t, dip),
        FLD_DEF(ACL_FIELD_TYPE_EXACT, flow_key_t, proto),
        FLD_DEF(ACL_FIELD_TYPE_RANGE, flow_key_t, sport),
        FLD_DEF(ACL_FIELD_TYPE_RANGE, flow_key_t, dport),
 }
};

ACL_RULE_DEF(flow_rule_t, FLD_MAX_FLDS);

static slab* flow_rule_slab_ =
    slab::factory("flow_rule", 0, sizeof(flow_rule_t), 10*1024, true, true, true);

static flow_rule_t *flow_rule_alloc(uint32_t priority, const void *userdata) {
    flow_rule_t *rule = (flow_rule_t*)flow_rule_slab_->alloc();
    rule->data.priority = priority;
    rule->data.userdata = (void *)userdata;
    rule->data.category_mask = 0x01;
    ref_init(&rule->ref_count, [](const ref_t *ref_count) {
            flow_rule_slab_->free((void*)acl_rule_from_ref(ref_count));
        });
    return rule;
}

static uint32_t myrandom (uint32_t i) { return std::rand()%i;}

class acl_test : public ::testing::Test {
public:
    
    
protected:
    acl_test(){}
    
    virtual ~acl_test(){}
    
    virtual void SetUp() {
        std::srand(1);
    }

    virtual void TearDown() {
        EXPECT_EQ(rule_slab_->num_in_use(), 0);
        EXPECT_EQ(flow_rule_slab_->num_in_use(), 0);
        EXPECT_EQ(acl_ctx_t::num_ctx_in_use(), 0);
        EXPECT_EQ(list_t::num_lists_in_use(), 0);
        EXPECT_EQ(list_t::num_items_in_use(), 0);
        EXPECT_EQ(itree_t::num_trees_in_use(), 0);
        EXPECT_EQ(itree_t::num_nodes_in_use(), 0);
    }
};


TEST_F(acl_test, acl_create) {
    const acl_ctx_t *ctx = acl_create("test.rules", &acl_cfg);

    acl_commit(ctx);

    acl_deref(ctx);

    ctx = acl_get("test.rules");
    EXPECT_NE(ctx, nullptr);

    acl_delete(ctx);

    ctx = acl_get("test.rules");
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
    const acl_ctx_t *ctx = acl_create("test.rules", &acl_cfg);

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

    acl_delete(ctx);
}

TEST_F(acl_test, acl_add_muti_field_rule) {
    const acl_ctx_t *ctx = acl_create("test.rules", &acl_cfg);

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

    acl_delete(ctx);
}

TEST_F(acl_test, acl_rule_priority) {
    const acl_ctx_t *ctx = acl_create("test.rules", &acl_cfg);

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

    acl_delete(ctx);
}

TEST_F(acl_test, acl_rule_update) {
    const acl_ctx_t *ctx = acl_create("test.rules", &acl_cfg);

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

    const acl_ctx_t *old = acl_get("test.rules");

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

    acl_delete(ctx);

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


#define PRINT_RULE(rule)                                                \
    printf("rule#%u tid=%d sip=%x/%d dip=%x/%d proto=%d sport=%u-%u dport=%u-%u\n", \
           rule->data.priority,                                         \
           rule->field[FLD_TID].value.u32,                              \
           rule->field[FLD_SIP].value.u32, rule->field[FLD_SIP].mask_range.u32, \
           rule->field[FLD_DIP].value.u32, rule->field[FLD_DIP].mask_range.u32, \
           rule->field[FLD_PROTO].value.u16,                            \
           rule->field[FLD_SPORT].value.u16, rule->field[FLD_SPORT].mask_range.u16, \
           rule->field[FLD_DPORT].value.u16, rule->field[FLD_DPORT].mask_range.u16)

#define PRINT_KEY(key)                                              \
    printf("key tid=%d sip=%x dip=%x proto=%d sport=%u dport=%u\n",     \
           key.tid, key.sip, key.dip, key.proto, key.sport, key.dport)

#define MASK(plen) (~(((uint32_t)1 << (32-plen)) - 1))

static void gen_rules(uint32_t num_rules, uint32_t num_tenants,
                      vector<flow_rule_t *> &rules,
                      unordered_map<uint32_t, flow_key_t> &keys)
{
    uint32_t num_ips_32 = num_rules/10, 
        num_ips_24 = num_rules/10, 
        num_ips_16 = num_rules/100, 
        num_port_ranges = 100,
        num_exact_ports = 100;
    
    // generate ips (ip, plen)
    vector<pair<uint32_t, uint8_t>> ips;
    for (uint32_t i = 0; i < num_ips_32; i++) {
        uint32_t ip = myrandom(0xFFFFFFFF);
        ips.push_back({ip, 32});
    }
    for (uint32_t i = 0; i < num_ips_24; i++) {
        uint32_t ip = myrandom(0xFFFFFF);
        ips.push_back({ip << 8, 24});
    }
    for (uint32_t i = 0; i < num_ips_16; i++) {
        uint32_t ip = myrandom(0xFFFF);
        ips.push_back({ip << 16, 16});
    }
    random_shuffle(ips.begin(), ips.end(), myrandom);

    // generate ports
    vector<pair<uint16_t, uint16_t>> ports;
    for (uint32_t i = 0; i < num_port_ranges; i++) {
        uint16_t port = myrandom(0xFFFF);
        ports.push_back({port, port});
    }

    for (uint32_t i = 0; i < num_exact_ports; i++) {
        uint16_t low = myrandom(0xFF00);
        uint16_t high = low + myrandom(0xFF);
        ports.push_back({low, high});
    }
    random_shuffle(ports.begin(), ports.end(), myrandom);

    // generate rules
    for (uint32_t i = 0; i < num_rules; i++) {
        uint32_t ip;
        uint8_t plen;
        uint16_t port_low, port_high;
        flow_key_t key{};

        flow_rule_t *rule = flow_rule_alloc(i, NULL);
        rule->field[FLD_TID].value.u16 = myrandom(num_tenants);
        rule->field[FLD_TID].mask_range.u16 = 0xFFFF;
        key.tid = rule->field[FLD_TID].value.u16;

        tie(ip, plen) = ips[myrandom(ips.size())];
        rule->field[FLD_SIP].value.u32 = ip;
        rule->field[FLD_SIP].mask_range.u32 = plen;
        key.sip = (plen >= 32) ? ip : ip + myrandom(~MASK(plen));

        tie(ip, plen) = ips[myrandom(ips.size())];
        rule->field[FLD_DIP].value.u32 = ip;
        rule->field[FLD_DIP].mask_range.u32 = plen;
        key.dip = (plen >= 32) ? ip: ip + myrandom(~MASK(plen));

        rule->field[FLD_PROTO].value.u16 = myrandom(1) == 0 ? 6 : 17;
        rule->field[FLD_PROTO].mask_range.u16 = 0xFFFF;
        key.proto = rule->field[FLD_PROTO].value.u16;

        tie(port_low, port_high) = ports[myrandom(ports.size())];
        rule->field[FLD_DPORT].value.u16 = port_low;
        rule->field[FLD_DPORT].mask_range.u16 = port_high;
        key.dport = (port_high == port_low) ? port_low :
            port_low + myrandom(port_high - port_low);

        tie(port_low, port_high) = ports[myrandom(ports.size())];
        rule->field[FLD_SPORT].value.u16 = port_low;
        rule->field[FLD_SPORT].mask_range.u16 = port_high;
        key.sport = (port_high == port_low) ? port_low :
            port_low + myrandom(port_high - port_low);

        rules.push_back(rule);
        keys[i] = key;

        //PRINT_RULE(rule);
        //PRINT_KEY(key);
     }

    random_shuffle(rules.begin(), rules.end(), myrandom);
}

TEST_F(acl_test, rule_random)
{
    vector<flow_rule_t *> rules;
    unordered_map<uint32_t, flow_key_t> keys;

    const acl_ctx_t *ctx;

    gen_rules(10 * 1000, 100, rules, keys);

    // insert all entries
    ctx = acl_create("flow.rules", &acl_flow_cfg);
    for (const auto rule: rules) {
        EXPECT_EQ(acl_add_rule(&ctx, acl_rule_clone((const acl_rule_t *)rule)), HAL_RET_OK);
    }

    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);

    // lookup
    for (auto &e: keys) {
        const flow_rule_t *rule;
        auto key = e.second;
        auto prio = e.first;
        acl_classify(ctx, (const uint8_t *)&key, (const acl_rule_t **)&rule, 0x01);
        EXPECT_NE(rule, nullptr);

        if (rule) {
            //PRINT_RULE(rule);
            //PRINT_KEY(key);
            
            EXPECT_LE(rule->data.priority, prio);
            EXPECT_EQ(rule->field[FLD_TID].value.u16, key.tid);
            EXPECT_EQ(rule->field[FLD_PROTO].value.u16, key.proto);
            EXPECT_EQ(rule->field[FLD_SIP].value.u32,
                      (key.sip & MASK(rule->field[FLD_SIP].mask_range.u32)));
            EXPECT_EQ(rule->field[FLD_DIP].value.u32,
                      (key.dip & MASK(rule->field[FLD_DIP].mask_range.u32)));
            EXPECT_LE(rule->field[FLD_SPORT].value.u16, key.sport);
            EXPECT_GE(rule->field[FLD_SPORT].mask_range.u16, key.sport);
            EXPECT_LE(rule->field[FLD_DPORT].value.u16, key.dport);
            EXPECT_GE(rule->field[FLD_DPORT].mask_range.u16, key.dport);
        }
    }

    // remove all even
    std::random_shuffle(rules.begin(), rules.end(), myrandom);
    for (const auto rule: rules) {
        if (rule->data.priority % 2 == 0) {
            EXPECT_EQ(acl_del_rule(&ctx, (const acl_rule_t *)rule), HAL_RET_OK);
        }
    }
    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);
    
    // lookup
    for (auto &e: keys) {
        const flow_rule_t *rule;
        auto key = e.second;
        auto prio = e.first;
        acl_classify(ctx, (const uint8_t *)&key, (const acl_rule_t **)&rule, 0x01);

        // cout << "lookup - rule#" << prio << "\n";
        // PRINT_KEY(key);

        if (prio % 2 != 0) {
            EXPECT_NE(rule, nullptr);
        }

        if (rule) {
            // PRINT_RULE(rule);
            EXPECT_LE(rule->data.priority, prio);
            EXPECT_NE(rule->data.priority % 2, 0);
        }
    }

    // remove all odd and add all even
    std::random_shuffle(rules.begin(), rules.end(), myrandom);
    for (const auto rule: rules) {
        if (rule->data.priority % 2 == 0) {
            EXPECT_EQ(acl_add_rule(&ctx, acl_rule_clone((const acl_rule_t *)rule)), HAL_RET_OK);
        } else {
            EXPECT_EQ(acl_del_rule(&ctx, (const acl_rule_t *)rule), HAL_RET_OK);
        }
    }
    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);
    
    // lookup
    for (auto &e: keys) {
        const flow_rule_t *rule;
        auto key = e.second;
        auto prio = e.first;
        acl_classify(ctx, (const uint8_t *)&key, (const acl_rule_t **)&rule, 0x01);

        // cout << "lookup - rule#" << prio << "\n";
        // PRINT_KEY(key);

        if (prio % 2 == 0) {
            EXPECT_NE(rule, nullptr);
        }

        if (rule) {
            // PRINT_RULE(rule);
            EXPECT_LE(rule->data.priority, prio);
            EXPECT_EQ(rule->data.priority % 2, 0);
        }
    }

    // remove all even again 
    std::random_shuffle(rules.begin(), rules.end(), myrandom);
    for (const auto rule: rules) {
        if (rule->data.priority % 2 == 0) {
            EXPECT_EQ(acl_del_rule(&ctx, (const acl_rule_t *)rule), HAL_RET_OK);
        }
    }
    EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);
    
    // lookup
    for (auto &e: keys) {
        const flow_rule_t *rule = nullptr;
        auto key = e.second;
        acl_classify(ctx, (const uint8_t *)&key, (const acl_rule_t **)&rule, 0x01);
        EXPECT_EQ(rule, nullptr);
    }

    // free
    acl_delete(ctx);

    for (const auto rule: rules) {
        acl_rule_deref((const acl_rule_t *)rule);
    }
}


TEST_F(acl_test, rule_benchmark)
{
    vector<flow_rule_t *> rules, rules2;
    unordered_map<uint32_t, flow_key_t> keys, keys2;

    const int num_rules = 100 * 1000, num_updates = num_rules/2;

    const acl_ctx_t *ctx;

    gen_rules(num_rules, 100, rules, keys);

    // insert all entries
    ctx = acl_create("flow.rules", &acl_flow_cfg);
    timeit("insert", rules.size(), [&]() {
            for (const auto rule: rules) {
                EXPECT_EQ(acl_add_rule(&ctx, acl_rule_clone((const acl_rule_t *)rule)), HAL_RET_OK);
            }
            EXPECT_EQ(acl_commit(ctx), HAL_RET_OK);
        });
    
    // lookup
    int matches = 0;
    timeit("lookup", keys.size(), [&]() {
            for (auto &e: keys) {
                const flow_rule_t *rule;
                auto key = e.second;
                acl_classify(ctx, (const uint8_t *)&key, (const acl_rule_t **)&rule, 0x01);
                EXPECT_NE(rule, nullptr);
                if (rule)
                    matches++;
            }
        });
    cout << "matches " << matches << "\n";

    // update
    gen_rules(num_updates, 100, rules2, keys2);
    timeit("updates", num_updates*2, [&]() {
            for (int i = 0; i < num_updates; i++) {
                EXPECT_EQ(acl_add_rule(&ctx, acl_rule_clone((const acl_rule_t *)rules2[i])), HAL_RET_OK);
                EXPECT_EQ(acl_del_rule(&ctx, (const acl_rule_t *)rules[i]), HAL_RET_OK);
            }
        });

    // lookup
    matches = 0;
    timeit("lookup", keys.size(), [&]() {
            for (auto &e: keys) {
                const flow_rule_t *rule;
                auto key = e.second;
                acl_classify(ctx, (const uint8_t *)&key, (const acl_rule_t **)&rule, 0x01);
                if (rule)
                    matches++;
            }
        });

    cout << "matches " << matches << "\n";

    // free
    timeit("deref", 0, [&]() {
            acl_delete(ctx);
        });

    for (const auto rule: rules) {
        acl_rule_deref((const acl_rule_t *)rule);
    }

    for (const auto rule: rules2) {
        acl_rule_deref((const acl_rule_t *)rule);
    }
}
