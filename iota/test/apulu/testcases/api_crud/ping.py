#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils

def Setup(tc):
    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        return api.types.status.FAILURE

    tc.selected_objs = config_api.SetupConfigObjects(tc.iterators.oper, tc.iterators.objtype)
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.cmd_cookies, tc.resp = traffic_utils.pingWorkloads(tc.workload_pairs, tc.iterators.ipaf)
    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    if tc.resp is None:
        api.Logger.error("verifyPing failed - no response")
        return api.types.status.FAILURE
    commands = tc.resp.commands
    cookie_idx = 0
    for cmd in commands:
        if getattr(tc.iterators, "oper", None) == 'delete':
            res = config_api.IsAnyConfigDeleted(tc.workload_pairs[cookie_idx])
            if (res is True and cmd.exit_code == 0) or (res is False and cmd.exit_code != 0):
                api.Logger.error("verifyPing failed for %s" % (tc.cmd_cookies[cookie_idx]))
                api.PrintCommandResults(cmd)
                result = api.types.status.FAILURE
        cookie_idx += 1
    api.Logger.debug(f"Verify result: {result}")
    return result

def Teardown(tc):
    rs = config_api.RestoreConfigObjects(tc.iterators.oper, tc.selected_objs)
    if rs is False:
        api.Logger.error(f"Teardown failed to restore objs: {rs}")
        return api.types.status.FAILURE
    tc.cmd_cookies, tc.resp = traffic_utils.pingWorkloads(tc.workload_pairs, tc.iterators.ipaf)
    rs = traffic_utils.verifyPing(tc.cmd_cookies, tc.resp)
    api.Logger.debug(f"Teardown result: {rs}")
    return rs
