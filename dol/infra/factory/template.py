# Template Manager
import pdb
import copy

import infra.common.defs            as defs
import infra.common.utils           as utils
import infra.common.dyml            as dyml
import infra.common.objects         as objects
import infra.common.parser          as parser
import infra.common.loader          as loader

from infra.common.logging   import pktlogger
from infra.factory.store    import FactoryStore

class FactoryTemplate(objects.FrameworkTemplateObject):
    def __init__(self, yobj):
        super().__init__()
        if yobj:
            self.Clone(yobj)
            self.GID(yobj.meta.id)
        return

class FactoryObjectTemplate(FactoryTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        if yobj:
            self.package    = yobj.meta.package
            self.module     = yobj.meta.module
            self.objname    = yobj.meta.objname
        return

    def CreateObjectInstance(self):
        obj = loader.CreateObjectInstance(self.package,
                                          self.module,
                                          self.objname)
        return obj

class DescriptorTemplate(FactoryObjectTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        return

class BufferTemplate(FactoryObjectTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        return

class HeaderTemplate(FactoryTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        return

class PacketHeadersTemplate(FactoryTemplate):
    def __init__(self):
        super().__init__(None)
        return

class TestobjectTemplate(FactoryTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        return

class PacketTemplate(FactoryTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        self.__process_headers(yobj)
        return

    def __process_one_header(self, header_spec):
        header_template = header_spec.template.Get(FactoryStore)
        assert(header_template)
        header = copy.deepcopy(header_template)
        header.fields = objects.MergeObjects(header_spec.fields,
                                             header_template.fields)
        self.headers.__dict__[header_spec.id] = header
        self.hdrsorder.append(header_spec.id)
        return
        

    def __process_headers(self, yobj):
        self.hdrsorder = []
        self.headers = PacketHeadersTemplate()
        for hentry in yobj.headers:
            self.__process_one_header(hentry.header)
        return

class PayloadTemplate(FactoryTemplate):
    def __init__(self, yobj):
        super().__init__(yobj)
        self.size = yobj.size
        self.data = utils.ParseIntegerList(yobj.data)
        return

def __parse_common(path, extn, template_class):
    parsed_objlist = parser.ParseDirectory(path, extn)
    objlist = []
    for p in parsed_objlist:
        obj = template_class(p)
        objlist.append(obj)
    return objlist

def ParseHeaderTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_HDRS_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          HeaderTemplate)

def ParsePacketTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_PKTS_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          PacketTemplate)

def ParsePayloadTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_PYLD_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          PayloadTemplate)

def ParseDescriptorTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_DESC_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          DescriptorTemplate)

def ParseBufferTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_BUFS_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          BufferTemplate)

def ParseTestobjectTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_TESTOBJECTS_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          TestobjectTemplate)

def ParseTestTypeTemplates():
    return __parse_common(defs.FACTORY_TEMPLATE_TESTTYPES_PATH,
                          defs.TEMPLATE_FILE_EXTN,
                          TestobjectTemplate)