#! /usr/bin/python3

def Setup(infra, module):
    print("Setup(): Sample Implementation")
    if module.args:
        for arg in module.args:
            print("- Arg: ", arg)
    return

def Teardown(infra, module):
    print("Teardown(): Sample Implementation.")
    return

def TestCaseSetup(tc):
    print("TestCaseSetup(): Sample Implementation.")
    return

def TestCaseVerify(tc):
    print("TestCaseVerify(): Sample Implementation.")
    return True

def TestCaseTeardown(tc):
    print("TestCaseTeardown(): Sample Implementation.")
    return

