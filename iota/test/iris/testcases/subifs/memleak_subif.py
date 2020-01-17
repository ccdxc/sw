#! /usr/bin/python3
import pdb
import time
import iota.protos.pygen.topo_svc_pb2 as topo_svc
import iota.harness.api as api
import iota.test.utils.naples_host as utils
import iota.test.iris.utils.subif_utils as subif_utils

def Setup(tc):
    tc.skip = False

    if tc.args.type == 'remote_only':
        tc.workload_pairs = subif_utils.getRemoteHostIntfWorkloadsPairs()
    else:
        tc.skip = True

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetNicMode() == 'hostpin' and tc.iterators.ipaf == 'ipv6':
        api.Logger.info("Skipping Testcase: IPv6 not supported in hostpin mode.")
        tc.skip = True

    return api.types.status.SUCCESS

# Create only subifs on Naples host interfaces
def __create_subifs(subif_count = 0, native_inf = None):
    for wl in api.GetWorkloads():
        if wl.parent_interface != wl.interface:
            continue
        if wl.IsNaples():
            ret_lst_subif = subif_utils.Create_Subifs(subif_count, wl.interface, wl.node_name)

# Delete subifs only on Naples host interfaces
def __delete_subifs(h_interface = None, node_name = None):
    for wl in api.GetWorkloads():
        if wl.parent_interface != wl.interface:
            continue
        if wl.IsNaples():
            subif_utils.Delete_Subifs(wl.interface, wl.node_name)

# Run ping traffic test
def __run_ping_test(req, tc):
    # run traffic between same vlan ifs
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if tc.args.ipv6:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                    (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                    "ping6 -i 0.2 -c 10 %s -I %s" % (w2.ipv6_address, w1.interface))
            tc.cmd_cookies.append(cmd_cookie)
        if tc.args.ipv4:
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                    (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                    "ping -i 0.2 -c 10 %s" % (w2.ip_address))
            tc.cmd_cookies.append(cmd_cookie)

def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    memslab_dump = {} # node name -> memslab info

    # clean up resources before run
    subif_utils.clearAll()

    # initialize config:
    subif_utils.initialize_tagged_config_workloads()

    # 1.1 = Delete existing subinterfaces
    # delete only tagged interfaces, not native interfaces
    __delete_subifs()

    time.sleep(5)

    # 1.2 = Store memslab info for Naples card
    for node_name in api.GetNaplesHostnames():
        memslab_dump[node_name] = utils.GetMemorySlabInNaples(node_name)

    # 1.3 = Create default no of subinterfaces for every workload/host interface
    __create_subifs()

    # 2.1 = Restore workloads
    __delete_subifs()

    time.sleep(5)

    # 2.2 = Get memslab for existing interfaces
    for node_name in api.GetNaplesHostnames():
        memslab_after = utils.GetMemorySlabInNaples(node_name)
        memslab_before = memslab_dump.get(node_name, None)
        ret_memslab_info = utils.ShowLeakInMemorySlabInNaples(memslab_before, memslab_after, node_name)
        tc.memslab_result = ret_memslab_info

    # 2.3 = Restore all workloads/subifs on all nodes
    __create_subifs()

    # Generate traffic pairs with same vlan id
    tc.workload_pairs = api.GetRemoteWorkloadPairs()

    # run ping test between newly created workload pairs
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []

    __run_ping_test(req, tc)
    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    subif_utils.clearAll()
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE


    result = api.types.status.SUCCESS

    # Print leaks in memslab
    for name, tups in tc.memslab_result.items():
        api.Logger.info("[%s] \t (In Use:  Allocs:  Frees:)" % (name))
        api.Logger.info("[(Before), (After)] \t %s" % (tups))
        result = api.types.status.FAILURE
    if tc.memslab_result is None:
        api.Logger.info("No leaks in memory slabs\n")
        result = api.types.status.SUCCESS

    cookie_idx = 0

    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
