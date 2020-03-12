#! /usr/bin/python3
import subprocess
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
import iota.harness.api as api

SRC_FILE_NAME = "arm-iperf"
SRC_FILE_PATH = "/iota/images/"
SRC_FILE = api.GetTopDir() + SRC_FILE_PATH + SRC_FILE_NAME

DST_FILE_NAME = SRC_FILE_NAME
DST_FILE_PATH = "/tmp/"
DST_FILE = DST_FILE_PATH + DST_FILE_NAME

class __TCData: pass

def delete_file(tc):
    result = True
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        api.Trigger_AddNaplesCommand(req, node, tc.delete_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.PrintCommandResults(cmd)
            result = False
    return result

def trigger_copy(tc):
    result = delete_file(tc)
    if result != True:
        api.Logger.error("Failed to delete %s before copy" % (DST_FILE))
        return result

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    for node in tc.nodes:
        api.CopyToNaples(node, [SRC_FILE], "", True)
        api.Trigger_AddNaplesCommand(req, node, tc.move_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to copy %s" % (DST_FILE))
            result = False
    return result

def __parse_md5sum(cmd):
    op = cmd.stdout.split()
    md5sum = None
    if len(op) != 0:
        md5sum = op[0]
    api.Logger.verbose("parsed md5sum %s" % (md5sum))
    return md5sum

def verify_copy(tc):
    result = True
    req = api.Trigger_CreateAllParallelCommandsRequest()
    for node in tc.nodes:
        api.Trigger_AddNaplesCommand(req, node, tc.verify_cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Failed to verify md5sum")
            result = False
        elif tc.src_file_md5sum != __parse_md5sum(cmd):
            if GlobalOptions.dryrun:
                continue
            api.Logger.error("md5sum mismatch")
            result = False
    return result

def __get_md5sum(file):
    cmd = "md5sum %s" % file
    cmdhandle = subprocess.Popen([cmd], stdout=subprocess.PIPE, shell=True)
    cmd_op = cmdhandle.stdout.read().decode('utf-8', 'ignore')
    src_file_md5sum = cmd_op.split()[0]
    return src_file_md5sum

def __setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    if len(tc.nodes) == 0:
        api.Logger.error("No naples nodes found")
        return False

    tc.move_cmd = "mv /%s %s" % (DST_FILE_NAME, DST_FILE_PATH)
    tc.verify_cmd = "md5sum %s" % (DST_FILE)
    tc.delete_cmd = "rm -rf %s" % (DST_FILE)
    tc.src_file_md5sum = __get_md5sum(SRC_FILE)
    api.Logger.verbose("Calculated md5sum for file %s is %s" % (SRC_FILE, tc.src_file_md5sum))
    return True

def __trigger(tc):
    return trigger_copy(tc)

def __verify(tc):
    return verify_copy(tc)

def __teardown(tc):
    #delete copied file
    result = delete_file(tc)
    if result != True:
        api.Logger.error("Failed to delete %s during teardown" % (DST_FILE))
    return result

def __verify_oob():
    if GlobalOptions.dryrun:
       return api.types.status.SUCCESS
    tc = __TCData()
    if __setup(tc) is False or __trigger(tc) is False or\
       __verify(tc) is False or __teardown(tc) is False:
       return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Main(step):
    api.Logger.info("Verifying OOB mnic interface")
    return __verify_oob()
