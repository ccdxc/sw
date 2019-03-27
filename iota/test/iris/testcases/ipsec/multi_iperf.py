#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
    api.Logger.info("IPSec iperf SETUP")
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

      for port,spi,aead in zip(tc.args.ports_list, tc.args.spi_list, tc.args.aead_list):
          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto tcp dport %s dir in tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % port)

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto tcp dport %s dir fwd tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % port)

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto tcp sport %s dir out tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % port)

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm state add src 192.168.100.103 dst 192.168.100.101 proto esp spi %s mode tunnel aead 'rfc4106(gcm(aes))' %s 128 sel src 192.168.100.103/32 dst 192.168.100.101/32" % (spi, aead))

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm state add src 192.168.100.101 dst 192.168.100.103 proto esp spi %s mode tunnel aead 'rfc4106(gcm(aes))' %s 128 sel src 192.168.100.101/32 dst 192.168.100.103/32" % (spi, aead))

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto udp dport %s dir in tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % port)

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto udp dport %s dir fwd tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % port)

          api.Trigger_AddCommand(req1, w1.node_name, w1.workload_name,
                                 "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto udp sport %s dir out tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % port)

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

        for port,spi,aead in zip(tc.args.ports_list, tc.args.spi_list, tc.args.aead_list):
            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto tcp dport %s dir in tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % port)

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto tcp dport %s dir fwd tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % port)

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto tcp sport %s dir out tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % port)

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm state add src 192.168.100.101 dst 192.168.100.103 proto esp spi %s mode tunnel aead 'rfc4106(gcm(aes))' %s 128 sel src 192.168.100.101/32 dst 192.168.100.103/32" % (spi, aead))

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm state add src 192.168.100.103 dst 192.168.100.101 proto esp spi %s mode tunnel aead 'rfc4106(gcm(aes))' %s 128 sel src 192.168.100.103/32 dst 192.168.100.101/32" % (spi, aead))

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto udp dport %s dir in tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % port)

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm policy add src 192.168.100.101/32 dst 192.168.100.103/32 proto udp dport %s dir fwd tmpl src 192.168.100.101 dst 192.168.100.103 proto esp mode tunnel" % port)

            api.Trigger_AddCommand(req2, w2.node_name, w2.workload_name,
                                   "sudo ip xfrm policy add src 192.168.100.103/32 dst 192.168.100.101/32 proto udp sport %s dir out tmpl src 192.168.100.103 dst 192.168.100.101 proto esp mode tunnel" % port)

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
        api.Logger.info("Both workloads are Naples, %s is iperf client, %s is iperf server, bypassing test" % (w1.node_name, w2.node_name))
        iperf_client_wl = w1
        iperf_server_wl = w2
        bypass_test = 1
    elif w1.IsNaples():
        api.Logger.info("%s is Naples and iperf client, %s is iperf server" % (w1.node_name, w2.node_name))
        iperf_client_wl = w1
        iperf_server_wl = w2
    elif w2.IsNaples():
        api.Logger.info("%s is Naples and iperf client, %s is iperf server" % (w2.node_name, w1.node_name))
        iperf_client_wl = w2
        iperf_server_wl = w1

    req3 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    cmd_cookie = "Set rcv socket buffer size on %s" %(w1.workload_name)
    api.Trigger_AddCommand(req3, w1.node_name, w1.workload_name,
                           "sysctl -w net.ipv4.tcp_rmem='4096 2147483647 2147483647'")
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Set rcv socket buffer size on %s" %(w2.workload_name)
    api.Trigger_AddCommand(req3, w2.node_name, w2.workload_name,
                           "sysctl -w net.ipv4.tcp_rmem='4096 2147483647 2147483647'")
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Setting MTU to smaller value on %s" % (iperf_client_wl.workload_name)
    api.Trigger_AddCommand(req3, iperf_client_wl.node_name, iperf_client_wl.workload_name,
                           "ifconfig %s mtu 1048" % iperf_client_wl.interface)
    tc.cmd_cookies.append(cmd_cookie)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s) on %s" %\
                   (iperf_server_wl.workload_name, iperf_server_wl.ip_address, iperf_client_wl.workload_name, iperf_client_wl.ip_address, tc.iterators.protocol)
    api.Logger.info("Starting Iperf test over IPSec from %s" % (tc.cmd_descr))

    if bypass_test == 0:
      for port in tc.args.ports_list:
          cmd_cookie = "Running iperf server on %s port %s" % (iperf_server_wl.workload_name, port)
          api.Trigger_AddCommand(req3, iperf_server_wl.node_name, iperf_server_wl.workload_name,
                                 "iperf -s -p %s" % (port), background = True)
          tc.cmd_cookies.append(cmd_cookie)

    req4 = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    if bypass_test == 0:
      cmd_cookie = "Brief Sleep" 
      api.Trigger_AddCommand(req4, iperf_client_wl.node_name, iperf_client_wl.workload_name,
                             "sleep 1")
      tc.cmd_cookies.append(cmd_cookie)
      for port in tc.args.ports_list:
          cmd_cookie = "Running iperf client on %s port %s" % (iperf_client_wl.workload_name, port)
          if tc.iterators.protocol == "tcp":
              api.Trigger_AddCommand(req4, iperf_client_wl.node_name, iperf_client_wl.workload_name,
                                     "iperf -c %s -p %s -M %s" % (iperf_server_wl.ip_address, port, tc.iterators.pktsize))
          else:
              api.Trigger_AddCommand(req4, iperf_client_wl.node_name, iperf_client_wl.workload_name,
                                     "iperf --udp -c %s -p %s -M %s" % (iperf_server_wl.ip_address, port, tc.iterators.pktsize))
          tc.cmd_cookies.append(cmd_cookie)

    req5 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if w1.IsNaples():
        cmd_cookie = "IPSec state on %s AFTER running iperf traffic" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req5, w1.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)
    else:
        cmd_cookie = "IPSec state on %s AFTER running iperf traffic" % (w1.node_name)
        api.Trigger_AddCommand(req5, w1.node_name, w1.workload_name, "sudo ip xfrm policy show")
        tc.cmd_cookies.append(cmd_cookie) 

    if w2.IsNaples():
        cmd_cookie = "IPSec state on %s AFTER running iperf traffic" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req5, w2.node_name, "/nic/bin/halctl show ipsec-global-stats")
        tc.cmd_cookies.append(cmd_cookie)
    else:
        cmd_cookie = "IPSec state on %s AFTER running iperf traffic" % (w2.node_name)
        api.Trigger_AddCommand(req5, w2.node_name, w2.workload_name, "sudo ip xfrm policy show")
        tc.cmd_cookies.append(cmd_cookie) 

    trig_resp3 = api.Trigger(req3)
    trig_resp4 = api.Trigger(req4)
    trig_resp5 = api.Trigger(req5)

    term_resp3 = api.Trigger_TerminateAllCommands(trig_resp3)
    term_resp4 = api.Trigger_TerminateAllCommands(trig_resp4)
    term_resp5 = api.Trigger_TerminateAllCommands(trig_resp5)

    agg_resp4 = api.Trigger_AggregateCommandsResponse(trig_resp4, term_resp4)
    tc.resp = agg_resp4

    return api.types.status.SUCCESS
    
def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    api.Logger.info("Multi Iperf Results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.Logger.info("%s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
