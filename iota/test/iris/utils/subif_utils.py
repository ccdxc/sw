 #! /usr/bin/python3
import sys
import re
import ipaddress
import iota.harness.infra.utils.utils as utils
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.resmgr as resmgr
import iota.test.iris.config.netagent.hw_push_config as hw_config
import iota.harness.infra.store as store
import iota.test.utils.naples_host as naples_utils

__local_workloads = {}
__deleted_store_subifs = {}  # node_name + host if --> list of subifs
__node_native_ifs = {} # node name --> list of host interfaces
__subifs_vlan_allocator = {} # vlan id --> nw_spec
__ipv4_allocators = {} # vlan id -> ipv4_allocator
__ipv6_allocators = {} # vlan id  -> ipv6_allocator
__free_vlans = [] # -> available/free vlans
__mac_allocator = resmgr.MacAddressStep("00AA.0000.0001", "0000.0000.0001")
__perhost_subifs = {} # host interface index -> - vlan id
__pernode_network_spec = {} # node name -> nw_spec
__ipv4_subnet_allocator = {} # network name(config.yaml) --> list of ipv4 addresses
__ipv6_subnet_allocator = {} # network name(config.yaml) --> list of ipv6 addresses

def __storeWorkloads_outisde_store(req, lst_add_subifs):
    global __local_workloads
    for wlmsg in req.workloads:
        if wlmsg.interface in lst_add_subifs:
            wl = store.Workload(wlmsg)
            __local_workloads[wl.workload_name] = wl
    return

def getNativeWorkloads():
    # Get all available host interfaces
    lst_nodes = api.GetWorkloadNodeHostnames()
    lst_native_if = {}
    mgmt_intf = {}

    # Get host interfaces on all nodes
    for node in lst_nodes:
        lst_native_if[node] = list(api.GetWorkloadNodeHostInterfaces(node))
        # Exclude host side management interface from this test case on naples
        if api.IsNaplesNode(node):
            mgmt_intf[node] = list(naples_utils.GetHostInternalMgmtInterfaces(node))

    for node, infs in mgmt_intf.items():
        for inf in infs:
            if inf in lst_native_if[node]:
                lst_native_if[node].remove(inf)

    # Get workloads corresponding to host interfaces
    total_workloads = api.GetWorkloads()
    native_workloads = []
    for w1 in total_workloads:
        for node, infs in lst_native_if.items():
            if w1.interface in infs and w1.node_name == node:
                native_workloads.append(w1)

    return native_workloads

def getRemoteHostIntfWorkloadsPairs():
    native_workloads = getNativeWorkloads()
    # Generate pairs from native workloads
    pairs = []
    for w1 in native_workloads:
        for w2 in native_workloads:
            if id(w1) == id(w2): continue
            if w1.uplink_vlan != w2.uplink_vlan: continue
            if w1.node_name != w2.node_name:
                pairs.append((w1, w2))
    return pairs

# Delete subifs created outside (global)store
def __delete_Subifs_Local_Store(req, host_inf, node_name):
    global __local_workloads
    global __free_vlans
    global __subifs_vlan_allocator
    global __node_native_ifs
    global __perhost_subifs

    assert(req.workload_op == topo_svc.DELETE)
    # Delete workloads outside store
    subifs_count = 0
    for wl in __local_workloads.values():
        if wl.parent_interface != host_inf or wl.node_name != node_name:
            continue
        # exclude native interfaces
        if wl.parent_interface == wl.interface:
            continue
        res = False
        for wreq in req.workloads:
            #Check if it is already added.
            if wreq.workload_name == wl.workload_name:
                res = True
                break
        if res:
            continue

        #assert(api.IsWorkloadRunning(wl.workload_name))
        wl_msg = req.workloads.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        subifs_count += 1

    # Delete local workload copies
    for wlmsg in req.workloads:
        if wlmsg.workload_name in __local_workloads:
            del __local_workloads[wlmsg.workload_name]


    # Release vlan id and other resources to reuse
    ifs = __node_native_ifs.get(node_name, None)
    ifs_index = ifs.index(str(host_inf))
    for j in range(subifs_count):
        key = "host_" + str(ifs_index) + "_" + str(j)
        __perhost_subifs[key] -= 1
        if __perhost_subifs[key] == 0:
            __free_vlans.append(__subifs_vlan_allocator[key])
            api.Logger.info("deleting vlan id for key %s: %d" % (key, __subifs_vlan_allocator[key]))
            del __subifs_vlan_allocator[key]
            del __perhost_subifs[key]

# Delete all sub-interfaces in store created on this host interface on a node
def delete_Subifs_For_HostInf(req, host_inf, node_name):
    global __deleted_store_subifs

    assert(req.workload_op == topo_svc.DELETE)
    # Delete workloads within store
    for wl in api.GetWorkloads():
        if wl.parent_interface != host_inf:
            continue
        if wl.node_name != node_name:
            continue
        # exclude native interfaces
        if wl.parent_interface == wl.interface:
            continue

        res = False
        for wreq in req.workloads:
            #Check if it is already added.
            if wreq.workload_name == wl.workload_name:
                res = True
                break
        if res:
            continue
        # add subifs for delete, only if not deleted already
        key = wl.node_name + '-' + wl.parent_interface
        lst_if = __deleted_store_subifs.get(key, None)
        if lst_if is None:
            #assert(api.IsWorkloadRunning(wl.workload_name))
            __deleted_store_subifs[key] = [wl.interface]
            wl_msg = req.workloads.add()
            wl_msg.workload_name = wl.workload_name
            wl_msg.node_name = wl.node_name
        else:
            if wl.interface not in lst_if:
                #assert(api.IsWorkloadRunning(wl.workload_name))
                (__deleted_store_subifs[key]).append(wl.interface)
                wl_msg = req.workloads.add()
                wl_msg.workload_name = wl.workload_name
                wl_msg.node_name = wl.node_name
        api.Logger.info("Deleting workload from store: %s for %s on %s\n" % (wl.interface, host_inf, node_name))

    # Check if local store has subifs for host_inf
    # and delete it
    __delete_Subifs_Local_Store(req, host_inf, node_name)

# Get workload from given interface name
def getWorkloadForInf(interface_name, node_name):
    global __local_workloads

    wls = api.GetWorkloads()
    for wl in wls:
        if wl.interface == interface_name and wl.node_name == node_name:
            return wl
    for wl in __local_workloads.values():
        if wl.interface == interface_name:
            return wl

# Get topology config from yaml file
def __get_topology_config():
    classic_yml = "{}/config.yml".format(api.GetTopologyDirectory())
    spec = parser.YmlParse(classic_yml)
    return spec

# Initialize configurations needed to add new workloads
# new workloads are stored outside (global) store
# NOTE: Make sure that this function is called before first call to create_subifs()
# for any host interface on any node
def initialize_tagged_config_workloads():
    # Get configuration from yaml
    spec = __get_topology_config()
    global __subifs_vlan_allocator
    global __ipv4_allocators
    global __ipv6_allocators
    global __mac_allocator
    global __ipv4_subnet_allocator
    global __ipv6_subnet_allocator
    global __pernode_network_spec
    global __node_native_ifs

    # Saving node IFs
    nodes = api.GetWorkloadNodeHostnames()
    for node in nodes:
        __node_native_ifs[node] = list(api.GetWorkloadNodeHostInterfaces(node))

   # Reading network and workload specs
    network_specs = {}
    workload_specs = {}
    for net in spec.spec.networks:
        network_specs[net.network.name] = net.network
        if net.network.default:
            def_nw_spec = net.network
    for wl in spec.spec.workloads:
        workload_specs[wl.workload.name] = wl.workload

    # Store network spec for nodes
    for lst_nodes in spec.instances.workloads:
        nd = lst_nodes.workload
        if nd.spec == 'tagged':
            wl_spec = workload_specs[nd.spec]
            nw_spec = network_specs[wl_spec.network_spec]
            __pernode_network_spec[nd.node] = nw_spec

    for node_name in api.GetWorkloadNodeHostnames():
        if node_name not in __pernode_network_spec:
            __pernode_network_spec[node_name] = def_nw_spec

    # Store subnet allocators for network spec
    # case 1: Get subnet from hw_config
    for k, net in network_specs.items() :
        ipv4_subnet = hw_config.GetIPv4Allocator(net.name)
        if ipv4_subnet is None and net.ipv4.enable:
            __ipv4_subnet_allocator[net.name] = resmgr.IpAddressStep(net.ipv4.ipam_base.split('/')[0], # 10.255.0.0/16 \
                    str(ipaddress.IPv4Address(1 << int(net.ipv4.ipam_base.split('/')[1]))))
        else:
            __ipv4_subnet_allocator[net.name] = ipv4_subnet

        ipv6_subnet = hw_config.GetIPv6Allocator(net.name)
        if ipv6_subnet is None and net.ipv6.enable:
            __ipv6_subnet_allocator[net.name] = resmgr.Ipv6AddressStep(net.ipv6.ipam_base.split('/')[0], # 2000::/48\
                    str(ipaddress.IPv6Address(1 << int(net.ipv6.ipv6.ipam_base.split('/')[1]))))
        else:
            __ipv6_subnet_allocator[net.name] = ipv6_subnet

    __mac_allocator = hw_config.GetMacAllocator()

# create subifs outside (global) store
# store it locally
def __create_new_workload_outside_store(req, node_name, parent_inf, subifs_count):
    global __pernode_network_spec
    global __perhost_subifs
    global __subifs_vlan_allocator
    global __perhost_subifs
    global __ipv4_subnet_allocator
    global __ipv6_subnet_allocator
    global __node_native_ifs
    global __free_vlans

    __add_subifs_wl = []

    assert(req.workload_op == topo_svc.ADD)
    ifs = __node_native_ifs[node_name]
    key = "host_" + str(ifs.index(str(parent_inf)))

    for j in range(subifs_count):
        index = key + "_" + str(j)
        if index not in __perhost_subifs:
            __perhost_subifs[index] = 1
        else:
            __perhost_subifs[index] += 1

        if index not in __subifs_vlan_allocator:
            nw_spec = __pernode_network_spec[node_name]
            # Check if free vlans available already, reuse it
            if len(__free_vlans):
                __subifs_vlan_allocator[index] = __free_vlans.pop(0)
                api.Logger.info("Reassigned vlan id: %d" % __subifs_vlan_allocator[index])
            else:
                for count in range(api.Testbed_GetVlanCount()):
                    vlan_id = api.Testbed_AllocateVlan()
                    if vlan_id == api.Testbed_GetVlanBase():
                        vlan_id = api.Testbed_AllocateVlan()
                    if vlan_id not in __subifs_vlan_allocator.values():
                        break
                else:
                    api.Logger.info("No vlans available. Delete existing subifs to add new subifs. Exiting, vlan count: %d\n" % api.Testbed_GetVlanCount())
                    return
                __subifs_vlan_allocator[index] = vlan_id

            ipv4_subnet = (__ipv4_subnet_allocator[nw_spec.name]).Alloc()
            ipv6_subnet = (__ipv6_subnet_allocator[nw_spec.name]).Alloc()
            __ipv4_allocators[index] = resmgr.IpAddressStep(ipv4_subnet, "0.0.0.1")
            __ipv6_allocators[index] = resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")
            __ipv4_allocators[index].Alloc() # To skip 0 ip
            __ipv6_allocators[index].Alloc() # To skip 0 ip

        subif_spec = __pernode_network_spec[node_name]
        ipv4_allocator = __ipv4_allocators[index]
        ipv6_allocator = __ipv6_allocators[index]
        vlan = __subifs_vlan_allocator[index]
        workload_name = node_name + "_" + parent_inf + "_subif_" + str(vlan)
        res = False
        for wreq in req.workloads:
            if wreq.workload_name == workload_name:
                res = True
                break
        if res:
            continue
        #assert(not api.IsWorkloadRunning(workload_name))
        wl_msg = req.workloads.add()
        ip4_addr_str = str(ipv4_allocator.Alloc())
        ip6_addr_str = str(ipv6_allocator.Alloc())
        wl_msg.ip_prefix = ip4_addr_str + "/" + str(subif_spec.ipv4.prefix_length)
        wl_msg.ipv6_prefix = ip6_addr_str + "/" + str(subif_spec.ipv6.prefix_length)
        wl_msg.mac_address = __mac_allocator.Alloc().get()
        wl_msg.encap_vlan = vlan
        wl_msg.uplink_vlan = wl_msg.encap_vlan
        wl_msg.workload_name = workload_name
        wl_msg.node_name = node_name
        wl_msg.pinned_port = 1
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        wl_msg.interface = parent_inf + "_" + str(vlan)
        wl_msg.parent_interface = parent_inf
        wl_msg.workload_type = api.GetWorkloadTypeForNode(node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(node_name)
        __add_subifs_wl.append(wl_msg.interface)

        # Safety check
        if vlan in __free_vlans:
            __free_vlans.remove(vlan)

    api.Logger.info("Created workload: %s for host: %s  on node: %s" % (__add_subifs_wl, parent_inf, node_name))
    # store these workloads locally
    __storeWorkloads_outisde_store(req, __add_subifs_wl)
    return  __add_subifs_wl

# add subifs present in store for given <parent_inf>
# return count of added subifs
def __add_from_store(req, node_name, parent_inf, total):
    count = 0
    global __deleted_store_subifs
    __add_subifs_wl = []

    key = node_name + '-' + parent_inf
    lst_del_subif = __deleted_store_subifs.get(key, None)
    for wl in api.GetWorkloads():
        if wl.parent_interface != parent_inf:
            continue
        if node_name != wl.node_name:
            continue
        # exclude native interfaces
        if wl.parent_interface == wl.interface:
            continue
        res = False
        for wreq in req.workloads:
            #Check if it is already added.
            if wreq.workload_name == wl.workload_name:
                res = True
                break
        if res:
            continue

        #assert(not api.IsWorkloadRunning(wl.workload_name))
        # remove the interface from deleted subifs
        is_deleted = False
        if lst_del_subif and wl.interface in lst_del_subif:
            lst_del_subif.remove(wl.interface)
            is_deleted = True

        if not is_deleted:
            continue

        api.Logger.info("adding to store again: %s" % wl.workload_name)
        wl_msg = req.workloads.add()
        wl_msg.ip_prefix = wl.ip_prefix
        wl_msg.ipv6_prefix = wl.ipv6_prefix
        wl_msg.mac_address = wl.mac_address
        wl_msg.encap_vlan = wl.encap_vlan
        wl_msg.uplink_vlan = wl.uplink_vlan
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        wl_msg.pinned_port = wl.pinned_port
        wl_msg.interface_type = wl.interface_type
        wl_msg.interface = wl.interface
        wl_msg.parent_interface = wl.parent_interface
        wl_msg.workload_type = wl.workload_type
        wl_msg.workload_image = wl.workload_image

        count += 1
        __add_subifs_wl.append(wl.interface)

        if count == total:
            break

    api.Logger.info("expected: %d, available in store: %d, workloads: %s for host %s on node %s: " % (total, count, __add_subifs_wl, parent_inf, node_name))

    return count, __add_subifs_wl

# <count> = 0, restore previous subif on <workload>
def create_Subifs_For_Inf(req, node_name, parent_inf, count = 0):
    __add_subifs_wl = []

    subif_count = int(count)

    assert(req.workload_op == topo_svc.ADD)
    # restore subifs from store, and delete all other subifs, if any present
    # on parent_inf
    if subif_count == 0:
        added, __add_subifs_wl = __add_from_store(req, node_name, parent_inf, subif_count)

        # Check if local store has subifs for parent_inf and then delete it
        del_req = api.TeardownWorkloadsRequest()
        __delete_Subifs_Local_Store(del_req, parent_inf, node_name)
        #Teardown workloads
        ret = api.Trigger_TeardownWorkloadsRequest(del_req)
        if ret != api.types.status.SUCCESS:
            api.Logger.info("FAILURE! Failure in deleting workloads for %s" % parent_inf)
            return __add_subifs_wl
    else:
        # add subifs from store first
        # and then create new workloads if remaining > 0
        added, __add_subifs_wl = __add_from_store(req, node_name, parent_inf, subif_count)
        remaining = abs(subif_count - added)
        if remaining > 0:
            lst_extended = __create_new_workload_outside_store(req, node_name, parent_inf, remaining)
            __add_subifs_wl.extend(lst_extended)
            api.Logger.info("Created %d new workloads for %s on %s" % (len(lst_extended), parent_inf, node_name))

    return __add_subifs_wl

# Return subifs for <hostname> from global store and local store
def GetSubifs(hostname, node_name):
    global __local_workloads
    global __deleted_store_subifs
    ret_lst_subifs = []

    for w1 in api.GetWorkloads():
        if w1.parent_interface != hostname:
            continue
        if w1.interface == w1.parent_interface:
            continue
        ret_lst_subifs.append(w1.interface)

    for wl_name, wl in __local_workloads.items():
        if wl.parent_interface != hostname:
            continue
        if wl.interface == wl.parent_interface:
            continue
        ret_lst_subifs.append(wl.interface)

    # Exclude interfaces which are deleted but kept in store
    key = node_name + '-' + hostname
    lst_del_subif = __deleted_store_subifs.get(key, None)
    if lst_del_subif:
        for subif in lst_del_subif:
            wl = getWorkloadForInf(subif, node_name)
            #assert(not api.IsWorkloadRunning(wl.workload_name))
            if subif in ret_lst_subifs:
                ret_lst_subifs.remove(subif)

    return ret_lst_subifs

# Create subifs as per <subif_count> on each workload
# subif_count = 0 indicates create/restore default subifs
# native_inf = None indicates that create subifs on all native ifs in all nodes
def Create_Subifs(subif_count = 0, native_inf = None, node_name = None):
    ret_lst_subif = []
    req = api.BringUpWorkloadsRequest()

    for w1 in getNativeWorkloads():
        if native_inf and native_inf != w1.interface:
            continue
        if node_name and w1.node_name != node_name:
            continue
        ret_lst_subif = create_Subifs_For_Inf(req, w1.node_name, w1.interface, subif_count)

    #now bring up workloads
    ret = api.Trigger_BringUpWorkloadsRequest(req)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return ret_lst_subif

# <h_interface> = interface name whose sub-interfaces are deleted
# <node_name> = node name where interface exists
# None means thats delete all subifs on all host interfaces on all nodes
def Delete_Subifs(h_interface = None, node_name = None):
    wload_teardown_req = api.TeardownWorkloadsRequest()

    # Delete existing subifs from all host interfaces
    for wl in getNativeWorkloads():
        if h_interface and h_interface != wl.interface:
            continue
        if node_name and node_name != wl.node_name:
            continue
        delete_Subifs_For_HostInf(wload_teardown_req, wl.interface, wl.node_name)

    #Teardown workloads
    ret = api.Trigger_TeardownWorkloadsRequest(wload_teardown_req)
    if ret != api.types.status.SUCCESS:
        api.Logger.info("FAILURE! Failure in deleting workloads for %s" % h_interface)
        return ret

# Call this routine after every iterator
def clearAll():
    global __local_workloads
    global __deleted_store_subifs
    global __subifs_vlan_allocator
    global __ipv4_allocators
    global __ipv6_allocators
    global __free_vlans
    global __mac_allocator
    global __perhost_subifs
    global __pernode_network_spec
    global __ipv4_subnet_allocator
    global __ipv6_subnet_allocator

    __local_workloads.clear()
    __deleted_store_subifs.clear()
    __subifs_vlan_allocator.clear()
    __ipv4_allocators.clear()
    __ipv6_allocators.clear()
    __free_vlans.clear()
    __perhost_subifs.clear()
    __pernode_network_spec.clear()
    __mac_allocator = resmgr.MacAddressStep("00AA.0000.0001", "0000.0000.0001")
