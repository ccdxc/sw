#! /usr/bin/python3
import glob

from iota.harness.infra.utils.logger import Logger as Logger
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.testsuite as testsuite
import iota.harness.infra.testbed as testbed
import iota.harness.api as api
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
    if GlobalOptions.cfgmode is None:
        expr = GlobalOptions.topdir +\
               '/iota/test/%s/**/*.testsuite' %\
               (GlobalOptions.pipeline)
    else:
        expr = GlobalOptions.topdir +\
               '/iota/test/%s/%s/**/*.testsuite' %\
               (GlobalOptions.pipeline, GlobalOptions.cfgmode)

    for filename in glob.iglob(expr, recursive = True):
        Logger.info("Reading testsuite: %s" % filename)
        data = parser.YmlParse(filename)
        suites.append(data)
    return suites

def Main():
    api.Init()

    # Parse all the testsuites
    suites = __discover_testsuites()

    for s in suites:
        ts = testsuite.TestSuite(s)
        ts.Main()
    return
