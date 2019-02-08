#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.filters.filters_utils as filters_utils

def getAllmcastEndPointsView(naples_node):
    # Host interface mcast endpoints
    host_mc_ep_set = filters_utils.getHostIntfMcastEndPoints(naples_node)

    # Naples intf mcast endpoints
    naples_mc_ep_set = filters_utils.getNaplesIntfMcastEndPoints(naples_node)

    # HAL view of endpoints
    hal_mc_ep_set = filters_utils.getNaplesHALmcastEndPoints(naples_node)

    #Keeping them separate as it is useful for debugging in scale
    return host_mc_ep_set, naples_mc_ep_set, hal_mc_ep_set

def verifyMCEndPoints(tc):
    host_mc_ep_view = tc.host_mc_ep_set
    naples_mc_ep_view = tc.naples_mc_ep_set
    hal_mc_ep_view = tc.hal_mc_ep_set

    # HAL's view of endpoints = Union of workload + Host + Naples Intf
    host_view = host_mc_ep_view | naples_mc_ep_view
    return filters_utils.verifyEndpoints(host_view, hal_mc_ep_view)

def Setup(tc):
    api.Logger.verbose("MC MAC filter : Setup")
    tc.skip = False
    result = api.types.status.SUCCESS

    tc.skip, tc.workloads, tc.naples_node = filters_utils.getNaplesNodeandWorkloads()
    if tc.skip:
        api.Logger.error("MC MAC filter : Setup -> No Naples Topology - So skipping the TC")
        return api.types.status.IGNORED

    api.Logger.info("MC MAC filter : Setup final result - ", result)
    return result

def Trigger(tc):
    api.Logger.verbose("MC MAC filter : Trigger")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # Triggers done - Now build endpoint view of Host and Naples
    tc.host_mc_ep_set, tc.naples_mc_ep_set, tc.hal_mc_ep_set = getAllmcastEndPointsView(tc.naples_node)
    api.Logger.debug("getAllmcastEndPointsView: host_mc_ep_set ", len(tc.host_mc_ep_set), tc.host_mc_ep_set)
    api.Logger.debug("getAllmcastEndPointsView: naples_mc_ep_set ", len(tc.naples_mc_ep_set), tc.naples_mc_ep_set)
    api.Logger.debug("getAllmcastEndPointsView: hal_mc_ep_set ", len(tc.hal_mc_ep_set), tc.hal_mc_ep_set)

    api.Logger.info("MC MAC filter : Trigger final result - ", result)
    return result

def Verify(tc):
    '''
    #TODO
        1. any HW table dump to check? - Not needed apparently
            Ideally, We can also check registered MAC table as follows
            halctl show table dump --entry-id 1840 --table-id 4
            Get the entry id from "halctl show endpoint --yaml"
        2. Check for memleaks [will pull from Amrita's TC]
            2.1 "halctl show system memory slab | grep mc_entry"
    '''
    api.Logger.verbose("MC MAC filter : Verify")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # Check if MACs in "halctl show multicast" match with host & naples interface mcast MAC
    if not verifyMCEndPoints(tc):
        api.Logger.error("MC MAC filter : Verify failed for verifyMCEndPoints")
        result = api.types.status.FAILURE
    else:
        api.Logger.debug("MC MAC filter : Verify - verifyMCEndPoints SUCCESS ")
    
    api.Logger.info("MC MAC filter : Verify final result - ", result)

    return result

def Teardown(tc):
    api.Logger.verbose("MC MAC filter : Teardown")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    api.Logger.info("MC MAC filter : Teardown final result - ", result)

    return result
