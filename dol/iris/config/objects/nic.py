#! /usr/bin/python3
import pdb
import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import logger
from iris.config.store               import Store

class NicObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('NIC'))
        return

    def Init(self, mode):
        self.device_mode = mode
        self.GID("Nic01")
        self.Show()
        return

    def Show(self, detail = False):
        logger.info("- NIC Mode = %d" % (self.device_mode))
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.device.device_mode = self.device_mode
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        logger.info("- NIC Response = %s" %\
                       (haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp_spec):
        if get_resp_spec.api_status == haldefs.common.ApiStatus.Value('API_STATUS_OK'):
            self.device_mode = get_resp_spec.device.device_mode
        else:
            logger.error("- NIC = %s is missing." %\
                       (haldefs.common.ApiStatus.Name(get_resp_spec.api_status)))

    def Get(self):
        halapi.GetDevices([self])

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Nic Objects.
class NicObjectHelper:
    def __init__(self):
        self.nics = []
        return

    def Configure(self):
        logger.info("Configuring Nic")
        if not GlobalOptions.agent:
            halapi.ConfigureDevice(self.nics)
        else:
            logger.info(" - Skipping in agent mode.")
        return

    def Generate(self):
        nic = NicObject()
        nic.Init(haldefs.nic.DeviceMode.Value('DEVICE_MODE_MANAGED_SWITCH'))
        self.nics.append(nic)
        logger.info("Creating Device Object")
        return

    def main(self):
        if not (GlobalOptions.hostpin or GlobalOptions.classic):
            self.Generate()
            self.Configure()
        return

NicHelper = NicObjectHelper()
