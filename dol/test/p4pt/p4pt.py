# Testcase definition file.

import pdb

def Setup(infra, module):
    print("Setup(): P4PT Setup")
    elem = module.iterator.Get()
    return

def Teardown(infra, module):
    print("Teardown(): P4PT Teardown")
    return

def TestCaseSetup(tc):
    print("Setup(): P4PT Test Case Setup")
    return

def TestCaseVerify(tc):
    print("Verify(): P4PT Verify Setup... ", tc.module.name)
    # 1. TBD: Read HBM p4pt records
    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): P4PT Teardown... ", tc.module.name)
    return
