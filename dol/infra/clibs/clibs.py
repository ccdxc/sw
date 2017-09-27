#! /usr/bin/python3

import ctypes
import os
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions

libHostMem = None

# A dict to map a library name to its handle.
testLibs = {}

OBJPATH = "nic/obj/lib"
LIBHOSTMEM = "hostmem.so"

def InitHostMem():
    global libHostMem
    libHostMem = ctypes.CDLL(os.path.join(os.environ['WS_TOP'], "bazel-bin/nic/utils/host_mem/libhost_mem.so"), mode=ctypes.RTLD_GLOBAL)
    libHostMem.alloc_host_mem.argtypes = [ctypes.c_uint64]
    libHostMem.alloc_host_mem.restype = ctypes.c_void_p
    libHostMem.alloc_page_aligned_host_mem.argtypes = [ctypes.c_uint64]
    libHostMem.alloc_page_aligned_host_mem.restype = ctypes.c_void_p
    libHostMem.host_mem_v2p.argtypes = [ctypes.c_void_p]
    libHostMem.host_mem_v2p.restype = ctypes.c_uint64
    libHostMem.host_mem_p2v.argtypes = [ctypes.c_uint64]
    libHostMem.host_mem_p2v.restype = ctypes.c_void_p
    libHostMem.free_host_mem.argtypes = [ctypes.c_void_p]
    libHostMem.free_host_mem.restype = None
    if not GlobalOptions.dryrun:
        libHostMem.init_host_mem()

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

def init():
     InitHostMem()
     return
