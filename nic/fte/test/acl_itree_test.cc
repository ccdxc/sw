#include <gtest/gtest.h>
#include <vector>

#include "nic/fte/acl/itree.hpp"

using namespace acl;
using namespace std;

struct entry_t {
    ref_t ref_count;
    int val;
};

static slab* entry_slab_ =
    slab::factory("test_entry", 0, sizeof(entry_t), 10*1024, true, true, true);

static entry_t *entry_alloc(int val) {
    entry_t *entry = (entry_t*)entry_slab_->alloc();
    entry->val = val;
    ref_init(&entry->ref_count, [](const ref_t *entry) {
            entry_slab_->free((void*)entry);
        });
    return entry;
}

class acl_itree_test : public ::testing::Test {
public:
    hal_ret_t insert(const itree_t **treep, uint32_t low, uint32_t high, uint32_t val) {
        auto match = [](const void *val, const ref_t *entry) {
            return ((entry_t*)entry)->val >= ((entry_t*)val)->val;
        };
        entry_t *entry = entry_alloc(val);
        
        return itree_t::insert(treep, low, high, &entry->ref_count, entry, match);
    }

    hal_ret_t remove(const itree_t **treep, uint32_t low, uint32_t high, uint32_t val) {
        auto match = [](const void *val, const ref_t *entry) {
            return ((entry_t *)val)->val == ((entry_t*)entry)->val;
        };
        entry_t entry;
        entry.val = val;

        return itree_t::remove(treep, low, high, &entry, match);
    }
    
    void collect(const itree_t *tree, uint32_t low, uint32_t high, vector<int> &v) {
        auto cb =  [](const void *v, const ref_t *entry) {
            ((vector<int> *)v)->push_back(((entry_t*)entry)->val);
            return true;
        };

        v.clear();
        tree->walk(low, high, &v, cb);
    }

    void sort_intervals(vector<tuple<int, uint32_t, uint32_t>> &in)
    {
        sort(in.begin(), in.end(), [](tuple<int, uint32_t, uint32_t> a,
                                      tuple<int, uint32_t, uint32_t> b) {
                 if (std::get<1>(a) != std::get<1>(b))
                     return std::get<1>(a) < std::get<1>(b);
                 if (std::get<2>(a) != std::get<2>(b))
                     return std::get<2>(a) < std::get<2>(b);

                 return std::get<0>(a) < std::get<0>(b);

            });
    }

    void intervals_in_range(vector<tuple<int, uint32_t, uint32_t>> &in,
                            vector<int> &out, uint32_t low, uint32_t high,
                            std::function<bool(int)> filter = nullptr) {
        uint32_t l, h;
        int v;

        out.clear();
        for (auto entry: in) {
            tie(v, l, h) = entry;
            if (l <= high && h >= low) {
                if (!filter || !filter(v)) {
                    out.push_back(v);
                }
            }
        }
    }

    static int myrandom (int i) { return std::rand()%i;}

protected:
    acl_itree_test(){}

    virtual ~acl_itree_test(){}

    virtual void SetUp() {
        std::srand(1);
    }

    virtual void TearDown() {
        EXPECT_EQ(entry_slab_->num_in_use(), 0);
        EXPECT_EQ(itree_t::num_trees_in_use(), 0);
        EXPECT_EQ(itree_t::num_nodes_in_use(), 0);
        EXPECT_EQ(list_t::num_lists_in_use(), 0);
        EXPECT_EQ(list_t::num_items_in_use(), 0);
    }

};

vector<tuple<int, uint32_t, uint32_t>> intervals = {
    {1, 0, 3}, {2, 5, 8}, {3, 6, 10}, {4, 8, 9}, {5, 15, 23},
    {6, 16, 21}, {7, 17, 19}, {8, 19, 20}, {9, 25, 30}, {10, 26, 26},
    {11, 8, 9}, {12, 25, 30}, {13, 6, 10}, {14, 16, 21}, {15, 16, 21}
};
vector<pair<uint32_t, uint32_t>> search_intervals = {
    {0,30}, {18,18}, {5, 16}, {13,14}
};

TEST_F(acl_itree_test, itree_insert)
{

    uint32_t low, high;
    int val;

    sort_intervals(intervals);

    for (int i = 0; i < 10; i++) {
        const itree_t *tree = itree_t::create();
        std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
        for (auto entry: intervals) {
            tie(val, low, high) = entry;
            EXPECT_EQ(insert(&tree, low, high, val), HAL_RET_OK);
            EXPECT_GE(tree->check(), 0);
        }
    
        sort_intervals(intervals);

        for (auto range: search_intervals) {
            vector<int> out, expected;
            collect(tree, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second);
            EXPECT_EQ(out, expected);
        }

        tree->deref();
    }
}

TEST_F(acl_itree_test, itree_clone)
{
    uint32_t low, high;
    int val;

    sort_intervals(intervals);

    for (int i = 0; i < 10; i++) {
        const itree_t *tree = itree_t::create();

        // insert all even entries
        std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
        for (auto entry: intervals) {
            tie(val, low, high) = entry;
            if (val % 2 == 0) {
                EXPECT_EQ(insert(&tree, low, high, val), HAL_RET_OK);
                EXPECT_GE(tree->check(), 0);
            }
        }
    
        // insert all odd
        const itree_t *tree2 = tree->clone();
        std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
        for (auto entry: intervals) {
            tie(val, low, high) = entry;
            if (val % 2 == 1) {
                EXPECT_EQ(insert(&tree2, low, high, val), HAL_RET_OK);
                EXPECT_GE(tree->check(), 0);
            }
        }

        sort_intervals(intervals);

        for (auto range: search_intervals) {
            vector<int> out, expected;

            // check even tree
            collect(tree, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second,
                               [](int val) { return val % 2 != 0; });
            EXPECT_EQ(out, expected);

            // check tree2
            collect(tree2, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second);
            EXPECT_EQ(out, expected);
        }

        tree->deref();
    
        // check tree2 is still valid
        for (auto range: search_intervals) {
            vector<int> out, expected;

            collect(tree2, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second);
            EXPECT_EQ(out, expected);
        }
            
    
        tree2->deref();
    }
}

TEST_F(acl_itree_test, itree_remove)
{
    uint32_t low, high;
    int val;

    sort_intervals(intervals);

    for (int i = 0; i < 10; i++) {
        const itree_t *tree = itree_t::create();

        // insert all entries
        std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
        for (auto entry: intervals) {
            tie(val, low, high) = entry;
            EXPECT_EQ(insert(&tree, low, high, val), HAL_RET_OK);
            EXPECT_GE(tree->check(), 0);
        }
        
        // remove all odd
        const itree_t *tree2 = tree->clone();

        for (auto entry: intervals) {
            tie(val, low, high) = entry;
            if (val % 2 == 1) {
                EXPECT_EQ(remove(&tree2, low, high, val), HAL_RET_OK);
                EXPECT_GE(tree->check(), 0);
                EXPECT_GE(tree2->check(), 0);
            }
        }


        sort_intervals(intervals);

        for (auto range: search_intervals) {
            vector<int> out, expected;

            // check tree
            collect(tree, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second);
            EXPECT_EQ(out, expected);

            // check even tree
            collect(tree2, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second,
                               [](int val) { return val % 2 != 0; });
            EXPECT_EQ(out, expected);
        }
        
        
        tree->deref();
        
        // check tree2 is still valid
        for (auto range: search_intervals) {
            vector<int> out, expected;
            
            collect(tree2, range.first, range.second, out);
            intervals_in_range(intervals, expected, range.first, range.second,
                               [](int val) { return val % 2 != 0; });
            EXPECT_EQ(out, expected);
        }

        tree2->deref();
    }
}

TEST_F(acl_itree_test, itree_random)
{
    uint32_t low, high;
    int val;

    const int num_intervals = 10000, max_range = 255, max_low = 1000; 

    // genenrate random intervals
    vector<tuple<int, uint32_t, uint32_t>> intervals;
    for (int i = 0; i < num_intervals; i++) {
        uint32_t low = myrandom(max_low);
        uint32_t high = low + myrandom(max_range);
        intervals.push_back({i, low, high});
    }

    // insert all entries
    const itree_t *tree = itree_t::create();
    std::random_shuffle(intervals.begin(), intervals.end(), myrandom);

    for (auto entry: intervals) {
        tie(val, low, high) = entry;
        EXPECT_EQ(insert(&tree, low, high, val), HAL_RET_OK);
    }
    EXPECT_GE(tree->check(), 0);

    // remove all even entries 
    const itree_t *tree_odd = tree->clone();
    std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
    for (auto entry: intervals) {
        tie(val, low, high) = entry;
        if (val % 2 == 0) {
            EXPECT_EQ(remove(&tree_odd, low, high, val), HAL_RET_OK);
        }
    }
    EXPECT_GE(tree_odd->check(), 0);


    // add all even and remove odd
    const itree_t *tree_even = tree_odd->clone();
    std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
    for (auto entry: intervals) {
        tie(val, low, high) = entry;
        if (val % 2 != 0) {
            EXPECT_EQ(remove(&tree_even, low, high, val), HAL_RET_OK);
        } else {
            EXPECT_EQ(insert(&tree_even, low, high, val), HAL_RET_OK);
        }
    }
    EXPECT_GE(tree_even->check(), 0);

    // remove all even
    const itree_t *tree_empty = tree_even->clone();
    std::random_shuffle(intervals.begin(), intervals.end(), myrandom);
    for (auto entry: intervals) {
        tie(val, low, high) = entry;
        if (val % 2 == 0) {
            EXPECT_EQ(remove(&tree_empty, low, high, val), HAL_RET_OK);
        }
    }
    EXPECT_GE(tree_empty->check(), 0);

    
    // check trees
    vector<int> out, expected;

    sort_intervals(intervals);

    collect(tree, 0, ~0, out);
    intervals_in_range(intervals, expected, 0, ~0);
    EXPECT_EQ(out, expected);
    tree->deref();
    
    
    // check even tree
    collect(tree_even, 0, ~0, out);
    intervals_in_range(intervals, expected, 0, ~0,
                       [](int v) { return v % 2 != 0;});
    EXPECT_EQ(out, expected);
    tree_even->deref();

    // check odd tree
    collect(tree_odd, 0, ~0, out);
    intervals_in_range(intervals, expected, 0, ~0,
                       [](int v) { return v % 2 == 0;});
    EXPECT_EQ(out, expected);
    tree_odd->deref();

    // check empty tree
    collect(tree_empty, 0, ~0, out);
    EXPECT_EQ(out, vector<int>{});
    
    tree_empty->deref();     
}

void timeit(const std::string &msg, int count, std::function<void()> fn)
{
    cout << msg << " " << count << " " << std::flush;

    std::clock_t start = clock();
    fn();
    int ticks = clock()-start;

    cout << " (" << 1000.0*ticks/CLOCKS_PER_SEC << " ms) ";
    if (count) {
        cout << count*CLOCKS_PER_SEC/ticks << "/sec";
    }
    cout << "\n";
}

TEST_F(acl_itree_test, itree_benchmark)
{
    uint32_t low, high;
    int val;

    const int num_intervals = 1000000,
        max_low = 100*num_intervals,
        max_range = 255,
        num_updates = num_intervals/2,
        num_lookups = num_intervals;
    

    // genenrate intervals
    vector<tuple<int, uint32_t, uint32_t>> intervals;
    for (int i = 0; i < num_intervals; i++) {
        uint32_t low = myrandom(max_low);
        uint32_t high = low + myrandom(max_range);
        intervals.push_back({i, low, high});
    }
    
    // insert all entries
    const itree_t *tree = itree_t::create();
    timeit("insert", intervals.size(), [&]() {
            for (auto entry: intervals) {
                tie(val, low, high) = entry;
                EXPECT_EQ(insert(&tree, low, high, val), HAL_RET_OK);
            }
        });

    
    // lookup cost
    vector<int> out;
    vector<int> vals; // random vals 
    for (int i = 0; i < num_lookups; i++) {
        vals.push_back(myrandom(max_low+max_range));
    }

    int matches;
    timeit("lookup", num_lookups, [&]() {
            for (int i = 0; i < num_lookups; i++) {
                collect(tree, vals[i], vals[i], out);
                matches += out.size();;
            }
        });

    cout << "matches " << matches << "\n";

    // update cost
    vector<tuple<int, uint32_t, uint32_t>> intervals2;
    for (int i = 0; i < num_updates; i++) {
        uint32_t low = myrandom(max_low);
        uint32_t high = low + myrandom(max_range);
        intervals2.push_back({i, low, high});
    }
    
    const itree_t *tree2 = tree->clone();
    timeit("updates", num_updates*2, [&]() {
            for (int i = 0; i < num_updates; i++) {
                tie(val, low, high) = intervals2[i];
                EXPECT_EQ(insert(&tree2, low, high, val), HAL_RET_OK);
                tie(val, low, high) = intervals[i];
                EXPECT_EQ(remove(&tree2, low, high, val), HAL_RET_OK);
            }
        });

    cout << "tree depth: " << tree->check() << "\n";
    cout << "tree2 depth: " << tree->check() << "\n";

    timeit("deref", 0, [&]() {
            tree->deref();
            tree2->deref();
        });
}

