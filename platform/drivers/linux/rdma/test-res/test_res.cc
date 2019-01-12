#include <random>
#include <gtest/gtest.h>
#include "wrap_res.h"

#define BIT(n) (1u << (n))

class Buddy : public ::testing::Test {
protected:
	Buddy() {
		order_max = 20;
		buddy_init(&buddy, BIT(order_max));
	}
	~Buddy() {
		check_invariants();
		buddy_destroy(&buddy);
	}

	void check_order_next(int order) const {
		int pos = bitmap_find_free_region(buddy.inuse,
						  buddy.inuse_size,
						  order);

		if (pos >= 0) {
			// if it was found just above, release it here
			bitmap_release_region(buddy.inuse, pos, order);

			// if it could be found, it must not be skipped
			EXPECT_LE(buddy.order_next[order], BIT_WORD(pos))
				<< "order: " << order;
		}
	}

	void check_order_next_all() const {
		for (int i = 0; i <= buddy.order_max; ++i)
			check_order_next(i);
	}

	void check_invariants() const {
		check_order_next_all();
	}

	struct buddy_bits buddy;
	int order_max;
};

TEST_F(Buddy, InitMaxOrder) {
	EXPECT_EQ(buddy.order_max, order_max);
}

TEST_F(Buddy, OrderZero) {
	int pos, i;

	for (i = 0; i < BITS_PER_LONG; ++i) {
		EXPECT_EQ(buddy.order_next[0], 0);
		EXPECT_EQ(buddy.order_next[1], 0);
		EXPECT_EQ(buddy.order_next[buddy.order_max], 0);

		pos = buddy_get(&buddy, 0);

		EXPECT_EQ(pos, i);
	}

	for (i = 0; i < BITS_PER_LONG; ++i) {
		EXPECT_EQ(buddy.order_next[0], 1);
		EXPECT_EQ(buddy.order_next[1], 1);
		EXPECT_EQ(buddy.order_next[buddy.order_max], BIT_WORD(BIT(buddy.order_max)));

		pos = buddy_get(&buddy, 0);

		EXPECT_EQ(pos, i + BITS_PER_LONG);
	}

	EXPECT_EQ(buddy.order_next[0], 2);
	EXPECT_EQ(buddy.order_next[1], 2);
	EXPECT_EQ(buddy.order_next[buddy.order_max], BIT_WORD(BIT(buddy.order_max)));

	buddy_put(&buddy, BITS_PER_LONG-1, 0);

	EXPECT_EQ(buddy.order_next[0], 0);
	EXPECT_EQ(buddy.order_next[1], 0);
	EXPECT_EQ(buddy.order_next[buddy.order_max], 0);

	pos = buddy_get(&buddy, 0);

	EXPECT_EQ(pos, BITS_PER_LONG-1);
	EXPECT_EQ(buddy.order_next[0], 1);
	EXPECT_EQ(buddy.order_next[1], 1);
	EXPECT_EQ(buddy.order_next[buddy.order_max], BIT_WORD(BIT(buddy.order_max)));

	pos = buddy_get(&buddy, 0);

	EXPECT_EQ(pos, 2*BITS_PER_LONG);
	EXPECT_EQ(buddy.order_next[0], 2);
	EXPECT_EQ(buddy.order_next[1], 2);
	EXPECT_EQ(buddy.order_next[buddy.order_max], BIT_WORD(BIT(buddy.order_max)));
}

TEST_F(Buddy, OrderMany) {
	int pos;

	pos = buddy_get(&buddy, 1);
	EXPECT_EQ(pos, 0);
	pos = buddy_get(&buddy, 9);
	EXPECT_EQ(pos, BIT(9));

	buddy_put(&buddy, BIT(9), 9);
	pos = buddy_get(&buddy, 9);
	EXPECT_EQ(pos, BIT(9));
	pos = buddy_get(&buddy, 9);
	EXPECT_EQ(pos, 2*BIT(9));

	buddy_put(&buddy, BIT(9), 9);
	pos = buddy_get(&buddy, 9);
	EXPECT_EQ(pos, BIT(9));
	pos = buddy_get(&buddy, 9);
	EXPECT_EQ(pos, 3*BIT(9));

	pos = buddy_get(&buddy, 0);
	EXPECT_EQ(pos, 2);

	pos = buddy_get(&buddy, 8);
	EXPECT_EQ(pos, BIT(8));

	pos = buddy_get(&buddy, 6);
	EXPECT_EQ(pos, BIT(6));
	pos = buddy_get(&buddy, 6);
	EXPECT_EQ(pos, 2*BIT(6));

	pos = buddy_get(&buddy, 10);
	EXPECT_EQ(pos, 2*BIT(10));
}

TEST_F(Buddy, OrderTenEleven) {
	int pos_10, pos_11;

	// A failing sequence found by randomized test
	// Made into this unit test and fixed:
	pos_10 = buddy_get(&buddy, 10);
	pos_11 = buddy_get(&buddy, 11);
	buddy_put(&buddy, pos_10, 10);
	buddy_put(&buddy, pos_11, 11);
	// invariants checked in destructor
}

TEST_F(Buddy, OrderZeroFive) {
	// A failing sequence found by randomized test
	// Made into this unit test and fixed:
	buddy_get(&buddy, 0);
	buddy_get(&buddy, 5);
	// invariants checked in destructor
}

TEST_F(Buddy, Randomized) {
	// buddy_get or put? , pos , order
	std::vector<std::tuple<bool,int,int>> hist;
	// pos -> order
	std::map<int,int> buds;
	// random source
	std::mt19937 rng;

	int iter, count = 5000;
	bool inject_failure;

	// usage:  INJECT_FAILURE= ./test_res [test options...]
	inject_failure = !!getenv("INJECT_FAILURE");

	// randomize with --gtest_shuffle
	rng.seed(::testing::UnitTest::GetInstance()->random_seed());

	for (iter = 0; iter < count; ++iter) {
		int pos, order;

		if (buds.empty() || !(rng() & 1)) {
			order = __builtin_popcount(rng() & ((1ull << buddy.order_max) - 1));
			pos = buddy_get(&buddy, order);

			hist.push_back(std::make_tuple(true, pos, order));

			if (pos >= 0)
				buds[pos] = order;
		} else {
			auto it = buds.begin();

			std::advance(it, rng() % buds.size());

			pos = it->first;
			order = it->second;

			buddy_put(&buddy, pos, order);

			hist.push_back(std::make_tuple(false, pos, order));

			it = buds.erase(it);
		}

		check_invariants();

		if (inject_failure && !(rng() & 0x3f))
			ADD_FAILURE() << "injecting test failure";

		// XXX should be just if this one test case failed
		if (::testing::UnitTest::GetInstance()->Failed())
			goto show_hist;
	}

	return;

show_hist:
	std::stringstream hist_msg;

	hist_msg << "history of operations:";

	for(auto it = hist.begin(); it != hist.end(); ++it) {
		bool is_get;
		int pos, order;

		std::tie(is_get, pos, order) = *it;

		hist_msg << std::endl;

		if (is_get) {
			hist_msg << "  get(" << order << ") -> " << pos << ";";
		} else {
			hist_msg << "    put(" << pos << ", " << order << ");";
		}
	}

	GTEST_FAIL() << hist_msg.str();
}
