# Configuration File Parser
import pdb

import infra.common.defs    as defs
import infra.common.parser  as parser
import infra.common.logging as logging

from infra.common.logging   import logger
from infra.common.glopts    import GlobalOptions

class ConfigFileParser(parser.ParserBase):
    def __init__(self):
        super().__init__()
        return

    def __parse(self, path, extn):
        return super().Parse(path, extn)

    def __resolve_obj(self, obj, store):
        obj.ID(obj.meta.id)
        return

    def __resolve(self, objlist, store):
        if store:
            for obj in objlist:
                self.__resolve_obj(obj, store)
        return

    def ParseSpecs(self, store = None):
        splist = self.__parse(defs.CONFIG_SPEC_PATH, defs.SPEC_FILE_EXTN)
        self.__resolve(splist, store)

        path = 'config/topology/%s/' % GlobalOptions.topology
        tsplist = self.__parse(path, defs.SPEC_FILE_EXTN)
        self.__resolve(tsplist, store)

        return splist + tsplist

    def ParseTemplates(self, store = None):
        objlist = self.__parse(defs.CONFIG_TEMPLATE_PATH,
                               defs.TEMPLATE_FILE_EXTN)
        self.__resolve(objlist, store)
        return objlist

ConfigParser = ConfigFileParser()
