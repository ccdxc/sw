#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
    api.Logger.info("IPSec iperf SETUP")
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []

    nodes = api.GetWorkloadNodeHostnames()
    push_node_1 = [nodes[0]]
    push_node_2 = [nodes[1]]

    if api.IsNaplesNode(nodes[0]):

      stored_objects = netagent_cfg_api.QueryConfigs(kind='IPSecSAEncrypt')
      if len(stored_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_encryption_node1.json")
        if len(newObjects) == 0:
             api.Logger.error("Adding new objects to store failed for ipsec_encryption_node1.json")
             return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_1)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

      stored_objects = netagent_cfg_api.QueryConfigs(kind='IPSecSADecrypt')
      if len(stored_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_decryption_node1.json")
        if len(newObjects) == 0:
             api.Logger.error("Adding new objects to store failed for ipsec_decryption_node1.json")
             return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_1)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

      stored_objects = netagent_cfg_api.QueryConfigs(kind='IPSecPolicy')
      if len(stored_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_policies_node1.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_policies_node1.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_1)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-policy objects to node %s" % nodes[0])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

    else:
      workloads = api.GetWorkloads(nodes[0])
      wl = workloads[0]

      req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

      cmd_cookie = "IPSec policy on %s BEFORE running iperf traffic" % (wl.node_name)
      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "sudo ip xfrm policy show")
      tc.cmd_cookies.append(cmd_cookie)

      cmd_cookie = "IPSec policy on %s BEFORE running iperf traffic" % (wl.node_name)
      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "sudo ip xfrm state show")
      tc.cmd_cookies.append(cmd_cookie)

      cmd_cookie = "Configure IPsec Policy on %s" %(wl.workload_name)
      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm state flush")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm policy flush")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 dir in tmpl src 192.168.100.104 dst 192.168.100.102 proto esp mode tunnel")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 dir fwd tmpl src 192.168.100.104 dst 192.168.100.102 proto esp mode tunnel")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 dir out tmpl src 192.168.100.102 dst 192.168.100.104 proto esp mode tunnel")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm state add src 192.168.100.104 dst 192.168.100.102 proto esp spi 0x01 mode tunnel aead \"rfc4106(gcm(aes))\" 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.103/32 dst 192.168.100.101/32")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm state add src 192.168.100.102 dst 192.168.100.104 proto esp spi 0x01 mode tunnel aead \"rfc4106(gcm(aes))\" 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.101/32 dst 192.168.100.103/32")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm state list")
      tc.cmd_cookies.append(cmd_cookie)

      api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                             "sudo ip xfrm policy list")
      tc.cmd_cookies.append(cmd_cookie)


    if api.IsNaplesNode(nodes[1]):

      stored_objects = netagent_cfg_api.QueryConfigs(kind='IPSecSAEncrypt')
      if len(stored_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_encryption_node2.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_encryption_node2.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_2)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

      stored_objects = netagent_cfg_api.QueryConfigs(kind='IPSecSADecrypt')
      if len(stored_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_decryption_node2.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_decryption_node2.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_2)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

      stored_objects = netagent_cfg_api.QueryConfigs(kind='IPSecPolicy')
      if len(stored_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_policies_node2.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_policies_node2.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_2)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

    else:

        workloads = api.GetWorkloads(nodes[1])
        wl = workloads[0]

        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

        cmd_cookie = "IPSec policy on %s BEFORE running iperf traffic" % (wl.node_name)
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "sudo ip xfrm policy show")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "IPSec policy on %s BEFORE running iperf traffic" % (wl.node_name)
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "sudo ip xfrm state show")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Configure IPsec Policy on %s" %(wl.workload_name)
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm state flush")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm policy flush")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 dir in tmpl src 192.168.100.102 dst 192.168.100.104 proto esp mode tunnel")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 dir fwd tmpl src 192.168.100.102 dst 192.168.100.104 proto esp mode tunnel")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 dir out tmpl src 192.168.100.104 dst 192.168.100.102 proto esp mode tunnel")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm state add src 192.168.100.104 dst 192.168.100.102 proto esp spi 0x01 mode tunnel aead \"rfc4106(gcm(aes))\" 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.103/32 dst 192.168.100.101/32")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "sudo ip xfrm state add src 192.168.100.102 dst 192.168.100.104 proto esp spi 0x01 mode tunnel aead \"rfc4106(gcm(aes))\" 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.101/32 dst 192.168.100.103/32")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "sudo ip xfrm state list")
        tc.cmd_cookies.append(cmd_cookie)

        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "sudo ip xfrm policy list")
        tc.cmd_cookies.append(cmd_cookie)


    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if w2.IsNaples():
        cmd_cookie = "IPSec state on %s BEFORE running iperf traffic" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)

    if w1.IsNaples():
        cmd_cookie = "IPSec state on %s BEFORE running iperf traffic" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s) on TCP port %s" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address, tc.iterators.port)
   
    api.Logger.info("Starting Iperf test over IPSec from %s" % (tc.cmd_descr))

    cmd_cookie = "Set rcv socket buffer size on %s" %(w1.workload_name)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "sysctl -w net.ipv4.tcp_rmem='4096 2147483647 2147483647'")
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Running iperf server on %s" % (w2.workload_name)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "iperf3 -s -p %s" % (tc.iterators.port), background = True)
    tc.cmd_cookies.append(cmd_cookie)
    
    cmd_cookie = "Running iperf client on %s" % (w1.workload_name)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "iperf3 -c %s -p %s -M %s -b 100M" % (w2.ip_address, tc.iterators.port, tc.iterators.pktsize))
    tc.cmd_cookies.append(cmd_cookie)
    
    if w1.IsNaples():
        cmd_cookie = "IPSec state on %s AFTER running iperf traffic" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)

    if w2.IsNaples():
        cmd_cookie = "IPSec state on %s AFTER running iperf traffic" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS
    
def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    api.Logger.info("Iperf Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.Logger.info("%s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
