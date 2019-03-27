#! /usr/bin/python3
import time
import filecmp
import os
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
    api.Logger.info("IPSec nc SETUP")
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []
    tc.cmd_cookies1 = []
    tc.cmd_cookies2 = []

    nodes = api.GetWorkloadNodeHostnames()
    push_node_0 = [nodes[0]]
    push_node_1 = [nodes[1]]

    encrypt_objects = netagent_cfg_api.QueryConfigs(kind='IPSecSAEncrypt')
    decrypt_objects = netagent_cfg_api.QueryConfigs(kind='IPSecSADecrypt')
    policy_objects = netagent_cfg_api.QueryConfigs(kind='IPSecPolicy')

    # Configure IPsec on Node 1

    if api.IsNaplesNode(nodes[0]):

      if len(encrypt_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_encryption_node1.json")
        if len(newObjects) == 0:
             api.Logger.error("Adding new objects to store failed for ipsec_encryption_node1.json")
             return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_0, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[0])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

      if len(decrypt_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_decryption_node1.json")
        if len(newObjects) == 0:
             api.Logger.error("Adding new objects to store failed for ipsec_decryption_node1.json")
             return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_0, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[0])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

      if len(policy_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_policies_node1.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_policies_node1.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_0, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-policy objects to node %s" % nodes[0])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            return api.types.status.FAILURE

    else:
      workloads = api.GetWorkloads(nodes[0])
      w1 = workloads[0]

      req1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                             "sudo ip xfrm state flush")

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                             "sudo ip xfrm policy flush")

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto tcp dport %s dir in tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % (tc.iterators.port))

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto tcp dport %s dir fwd tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % (tc.iterators.port))

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto tcp sport %s dir out tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % (tc.iterators.port))

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm state add src 192.168.100.103 dst 192.168.100.101 proto esp spi 0x01 mode tunnel aead 'rfc4106(gcm(aes))' 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.103/32 dst 192.168.100.101/32")

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm state add src 192.168.100.101 dst 192.168.100.103 proto esp spi 0x01 mode tunnel aead 'rfc4106(gcm(aes))' 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.101/32 dst 192.168.100.103/32")

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto udp dport %s dir in tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % (tc.iterators.port))

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto udp dport %s dir fwd tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % (tc.iterators.port))

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto udp sport %s dir out tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % (tc.iterators.port))

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                             "sudo ip xfrm state list")

      api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                             "sudo ip xfrm policy list")

      trig_resp1 = api.Trigger(req1)
      term_resp1 = api.Trigger_TerminateAllCommands(trig_resp1)

    # Configure IPsec on Node 2

    if api.IsNaplesNode(nodes[1]):

      if len(encrypt_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_encryption_node2.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_encryption_node2.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_1, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            #return api.types.status.FAILURE

      if len(decrypt_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_decryption_node2.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_decryption_node2.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_1, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            #return api.types.status.FAILURE

      if len(policy_objects) == 0:

        newObjects = netagent_cfg_api.AddOneConfig(api.GetTopologyDirectory() + "/ipsec/ipsec_policies_node2.json")
        if len(newObjects) == 0:
            api.Logger.error("Adding new objects to store failed for ipsec_policies_node2.json")
            return api.types.status.FAILURE

        ret = netagent_cfg_api.PushConfigObjects(newObjects, node_names = push_node_1, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Unable to push ipsec-encryption objects to node %s" % nodes[1])
            return api.types.status.FAILURE

        get_config_objects = netagent_cfg_api.GetConfigObjects(newObjects)
        if len(get_config_objects) == 0:
            api.Logger.error("Unable to fetch newly pushed objects")
            #return api.types.status.FAILURE

    else:
        workloads = api.GetWorkloads(nodes[1])
        w2 = workloads[0]

        req2 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm state flush")

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy flush")

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto tcp dport %s dir in tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % (tc.iterators.port))

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto tcp dport %s dir fwd tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % (tc.iterators.port))

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto tcp sport %s dir out tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % (tc.iterators.port))

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm state add src 192.168.100.101 dst 192.168.100.103 proto esp spi 0x01 mode tunnel aead 'rfc4106(gcm(aes))' 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.101/32 dst 192.168.100.103/32")

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm state add src 192.168.100.103 dst 192.168.100.101 proto esp spi 0x01 mode tunnel aead 'rfc4106(gcm(aes))' 0x414141414141414141414141414141414141414141414141414141414141414100000000 128 sel src 192.168.100.103/32 dst 192.168.100.101/32")

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto udp dport %s dir in tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % (tc.iterators.port))

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto udp dport %s dir fwd tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % (tc.iterators.port))

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                               "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto udp sport %s dir out tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % (tc.iterators.port))

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name, "sudo ip xfrm state list")

        api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name, "sudo ip xfrm policy list")

        trig_resp2 = api.Trigger(req2)
        term_resp2 = api.Trigger_TerminateAllCommands(trig_resp2)

    workloads = api.GetWorkloads(nodes[0])
    w1 = workloads[0]
    workloads = api.GetWorkloads(nodes[1])
    w2 = workloads[0]
    bypass_test = 0

    if w1.IsNaples() and w2.IsNaples():
        api.Logger.info("Both workloads are Naples, %s is nc client, %s is nc server, bypassing test" % (w1.node_name, w2.node_name))
        nc_client_wl = w1
        nc_server_wl = w2
        bypass_test = 1
    elif w1.IsNaples():
        api.Logger.info("%s is Naples and nc client, %s is nc server" % (w1.node_name, w2.node_name))
        nc_client_wl = w1
        nc_server_wl = w2
    elif w2.IsNaples():
        api.Logger.info("%s is Naples and nc client, %s is nc server" % (w2.node_name, w1.node_name))
        nc_client_wl = w2
        nc_server_wl = w1

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s) on %s port %s" %\
                   (nc_server_wl.workload_name, nc_server_wl.ip_address, nc_client_wl.workload_name, nc_client_wl.ip_address, tc.iterators.protocol, tc.iterators.port)
   
    api.Logger.info("Starting NC test over IPSec from %s" % (tc.cmd_descr))

    if bypass_test == 0:
        cmd_cookie = "Creating test file on %s" % (nc_client_wl.workload_name)
        api.Trigger_AddCommand(req, nc_client_wl.node_name, nc_client_wl.workload_name,
                               "base64 /dev/urandom | head -1000 > ipsec_client.dat")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Setting MTU to smaller value on %s" % (nc_client_wl.workload_name)
        api.Trigger_AddCommand(req, nc_client_wl.node_name, nc_client_wl.workload_name,
                               "ifconfig %s mtu 1048" % nc_client_wl.interface)
        tc.cmd_cookies.append(cmd_cookie)
    
        cmd_cookie = "Running nc server on %s" % (nc_server_wl.workload_name)
        if tc.iterators.protocol == "tcp":
            api.Trigger_AddCommand(req, nc_server_wl.node_name, nc_server_wl.workload_name,
                                   "nc -l %s > ipsec_server.dat" % (tc.iterators.port), background = True)
        else:
            api.Trigger_AddCommand(req, nc_server_wl.node_name, nc_server_wl.workload_name,
                                   "nc --udp -l %s > ipsec_server.dat" % (tc.iterators.port), background = True)
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Running nc client on %s" % (nc_client_wl.workload_name)
        if tc.iterators.protocol == "tcp":
            api.Trigger_AddCommand(req, nc_client_wl.node_name, nc_client_wl.workload_name,
                                   "nc %s %s < ipsec_client.dat" % (nc_server_wl.ip_address, tc.iterators.port))
        else:
            api.Trigger_AddCommand(req, nc_client_wl.node_name, nc_client_wl.workload_name,
                                   "nc --udp %s %s < ipsec_client.dat" % (nc_server_wl.ip_address, tc.iterators.port))
        tc.cmd_cookies.append(cmd_cookie)
    else:
        cmd_cookie = "Creating dummy file on %s" % (nc_client_wl.workload_name)
        api.Trigger_AddCommand(req, nc_client_wl.node_name, nc_client_wl.workload_name,
                               "rm -f ipsec_client.dat ; touch ipsec_client.dat")
        tc.cmd_cookies.append(cmd_cookie)

        cmd_cookie = "Creating dummy file on %s" % (nc_server_wl.workload_name)
        api.Trigger_AddCommand(req, nc_server_wl.node_name, nc_server_wl.workload_name,
                               "rm -f ipsec_server.dat ; touch ipsec_server.dat")
        tc.cmd_cookies.append(cmd_cookie)

    if nc_client_wl.IsNaples():
        cmd_cookie = "IPSec state on %s AFTER running nc test" % (nc_client_wl.node_name)
        api.Trigger_AddNaplesCommand(req, nc_client_wl.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)
    else:
        cmd_cookie = "IPSec state on %s AFTER running nc test" % (nc_client_wl.node_name)
        api.Trigger_AddCommand(req, nc_client_wl.node_name, nc_client_wl.workload_name, "sudo ip xfrm policy show")
        tc.cmd_cookies.append(cmd_cookie) 

    if nc_server_wl.IsNaples():
        cmd_cookie = "IPSec state on %s AFTER running nc test" % (nc_server_wl.node_name)
        api.Trigger_AddNaplesCommand(req, nc_server_wl.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)
    else:
        cmd_cookie = "IPSec state on %s AFTER running nc test" % (nc_server_wl.node_name)
        api.Trigger_AddCommand(req, nc_server_wl.node_name, nc_server_wl.workload_name, "sudo ip xfrm policy show")
        tc.cmd_cookies.append(cmd_cookie) 

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    resp = api.CopyFromWorkload(nc_client_wl.node_name, nc_client_wl.workload_name, ['ipsec_client.dat'], tc.GetLogsDir())
    if resp is None:
        api.Logger.error("Could not find ipsec_client.dat")
        return api.types.status.FAILURE
    resp = api.CopyFromWorkload(nc_server_wl.node_name, nc_server_wl.workload_name, ['ipsec_server.dat'], tc.GetLogsDir())
    if resp is None:
        api.Logger.error("Could not find ipsec_server.dat")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS
    
def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    file1 = tc.GetLogsDir() + '/ipsec_client.dat'
    file2 = tc.GetLogsDir() + '/ipsec_server.dat'
    if not filecmp.cmp(file1, file2, shallow=False):
        api.Logger.error("Client and server files do not match")
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    api.Logger.info("NC Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.Logger.info("%s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
