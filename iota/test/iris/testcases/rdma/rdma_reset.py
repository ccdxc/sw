#! /usr/bin/python3
#
# Check that the RDMA LIF reset code is functional.
#
import pdb
import time
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

def Setup(tc):
    api.Logger.info("RDMA Driver LIF Reset")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os not in [host.OS_TYPE_BSD, host.OS_TYPE_LINUX]:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    tc.stg1 = {}
    tc.stg2 = {}

    tc.intfs = []
    for node in tc.nodes:
        for intf in api.GetNaplesHostInterfaces(node):
            tc.intfs.append((node,intf))

    if tc.os == host.OS_TYPE_LINUX:
        # Cache this to save time
        tc.pci = {}
        for node in tc.nodes:
            for intf in api.GetNaplesHostInterfaces(node):
                pci = host.GetNaplesPci(node, intf)
                if pci is None:
                    api.Logger.warn("%s %s couldn't find PCI device" % (
                        node, intf))
                tc.pci[(node,intf)] = pci

    return api.types.status.SUCCESS

def get_rdmaresets(req, node, intf, tc):
    if tc.os == host.OS_TYPE_BSD:
        cmd = 'sysctl -n dev.' + host.GetNaplesSysctl(intf) + '.rdma.info.reset_cnt'
    else:
        pci = tc.pci[(node,intf)]
        if pci is None:
            cmd = 'echo 0'
        else:
            cmd = 'grep reset_cnt /sys/kernel/debug/ionic/' + pci + '/lif0/rdma/info | cut -f 2'
    return api.Trigger_AddHostCommand(req, node, cmd)

def set_rdma_lif_reset(req, node, intf, tc):
    if tc.os == host.OS_TYPE_BSD:
        cmd = 'sysctl dev.' + host.GetNaplesSysctl(intf) + '.rdma.reset=1'
    else:
        pci = tc.pci[(node,intf)]
        if pci is None:
            cmd = 'echo 0'
        else:
            cmd = 'echo 1 > /sys/kernel/debug/ionic/' + pci + '/lif0/rdma/reset'
    return api.Trigger_AddHostCommand(req, node, cmd)

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Triggering RDMA LIF resets")
    for (node, intf) in tc.intfs:
        # Read the reset counter
        tc.stg1[(node,intf)] = get_rdmaresets(req, node, intf, tc)

        # Force an RDMA reset
        set_rdma_lif_reset(req, node, intf, tc)

    api.Trigger_AddHostCommand(req, node, "sleep 15")

    for (node, intf) in tc.intfs:
        # Read the updated reset counter
        tc.stg2[(node,intf)] = get_rdmaresets(req, node, intf, tc)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    api.Logger.info("Verifying RDMA LIF resets")
    for (node, intf) in tc.intfs:
        pre = tc.stg1[(node,intf)]
        post = tc.stg2[(node,intf)]

        if int(post.stdout) == int(pre.stdout) + 1:
            api.Logger.info("%s %s counter value incremented %s -> %s" % (
                node, intf, pre.stdout, post.stdout))
        else:
            if int(post.stdout) != int(pre.stdout):
                api.Logger.warn("%s %s counter value incremented %s -> %s ???" % (
                    node, intf, pre.stdout, post.stdout))
            api.Logger.warn("%s %i No RDMA LIF reset activity found" % (
                node, intf))
            return api.types.status.FAILURE

    api.Logger.info("RDMA LIF resets confirmed")

    return api.types.status.SUCCESS

def Teardown(tc):
    # None required
    return api.types.status.SUCCESS
