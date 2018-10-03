#! /usr/bin/python3
import glob

import iota.harness.infra.utils.parser as parser
import iota.harness.infra.testsuite as testsuite
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
    expr = GlobalOptions.topdir + '/iota/test/**/*.testsuite'
    for filename in glob.iglob(expr, recursive = True):
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
