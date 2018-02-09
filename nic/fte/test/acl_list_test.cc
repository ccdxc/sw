#include <gtest/gtest.h>
#include <vector>

#include "nic/fte/acl/list.hpp"

using namespace acl;
using namespace std;

struct entry_t {
    ref_t ref_count;
    int   val;
};

static slab* entry_slab_ =
    slab::factory("test_entry", 0, sizeof(entry_t), 16, true, true, true);

static entry_t *entry_alloc(int val) {
    entry_t *entry = (entry_t*)entry_slab_->alloc();
    entry->val = val;
    ref_init(&entry->ref_count, [](const ref_t *entry) {
            entry_slab_->free((void*)entry);
        });
    return entry;
}

class acl_list_test : public ::testing::Test {
public:
    hal_ret_t insert(const list_t **listp, int val) {
        auto match = [](const void *val, const ref_t *entry) {
            return ((entry_t*)entry)->val >= ((entry_t*)val)->val;
        };
        entry_t *entry = entry_alloc(val);

        return list_t::insert(listp, &entry->ref_count, entry, match);
    }

    hal_ret_t remove(const list_t **listp, int val) {
        auto match = [](const void *val, const ref_t *entry) {
            return ((entry_t*)entry)->val ==  ((entry_t*)val)->val;
        };
        entry_t entry;
        entry.val = val;

        return list_t::remove(listp, &entry, match);
    }
    
    void collect(const list_t *list, vector<int> &v) {
        auto cb =  [](const void *v, const ref_t *entry) {
            ((vector<int> *)v)->push_back(((entry_t*)entry)->val);
            return true;
        };

        v.clear();
        list->walk(&v, cb);
    }

protected:
    acl_list_test(){}

    virtual ~acl_list_test(){}

    virtual void SetUp() {
    }

    virtual void TearDown() {
        EXPECT_EQ(entry_slab_->num_in_use(), 0);
        EXPECT_EQ(list_t::num_lists_in_use(), 0);
        EXPECT_EQ(list_t::num_items_in_use(), 0);
    }
};

TEST_F(acl_list_test, list_insert) {
    vector<int> in {1, 10, 20, 30, 5, 6};
    vector<int> out;

    const list_t *list = list_t::create();

    for (auto i : in) {
        EXPECT_EQ(insert(&list, i), HAL_RET_OK);
    }

    EXPECT_EQ(list_t::num_lists_in_use(), 1);
    EXPECT_EQ(list_t::num_items_in_use(), in.size());


    collect(list, out);
    sort(in.begin(), in.end());
    EXPECT_EQ(out, in);

    list->deref();
}

TEST_F(acl_list_test, list_clone) {
    vector<int> in {10, 20, 30, 5, 8};
    vector<int> in2 { 4, 1, 7, 2, 18};
    vector<int> out;

    const list_t *list = list_t::create();

    for (auto i : in) {
        EXPECT_EQ(insert(&list, i), HAL_RET_OK);
    }

    const list_t *list2 = list->clone();

    for (auto i : in2) {
        EXPECT_EQ(insert(&list2, i), HAL_RET_OK);
    }

    EXPECT_EQ(list->size(), in.size());
    EXPECT_EQ(list2->size(), in.size() + in2.size());

    EXPECT_EQ(list_t::num_lists_in_use(), 2);

    // make sure 20 and 30 are shared
    EXPECT_EQ(list_t::num_items_in_use(), list->size() +  list2->size() - 2);

    collect(list, out);
    sort(in.begin(), in.end());
    EXPECT_EQ(out, in);

    list->deref();


    collect(list2, out);
    for (auto i : in2) {
        in.push_back(i);
    }

    EXPECT_EQ(list_t::num_lists_in_use(), 1);
    EXPECT_EQ(list_t::num_items_in_use(), in.size());

    sort(in.begin(), in.end());
    EXPECT_EQ(out, in);

    list2->deref();
}

TEST_F(acl_list_test, list_remove) {
    vector<int> in {4, 1, 10, 20, 30, 5, 8};
    vector<int> in2 { 1, 10 };
    vector<int> exp { 4, 5, 8, 20, 30};
    vector<int> out;

    const list_t *list = list_t::create();

    for (auto i : in) {
        EXPECT_EQ(insert(&list, i), HAL_RET_OK);
    }

    const list_t *list2 = list->clone();

    for (auto i : in2) {
        EXPECT_EQ(remove(&list2, i), HAL_RET_OK);
    }

    for (auto i : in2) {
        EXPECT_EQ(remove(&list2, i), HAL_RET_ENTRY_NOT_FOUND);
    }

    EXPECT_EQ(list->size(), in.size());
    EXPECT_EQ(list2->size(), in.size() - in2.size());

    EXPECT_EQ(list_t::num_lists_in_use(), 2);

    // make sure 20 and 30 are shared
    EXPECT_EQ(list_t::num_items_in_use(), list->size() +  list2->size() - 2);

    collect(list, out);
    sort(in.begin(), in.end());
    EXPECT_EQ(out, in);

    list->deref();

    EXPECT_EQ(list_t::num_lists_in_use(), 1);
    EXPECT_EQ(list_t::num_items_in_use(), list2->size());

    collect(list2, out);
    EXPECT_EQ(out, exp);

    list2->deref();
}






