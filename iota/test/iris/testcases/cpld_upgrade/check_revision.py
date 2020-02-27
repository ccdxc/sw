#! /usr/bin/python3
import time
import json
import tarfile
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common

MANIFEST = "MANIFEST"
SRC_FILE_NAME = "naples_fw_.tar"
SRC_FILE_PATH = "/nic/"
SRC_FILE = api.GetTopDir() + SRC_FILE_PATH + SRC_FILE_NAME
def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting CPLD Upgrade revision test")
    tc.Nodes = api.GetNaplesHostnames()

    for node in tc.Nodes:
        naples_fru = common.GetNaplesFruJson(node)
        # Find the naples part number
        naples_partnum = naples_fru["part-number"]
        if naples_partnum == None:
            api.Logger.error("Cannot find naples part number")
            return api.types.status.FAILURE

        # Find the naples cpld revision
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        api.Trigger_AddNaplesCommand(req, node, "LD_LIBRARY_PATH=/nic/lib:/platform/lib PATH=/nic/bin:$PATH /nic/tools/fwupdate -l")
        resp = api.Trigger(req)
        for cmd in resp.commands:
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
            out = None
            try:
                out = json.loads(cmd.stdout)
            except:
                api.Logger.error("Error using fwupdate {}".format(cmd.stdout))
                return api.types.status.FAILURE
            cpld_fwupdate = out.get('cpld', 'null')
            if cpld_fwupdate == 'null':
                api.Logger.error("Naples on node {} doesnt have the right fwupdate." .format(node))
                return api.types.status.FAILURE
            naples_revision = out["cpld"]["bitfile"]["version"]
        api.Logger.info("Naples cpld version is {}" .format(naples_revision))

        # Use manifest to check if cpld is the latest
        cpld_latest = False
        tf = tarfile.open(SRC_FILE, mode="r")
        if tf == 'null':
            api.Logger.error("Naples image not found")
            return api.types.status.FAILURE
        tf.extract(MANIFEST)
        with open(MANIFEST, 'r') as json_file:
            data = json.load(json_file)
            # Check if bitfiles present
            mani_bitfiles = data.get('bitfiles', 'null')
            if mani_bitfiles == 'null':
                api.Logger.error("Not using the latest build.")
                return api.types.status.FAILURE
            # iterate through all the bit files
            for counter in range(len(data["bitfiles"])):
                # iterate through all part number for each bit file
                for pcounter in range(len(data["bitfiles"][counter]["partnumber"])):
                    mani_partnumber = data["bitfiles"][counter]["partnumber"][pcounter]
                    if mani_partnumber in naples_partnum:
                        mani_revision = data["bitfiles"][counter]["version"]
                        if mani_revision == int(naples_revision):
                            cpld_latest = True
        if cpld_latest == False:
            api.Logger.error("Naples doesnt have the latest cpld.")
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

