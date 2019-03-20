#! /usr/bin/python3
import time
import filecmp
import os
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
    api.Logger.info("tcp_proxy nc SETUP")
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies = []

    pairs = api.GetRemoteWorkloadPairs()
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    if w1.IsNaples() and w2.IsNaples():
        api.Logger.info("naples-naples unsupported currently for tcp-proxy")
        return api.types.status.DISABLED

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

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if w2.IsNaples():
        cmd_cookie = "tcp proxy sessions on %s BEFORE running nc" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    if w1.IsNaples():
        cmd_cookie = "tcp proxy sessions on %s BEFORE running nc" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    tc.cmd_descr = "Client: %s(%s) <--> Server: %s(%s) on tcp proxy port %s" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address, tc.iterators.port)
   
    api.Logger.info("Starting netcat test from %s" % (tc.cmd_descr))

    cmd_cookie = "Creating test file on %s" % (w1.workload_name)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "base64 /dev/urandom | head -1000 > tcp_proxy_client.dat")
    tc.cmd_cookies.append(cmd_cookie)
    
    cmd_cookie = "Running nc server on %s" % (w2.workload_name)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                           "nc -l %s > tcp_proxy_server.dat" % (tc.iterators.port), background = True)
    tc.cmd_cookies.append(cmd_cookie)

    cmd_cookie = "Running nc client on %s" % (w1.workload_name)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                           "nc %s %s < tcp_proxy_client.dat" % (w2.ip_address, tc.iterators.port))
    tc.cmd_cookies.append(cmd_cookie)
    
    if w1.IsNaples():
        cmd_cookie = "tcp proxy sessions on %s AFTER running nc" % (w1.node_name)
        api.Trigger_AddNaplesCommand(req, w1.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    if w2.IsNaples():
        cmd_cookie = "tcp proxy sessions on %s AFTER running nc" % (w2.node_name)
        api.Trigger_AddNaplesCommand(req, w2.node_name, "/nic/bin/halctl show tcp-proxy session")
        tc.cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    resp = api.CopyFromWorkload(w1.node_name, w1.workload_name, ['tcp_proxy_client.dat'], tc.GetLogsDir())
    if resp is None:
        api.Logger.error("Could not find tcp_proxy_client.dat")
        return api.types.status.FAILURE
    resp = api.CopyFromWorkload(w2.node_name, w2.workload_name, ['tcp_proxy_server.dat'], tc.GetLogsDir())
    if resp is None:
        api.Logger.error("Could not find tcp_proxy_server.dat")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS
    
def Verify(tc):
    nodes = api.GetWorkloadNodeHostnames()
    if api.IsNaplesNode(nodes[0]) and api.IsNaplesNode(nodes[1]):
        return api.types.status.DISABLED
    if tc.resp is None:
        return api.types.status.FAILURE

    file1 = tc.GetLogsDir() + '/tcp_proxy_client.dat'
    file2 = tc.GetLogsDir() + '/tcp_proxy_server.dat'
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
