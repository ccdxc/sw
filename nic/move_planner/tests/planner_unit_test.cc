#include <iostream>
#include "gtest/gtest.h"
#include "nic/move_planner/lib/planner.h"
#include "nic/move_planner/lib/metadata_parser.h"

class ExpectationTest : public ::testing::Test {
    protected:
	virtual void SetUp() {
	}

	virtual void TearDown() {
	}
};

/* NEGATIVE CASES */
TEST_F(ExpectationTest, EmptyRawMapTest ) {
    planner::planner_t plan;
    vector<region_metadata_t> raw_expected_map;

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

TEST_F(ExpectationTest, NonEmptyPlanExpectedMapTest ) {
    planner::planner_t plan;
    vector<region_metadata_t> raw_expected_map;
    region_t reg;

    plan.expected_map.insert(pair<string, region_t>("ABC", reg));

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

TEST_F(ExpectationTest, EmptyRawMapNameTest) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;

    memset(&region_meta, 0, sizeof(region_metadata_t));
    region_meta.size_kb = 10;
    region_meta.entry_size = 1;
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

TEST_F(ExpectationTest, ZeroSizeRawMapRegionTest ) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;

    region_meta.size_kb = 0;
    region_meta.entry_size = 1;
    strcpy(region_meta.name, "PENSANDO");
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

TEST_F(ExpectationTest, ZeroEntrySizeRawMapTest ) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;

    region_meta.size_kb = 10;
    region_meta.entry_size = 0;
    strcpy(region_meta.name, "PENSANDO");
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

TEST_F(ExpectationTest, SizeKBLessThanEntrySizeTest ) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;

    region_meta.size_kb = 12;
    region_meta.entry_size = 20;
    strcpy(region_meta.name, "PENSANDO");
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

TEST_F(ExpectationTest, SizeKBEntrySizeNonMultipleTest ) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;

    region_meta.size_kb = 12;
    region_meta.entry_size = 5;
    strcpy(region_meta.name, "PENSANDO");
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_FAIL);
}

/* POSITIVE CASES */
TEST_F(ExpectationTest, SingleRegionTest) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;
    map<string, region_t>::iterator exp_map_it;

    region_meta.size_kb = 12;
    region_meta.entry_size = 2;
    strcpy(region_meta.name, "PENSANDO");
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_SUCCESS);
    ASSERT_EQ(plan.expected_map.size(), 1);

    exp_map_it = plan.expected_map.find("PENSANDO"); 
    EXPECT_NE(exp_map_it, plan.expected_map.end());
    ASSERT_EQ(exp_map_it->second.size, region_meta.size_kb);
}

TEST_F(ExpectationTest, TwoRegionsTest) {
    planner::planner_t plan;
    region_metadata_t region_meta;
    vector<region_metadata_t> raw_expected_map;
    map<string, region_t>::iterator exp_map_it;

    region_meta.size_kb = 12;
    region_meta.entry_size = 2;
    strcpy(region_meta.name, "PENSANDO");
    raw_expected_map.push_back(region_meta);

    region_meta.size_kb = 48;
    region_meta.entry_size = 2;
    strcpy(region_meta.name, "CAPRI");
    raw_expected_map.push_back(region_meta);

    EXPECT_EQ(planner::get_expected_map(raw_expected_map, plan), planner::PLAN_SUCCESS);
    ASSERT_EQ(plan.expected_map.size(), 2);

    exp_map_it = plan.expected_map.find("CAPRI");
    EXPECT_NE(exp_map_it, plan.expected_map.end());
    ASSERT_EQ(exp_map_it->second.size, region_meta.size_kb);
}

class DependencyGraphTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {
        }
};

TEST_F(DependencyGraphTest, NoChangeTest) {
    planner::planner_t plan;
    		   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10));
    plan.start_address.push_back(3 * (sizeof(region_t) + 10));
 
    plan.end_address.push_back(sizeof(region_t) + 10);
    plan.end_address.push_back(2 * (sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));
    plan.end_address.push_back(4 * (sizeof(region_t) + 10));

    planner::build_dependency_graph(plan);

    for(uint32_t i = 0; i < plan.input_map.size(); i++) {
	ASSERT_EQ(plan.dependency_count[0], 0);
    }

    for(uint32_t i = 0; i < plan.input_map.size(); i++) {
        ASSERT_EQ(plan.dependent_count[i].size(), 1); // All will have "-1" pushed in
    }
}

TEST_F(DependencyGraphTest, TopGrowsBy30Test) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10 + 30);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10) + 30);
    plan.start_address.push_back(3 * (sizeof(region_t) + 10) + 30);

    plan.end_address.push_back(sizeof(region_t) + 10 + 30);
    plan.end_address.push_back(2 * (sizeof(region_t) + 10) + 30);
    plan.end_address.push_back(3 * (sizeof(region_t) + 10) + 30);
    plan.end_address.push_back(4 * (sizeof(region_t) + 10) + 30);

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 1);
    ASSERT_EQ(plan.dependency_count[1], 1);
    ASSERT_EQ(plan.dependency_count[2], 1);
    ASSERT_EQ(plan.dependency_count[3], 0);

    ASSERT_EQ(plan.dependent_count[0].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 2); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, TopShrinksBy5Test) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10 - 5);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10) - 5);
    plan.start_address.push_back(3 * (sizeof(region_t) + 10) - 5);

    plan.end_address.push_back(sizeof(region_t) + 10 - 5 );
    plan.end_address.push_back(2 * (sizeof(region_t) + 10) - 5);
    plan.end_address.push_back(3 * (sizeof(region_t) + 10) - 5);
    plan.end_address.push_back(4 * (sizeof(region_t) + 10) - 5);

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 1);
    ASSERT_EQ(plan.dependency_count[2], 1);
    ASSERT_EQ(plan.dependency_count[3], 1);

    ASSERT_EQ(plan.dependent_count[0].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 1); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, TopDeletedTest) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(0);
    plan.start_address.push_back((sizeof(region_t) + 10));
    plan.start_address.push_back(2 * (sizeof(region_t) + 10));

    plan.end_address.push_back(0);
    plan.end_address.push_back(sizeof(region_t) + 10);
    plan.end_address.push_back(2 * (sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 1);
    ASSERT_EQ(plan.dependency_count[2], 1);
    ASSERT_EQ(plan.dependency_count[3], 1);

    ASSERT_EQ(plan.dependent_count[0].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 1); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, OnlyBottomLeftTest) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10));
    plan.start_address.push_back(3 * (sizeof(region_t) + 10));
    plan.start_address.push_back(0);

    plan.end_address.push_back(0);
    plan.end_address.push_back(2 * (sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));
    plan.end_address.push_back((sizeof(region_t) + 10));

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 0);
    ASSERT_EQ(plan.dependency_count[2], 0);
    ASSERT_EQ(plan.dependency_count[3], 1);

    ASSERT_EQ(plan.dependent_count[0].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 1); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, OnlyTopLeftTest) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10));
    plan.start_address.push_back(3 * (sizeof(region_t) + 10));

    plan.end_address.push_back(sizeof(region_t) + 10);
    plan.end_address.push_back((sizeof(region_t) + 10));
    plan.end_address.push_back(2 * (sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 0);
    ASSERT_EQ(plan.dependency_count[2], 0);
    ASSERT_EQ(plan.dependency_count[3], 0);

    ASSERT_EQ(plan.dependent_count[0].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 1); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, BottomDeleteTest) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10));
    plan.start_address.push_back(3 * (sizeof(region_t) + 10));

    plan.end_address.push_back(sizeof(region_t) + 10);
    plan.end_address.push_back(2 *(sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 0);
    ASSERT_EQ(plan.dependency_count[2], 0);
    ASSERT_EQ(plan.dependency_count[3], 0);

    ASSERT_EQ(plan.dependent_count[0].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 1); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, BottomGrowsTest) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10));
    plan.start_address.push_back(3 * (sizeof(region_t) + 10));

    plan.end_address.push_back(sizeof(region_t) + 10);
    plan.end_address.push_back(2 *(sizeof(region_t) + 10));
    plan.end_address.push_back(3 * (sizeof(region_t) + 10));
    plan.end_address.push_back(5 * (sizeof(region_t) + 10));

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 0);
    ASSERT_EQ(plan.dependency_count[2], 0);
    ASSERT_EQ(plan.dependency_count[3], 0);

    ASSERT_EQ(plan.dependent_count[0].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 1); // All will have "-1" pushed in
}

TEST_F(DependencyGraphTest, MiddleGrowsTest) {
    planner::planner_t plan;
                   // NAME, start address, entry_size, size
    region_t reg1 = {"REGION-1", sizeof(region_t), 2, 10};
    region_t reg2 = {"REGION-2", 2 * sizeof(region_t) + 10, 2, 10};
    region_t reg3 = {"REGION-3", 3 * sizeof(region_t) + 2 * 10, 2, 10};
    region_t reg4 = {"REGION-4", 4 * sizeof(region_t) + 3 * 10, 2, 10};

    plan.input_map.push_back(reg1);
    plan.input_map.push_back(reg2);
    plan.input_map.push_back(reg3);
    plan.input_map.push_back(reg4);

    plan.start_address.push_back(0);
    plan.start_address.push_back(sizeof(region_t) + 10);
    plan.start_address.push_back(2 * (sizeof(region_t) + 10) + 30);
    plan.start_address.push_back(3 * (sizeof(region_t) + 10) + 30);

    plan.end_address.push_back(sizeof(region_t) + 10);
    plan.end_address.push_back(2 *(sizeof(region_t) + 10) + 30);
    plan.end_address.push_back(3 * (sizeof(region_t) + 10) + 30);
    plan.end_address.push_back(4 * (sizeof(region_t) + 10) + 30);

    planner::build_dependency_graph(plan);

    ASSERT_EQ(plan.dependency_count[0], 0);
    ASSERT_EQ(plan.dependency_count[1], 1);
    ASSERT_EQ(plan.dependency_count[2], 1);
    ASSERT_EQ(plan.dependency_count[3], 0);

    ASSERT_EQ(plan.dependent_count[0].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[1].size(), 1); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[2].size(), 2); // All will have "-1" pushed in
    ASSERT_EQ(plan.dependent_count[3].size(), 2); // All will have "-1" pushed in
}

class TopologicalSortTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {
        }
};

class DetermineMovesTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
        }

        virtual void TearDown() {
        }
};

int main(int argc, char **argv) {
	  ::testing::InitGoogleTest(&argc, argv);
	  return RUN_ALL_TESTS();
}
