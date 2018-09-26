#! /usr/bin/python3
import pdb

from iota.harness.infra.utils.logger import Logger as Logger

import iota.harness.infra.utils.loader as loader

class TestSuite:
    def __init__(self, spec):
        self.spec = spec
        self.__resolve_setup()
        self.__resolve_common_verifs()
        self.__resolve_testcases()
        self.__resolve_teardown()
        return

    def __resolve_setup(self):
        for s in self.spec.setup:
            Logger.debug("Resolving setup module: %s" % s.step)
            s.step = loader.Import(s.step, self.spec.packages)
        return

    def __resolve_common_verifs(self):
        for cv in self.spec.common.verifs:
            Logger.debug("Resolving common verif module: %s" % cv.step)
            cv.step = loader.Import(cv.step, self.spec.packages)
        return

    def __resolve_testcases(self):
        for tc in self.spec.testcases:
            Logger.debug("Resolving testcase module: %s" % tc.testcase)
            tc.testcase = loader.Import(tc.testcase, self.spec.packages)
            for v in tc.verifs:
                Logger.debug("Resolving testcase verif module: %s" % v.step)
                v.step = loader.Import(v.step, self.spec.packages)
            tc.verifs.extend(self.spec.common.verifs)
        return

    def __resolve_teardown(self):
        for s in self.spec.teardown:
            Logger.debug("Resolving teardown module: %s" % s.step)
            s.step = loader.Import(s.step, self.spec.packages)
        return

    def Main(self):
        Logger.info("Running Testsuite: %s" % self.spec.meta.suite)
        return
