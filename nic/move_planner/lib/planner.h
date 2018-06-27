//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <vector>
#include <map>
#include "pal.h"
#include "metadata_parser.h"

#ifndef PLANNER_H
#define PLANNER_H
using namespace std;
/* 
 * Planner is the class which prepares the plan for moving of regions 
 * across upgrades. The regions are tagged with their names. Planner 
 * provides the final map and order to PAL allocator, to perform the moves
 */

namespace planner {
typedef enum plan_ret_e {
    PLAN_FAIL = 0,
    PLAN_SUCCESS,
    PLAN_CANNOT_PROCEED,
    PLAN_CATASTROPHIC
} plan_ret_t;

typedef struct planner_s {
    string current_hbm_json;		
    string target_hbm_json;
    uint64_t max_moveable;		// Max memory available for moveable 
    vector<region_t> input_map;         // Map of current region allocation
    map<string, region_t> expected_map; // Map of regions post upgrade
    vector<int> dependency_count;       // Count of regions a region is dependent on
    vector<vector<int> > dependent_count;// Adjacency Matrix
    vector<int> dependency;             // Order of region moves as per topo-sort
    vector<uint64_t> start_address;     // Start addresses of existing regions post-upgrade
    vector<uint64_t> end_address;       // End addresses of existing regions post-upgrade
    vector<string> added_regions;       // NEW regions added post-upgrade
    int free_space;                     // Additional space needed/to-be released
    uint64_t base_address;              // Base address of moveable region
} planner_t;

plan_ret_t setup_plan(string current_hbm_json, string target_hbm_json, uint64_t max_moveable, planner_t &plan, bool is_test);

plan_ret_t get_expected_map(vector<region_metadata_t> &raw_expected_map, planner_t &plan);     
plan_ret_t build_dependency_graph(planner_t &plan);
plan_ret_t topological_sort(planner_t &plan); 
plan_ret_t get_added_regions(planner_t &plan);
plan_ret_t determine_moves(planner_t &plan);
plan_ret_t alloc_added_regions(planner_t &plan);
plan_ret_t check_upgrade(planner_t &plan);
plan_ret_t prepare_plan(planner_t &plan);
plan_ret_t move_regions(planner_t &plan);

plan_ret_t plan_and_move(string current_json,
                         string target_json,
                         bool is_test);
}
#endif
