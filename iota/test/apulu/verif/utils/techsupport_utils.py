#! /usr/bin/python3
from pathlib import Path
import tarfile

import iota.harness.api as api
import iota.test.apulu.utils.naples as naples_utils

TS_DIR = "/data/techsupport/"

def GetTechsupportFilename(tsFile):
    # return only the filename without path
    return Path(tsFile).resolve().name

def DeleteTSDir(naples_nodes):
    return naples_utils.DeleteDirectories(naples_nodes, [TS_DIR])

def GetCoreFiles(tsFiles):
    """
    Returns
        list of core filenames - if core is present in techsupport
        None - otherwise
    ideally core must not be present.
    If core is present, then some process has crashed which need to be debugged.
    """
    cores = []
    for tsFile in tsFiles:
        with tarfile.open(tsFile, "r:gz") as tsTar:
            for tinfo in tsTar:
                fName = tinfo.name
                if not "data_core" in fName:
                    continue
                # look into core dir tarball to see if there is a corefile
                coreTarball = tsTar.extractfile(fName)
                with tarfile.open(fileobj=coreTarball, mode="r:gz") as coreTar:
                    for ctinfo in coreTar:
                        cfPath = Path(ctinfo.name).resolve()
                        cfName = cfPath.name
                        if cfPath.suffix == "":
                            # to avoid data/core dir
                            continue
                        if "core" in cfName:
                            # core file found
                            cores.append(cfName)
    return None if len(cores) is 0 else cores

def IsCoreCollected(tsFiles):
    """
    Returns
        True  - if core directory got collected as part of techsupport
        False - otherwise
    """
    for tsFile in tsFiles:
        with tarfile.open(tsFile, "r:gz") as tsTar:
            for tinfo in tsTar:
                if "data_core" in tinfo.name:
                    # /data/core/ directory is present
                    return True
    return False
