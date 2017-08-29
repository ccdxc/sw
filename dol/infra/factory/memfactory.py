#! /usr/bin/python3
import pdb
import infra.factory.template   as template
import infra.common.loader      as loader

from infra.factory.store    import FactoryStore
from infra.common.logging   import memlogger

def init():
    memlogger.info("Loading Descriptor templates.")
    objlist = template.ParseDescriptorTemplates()
    FactoryStore.templates.SetAll(objlist)

    memlogger.info("Loading Buffer templates.")
    objlist = template.ParseBufferTemplates()
    FactoryStore.templates.SetAll(objlist)

def __generate_common(tc, spec):
    template = spec.template.Get(FactoryStore)
    obj = template.CreateObjectInstance()
    obj.SetLogger(tc)
    obj.GID(spec.id)
    obj.Init(spec)
    return obj

def GenerateDescriptors(tc):
    tc.info("Generating Descriptors")
    for desc_entry in tc.testspec.descriptors:
        if desc_entry.descriptor.id == None: continue
        obj = __generate_common(tc, desc_entry.descriptor)
        tc.descriptors.Add(obj)
    return

def GenerateBuffers(tc):
    tc.info("Generating Buffers")
    for buff_entry in tc.testspec.buffers:
        if buff_entry.buffer.id == None: continue
        obj = __generate_common(tc, buff_entry.buffer)
        tc.buffers.Add(obj)
    return
