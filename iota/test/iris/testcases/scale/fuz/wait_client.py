#! /usr/bin/python3
import time
import json
import os
import pdb
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.testcases.scale.fuz.init as fuz_init



def Setup(tc):
    tc.action = str(getattr(tc.args, "action", "PERMIT"))
    return api.types.status.SUCCESS

def Trigger(tc):

    store = tc.GetBundleStore()
    client_req = store["client_req"]

    #wait for fuz connections
    tc.fuz_client_resp = api.Trigger_WaitForAllCommands(client_req)

    return api.types.status.SUCCESS

def __get_json(fuz_out):
    try:
        return json.loads(fuz_out)
    except:
        api.Logger.error("Failed to parse fuz json output :", fuz_out)
        pdb.set_trace()
        assert(0)

def verify_fuz(tc):
    failed_connections = 0
    success_connections = 0
    for idx, cmd in enumerate(tc.fuz_client_resp.commands):
        jsonOut = __get_json(cmd.stdout)
        success_connections = success_connections + jsonOut["successConnections"]
        if cmd.exit_code != 0:
            failed_connections = failed_connections + jsonOut["failedConnections"]
            for conn in jsonOut["connections"]:
                if  conn["error"] != "":
                    api.Logger.error("Failed Connection {}:{}, err : {}, sent/received : {}/{}".format(conn["ServerIPPort"],
                     conn["ClientIPPort"], conn["error"], conn["dataSent"], conn["dataReceived"]))

    api.Logger.info("Total Success connections {}".format(success_connections))

    if tc.action == "PERMIT" and failed_connections:
        api.Logger.info("Total Failed connections (Policy : ALLOW) {}".format(failed_connections))
        api.Logger.error("fuz test had errors")
        return api.types.status.FAILURE
    elif tc.action == "DENY" and success_connections:
        api.Logger.info("Total Success connections  (Policy : DENY)  {}".format(success_connections))
        api.Logger.error("fuz test had errors")
        return api.types.status.FAILURE
    else:
        api.Logger.info("Fuz test successfull")
    return api.types.status.SUCCESS

def Verify(tc):

    if verify_fuz(tc) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
