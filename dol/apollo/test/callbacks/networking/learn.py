# /usr/bin/python3
import pdb

def GetLearnDelay(testcase, args):
    if args.delay_type == "age":
        return testcase.config.devicecfg.LearnAgeTimeout
    else:
        return 30
