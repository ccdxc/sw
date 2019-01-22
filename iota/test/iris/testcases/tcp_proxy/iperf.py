#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
    api.Logger.info("tcp_proxy iperf SETUP")
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []

    store_proxy_objects = netagent_cfg_api.QueryConfigs(kind='TCPProxyPolicy')
    if len(store_proxy_objects) == 0:
        api.Logger.error("No tcp proxy objects in store")
        return api.types.status.FAILURE
    
    ret = netagent_cfg_api.PushConfigObjects(store_proxy_objects, ignore_error=True)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Unable to push tcp_proxy policies")
        return api.types.status.FAILURE
    
    get_config_objects = netagent_cfg_api.GetConfigObjects(store_proxy_objects)
    if len(get_config_objects) == 0:
        api.Logger.error("Unable to fetch newly pushed objects")
        return api.types.status.FAILURE

    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if w2.IsNaples():
        cmd_cookie = "hal cfg to Reset Default Security Profile on %s" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "LD_LIBRARY_PATH=/nic/lib:/platform/lib /nic/bin/hal_test def_sec_prof_reset")
        tc.cmd_cookies.append(cmd_cookie)
        cmd_cookie = "tcp proxy sessions on %s BEFORE running iperf traffic" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    if w1.IsNaples():
        cmd_cookie = "hal cfg to Reset Default Security Profile on %s" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "LD_LIBRARY_PATH=/nic/lib:/platform/lib /nic/bin/hal_test def_sec_prof_reset")
        tc.cmd_cookies.append(cmd_cookie)
        cmd_cookie = "tcp proxy sessions on %s BEFORE running iperf traffic" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s) on tcp proxy port %s" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address, tc.iterators.port)
   
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))

    cmd_cookie = "Set rcv socket buffer size on %s" %(w1.workload_name)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "sysctl -w net.ipv4.tcp_rmem='4096 2147483647 2147483647'")
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Running iperf server on %s" % (w1.workload_name)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "iperf3 -s -p %s" % (tc.iterators.port), background = True)
    tc.cmd_cookies.append(cmd_cookie)
    
    cmd_cookie = "Running iperf client on %s" % (w2.workload_name)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "iperf3 -c %s -p %s -M 1024 -b 100M" % (w1.ip_address, tc.iterators.port))
    tc.cmd_cookies.append(cmd_cookie)
    
    if w1.IsNaples():
        cmd_cookie = "tcp proxy sessions on %s AFTER running iperf traffic" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    if w2.IsNaples():
        cmd_cookie = "tcp proxy sessions on %s AFTER running iperf traffic" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "/nic/bin/halctl show tcp-proxy session")
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
