#! /usr/bin/python3
import pdb
import time
import os
import json
import subprocess
import threading
import iota.harness.api as api
import iota.test.common.utils.naples_upgrade.utils as utils
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

def Main(step):
    if GlobalOptions.skip_setup:
        return api.types.status.SUCCESS

    upgr_images = getattr(step, "naples_upgr_pkgs", "naples_fw.tar")
    api.Logger.info("Upgrade pkt list {}".format(upgr_images))

    naplesHosts = api.GetNaplesHostnames()

    for image in upgr_images:
        api.Logger.info("upgrade img {}".format(image))
        for naplesHost in naplesHosts:
            ret = utils.installNaplesFwLatestImage(naplesHost, image)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed in upgrade pkg install")
                return ret
            ret = utils.copyNaplesFwImage(naplesHost, image, "/update/")
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Failed in copy of pkg to naples")
                return ret

    api.Logger.info("Upgrade pkg installed successfully")
    return api.types.status.SUCCESS
