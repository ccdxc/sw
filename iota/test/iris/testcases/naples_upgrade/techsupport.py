#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.penctl.common as common

def_tech_support_file_name = "naples-tech-support.tar"
def_tech_support_dir_name = "NaplesTechSupport"
def_tech_support_dirs = ["cmd_out", "cores", "events", "logs", "obfl", "penctl.ver", "upgrade_logs"]
def_tech_support_log_files = ["post-upgrade-logs.tar.gz", "pre-upgrade-logs.tar"]


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    return api.types.status.SUCCESS

def Trigger(tc):

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        common.AddPenctlCommand(req, n, "system tech-support")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    supportDir = {}
    def untar():
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in tc.Nodes:
            api.Trigger_AddHostCommand(req, n, "ls %s" % (def_tech_support_file_name))
            api.Trigger_AddHostCommand(req, n, "tar -xvzf %s" % (def_tech_support_file_name))
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        return api.types.status.SUCCESS

    def findSupportDir():
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in tc.Nodes:
            api.Trigger_AddHostCommand(req, n, "find . -name \"%s*\"" % (def_tech_support_dir_name))
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        for n, cmd in zip(tc.Nodes, resp.commands):
            dirs = list(filter(None, cmd.stdout.split("\n")))
            if len(dirs) != 1:
                api.Logger.error("Mismatch of naples tech support directories!")
                return api.types.status.FAILURE
            supportDir[n] = dirs[0]
        return api.types.status.SUCCESS

    def deleteSupportDir():
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in tc.Nodes:
            api.Trigger_AddHostCommand(req, n, "rm -rf \"%s*\"" % (def_tech_support_dir_name))
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        return api.types.status.SUCCESS

    def check_sub_dirs():
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in tc.Nodes:
            api.Trigger_AddHostCommand(req, n, "ls %s" % (supportDir[n]))
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        for n, cmd in zip(tc.Nodes, resp.commands):
            sub_dirs = list(filter(None, cmd.stdout.split("\n")))
            if set(sub_dirs) != set(def_tech_support_dirs):
                api.Logger.error("Tech support dirs  don't match : expected %s, actual %s" %(def_tech_support_dirs, sub_dirs))
                return api.types.status.FAILURE

        return api.types.status.SUCCESS

    def check_log_files():
        req = api.Trigger_CreateExecuteCommandsRequest()
        for n in tc.Nodes:
            api.Trigger_AddHostCommand(req, n, "ls %s/%s" % (supportDir[n], "upgrade_logs"))
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
        for n, cmd in zip(tc.Nodes, resp.commands):
            log_files = list(filter(None, cmd.stdout.split("\n")))
            if not all(x in log_files for x in def_tech_support_log_files):
                api.Logger.error("Tech support log files missing : %s" %(set(def_tech_support_log_files) - set(log_files)))
                return api.types.status.FAILURE

        return api.types.status.SUCCESS

    validators = [untar, findSupportDir, check_sub_dirs, check_log_files, deleteSupportDir]
    for validator in validators:
        ret = validator()
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
