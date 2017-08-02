#include <arpa/inet.h>
#include <base.h>
#include <ip.h>
#include <pt.hpp>
#include <gtest/gtest.h>

using hal::utils::pt;

typedef struct test_route_s {
    ip_prefix_t    pfx;
    uint32_t       nh_id;
    uint32_t       sg_label;
} __PACK__ test_route_t;

// maximum key len for patricia tree
// 128 bits of IPv6, 1 byte for address family + 7 bytes to round up next
// multiple of 8
#define PT_MAX_KEY_LEN                     136
#define PT_KEY_LEN_IN_BITS(pfx_len)        (8 + (pfx_len))    // add 8 bits of address-family

void
dump_route(test_route_t *route, uint16_t key_len, bool dump_data)
{
    printf("IP Pfx %s/%u ==> ",
           ipv4addr2str(ntohl(route->pfx.addr.addr.v4_addr)), key_len - 8);
    if (dump_data) {
        printf("nh_id = %u, sg_label %u\n", route->nh_id, route->sg_label);
    } else {
        printf("Internal node\n");
    }
}

void
test_route_pt_dump (uint8_t *key, uint16_t key_len, void *entry, void *arg)
{
    dump_route((test_route_t *)key, key_len, entry ? true : false);
}

class pt_test : public ::testing::Test {
protected:
    pt_test() {
    }

    virtual ~pt_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

// test pt instantiation and deletion
TEST_F(pt_test, create_delete) {
    pt *test_pt;

    test_pt = pt::factory("test-pt", PT_MAX_KEY_LEN, true);
    ASSERT_TRUE(test_pt != NULL);

    delete test_pt;
}

// test node insertion deletion
TEST_F(pt_test, insert_deletes) {
    hal_ret_t       ret;
    pt              *test_pt;
    test_route_t    routes[16], *route_p;
    ip_addr_t       lookup_key;
    uint32_t        i = 1, r = 0;

    // invalid args
    test_pt = pt::factory("test-pt", PT_MAX_KEY_LEN, true);
    ASSERT_TRUE(test_pt != NULL);

    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0a01);
    routes[r].pfx.len = 26;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_EQ(test_pt->num_entries(), 1);

    // reinsert with same key and make sure duplicate is detected
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);
    ASSERT_EQ(test_pt->num_entries(), 1);

    // reduce the prefix length and make sure we can insert overlapping prefix
    r++;
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0a01);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_TRUE(ret == HAL_RET_OK);
    test_pt->iterate(test_route_pt_dump, NULL);
    ASSERT_EQ(test_pt->num_entries(), 2);

    // do a lookup for 10.10.10.2 and make sure LPM match happens
    // with 10.10.10.1/26 prefix inserted first
    test_pt->iterate(test_route_pt_dump, NULL);
    lookup_key.af = IP_AF_IPV4;
    lookup_key.addr.v4_addr = htonl(0x0a0a0a02);
    route_p = (test_route_t *)test_pt->lookup((uint8_t *)&lookup_key,
                                              PT_KEY_LEN_IN_BITS(32), false);
    ASSERT_TRUE(route_p != NULL);
    ASSERT_EQ(route_p->pfx.addr.addr.v4_addr, htonl(0x0a0a0a00));
    ASSERT_EQ(PT_KEY_LEN_IN_BITS(route_p->pfx.len), PT_KEY_LEN_IN_BITS(26));

    // create a branch in the tree
    r++;
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0801);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_TRUE(ret == HAL_RET_OK);
    test_pt->iterate(test_route_pt_dump, NULL);
    ASSERT_EQ(test_pt->num_entries(), 4);
    ASSERT_EQ(test_pt->num_internal_entries(), 1);

    // remove this branch
    route_p = (test_route_t *)test_pt->remove((uint8_t *)&routes[r].pfx,
                                              PT_KEY_LEN_IN_BITS(routes[r].pfx.len));
    ASSERT_TRUE(route_p != NULL);
    ASSERT_TRUE(route_p == &routes[r]);
    ASSERT_EQ(test_pt->num_entries(), 3);
    ASSERT_EQ(test_pt->num_internal_entries(), 1);
    test_pt->iterate(test_route_pt_dump, NULL);

    // remove 10.10.10.1/24 route
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0a01);
    routes[r].pfx.len = 24;
    route_p = (test_route_t *)test_pt->remove((uint8_t *)&routes[r].pfx,
                                              PT_KEY_LEN_IN_BITS(routes[r].pfx.len));
    test_pt->iterate(test_route_pt_dump, NULL);
    ASSERT_TRUE(route_p != NULL);
    ASSERT_EQ(test_pt->num_entries(), 3);
    ASSERT_EQ(test_pt->num_internal_entries(), 2);

    // remove 10.10.10.1/26 route and make sure tree is empty
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0a01);
    routes[r].pfx.len = 26;
    route_p = (test_route_t *)test_pt->remove((uint8_t *)&routes[r].pfx,
                                              PT_KEY_LEN_IN_BITS(routes[r].pfx.len));
    test_pt->iterate(test_route_pt_dump, NULL);
    ASSERT_TRUE(route_p != NULL);
    ASSERT_EQ(test_pt->num_entries(), 0);
    ASSERT_EQ(test_pt->num_internal_entries(), 0);

    delete test_pt;
}

// test multiple prefixes of same length to build multiple internal nodes
TEST_F(pt_test, insert_leaves) {
    hal_ret_t       ret;
    pt              *test_pt;
    test_route_t    routes[16];
    uint32_t        i = 1, r = 0;

    // invalid args
    test_pt = pt::factory("test-pt", PT_MAX_KEY_LEN, true);
    ASSERT_TRUE(test_pt != NULL);

    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0a00);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_EQ(ret, HAL_RET_OK);

    r++;
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0800);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_EQ(ret, HAL_RET_OK);

    r++;
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0c00);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(test_pt->num_entries(), 5);
    ASSERT_EQ(test_pt->num_internal_entries(), 2);

    r++;
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0e00);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_EQ(ret, HAL_RET_OK);
    ASSERT_EQ(test_pt->num_entries(), 7);
    ASSERT_EQ(test_pt->num_internal_entries(), 3);

    r++;
    routes[r].pfx.addr.af = IP_AF_IPV4;
    routes[r].pfx.addr.addr.v4_addr = htonl(0x0a0a0200);
    routes[r].pfx.len = 24;
    routes[r].nh_id = i;
    routes[r].sg_label = i++;
    ret = test_pt->insert((uint8_t *)&routes[r].pfx,
                          PT_KEY_LEN_IN_BITS(routes[r].pfx.len), &routes[r]);
    ASSERT_EQ(ret, HAL_RET_OK);
    test_pt->iterate(test_route_pt_dump, NULL);
    ASSERT_EQ(test_pt->num_entries(), 9);
    ASSERT_EQ(test_pt->num_internal_entries(), 4);

    delete test_pt;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
