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
        // Find out all the regions which are dependent on region at "i"
        for(uint32_t j = 0; j < plan.input_map.size(); j++) {
        /* Check if the current input_map element's old address
         * falls within the address of new allocations.  */
            cout << "\n I : "<< i << " J : " << j;
            cout << "\nINP START " << std::hex << plan.input_map[i].start_address - sizeof(region_t);
            cout << "\nINP END " << std::hex << plan.input_map[i].start_address + plan.input_map[i].size;
            cout << "\nPLAN START " << std::hex << plan.start_address[j];
            cout << "\nPLAN END " << std::hex << plan.end_address[j] << endl;

            if(i == j || 
                plan.input_map[i].start_address - sizeof(region_t) >= plan.end_address[j] ||
                plan.input_map[i].start_address + plan.input_map[i].size - 1 < plan.start_address[j] 
                ) {
		continue;
            }

            printf("\nDEPENDENT REGION : %d", j);
            plan.dependency_count[j]++;
            dependent_regions.push_back(j);
            cout << "\nINCREMENTING the dependency count for J " << j
                 << " I " << i << endl;
	    cout << "DEP "<< plan.dependency_count[j] << endl;
        }

	// -1 indicates the end of the dependency list
	dependent_regions.push_back(-1);
        plan.dependent_count.push_back(dependent_regions);
    }

    cout << "\nDEP COUNT " << plan.dependency_count.size()
         << " DEP MAT GRAPH " << plan.dependent_count.size() << endl ;

    return PLAN_SUCCESS;
}

planner::plan_ret_t 
get_added_regions(planner::planner_t &plan) {
    /* Find all newly added regions - O(n**2) - There should be a better way.*/
    for(map<string, region_t>::iterator it = plan.expected_map.begin();
        it != plan.expected_map.end();
        it++) {
        bool found = false;

        cout<<"\nSEARCHING : " << it->first;

        for(vector<region_t>::iterator it2 = plan.input_map.begin();
            it2 != plan.input_map.end();
            it2++) {

            if(strcmp((*it2).region_name, it->first.c_str()) == 0) {
                found = true;
                break;
            }
        }

        if(!found) {
            printf("\nNEW REGION : %s REGION_NAME : %s",
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
    cout<<"\nTOPO SORT Dep Size : " << plan.dependency_count.size() << endl;

    for(uint32_t i = 0; i < plan.dependency_count.size(); i++) {
        if(plan.dependency_count[i] == 0) {
            top_q.push(i);
            cout<<"\nTOPO SORT Q " << i <<endl;
        }
    }

    while(!top_q.empty()) {
        int u = top_q.front();
        top_q.pop();
        plan.dependency.push_back(u);
        cout<<"\nTOPO SORT U = " << u <<endl;
        for(vector<int>::iterator it = plan.dependent_count[u].begin();
            it != plan.dependent_count[u].end();
            it++) {
          
            cout<<"\nTOPO SORT IT : " << *it <<endl;
            
            if(*it != -1) {
                plan.dependency_count[*it]--;

                if(plan.dependency_count[*it] == 0) {
                    top_q.push(*it);
                    cout<<"\nTOPO SORT PUSHING " << *it <<endl;
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
	cout << endl << __FUNCTION__ << ":raw_expected_map's size is zero";
	return PLAN_FAIL;
    }

    if(plan.expected_map.size() != 0) {
	cout << endl << __FUNCTION__ << ":plan.expected_map's size is zero";
	return PLAN_FAIL;
    }

    for(region_map_it = raw_expected_map.begin();
        region_map_it != raw_expected_map.end();
        region_map_it++) {
        region_t reg;

        if(strlen((*region_map_it).name) == 0) {
	    cout << endl << __FUNCTION__ << ":Name of region is zero";
	    return PLAN_FAIL;
 	}

        if(((*region_map_it).size_kb <= 0) ||
	   ((*region_map_it).entry_size <= 0) || 
           (((*region_map_it).size_kb) < (*region_map_it).entry_size)) {
           //(((*region_map_it).size_kb * 1024) % (*region_map_it).entry_size != 0)) {
  	    cout << endl << __FUNCTION__ << ": SIZE KB = "<< (*region_map_it).size_kb
                 << " ENTRY SIZE = " << (*region_map_it).entry_size << endl;;
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

    cout << "\nPLANNER MOVE REGIONS : dependency_size " << plan.dependency.size(); 

    /* Loop bottom up to figure out the moves */
    for(i = 0; i < plan.dependency.size(); i++) {
        int cur_index = plan.dependency[i];
        from = plan.input_map[cur_index].start_address - sizeof(region_t);
        pal_mem_read(from, (uint8_t*)&ch, sizeof(region_t));

        to = plan.start_address[cur_index];
        if(to == plan.end_address[cur_index]) {
            continue;
        }

        ch.start_address = to + sizeof(region_t);

        entry_size = plan.input_map[cur_index].entry_size;
        entry_count = plan.input_map[cur_index].size/entry_size;
        //printf("\nBottom up move entry[%s] FROM : %lx TO : %lx, ENTRY SIZE : %x, ENTRY_COUNT : %x\n\n", input_map[cur_index].region_name, from - base_address, to - base_address, entry_size, entry_count);
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
        cout << "FROM REGIONMAP - NAME : "<<(*region_it).name << " SIZE " << (*region_it).size_kb << endl;
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
	cout << "\nCouldn't get the expected map." ;
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
            printf("\n--- JUST DELETED : %s", (*it).region_name);
        } else {
            plan.start_address.push_back(cur_address_start - plan.free_space);
            plan.end_address.push_back(plan.start_address[index] + plan.expected_map[(*it).region_name].size + sizeof(region_t));

            printf("\nNAME : %s FREE SPACE : %d",(*it).region_name, plan.free_space);
            /* A negative value of free_space indicates that moveable region has to grow/move down */
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
        printf("\nAllocing %s", plan.expected_map[*it].region_name);
        pal_alloc_memory(plan.expected_map[*it].region_name, plan.expected_map[*it].size);
    }

    return PLAN_SUCCESS;
}

planner::plan_ret_t
check_upgrade(planner::planner_t &plan) {
    uint64_t space_needed = 0;

    //cout << "\nINPUT MAP SIZE : " << plan.input_map.size();
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
plan_and_move(string current_json,
              string target_json,
	      bool is_test) {
     planner::planner_t plan;

     if(setup_plan(current_json,
                   target_json,
                   65536, 
                   plan,
                   is_test) != PLAN_SUCCESS) {
        cout << "\nPlan setup failed."<<endl;
	return PLAN_FAIL;
    }

    cout << "\n<<<<<< STAGE 1 >>>>>>"<<endl;
    check_upgrade(plan);
 
    cout << "\n<<<<<< STAGE 2 >>>>>>"<<endl;
/*
    if(get_added_regions(plan) != PLAN_SUCCESS) {
        cout << "\nCannot get added regions"<<endl;
        return PLAN_FAIL;
    }
*/
    cout << "\n<<<<<< STAGE 3 >>>>>>"<<endl;
    /* Negative free_space would mean, that additional space is required. */
    if(plan.free_space < 0) {
        pal_moveable_sbrk((-1) * plan.free_space);
    } 

    cout << "\n<<<<<< STAGE 4 >>>>>>"<<endl;
    if(move_regions(plan) != PLAN_SUCCESS) {
        cout << "\nCannot move regions"<<endl;
	return PLAN_CATASTROPHIC;
    }

    cout << "\n<<<<<< STAGE 5 >>>>>>"<<endl;
    /* Positive free_space would mean, we must return memory back. */
    if(plan.free_space > 0) {
        pal_moveable_sbrk((-1) * plan.free_space);
    }

    cout << "\n<<<<<< STAGE 6 >>>>>>"<<endl;
    if(alloc_added_regions(plan)!= PLAN_SUCCESS) {
        cout << "\nCannot alloc added regions"<<endl;
        return PLAN_CATASTROPHIC;
    }

    return PLAN_SUCCESS;
}

}
