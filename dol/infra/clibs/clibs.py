#! /usr/bin/python3

import ctypes
import os
from infra.common.logging       import logger


# A dict to map a library name to its handle.
testLibs = {}

OBJPATH = "nic/obj/lib"


def LoadCLib(name, is_global=False):
    libpath = os.path.join(os.environ['WS_TOP'], OBJPATH + name + '.so')
    logger.info("Trying to load library %s" % libpath)

    global testLibs
    if name in testLibs:
        logger.info("Library %s already loaded" % libpath)
        return testLibs[name]

    try:
        if is_global:
            handle = ctypes.CDLL(libpath, mode=ctypes.RTLD_GLOBAL)
        else:
            handle = ctypes.CDLL(libpath)
    except:
        handle = None
        logger.info("Failed to load library %s" % libpath)

    if handle:
        testLibs[name] = handle
    return handle
