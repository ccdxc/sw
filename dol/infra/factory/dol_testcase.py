#! /usr/bin/python3

import copy

from infra.factory.testcase import * 
from infra.engine.trigger2  import DolTriggerEngine
from infra.engine.verif import DolVerifEngine

gl_ignore_fields = {
#    'IP'    : [ 'chksum' ],
#    'TCP'   : [ 'chksum' ],
#    'UDP'   : [ 'chksum' ],
#    'ICMP'  : [ 'chksum' ],
    'CRC'       : [ 'crc' ],
    'ESP'       : [ 'data' ],
#    'PADDING'   : [ 'data' ],
}

class DOLTestCaseTxRxStatsEntry:
    def __init__(self):
        self.obj = None
        self.npkts = 0
        self.nbytes = 0
        return

class DOLTestCaseTxRxStats:
    def __init__(self):
        self.entries = []
        return

    def AddStatsEntry(self, obj, npkts, nbytes):
        entry = DOLTestCaseTxRxStatsEntry()
        entry.obj = obj
        entry.npkts = npkts
        entry.nbytes = nbytes
        self.entries.append(entry)
        return

class DOLTestCaseStats:
    def __init__(self):
        self.tx = DOLTestCaseTxRxStats()
        self.rx = DOLTestCaseTxRxStats()
        return

class DOLTestCase(TestCase):
    def __init__(self, tcid, root, module, loopid = 0):
        super().__init__(tcid, root, module, loopid)
        self.trigger_engine = DolTriggerEngine
        self.verif_engine = DolVerifEngine
        self.drop = False

        if root:
           self.SetRetryEnabled(root.IsRetryEnabled())
        self.pendol = module.IsPendolHeaderEnabled()
        self._setup_config(root)

        self.packets        = objects.ObjectDatabase()
        self.descriptors    = objects.ObjectDatabase()
        self.buffers        = objects.ObjectDatabase()
        self.objects        = objects.ObjectDatabase()

        self.tracker        = module.GetTracker()
        self.coverage       = TestCaseCoverageHelper(self)
        self.stats          = DOLTestCaseStats()

        self.ignore_pkt_fields = copy.deepcopy(gl_ignore_fields)
        super()._generate()
        return

    def IsPendolHeaderEnabled(self):
        return self.pendol

    def SetDrop(self):
        self.drop = True
        return
    
    def IsDrop(self):
        return self.drop

    def AddIgnorePacketField(self, hdr, field):
        if hdr not in self.ignore_pkt_fields:
            self.ignore_pkt_fields[hdr] = []
        self.ignore_pkt_fields[hdr].append(field)
        return

    def GetIgnorePacketFields(self):
        return self.ignore_pkt_fields

    def __generate_packets_from_spec(self, pspec):
        pktspec = getattr(pspec, 'packet', None)
        if pktspec is None: return
        if pktspec.id is None: return
        packet = pktfactory.Packet(self, pktspec)
        packet.Build(self)
        self.packets.Add(packet)
        return

    def __generate_packets_from_callback(self, pspec):
        cb = getattr(pspec, 'callback', None)
        if cb is None: return
        pkts = cb.call(self)
        if pkts:
            self.packets.SetAll(pkts)
        return

    def __generate_packets(self):
        logger.info("Generating Packet Objects")
        if self.testspec.packets == None: return
        for pspec in self.testspec.packets:
            self.__generate_packets_from_spec(pspec)
            self.__generate_packets_from_callback(pspec)
        return

    def _generate_objects(self):
        if self.testspec is None:
            return

        self.__generate_packets()
        memfactory.GenerateBuffers(self)
        memfactory.GenerateDescriptors(self)
        return

    def __setup_packet(self, step_id, spkt):
        if spkt.packet.object == None:
            return

        tpkt = TestCaseTrigExpPacketObject()
        if objects.IsCallback(spkt.packet.object):
            tpkt.packet = spkt.packet.object.call(self)
            if tpkt.packet is None:
                logger.info("- Packet Callback return None")
                return None
        else: # Its a reference
            tpkt.packet = spkt.packet.object.Get(self)
        
        tpkt.packet = copy.deepcopy(tpkt.packet)
        tpkt.packet.SetStepId(step_id)
        tpkt.packet.Build(self)
        return tpkt

    def __setup_packet_ports(self, tpkt, spkt):
        # Resolve the ports
        if objects.IsReference(spkt.packet.port):
            tpkt.ports = spkt.packet.port.Get(self)
        elif objects.IsCallback(spkt.packet.port):
            tpkt.ports = spkt.packet.port.call(self)
        else:
            tpkt.ports = [ spkt.packet.port ]

        if not isinstance(tpkt.ports, list):
            tpkt.ports = [ tpkt.ports ]
        return

    def __setup_packets(self, step_id, tcsn, spsn, stats):
        if spsn.packets == None: return
        for spkt in spsn.packets:
            logger.info("- Setting up  Packet: %s:" % spkt)
            tpkt = self.__setup_packet(step_id, spkt)            
            if tpkt is None:
                continue

            self.__setup_packet_ports(tpkt, spkt)
            logger.info("- Adding Packet: %s, Ports :" %\
                        tpkt.packet.GID(), tpkt.ports)
            tpkt.packet.Show()
            tcsn.packets.append(tpkt)
            stats.AddStatsEntry(tpkt.ports[0], 1, len(tpkt.packet.rawbytes))
        return

    def __setup_descriptors(self, tcsn, spsn, stats):
        if spsn.descriptors == None: return
        for spec_desc_entry in spsn.descriptors:
            if spec_desc_entry.descriptor.object == None: continue
            tc_desc_spec = TestCaseTrigExpDescriptorSpec()
            tc_desc_spec.descriptor.negtest = getattr(spec_desc_entry.descriptor, 'negtest', False)
            tc_desc_spec.descriptor.object = spec_desc_entry.descriptor.object.Get(self)
            if objects.IsCallback(spec_desc_entry.descriptor.ring):
                tc_desc_spec.descriptor.ring = spec_desc_entry.descriptor.ring.call(self)
            else:
                tc_desc_spec.descriptor.ring   = spec_desc_entry.descriptor.ring.Get(self)
            if tc_desc_spec.descriptor.ring == None: continue
            logger.info("- Adding Descriptor: %s, Ring: %s" %\
                      (tc_desc_spec.descriptor.object.GID(), tc_desc_spec.descriptor.ring.GID()))
            buff = getattr(spec_desc_entry.descriptor, 'buffer', None)
            if buff:
                tc_desc_spec.descriptor.buffer = spec_desc_entry.descriptor.buffer.Get(self)
                logger.info("  - Expected Buffer: %s" %\
                          tc_desc_spec.descriptor.buffer.GID())
                packet = getattr(spec_desc_entry.descriptor, 'packet', None)
                if packet:
                    tc_desc_spec.descriptor.packet = spec_desc_entry.descriptor.packet.Get(self)
                    logger.info("  - Expected Packet: %s" %\
                              tc_desc_spec.descriptor.packet.GID())
            tcsn.descriptors.append(tc_desc_spec)
            npkts,nbytes = tc_desc_spec.descriptor.object.GetTxPktByteStats()
            stats.AddStatsEntry(tc_desc_spec.descriptor.ring.GetLif(), npkts, nbytes)
        return
 
    def __setup_doorbell(self, tcsn, spsn):
        if spsn.doorbell == None: return
        if spsn.doorbell.object == None: return
        if objects.IsCallback(spsn.doorbell.object):
            tcsn.doorbell.object = spsn.doorbell.object.call(self)
        else:
            tcsn.doorbell.object = spsn.doorbell.object.Get(self)
        tcsn.doorbell.spec = spsn.doorbell.fields
        logger.info("- Adding Doorbell: %s" % tcsn.doorbell.object.GID())
        return
   
    def __setup_config_objects(self, tcsn, spsn):
        if not hasattr(spsn, "configs") or spsn.configs == None:
            return
        for spec_config_entry in spsn.configs:
            cfg_obj = spec_config_entry.object
            if cfg_obj == None:
                continue
            tc_config = TestCaseTrigExpConfigObject()
            if objects.IsCallback(cfg_obj):
                tc_config.actual_object = cfg_obj.call(self)
            else: # Its a reference
                tc_config.actual_object = cfg_obj.Get(self)
            tc_config.original_object = copy.copy(tc_config.actual_object)
            cfg_method = getattr(spec_config_entry, "method", None)
            if objects.IsCallback(cfg_method):
                tc_config.method = cfg_method.call(self)
            else:
                tc_config.method = cfg_method
            tc_config.spec = getattr(spec_config_entry, "fields", None)
            tcsn.configs.append(tc_config)
   
    #def __setup_ignore_excess_packets(self, tcsn, spsn):
    #    tcsn.ignore_excess_packets = getattr(spsn, 'ignore_excess_packets', False)

    def __setup_delay(self, tcsn, spsn):
        spdelay = getattr(spsn, 'delay', 0)
        if objects.IsCallback(spdelay):
            tcsn.delay = spdelay.call(self)
        elif objects.IsReference(spdelay):
            tcsn.delay = spdelay.Get(self)
        else:
            tcsn.delay = spdelay
        if tcsn.delay:
            logger.info("  - Adding Delay of %d seconds." % tcsn.delay)
        return

    def _setup_trigger(self, tcstep, spstep):
        logger.info("- Setting up Trigger.")
        if spstep.trigger == None: return
        self.__setup_delay(tcstep.trigger, spstep.trigger)
        self.__setup_packets(tcstep.step_id, tcstep.trigger,
                             spstep.trigger, self.stats.tx)
        self.__setup_descriptors(tcstep.trigger, spstep.trigger, self.stats.tx)
        self.__setup_config_objects(tcstep.trigger, spstep.trigger)
        self.__setup_doorbell(tcstep.trigger, spstep.trigger)
        return defs.status.SUCCESS

    def _setup_expect(self, tcstep, spstep):
        logger.info("- Setting up Expect.")
        if spstep.expect == None: return
        self.__setup_delay(tcstep.expect, spstep.expect)
        #self.__setup_ignore_excess_packets(tcstep.expect, spstep.expect)
        self.__setup_packets(tcstep.step_id, tcstep.expect,
                             spstep.expect, self.stats.rx)
        self.__setup_descriptors(tcstep.expect, spstep.expect, self.stats.rx)
        self.__setup_config_objects(tcstep.expect, spstep.expect)
        if hasattr(spstep.expect, "callback"):
            assert(objects.IsCallback(spstep.expect.callback))
            tcstep.expect.callback = spstep.expect.callback
        return defs.status.SUCCESS

    
    def TeardownCallback(self):
        super().TeardownCallback()
        ModelConnector.TestCaseEnd(self.GID(), self.loopid)
        logger.SetTestcase(None)
        self.coverage.Process()
        return
