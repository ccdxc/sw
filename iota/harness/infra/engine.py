#! /usr/bin/python3
import glob
import json
import pdb

from iota.harness.infra.utils.logger import Logger as Logger
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.testsuite as testsuite
import iota.harness.infra.testbed as testbed
import iota.harness.api as api
import iota.harness.infra.types as types
#import iota.harness.infra.store as store


from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.harness.infra.exceptions import *

SuiteRunFile = GlobalOptions.logdir +  "/" +   "testsuite.log"

class YmlObject(object):
    def __init__(self, d):
        for a, b in d.items():
            if isinstance(b, (list, tuple)):
                setattr(self, a, [YmlObject(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, YmlObject(b) if isinstance(b, dict) else b)

def __discover_testsuites():
    suites = []
    expr = GlobalOptions.topdir + '/iota/test/%s/**/*.testsuite' %\
                                  (GlobalOptions.pipeline)
    for filename in glob.iglob(expr, recursive = True):
        Logger.debug("Reading testsuite: %s" % filename)
        data = parser.YmlParse(filename)
        suites.append(data)
    return suites

def SkipSetupValid():
    if len(GlobalOptions.testsuites) != 1:
        return False

    try:
        with open(SuiteRunFile, 'r') as json_file:
            data = json.load(json_file)
            return data.get("name", None) == GlobalOptions.testsuites[0] and data.get("setupDone", False)
    except:
        pass
    Logger.info("Skip setup not valid...")
    return False

def SaveSuiteInfo(suite):
    testsuiteInfo = {"setupDone" : True,
                    "name" : suite.meta.name }
    with open(SuiteRunFile, 'w') as outfile:
        json.dump(testsuiteInfo, outfile, indent=4)

def Main():
    api.Init()

    # Parse all the testsuites
    ts_specs = __discover_testsuites()

    testsuites = []
    result = types.status.SUCCESS
    for ts_spec in ts_specs:
        ts = testsuite.TestSuite(ts_spec)
        testsuites.append(ts)
        if ts.IsSkipped():
            continue
        try:
            ret = ts.Main()
        except OfflineTestbedException:
            result = types.status.OFFLINE_TESTBED
            break
        #Help in skip setup option
        finally:
            if ts.SetupComplete():
                SaveSuiteInfo(ts_spec)
        if ret != types.status.SUCCESS:
            result = ret
            if GlobalOptions.no_keep_going:
                break
            if result == types.status.OFFLINE_TESTBED:
                break
    for ts in testsuites:
        ts.PrintReport()

    for ts in testsuites:
        ts.PrintBundleSummary()
   
    print("\n\nOverall Run Summary")
    print(types.HEADER_SHORT_SUMMARY)
    print(types.FORMAT_ALL_TESTSUITE_SUMMARY %\
          ("Testsuite", "Pass", "Fail", "Ignore", "Dis/Err", "Total", "Target", "%Done", "Result", "Duration"))
    print(types.HEADER_SHORT_SUMMARY)
    for ts in testsuites:
        ts.PrintSummary()
    print(types.HEADER_SHORT_SUMMARY)

    if GlobalOptions.coverage_file and GlobalOptions.coverage_dir:
       api.GetCoverageFiles(GlobalOptions.coverage_file, GlobalOptions.coverage_dir)

    return result
