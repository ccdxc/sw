#include <gtest/gtest.h>
#include <vector>

#include "nic/utils/list/list.hpp"

using namespace std;
using namespace hal::utils;

struct entry_t {
    int val;
    dllist_ctxt_t next;
    entry_t(int v): val(v){} 
};

vector<entry_t> g_entries {1,2,3,4,5};
vector<int> tovec(dllist_ctxt_t *head) {
    vector<int> v;
    dllist_ctxt_t *entry;
    dllist_for_each(entry, head) {
        entry_t *e = dllist_entry(entry, entry_t, next);
        v.push_back(e->val);
    }
    return v;
}

class list_test : public ::testing::Test {
protected:
    list_test() {}
    virtual ~list_test(){}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
        // build a temp list
        for (auto &entry : g_entries) {
            dllist_add_tail(&head, &entry.next);
        }
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
        dllist_reset(&head);
    }

    dllist_ctxt_t head;
};

// empty list
TEST_F(list_test, list_empty) {
    dllist_ctxt_t list;
    EXPECT_EQ(dllist_empty(&list), true);
}

// list entry
TEST_F(list_test, list_entry) {
    entry_t entry(10);
    int val = dllist_entry(&entry.next, entry_t, next)->val; 
    EXPECT_EQ(val, 10);
}

// list add
TEST_F(list_test, list_add) {
    entry_t entry(10);
    dllist_add(&head, &entry.next);
    vector<int> expected{10,1,2,3,4,5};
    EXPECT_EQ(tovec(&head), expected);
}

// list add tail
TEST_F(list_test, list_add_tail) {
    entry_t entry(10);
    dllist_add_tail(&head, &entry.next);
    vector<int> expected{1,2,3,4,5,10};
    EXPECT_EQ(tovec(&head), expected);
}

// list del
TEST_F(list_test, list_del) {
    dllist_ctxt_t *e, *n;
    dllist_for_each_safe(e, n, &head) {
        auto *entry = dllist_entry(e, entry_t, next);
        if (entry->val % 2 == 0)
            dllist_del(e);
    }
    vector<int> expected{1,3,5};
    EXPECT_EQ(tovec(&head), expected);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

