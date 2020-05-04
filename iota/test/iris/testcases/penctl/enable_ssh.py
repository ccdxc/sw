#! /usr/bin/python3
import pdb
import os
import iota.harness.api as api
import iota.protos.pygen.iota_types_pb2 as types_pb2
import iota.test.iris.testcases.penctl.common as common
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.test.iris.testcases.penctl.common import GetNaplesUUID

def Main(step):
    if GlobalOptions.skip_setup:
        #No mode switch required for skeip setup
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for n in api.GetNaplesHostnames():
        enable_sshd = " -a " + common.PENCTL_TOKEN[n] +  " system enable-sshd"
        #enable_sshd =  "system enable-sshd"
        common.AddPenctlCommand(req, n, enable_sshd)
        copy_key =  " -a " + common.PENCTL_TOKEN[n] +  " update ssh-pub-key -f ~/.ssh/id_rsa.pub"
        #copy_key = "update ssh-pub-key -f ~/.ssh/id_rsa.pub"
        common.AddPenctlCommand(req, n, copy_key)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    for n in api.GetNaplesHostnames():
        uuid = GetNaplesUUID(n)
        api.SetNaplesNodeUuid(n, uuid)

    return api.types.status.SUCCESS
