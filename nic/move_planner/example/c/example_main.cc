// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>
#include <vector>

#include "nic/move_planner/example/c/example.hpp"
#include "nic/move_planner/lib/pal.h"
#include "nic/move_planner/lib/metadata_parser.h"

using namespace std;
using namespace upgrade;
using namespace example;

int fte_session_create(uint32_t sip,
                       uint32_t dip,
                       uint16_t proto,
                       uint16_t sport,
                       uint16_t dport) {

    static uint64_t index = 0;
    uint64_t session_table_base = pal_get_region("FTE_SESSION");
    uint64_t address = session_table_base + index * (sizeof(struct flow_t));
    index++;

    struct flow_t flow;
    flow.flow_key.sip = sip;
    flow.flow_key.dip = dip;
    flow.flow_key.proto = proto;
    flow.flow_key.sport = sport;
    flow.flow_key.dport = dport;
     
    pal_mem_write(address, (uint8_t*)&flow, sizeof(flow_t));
    return 1;
}

int main(int argc, char **argv) {
    // Create delphi SDK
    delphi::SdkPtr sdk(make_shared<delphi::Sdk>());
    string myName = "ExampleUpgradeService";
    vector<region_metadata_t> region_map;

    /* TESTING */
    setup_memory();
    region_map = metadata_read_region_map("/sw/nic/move_planner/example/c/hal_mem_example.json");

    for(vector<region_metadata_t>::iterator region_it = region_map.begin();
        region_it != region_map.end();
        region_it++) {
        cout << "FROM REGIONMAP - NAME : "<<(*region_it).name << " SIZE " << (*region_it).size_kb << endl;
        pal_alloc_memory((*region_it).name, (*region_it).size_kb);
    }

    // Make 10 entries 
    for(uint32_t i = 0; i < 10; i++) {
        fte_session_create(i, i+20, 2, 0XDEAD+i, 0xBEEF+i);
    }

    // END TEST SETUP 

    // Create a service instance
    shared_ptr<ExUpgSvc> exupgsvc = make_shared<ExUpgSvc>(sdk, myName);
    assert(exupgsvc != NULL);

    sdk->RegisterService(exupgsvc);

    // start a timer to create an object
    //exupgsvc->createTimer.set<ExUpgSvc, &ExUpgSvc::createTimerHandler>(exupgsvc.get());
    //exupgsvc->createTimer.start(55, 0);

    // run the main loop
    return sdk->MainLoop();
}
