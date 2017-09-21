#! /usr/bin/python3

import infra.common.parser      as parser
import infra.common.objects     as objects

from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
from infra.engine.modmgr        import ModuleStore

FeatureStore = objects.ObjectDatabase(logger)

class FeatureObject:
    def __init__(self, spec):
        self.spec       = spec
        self.id         = spec.feature.id
        self.package    = spec.feature.package
        self.module     = spec.feature.module
        self.enable     = spec.feature.enable
        self.ignore     = spec.feature.ignore

        self.tms        = []
        self.__parse_modules()
        return

    def __parse_modules(self):
        for m in self.spec.modules:
            mspec = m.module
            mspec.feature = self.id
            mspec.enable = getattr(mspec, 'enable', self.enable)
            mspec.module = getattr(mspec, 'module', self.module)
            mspec.package = getattr(mspec, 'package', self.package)
            ModuleStore.Add(mspec)
        return

class FeatureObjectHelper(parser.ParserBase):
    def __init__(self):
        self.features = []
        return

    def __is_match(self, spec):
        if GlobalOptions.feature is None:
            # For Global runs, dont run FTE tests.
            # TEMP HACK: till all the features have their mlist files.
            if spec.feature.id == 'fte':
                return False
            return True
        return spec.feature.id in GlobalOptions.feature

    def __is_enabled(self, spec):
        if spec.feature.enable is False:
            return False

        return self.__is_match(spec)

    def Parse(self):
        ftlist = super().Parse('test/', '*.mlist')
        for fspec in ftlist:
            logger.info("Loading Feature Test Module List: %s" % fspec.feature.id)
            if self.__is_enabled(fspec) is False:
                logger.info("  - Disabled....Skipping")
                continue
            obj = FeatureObject(fspec)
            self.features.append(obj)
        return

def Init():
    if GlobalOptions.feature is not None:
        GlobalOptions.feature = GlobalOptions.feature.split(',')
    FtlHelper = FeatureObjectHelper()
    FtlHelper.Parse()
    return
