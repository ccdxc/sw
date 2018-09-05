#include <vector>
#include <iostream>
#include <string.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "metadata_parser.h"

// Short alias for this namespace
namespace pt = boost::property_tree;
using namespace std;

vector<region_metadata_t> metadata_read_region_map(string mem_json) {
    // Create a root
    pt::ptree root;
    vector<region_metadata_t> region_map;

    // Load the json file in this ptree
    read_json(mem_json, root);
    
    for(pt::ptree::value_type region : root.get_child("regions")) {
	region_metadata_t reg_meta;
        memset(&reg_meta, 0, sizeof(region_metadata_t));
	strcpy(reg_meta.name, region.second.get<std::string>("name").c_str());
	reg_meta.size_kb = region.second.get<int>("size_kb");
	reg_meta.entry_size = region.second.get<int>("entry_size");

	UPG_LOG_DEBUG("[META] REGION NAME : {}\tSIZE : {}", reg_meta.name, reg_meta.size_kb);
        region_map.push_back(reg_meta);
    }

    return region_map;
}
