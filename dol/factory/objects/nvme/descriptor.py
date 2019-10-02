#! /usr/bin/python3

from scapy.all import *

import iris.config.resmgr as resmgr

import infra.factory.base as base
import infra.common.objects as objects
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap

from factory.objects.nvme import buffer as nvmebuffer

from infra.factory.store    import FactoryStore

#64B
class NvmeSqDescriptor(Packet):
    fields_desc = [
        #dword 0
        ByteField("opc", 0),
        BitField("fuse", 0, 2),
        BitField("rsvd0", 0, 4),
        BitField("psdt", 0, 2),
        XLEShortField("cid", 0),
    
        #dword 1
        XLEIntField("nsid", 0),
    
        #dword 2-3
        XLEIntField("rsvd2", 0),
        XLEIntField("rsvd3", 0),

        #dword 4-5
        XLELongField("mptr", 0),

        #dword 6-9
        XLELongField("prp1", 0),
        XLELongField("prp2", 0),

        #dword 10-15
        XLEIntField("cdw10", 0),
        XLEIntField("cdw11", 0),
        XLEIntField("cdw12", 0),
        XLEIntField("cdw13", 0),
        XLEIntField("cdw14", 0),
        XLEIntField("cdw15", 0),
    ]

#40B
class NvmeSqDescriptorBase(Packet):
    fields_desc = [
        #dword 0
        ByteField("opc", 0),
        BitField("fuse", 0, 2),
        BitField("rsvd0", 0, 4),
        BitField("psdt", 0, 2),
        XLEShortField("cid", 0),
    
        #dword 1
        XLEIntField("nsid", 0),
    
        #dword 2-3
        XLEIntField("rsvd2", 0),
        XLEIntField("rsvd3", 0),

        #dword 4-5
        XLELongField("mptr", 0),

        #dword 6-9
        XLELongField("prp1", 0),
        XLELongField("prp2", 0),
    ]

#24B
class NvmeSqDescriptorWrite(Packet):
    fields_desc = [
        #dword 10-11
        XLELongField("slba", 0),

        #dword 12
        XLEShortField("nlb", 0),
        BitField("rsvd12", 0, 10),
        BitField("prinfo", 0, 4),
        BitField("fua", 0, 1),
        BitField("lr", 0, 1),

        #dword 13
        BitField("dsm", 0, 8),
        BitField("rsvd13", 0, 24),

        #dword 14
        XLEIntField("ilbrt", 0),

        #dword 15
        XLEShortField("lbat", 0),
        XLEShortField("lbatm", 0),
    ]

#16B
class NvmeCqDescriptor(Packet):
    fields_desc = [
        #dword 0
        IntField("cdw0", 0),

        #dword 1
        IntField("rsvd1", 0), 

        #dword 2
        LEShortField("sqhd", 0),
        LEShortField("sqid", 0),

        #dword 3
        LEShortField("cid", 0),
        BitField("p", 0, 1),
        BitField("sc", 0, 8),
        BitField("sct", 0, 3),
        BitField("rsvd2", 0, 2),
        BitField("m", 0, 1),
        BitField("dnr", 0, 1),
    ]

#TBD, for now reserve 16B
class NvmeArmqDescriptor(Packet):
    fields_desc = [
        BitField("rsvd1", 0, 128),
    ]

#2B
class NvmeSessqDescriptor(Packet):
    fields_desc = [
        ShortField("cid", 0),
    ]

#8B
class NvmeTcprqDescriptor(Packet):
    fields_desc = [
        ShortField("pad", 0),
        BitField("len", 0, 14),
        BitField("addr", 0, 34),
    ]

NVME_OPC_FLUSH = 0
NVME_OPC_WRITE = 1
NVME_OPC_READ  = 2
NVME_OPC_WRITE_UNC = 3
NVME_OPC_WRITE_ZEROES = 4


class NvmeSqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        self.nvme_session = None
        self.prp1 = 0
        self.prp2 = 0

    def Init(self, spec):
        super().Init(spec)

        if hasattr(spec.fields, 'session'):
            self.nvme_session = spec.fields.session
        else:
            logger.error("Error!! nvme session needs to be specified for the buffer")
            exit

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """

        opc = self.spec.fields.opc
        if opc in (NVME_OPC_FLUSH, NVME_OPC_WRITE, NVME_OPC_READ, NVME_OPC_WRITE_UNC, NVME_OPC_WRITE_ZEROES):
            #requires data to be specified
            if hasattr(self.spec.fields, 'data'):
                data_buffer = self.spec.fields.data
            else:
                logger.error("Error!! nvme buffer needs to be specified for the descriptor")
                return

            self.prp1 = data_buffer.phy_pages[0]
    
            num_pages = data_buffer.num_pages

            if num_pages == 2:
               self.prp2 = data_buffer.phy_pages[1]
            #prepare PRP list page
            elif num_pages > 2:

               if hasattr(self.spec.fields, "prp2_offset"):
                   prp2_offset = self.spec.fields.prp2_offset
                   assert prp2_offset < data_buffer.page_size - 8
               else:
                   prp2_offset = 0

               prp2_entries = (data_buffer.page_size - prp2_offset) / 8

               prp2_begin = 1
               prp2_end = (int) (min(prp2_entries, num_pages-1))

               logger.info("PRP2 begin: %d, PRP2 end: %d, PRP2 offset: %d, prp2_entries: %d" %
                           (prp2_begin, prp2_end, prp2_offset, prp2_entries))

               prp3_required = 0
               #prp3 required
               if prp2_end < num_pages - 1:

                  prp3_required = 1

                  prp2_end -=1

                  prp3_begin = (int) (prp2_end + 1)
                  prp3_entries = num_pages - prp2_end - 1
                  prp3_end = (int) (num_pages - 1)
                  assert prp3_end > prp3_begin
                  prp3_offset = 0
     
                  logger.info("PRP3 begin: %d, PRP3 end: %d, PRP3 offset: %d, prp3_entries: %d" %
                              (prp3_begin, prp3_end, prp3_offset, prp3_entries))

                  prp3_slab = self.nvme_session.lif.GetNextSlab()
                  logger.info("Slab with address 0x%x allocated for PRP3 list" % (prp3_slab.address))
                  self.prp3 = resmgr.HostMemoryAllocator.v2p(prp3_slab.address)
                  data = []
                  for i in range(prp3_begin, prp3_end+1):
                      data += data_buffer.phy_pages[i].to_bytes(8, 'little')
                  print ("%s" % (data))
                  mem_handle = objects.MemHandle(prp3_slab.address + prp3_offset, resmgr.HostMemoryAllocator.v2p(prp3_slab.address + prp3_offset))
                  resmgr.HostMemoryAllocator.write(mem_handle, bytes(data))

               prp2_slab = self.nvme_session.lif.GetNextSlab()
               logger.info("Slab with address 0x%x allocated for PRP2 list" % (prp2_slab.address))
               self.prp2 = resmgr.HostMemoryAllocator.v2p(prp2_slab.address + prp2_offset)
               data = []
               for i in range(prp2_begin, prp2_end+1):
                   data += data_buffer.phy_pages[i].to_bytes(8, 'little')
               print ("%s" % (data))
               if prp3_required:
                   data += resmgr.HostMemoryAllocator.v2p(prp3_slab.address).to_bytes(8, "little")
               print ("%s" % (data))
               mem_handle = objects.MemHandle(prp2_slab.address + prp2_offset,
                                             resmgr.HostMemoryAllocator.v2p(prp2_slab.address + prp2_offset))
               resmgr.HostMemoryAllocator.write(mem_handle, bytes(data))
    
        desc = NvmeSqDescriptorBase(opc=self.spec.fields.opc,
                                    cid=self.spec.fields.cid,
                                    nsid=self.spec.fields.nsid,
                                    prp1=self.prp1, 
                                    prp2=self.prp2)

        logger.ShowScapyObject(desc)

        self.desc = desc

        if hasattr(self.spec.fields, 'write'):
            slba = self.spec.fields.write.slba if hasattr(self.spec.fields.write, 'slba') else 0
            nlb = self.spec.fields.write.nlb if hasattr(self.spec.fields.write, 'nlb') else 0
            write = NvmeSqDescriptorWrite(slba=slba,
                                          nlb=nlb)

            self.desc = self.desc / write

            logger.ShowScapyObject(write)

        logger.info("desc_size = %d" %(len(self.desc)))

        resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(self.desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        desc = NvmeSqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, 64))

        logger.ShowScapyObject(desc)

    def Show(self):
        pass

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor")
        return  self == other

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor, ignoring for now..")

        nvmebuff = nvmebuffer.NvmeBufferObject()
        nvmebuff.data = None
        nvmebuff.size = 0
        return nvmebuff

class NvmeCqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()

    def Init(self, spec):
        super().Init(spec)

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        desc = NvmeCqDescriptor(sqhd=self.spec.fields.sqhd,
                                sqid=self.spec.fields.sqid,
                                p=self.spec.fields.p,
                                sc=self.spec.fields.sc,
                                sct=self.spec.fields.sct)
                                
        self.desc = desc

        logger.ShowScapyObject(desc)

        logger.info("desc_size = %d" %(len(desc)))

        resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
        mem_handle = objects.MemHandle(self.address, self.phy_address)
        desc = NvmeCqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, 64))

        logger.ShowScapyObject(desc)

    def Show(self):
        pass

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor")
        return  self == other

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor, ignoring for now..")

        nvmebuff = nvmebuffer.NvmeBufferObject()
        nvmebuff.data = None
        nvmebuff.size = 0
        return nvmebuff

class NvmeArmqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()

    def Init(self, spec):
        super().Init(spec)

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        desc = NvmeArmqDescriptor()
                                
        self.desc = desc

        logger.ShowScapyObject(desc)

        logger.info("desc_size = %d" %(len(desc)))

        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        #TBD: Do we need it ?
        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = objects.MemHandle(self.address, self.phy_address)
            desc = NvmeArmqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, 64))
        else:
            hbm_addr = self.address
            desc = NvmeArmqDescriptor(model_wrap.read_mem(hbm_addr, 64))

        logger.ShowScapyObject(desc)

    def Show(self):
        pass

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor")
        return  self == other

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor, ignoring for now..")

        nvmebuff = nvmebuffer.NvmeBufferObject()
        nvmebuff.data = None
        nvmebuff.size = 0
        return nvmebuff

class NvmeSessqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()

    def Init(self, spec):
        super().Init(spec)

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        desc = NvmeSessqDescriptor()
                                
        self.desc = desc

        logger.ShowScapyObject(desc)

        logger.info("desc_size = %d" %(len(desc)))

        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        #TBD: Do we need it ?
        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = objects.MemHandle(self.address, self.phy_address)
            desc = NvmeSessqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, 64))
        else:
            hbm_addr = self.address
            desc = NvmeSessqDescriptor(model_wrap.read_mem(hbm_addr, 64))

        logger.ShowScapyObject(desc)

    def Show(self):
        pass

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor")
        return  self == other

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor, ignoring for now..")

        nvmebuff = nvmebuffer.NvmeBufferObject()
        nvmebuff.data = None
        nvmebuff.size = 0
        return nvmebuff

class NvmeTcprqDescriptorObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()

    def Init(self, spec):
        super().Init(spec)

    def Write(self):
        """
        Creates a Descriptor at "self.address"
        :return:
        """
        desc = NvmeTcprqDescriptor()
                                
        self.desc = desc

        logger.ShowScapyObject(desc)

        logger.info("desc_size = %d" %(len(desc)))

        if self.mem_handle:
            resmgr.HostMemoryAllocator.write(self.mem_handle, bytes(desc))
        else:
            model_wrap.write_mem_pcie(self.address, bytes(desc), len(desc))

        #TBD: Do we need it ?
        self.Read()

    def Read(self):
        """
        Reads a Descriptor from "self.address"
        :return:
        """
        if self.mem_handle:
            self.phy_address = resmgr.HostMemoryAllocator.v2p(self.address)
            mem_handle = objects.MemHandle(self.address, self.phy_address)
            desc = NvmeTcprqDescriptor(resmgr.HostMemoryAllocator.read(mem_handle, 64))
        else:
            hbm_addr = self.address
            desc = NvmeTcprqDescriptor(model_wrap.read_mem(hbm_addr, 64))

        logger.ShowScapyObject(desc)

    def Show(self):
        pass

    def __eq__(self, other):
        logger.info("__eq__ operator invoked on descriptor")
        return  self == other

    def GetBuffer(self):
        logger.info("GetBuffer() operator invoked on descriptor, ignoring for now..")

        nvmebuff = nvmebuffer.NvmeBufferObject()
        nvmebuff.data = None
        nvmebuff.size = 0
        return nvmebuff

