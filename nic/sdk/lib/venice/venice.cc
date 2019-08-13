// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "lib/venice/venice.hpp"
#include "include/sdk/mem.hpp"
#include "lib/utils/utils.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include<iostream>

namespace sdk {
namespace lib {

using boost::property_tree::ptree;
using namespace std;

venice *
venice::factory(std::string naples_status_file)
{
    void       *mem;
    venice     *new_venice;
    sdk_ret_t  ret = SDK_RET_OK;

    if (naples_status_file.empty()) {
        naples_status_file = DEFAULT_VENICE_FILE0;
    }

    SDK_TRACE_DEBUG("naples_status_file: %s", naples_status_file.c_str());

    mem = SDK_CALLOC(sdk::SDK_MEM_ALLOC_DEVICE, sizeof(venice));
    if (!mem) {
        SDK_TRACE_ERR("Failed to allocate memory for venice");
        return NULL;
    }

    new_venice = new (mem) venice();
    ret = new_venice->init(naples_status_file);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to init venice");
        new_venice->~venice();
        SDK_FREE(sdk::SDK_MEM_ALLOC_DEVICE, new_venice);
        return NULL;
    }

    return new_venice;
}


venice::~venice()
{
}

void
venice::destroy(venice *dev)
{
    if (!dev) {
        return;
    }
    dev->~venice();
    SDK_FREE(sdk::SDK_MEM_ALLOC_DEVICE, dev);
}

sdk_ret_t
venice::init(std::string &naples_status_file)
{
    sdk_ret_t ret;
    ptree prop_tree;

    venice_db_.naples_status_file = naples_status_file;
    ret = get_ptree_(naples_status_file, prop_tree);
    if (ret != SDK_RET_OK) {
        // No venice.conf file ... return defaults
        populate_venice_defaults_();
        return SDK_RET_OK;
    }

    return populate_venice(prop_tree);
}

sdk_ret_t
venice::get_ptree_(std::string& naples_status_file, ptree& prop_tree)
{
    if (access(naples_status_file.c_str(), R_OK) < 0) {
        SDK_TRACE_ERR("naples_status_file %s has no read permissions",
                      naples_status_file.c_str());
        return SDK_RET_ERR;
    }
    SDK_TRACE_DEBUG("naples_status_file has permission %s", naples_status_file.c_str());

    try {
	usleep(100);
        read_json(naples_status_file.c_str(), prop_tree);
    } catch (exception const& e) {
        SDK_TRACE_DEBUG("exception %s", e.what());
    }
    return SDK_RET_OK;
}

sdk_ret_t
venice::populate_venice(ptree &root)
{
    populate_venice_defaults_();

    BOOST_FOREACH(ptree::value_type &v, root)
    {
	if (!strcmp(v.first.c_str(), "NaplesMode")) {
            venice_db_.naplesMode = v.second.data().c_str();
            SDK_TRACE_DEBUG("NaplesMode : %s", venice_db_.naplesMode.c_str());
        }
        if (!strcmp(v.first.c_str(), "TransitionPhase")) {
            venice_db_.transitionPhase = v.second.data().c_str();
            SDK_TRACE_DEBUG("TransitionPhase: %s", venice_db_.transitionPhase.c_str());
        }
        if (!strcmp(v.first.c_str(), "ID")) {
            venice_db_.id = v.second.data().c_str();
            SDK_TRACE_DEBUG("ID: %s", venice_db_.id.c_str());
        }
        if (!strcmp(v.first.c_str(), "MgmtIP")) {
            venice_db_.mgmtIp = v.second.data().c_str();
            SDK_TRACE_DEBUG("MgmtIP: %s", venice_db_.mgmtIp.c_str());
        }
        if (!strcmp(v.first.c_str(), "SmartNicName")) {
            venice_db_.smartNicName = v.second.data().c_str();
            SDK_TRACE_DEBUG("SmartNicName: %s", venice_db_.smartNicName.c_str());
        }
    }
    BOOST_FOREACH(ptree::value_type &v, root.get_child("Controllers"))
    {
        venice_db_.controllers.push_back(v.second.data().c_str());
        SDK_TRACE_DEBUG("Controller: %s", v.second.data().c_str());
    }
    BOOST_FOREACH(ptree::value_type &it, root.get_child("Fru"))
    {
        if (!strcmp(it.first.c_str(), "ManufacturingDate")) {
            venice_db_.fru.manufacturingDate = it.second.data().c_str();
            SDK_TRACE_DEBUG("ManufacturingDate: %s", venice_db_.fru.manufacturingDate.c_str());
        }
        if (!strcmp(it.first.c_str(), "Manufacturer")) {
            venice_db_.fru.manufacturer = it.second.data().c_str();
            SDK_TRACE_DEBUG("Manufacturer: %s", venice_db_.fru.manufacturer.c_str());
        }
        if (!strcmp(it.first.c_str(), "ProductName")) {
            venice_db_.fru.productName = it.second.data().c_str();
            SDK_TRACE_DEBUG("ProductName: %s", venice_db_.fru.productName.c_str());
        }
        if (!strcmp(it.first.c_str(), "SerialNum")) {
            venice_db_.fru.serialNum = it.second.data().c_str();
            SDK_TRACE_DEBUG("SerialNum: %s", venice_db_.fru.serialNum.c_str());
        }
        if (!strcmp(it.first.c_str(), "PartNum")) {
            venice_db_.fru.partNum = it.second.data().c_str();
            SDK_TRACE_DEBUG("PartNum: %s", venice_db_.fru.partNum.c_str());
        }
        if (!strcmp(it.first.c_str(), "BoardId")) {
            venice_db_.fru.boardId = it.second.data().c_str();
            SDK_TRACE_DEBUG("BoardId: %s", venice_db_.fru.boardId.c_str());
        }
        if (!strcmp(it.first.c_str(), "EngChangeLevel")) {
            venice_db_.fru.engChangeLevel = it.second.data().c_str();
            SDK_TRACE_DEBUG("EngChangeLevel: %s", venice_db_.fru.engChangeLevel.c_str());
        }
        if (!strcmp(it.first.c_str(), "NumMacAddr")) {
            venice_db_.fru.numMacAddr = it.second.data().c_str();
            SDK_TRACE_DEBUG("NumMacAddr: %s", venice_db_.fru.numMacAddr.c_str());
        }
        if (!strcmp(it.first.c_str(), "MacStr")) {
            venice_db_.fru.macStr = it.second.data().c_str();
            SDK_TRACE_DEBUG("MacStr: %s", venice_db_.fru.macStr.c_str());
        }
    }

    return SDK_RET_OK;
}

void
venice::populate_venice_defaults_(void)
{
}

}    // namespace lib
}    // namespace sdk
