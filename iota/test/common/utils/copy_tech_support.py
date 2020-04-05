#! /usr/bin/python3
import os
import iota.harness.api as api
import iota.protos.pygen.iota_types_pb2 as types_pb2


def CopyTechSupportFiles(naples_nodes, techsupport_files, log_dir):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in naples_nodes:
        api.Trigger_AddNaplesCommand(req, node, "ls /data/techsupport/")
        for file in techsupport_files:
            cmd = "chmod 666 %s" % file
            api.Trigger_AddNaplesCommand(req, node, cmd)
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)

    for node in naples_nodes:
        resp = api.CopyFromNaples(node, techsupport_files, log_dir)
        if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
            api.Logger.info("Failed to copy tech-support file(s) to logdir:%s"%(log_dir))
            return api.types.status.FAILURE
        for file in techsupport_files:
            cmd = "mv " + log_dir + "/" + os.path.basename(file) + " " + log_dir + "/" + os.path.basename(file) + "-" + node
            #Rename file based on node
            api.Logger.info("executing cmd %s" % cmd)
            os.system(cmd)

    return api.types.status.SUCCESS
