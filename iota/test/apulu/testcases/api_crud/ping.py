#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils

def __getOperations(tc_operation):
    opers = list()
    if tc_operation is None:
        return opers
    else:
        opers = list(map(lambda x:x.capitalize(), tc_operation))
    return opers

def Setup(tc):
    result = api.types.status.SUCCESS
    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        result = api.types.status.FAILURE

    selected_objs = None
    tc.opers = __getOperations(tc.iterators.oper)
    tc.is_config_deleted = False
    tc.is_config_updated = False
    for op in tc.opers:
        selected_objs = config_api.ProcessObjectsByOperation(op, tc.iterators.objtype, selected_objs)
        if op == 'Delete':
            tc.is_config_deleted = True
        elif op == 'Update':
            tc.is_config_updated = True
    tc.selected_objs = selected_objs
    return result

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
        if tc.is_config_deleted:
            res = config_api.IsAnyConfigDeleted(tc.workload_pairs[cookie_idx])
            if (res is True and cmd.exit_code == 0) or (res is False and cmd.exit_code != 0):
                api.Logger.error("verifyPing failed for %s" % (tc.cmd_cookies[cookie_idx]))
                api.PrintCommandResults(cmd)
                result = api.types.status.FAILURE
        if tc.is_config_updated:
            if cmd.exit_code != 0:
                api.PrintCommandResults(cmd)
                # TODO: some objetcs(tunnel, interface) fail due to 1-1 config, move to dol config
                # result = api.types.status.FAILURE
        if tc.is_config_deleted is False and tc.is_config_updated is False:
            if cmd.exit_code != 0:
                api.PrintCommandResults(cmd)
                result = api.types.status.FAILURE
        cookie_idx += 1
    api.Logger.debug(f"Verify result: {result}")
    return result

def Teardown(tc):
    if tc.is_config_updated:
        rs = config_api.RestoreObjects('Update', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"Teardown failed to restore objs from Update operation: {rs}")
    if tc.is_config_deleted:
        rs = config_api.RestoreObjects('Delete', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"Teardown failed to restore objs from Delete operation: {rs}")
    tc.cmd_cookies, tc.resp = traffic_utils.pingWorkloads(tc.workload_pairs, tc.iterators.ipaf)
    rs = traffic_utils.verifyPing(tc.cmd_cookies, tc.resp)
    api.Logger.debug(f"Teardown result: {rs}")
    return rs
