//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <queue>
#include "pal.h"
#include "metadata_parser.h"
#include "planner.h"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

#define MAXMEMORY 65536 
using namespace std;
extern uint64_t base_address;

namespace planner {

planner::plan_ret_t
build_dependency_graph(planner::planner_t& plan) {
    uint32_t index = 0;

    for(index = 0; index < plan.input_map.size(); index++) {
        plan.dependency_count.push_back(0);
    }

    /* Build the Dependency graph - O(n**2) :( */
    for(uint32_t i = 0; i < plan.input_map.size(); i++) {
        vector<int> dependent_regions;

        /* Find out all the regions which are dependent on region at "i" */
        for(uint32_t j = 0; j < plan.input_map.size(); j++) {
        /* Check if the current input_map element's old address
         * falls within the address of new allocations.  */
            UPG_LOG_DEBUG("I : {}\t I : {}", i, j);
            UPG_LOG_DEBUG("INP START {} INP END {}",
                          plan.input_map[i].start_address - sizeof(region_t),
                          plan.input_map[i].start_address + plan.input_map[i].size);
            UPG_LOG_DEBUG("PLAN START {} PLAN END {}",
                          plan.start_address[j], 
                          plan.end_address[j]);

            if(i == j || 
                plan.input_map[i].start_address - sizeof(region_t) >= plan.end_address[j] ||
                plan.input_map[i].start_address + plan.input_map[i].size - 1 < plan.start_address[j] 
                ) {
		        continue;
            }

            UPG_LOG_DEBUG("DEPENDENT REGION : {}", j);
            UPG_LOG_DEBUG("INCREMENTING the dependency count for J[{}] I[{}] ", j, i);
            plan.dependency_count[j]++;
            dependent_regions.push_back(j);
        }

	    /* -1 indicates the end of the dependency list */
	    dependent_regions.push_back(-1);
        plan.dependent_count.push_back(dependent_regions);
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t 
get_added_regions(planner::planner_t &plan) {
    /* Find all newly added regions - O(n**2) - There should be a better way.*/
    for(map<string, region_t>::iterator it = plan.expected_map.begin();
        it != plan.expected_map.end();
        it++) {
        bool found = false;

        for(vector<region_t>::iterator it2 = plan.input_map.begin();
            it2 != plan.input_map.end();
            it2++) {

            if(strcmp((*it2).region_name, it->first.c_str()) == 0) {
                found = true;
                break;
            }
        }

        if(!found) {
            UPG_LOG_DEBUG("\nNEW REGION : {} REGION_NAME : {}",
                         it->first.c_str(),
                         plan.expected_map[it->first.c_str()].region_name);

            plan.added_regions.push_back(it->first);
        }
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t 
topological_sort(planner::planner_t &plan) {
    queue<int> top_q;

    for(uint32_t i = 0; i < plan.dependency_count.size(); i++) {
        if(plan.dependency_count[i] == 0) {
            top_q.push(i);
        }
    }

    while(!top_q.empty()) {
        int u = top_q.front();
        top_q.pop();
        plan.dependency.push_back(u);
        for(vector<int>::iterator it = plan.dependent_count[u].begin();
            it != plan.dependent_count[u].end();
            it++) {
          
            if(*it != -1) {
                plan.dependency_count[*it]--;

                if(plan.dependency_count[*it] == 0) {
                    top_q.push(*it);
                }
            }
        }
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
get_expected_map(vector<region_metadata_t> &raw_expected_map, planner::planner_t &plan) {
    vector<region_metadata_t>::iterator region_map_it;

    if(raw_expected_map.size() == 0) {
	UPG_LOG_ERROR("raw_expected_map's size is zero");
	return PLAN_FAIL;
    }

    if(plan.expected_map.size() != 0) {
	UPG_LOG_ERROR("plan.expected_map's size is zero");
	return PLAN_FAIL;
    }

    for(region_map_it = raw_expected_map.begin();
        region_map_it != raw_expected_map.end();
        region_map_it++) {
        region_t reg;

        if(strlen((*region_map_it).name) == 0) {
	    UPG_LOG_ERROR(":Name of region is zero");
	    return PLAN_FAIL;
 	}

        if(((*region_map_it).size_kb <= 0) ||
	   ((*region_map_it).entry_size <= 0) || 
           (((*region_map_it).size_kb) < (*region_map_it).entry_size)) {
  	    UPG_LOG_DEBUG("SIZE KB = {} \t ENTRY_SIZE = {}",
                          (*region_map_it).size_kb,
                          (*region_map_it).entry_size);
	    return PLAN_FAIL;
	}
     
        strcpy(reg.region_name, (*region_map_it).name);
        reg.size = (*region_map_it).size_kb;
        plan.expected_map.insert(pair<string, region_t>(reg.region_name, reg));
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
move_regions(planner::planner_t &plan) {
    uint32_t i = 0;
    char x_str[sizeof(region_t)];
    region_t ch;
    uint64_t from, to; 
    uint64_t entry_size;
    uint64_t entry_count;

    /* Loop bottom up to figure out the moves */
    for(i = 0; i < plan.dependency.size(); i++) {
        int cur_index = plan.dependency[i];
	    map<string, region_t>::iterator cur_meta_it;
	    string region_name = plan.input_map[cur_index].region_name;

        from = plan.input_map[cur_index].start_address - sizeof(region_t);
        pal_mem_read(from, (uint8_t*)&ch, sizeof(region_t));

        to = plan.start_address[cur_index];
        if(to == plan.end_address[cur_index]) {
            continue;
        }

	    cur_meta_it = plan.expected_map.find(region_name);
        ch.start_address = to + sizeof(region_t);
        entry_size = plan.input_map[cur_index].entry_size;
        entry_count = plan.input_map[cur_index].size/entry_size;
 
        ch.entry_size = cur_meta_it->second.entry_size;
	    ch.size = cur_meta_it->second.size;

        UPG_LOG_DEBUG("Bottom up move entry[{}] FROM : {} TO : {}, ENTRY SIZE : {} TOTAL SIZE : {}",
                      plan.input_map[cur_index].region_name,
                      from - base_address,
                      to - base_address,
                      ch.entry_size,
                      ch.size);

        bottom_up_move_entry(from + sizeof(region_t), to + sizeof(region_t), entry_size, entry_count);

        memset(x_str, 'X', sizeof(region_t));
        pal_mem_write(from, (uint8_t*)x_str, sizeof(region_t));
        pal_mem_write(to, (uint8_t*)&ch, sizeof(region_t));
    }

    return PLAN_SUCCESS;
}

uint64_t
setup_test_memory(string current_hbm_json) {
    vector<region_metadata_t> region_map;
    uint64_t base_address = setup_memory();

    region_map = metadata_read_region_map(current_hbm_json);

    for(vector<region_metadata_t>::iterator region_it = region_map.begin();
        region_it != region_map.end();
        region_it++) {
        UPG_LOG_DEBUG("FROM REGIONMAP - NAME : {}\t SIZE : {}",
                      (*region_it).name,
                      (*region_it).size_kb);

        pal_alloc_memory((*region_it).name, (*region_it).size_kb);
    }

    return base_address;
}

planner::plan_ret_t
setup_plan(string current_hbm_json,
                       string target_hbm_json,
                       uint64_t max_moveable,
		       planner::planner_t &plan,
                       bool is_test = false) {
    vector<region_metadata_t> region_map;
    vector<region_metadata_t> raw_expected_map;

    if(is_test) {
        plan.base_address = setup_test_memory(current_hbm_json);
    }

    plan.current_hbm_json = current_hbm_json;
    plan.target_hbm_json = target_hbm_json;
    plan.max_moveable = max_moveable;
    plan.input_map = pal_get_map();
    raw_expected_map = metadata_read_region_map(target_hbm_json);
    if(get_expected_map(raw_expected_map, plan) != PLAN_SUCCESS) {
	UPG_LOG_ERROR("Couldn't get the expected map.");
	return PLAN_FAIL;
    } 

    return PLAN_SUCCESS;
}

planner::plan_ret_t
determine_moves(planner::planner_t &plan) {
    uint32_t index = 0; 
    uint64_t cur_address_start = 0;
    plan.free_space = 0;

    /* Find out the amount of moves for each existing region. - O(n) */
    for(vector<region_t>::iterator it = plan.input_map.begin();
        it != plan.input_map.end();
        it++) {
        cur_address_start = plan.input_map[index].start_address - sizeof(region_t);

        if(plan.expected_map.count((*it).region_name) == 0) {
            plan.start_address.push_back(cur_address_start);
            plan.end_address.push_back(cur_address_start);
            /* Don't move if the region is to be deleted */
            plan.free_space += (*it).size + sizeof(region_t);
            UPG_LOG_DEBUG("--- JUST DELETED : {}", (*it).region_name);
        } else {
            plan.start_address.push_back(cur_address_start - plan.free_space);
            plan.end_address.push_back(plan.start_address[index]
            + plan.expected_map[(*it).region_name].size
            + sizeof(region_t));

            UPG_LOG_DEBUG("NAME : {} FREE SPACE : {}",
                         (*it).region_name,
                         plan.free_space);
            /* A negative value of free_space indicates that moveable region
             * has to grow/move down. */
            plan.free_space = plan.free_space
                         - plan.expected_map[(*it).region_name].size
                         + (*it).size;
        }

        index++;
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
alloc_added_regions(planner::planner_t &plan) {
    /* All the new regions are added at the bottom */
    for(vector<string>::iterator it = plan.added_regions.begin();
        it != plan.added_regions.end();
        it++) {
        UPG_LOG_DEBUG("Allocing {}", plan.expected_map[*it].region_name);
        pal_alloc_memory(plan.expected_map[*it].region_name,
                         plan.expected_map[*it].size);
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
check_upgrade(planner::planner_t &plan) {
    uint64_t space_needed = 0;

    if(determine_moves(plan) != PLAN_SUCCESS) {
        return PLAN_FAIL;
    }

    if(build_dependency_graph(plan) != PLAN_SUCCESS) {
        return PLAN_FAIL;
    }

    if(topological_sort(plan) != PLAN_SUCCESS) {
        return PLAN_FAIL;
    }

    if(get_added_regions(plan) != PLAN_SUCCESS) {
        return PLAN_FAIL;
    }

    for(vector<string>::iterator reg_it = plan.added_regions.begin();
	reg_it != plan.added_regions.end();
	reg_it++) {
	space_needed += plan.expected_map[*reg_it].size;
    }

    if(space_needed + plan.free_space > plan.max_moveable) {
	return PLAN_FAIL;
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
validate_region_move(string target_json) {
    /* Validate */
    vector<region_t> new_input_map = pal_get_map();
    vector<region_metadata_t> raw_expected_map;

    raw_expected_map = metadata_read_region_map(target_json);

    UPG_LOG_DEBUG("TEST 1 : Count of alloced region is as expected.");
    if(raw_expected_map.size() != new_input_map.size()) {
        UPG_LOG_ERROR("Expected region count[{}] and alloced region count[{}] don't match.",
                       raw_expected_map.size(), new_input_map.size());
        return PLAN_CATASTROPHIC;
    }

    UPG_LOG_DEBUG("TEST 2 : Ensure all requested regions have correct size.");
    for(vector<region_metadata_t>::iterator it = raw_expected_map.begin();
        it != raw_expected_map.end();
        it++) {
        bool found = false;
        UPG_LOG_DEBUG("Searching JSON region [{}] SIZE [{}] in PAL.",
                     it->name, it->size_kb);

        for(vector<region_t>::iterator it_inp = new_input_map.begin();
            it_inp != new_input_map.end();
            it_inp++) {
            UPG_LOG_DEBUG("PAL Region [{}] SIZE [{}]",
                         it_inp->region_name, it_inp->size);

            if(strcmp(it->name, it_inp->region_name) == 0) {
                if(it->size_kb != it_inp->size) {
                    UPG_LOG_ERROR("Memory validation failed.");
                        UPG_LOG_ERROR("Memory region [{}] had size [{}] in PAL, expected is [{}].",
                                       it->name,
                                       it_inp->size,
                                       it->size_kb);
                        return PLAN_CATASTROPHIC;
                }
 
                found = true;
                break;
           }
        }

        if(found == false) {
            UPG_LOG_ERROR("Memory Validation failed. Memory region [{}] was not found in PAL.",
                           it->name);
            return PLAN_CATASTROPHIC;
        }
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
plan_and_move(string current_json,
              string target_json,
	      bool is_test) {
     planner::planner_t plan;

     if(setup_plan(current_json,
                   target_json,
	/* TODO : Ensure this reflects the actual memory size instead of magic */
                   65536, 
                   plan,
                   is_test) != PLAN_SUCCESS) {
        UPG_LOG_ERROR("Plan setup failed.");
	return PLAN_FAIL;
    }

    UPG_LOG_DEBUG("<<<<<< STAGE 1 >>>>>>");
    check_upgrade(plan);
 
    UPG_LOG_DEBUG("<<<<<< STAGE 2 >>>>>>");
/*
    if(get_added_regions(plan) != PLAN_SUCCESS) {
        cout << "\nCannot get added regions"<<endl;
        return PLAN_FAIL;
    }
*/
    UPG_LOG_DEBUG("<<<<<< STAGE 3 >>>>>>");
    /* Negative free_space would mean, that additional space is required. */
    if(plan.free_space < 0) {
        pal_moveable_sbrk((-1) * plan.free_space);
    } 

    UPG_LOG_DEBUG("<<<<<< STAGE 4 >>>>>>");
    if(move_regions(plan) != PLAN_SUCCESS) {
        UPG_LOG_ERROR("Cannot move regions");
	return PLAN_CATASTROPHIC;
    }

    UPG_LOG_DEBUG("<<<<<< STAGE 5 >>>>>>");
    /* Positive free_space would mean, we must return memory back. */
    if(plan.free_space > 0) {
        pal_moveable_sbrk((-1) * plan.free_space);
    }

    UPG_LOG_DEBUG("<<<<<< STAGE 6 >>>>>>");
    if(alloc_added_regions(plan)!= PLAN_SUCCESS) {
        UPG_LOG_ERROR("Cannot alloc added regions");
        return PLAN_CATASTROPHIC;
    }

    if(validate_region_move(target_json) != PLAN_SUCCESS) {
	return PLAN_CATASTROPHIC;
    }

    return PLAN_SUCCESS;
}

} // END namespace planner
