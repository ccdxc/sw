#! /usr/bin/python3
import random

import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.config.bringup_workloads as wl_api
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils
import iota.test.apulu.config.add_routes as add_routes
import iota.test.apulu.utils.move as move_utils
import iota.harness.infra.store as store
import iota.test.utils.arping as arp

intf_client   = config_api.GetObjClient('interface')

def Setup(tc):
    # select one of the LearnIP objects from dictionary
    tc.node = random.choice(api.GetNaplesHostnames())
    tc.learn_mac_obj = random.choice(learn_utils.GetLearnMACObjects(tc.node))
    tc.subnet = tc.learn_mac_obj.SUBNET
    tc.hostifidx = tc.subnet.HostIfIdx
    tc.wload = config_api.FindWorkloadByVnic(tc.learn_mac_obj)
    api.Logger.debug("Chosen subnet %s" % tc.subnet)
    return api.types.status.SUCCESS

def __modify_workload_interface(tc):
    wl_api.DeleteWorkload(tc.wload)
    if api.IsDryrun():
        tc.wload.parent_interface = 'dryrun'
    else:
        tc.wload.parent_interface = intf_client.FindHostInterface(tc.subnet.Node, tc.subnet.HostIfIdx).GetInterfaceName()
    tc.wload.interface = tc.wload.parent_interface
    store.SetWorkloadRunning(tc.wload.workload_name)
    wl_api.ReAddWorkload(tc.wload)
    add_routes.AddRoutes(tc.learn_mac_obj)
    arp.SendGratArp([tc.wload])
    return

def Trigger(tc):
    ret = tc.subnet.ModifyHostInterface()
    if not ret:
        api.Logger.error("Failed to modify host interface association for subnet")
        return api.types.status.FAILURE

    if api.IsDryrun():
        old_intf = new_intf = 'dryrun'
    else:
        old_intf = intf_client.FindHostInterface(tc.subnet.Node, tc.hostifidx).GetInterfaceName()
        new_intf = intf_client.FindHostInterface(tc.subnet.Node, tc.subnet.HostIfIdx).GetInterfaceName()
    api.Logger.debug(f"Subnet moved from HostInterface {old_intf} to {new_intf}")
    ret = tc.subnet.VerifyDepsOperSt('Delete')
    if not ret:
        return api.types.status.FAILURE
    __modify_workload_interface(tc)
    return api.types.status.SUCCESS

def __validate_trigger():
    misc_utils.Sleep(40) # letting metaswitch sync data
    learn_utils.DumpLearnData()
    if move_utils.ValidateEPMove() != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    return __validate_trigger()

def Teardown(tc):
    ret = tc.subnet.ModifyHostInterface(tc.hostifidx)
    if not ret:
        api.Logger.error("Failed to revert host interface modification made to subnet")
        return api.types.status.FAILURE
    __modify_workload_interface(tc)
    return __validate_trigger()
