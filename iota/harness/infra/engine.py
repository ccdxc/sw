#! /usr/bin/python3
import glob

from iota.harness.infra.utils.logger import Logger as Logger
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.testsuite as testsuite
import iota.harness.infra.testbed as testbed
import iota.harness.api as api
import iota.harness.infra.types as types
#import iota.harness.infra.store as store

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

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

def Main():
    api.Init()

    # Parse all the testsuites
    ts_specs = __discover_testsuites()

    testsuites = []
    result = types.status.SUCCESS
    for ts_spec in ts_specs:
        ts = testsuite.TestSuite(ts_spec)
        testsuites.append(ts)
        ret = ts.Main()
        if ret != types.status.SUCCESS:
            result = ret
            if GlobalOptions.no_keep_going:
                break

    for ts in testsuites:
        ts.PrintReport()

    for ts in testsuites:
        ts.PrintBundleSummary()
   
    print("\n\nOverall Run Summary")
    print(types.HEADER_SHORT_SUMMARY)
    print(types.FORMAT_ALL_TESTSUITE_SUMMARY %\
          ("Testsuite", "Pass", "Fail", "Ignore", "Error", "Total", "Target", "Result", "Duration"))
    print(types.HEADER_SHORT_SUMMARY)
    for ts in testsuites:
        ts.PrintSummary()
    print(types.HEADER_SHORT_SUMMARY)

    return result
