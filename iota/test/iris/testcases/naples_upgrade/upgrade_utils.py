#! /usr/bin/python3
import os
import datetime
import re
import iota.harness.api as api

__upg_log_path = "/obfl/upgrade.log"
__upg_log_fname = "upgrade.log"
__ERROR = "E"

def __find_err_in_upg_log(node, records):
    found_error = False

    for r in records:
        if r['lvl'] == __ERROR:
            api.Logger.error(f"Found error message in upg log on {node}: {r['raw']}")
            found_error = True
    return api.types.status.FAILURE if found_error  else api.types.status.SUCCESS

def __is_record_type_state(record):
    return True if record and "MoveStateMachine" in record['fname'] else False

def __get_upg_log_fname_from_node(node, log_dir):
    return f"{log_dir}/upgrade_{node}.log"

def __get_datetime_from_record(record):
    if record:
        return datetime.datetime.strptime(record['ts'],"%Y-%m-%d %H:%M:%S.%f+00:00")
    return ts

def __disset_upg_log(node, logs):
    records = []

    for log in logs:
        r_exp = r"(?P<lvl>[I,D,E]) \[(?P<ts>.*)\] \[(?P<fname>.*)\] (?P<msg>.*)"
        m = re.search(r_exp, log)
        if m:
            records.append({e: m.group(e) for e in ["lvl", "ts", "fname", "msg"]})
            records[-1]["raw"] = log
        else:
            api.Logger.error(f"Failed to dissect log on {node} : {log}")
    return records

def __calculate_upg_state_duration(node, records):
    last_ts = None

    for r in reversed(records):
        if not __is_record_type_state(r):
            continue
        if last_ts == None:
            last_ts = __get_datetime_from_record(r)
            r['duration'] = 0
        else:
            r['duration'] = last_ts - __get_datetime_from_record(r)
            last_ts = __get_datetime_from_record(r)

def __dump_upg_log(node, logs):
    api.Logger.SetNode(node)
    indent = "-" * 25
    api.Logger.info(f"{indent} U P G R A D E   L O G S {indent}")
    for log in logs:
        api.Logger.info(log)
    api.Logger.SetNode(None)

def __display_upg_state_transition(node, records):
    __calculate_upg_state_duration(node, records)
    api.Logger.SetNode(node)
    indent = "-" * 25
    api.Logger.info("\n")
    api.Logger.info(f"{indent} U P G R A D E   S T A T E   T R A N S I T I O N {indent}")
    for r in records:
        if __is_record_type_state(r):
            api.Logger.info("-    {}  {:<45} {}".format(r['ts'], r['msg'], r['duration']))
    api.Logger.info("Total Time : %s\n\n"%(__get_datetime_from_record(records[-1]) - \
                                           __get_datetime_from_record(records[1])))
    api.Logger.SetNode(None)

def ResetUpgLog(nodes):
    nodes = nodes if nodes else api.GetNaplesWorkloads()
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    for node in nodes:
        cmd = f":>{__upg_log_path}"
        api.Trigger_AddNaplesCommand(req, node, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error(f"Failed to reset upgrade log on {cmd.node_name}")
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def __dhcp_oob_mnic0(nodes):
    nodes = nodes if nodes else api.GetNaplesWorkloads()
    dhclient_cmd = "dhclient oob_mnic0"
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    for node in nodes:
        api.Trigger_AddNaplesCommand(req, node, dhclient_cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error(f"Failed to run dhclient on {cmd.node_name}")
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def GetUpgLog(nodes, log_dir):
    nodes = nodes if nodes else api.GetNaplesWorkloads()
    file_name = f"{log_dir}/{__upg_log_fname}"
    for node in nodes:
        if __dhcp_oob_mnic0([node]) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        api.CopyFromNaples(node, [__upg_log_path], log_dir, via_oob=True)
        if os.path.exists(file_name):
            os.rename(file_name, __get_upg_log_fname_from_node(node, log_dir))
        else:
            api.Logger.error(f"Upgrade logs for {node} not found @ {file_name}")
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def VerifyUpgLog(nodes, log_dir):
    for node in nodes:
        if GetUpgLog([node], log_dir) != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to get the upgrade log for {node}")
            return api.types.status.FAILURE

        with open(__get_upg_log_fname_from_node(node, log_dir)) as f:
            logs = f.readlines()
            if not logs:
                api.Logger.error(f"Failed to read logs from {node}")
                return api.types.status.FAILURE
            __dump_upg_log(node, logs)

            records = __disset_upg_log(node, logs)
            if not records:
                api.Logger.error(f"Failed to dissect the upgrade logs from {node}")
                return api.types.status.FAILURE

            if __find_err_in_upg_log(node, records) != api.types.status.SUCCESS:
                return api.types.status.FAILURE

            __display_upg_state_transition(node, records)

    return api.types.status.SUCCESS
