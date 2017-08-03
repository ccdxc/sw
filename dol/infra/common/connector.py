#! /usr/bin/python3
import socket
import json
import pickle
import model_sim.src.model_wrap as model_wrap
from infra.common.logging import logger
from infra.common.thread import EvQueue
import queue as Queue
from infra.common.utils import convert_object_to_dict
from collections import namedtuple
from infra.penscapy import penscapy
from factory.objects.ut.buffer import InfraUtBufferObject

PacketContext = namedtuple('PacketContext', ['sender_info', 'packet', 'port'])
RingContext = namedtuple('DescriptorContext', ['ring', 'descriptors'])

connector_registry = {}


def register_connector(target_class):
    connector_registry[target_class.__name__] = target_class


class Connector(object):
    _connectors_ = {}

    class Timeout(Exception):
        pass

    @classmethod
    def factory(cls, connector):
        try:
            return cls._connectors_[connector]
        except KeyError:
            assert 0

    @classmethod
    def register(cls, connector):
        def decorator(subclass):
            cls._connectors_[connector] = subclass
            return subclass
        return decorator

    def __init__(self, dest_addr):
        self._addr = dest_addr

    def send(self, port, data):
        pass

    def recv(self, size=0):
        pass

    def bind(self):
        pass

    def set_recv_timeout(self, timeout=0.5):
        pass

    def close(self):
        pass

    def doorbell(self, doorbell):
        pass

    def consume_rings(self, rings):
        pass


@Connector.register('DumbConnector')
class DumbConnector(Connector):

    def recv(self, size=16384):
        raise Connector.Timeout


@Connector.register('SocketDgram')
class SocketDgram(Connector):

    def __init__(self, dest_addr):
        super(SocketDgram, self).__init__(dest_addr)
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.set_recv_timeout()
        self._eventQueue = EvQueue()

    def send(self, data, port):
        raw_data = pickle.dumps({'port': port, "data": data})
        self._sock.sendto(raw_data, self._addr)

    def send_to_client(self, addr, data):
        self._sock.sendto(data, addr)

    def doorbell(self, doorbell):
        raw_data = pickle.dumps(
            {'port': None, 'data': convert_object_to_dict(doorbell)})
        self._sock.sendto(raw_data, self._addr)

    def consume_rings(self, rings):
        for ring in rings:
            descrs = [descr for descr in ring.consume()]
            if descrs:
                self._eventQueue.enqueueEv((ring, descrs))

    def recv(self, size=16384):
        try:
            raw_data, addr = self._sock.recvfrom(size)
            data = pickle.loads(raw_data)
            try:
                spkt = penscapy.Parse(data["data"])
            except:
                # It might be door bell
                return addr, data["data"], None
            return PacketContext(addr, spkt, data["port"])
        except socket.timeout:
            try:
                ring, descrs = self._eventQueue.dequeueEv()
                return RingContext(ring, descrs)
            except Queue.Empty:
                raise Connector.Timeout
        except:
            return

    def bind(self):
        self._sock.bind(self._addr)

    def set_recv_timeout(self, timeout=0.5):
        self._sock.settimeout(timeout)

    def close(self):
        self._sock.close()


@Connector.register('ModelConnector')
class ModelConnector(Connector):

    def __init__(self, addr=None):
        super(ModelConnector, self).__init__(addr)
        self._eventQueue = EvQueue()
        self.set_recv_timeout()

    def send(self, data, port):
        logger.info("Sending packet TO THE MODEL............")
        model_wrap.step_network_pkt(data, port)

    def send_to_client(self, addr, data):
        self._sock.sendto(data, addr)

    def _packet_recv(self):
        opkt, port, cos = model_wrap.get_next_pkt()
        if len(opkt) == 0:
            raise Connector.Timeout
        logger.info("Received packet from model", len(opkt))
        # Hack, Remove model header for now.
        #opkt = opkt[47:]
        try:
            spkt = penscapy.Parse(opkt)
        except:
            logger.critical("Received invalid packet...")
            return
        spkt.show()
        return PacketContext(None, spkt, port)

    def recv(self, size=16384):
        try:
            return self._packet_recv()
        except Connector.Timeout:
            try:
                ring, descrs = self._eventQueue.dequeueEv()
                return RingContext(ring, descrs)
            except Queue.Empty:
                raise Connector.Timeout

    def bind(self):
        pass
        # self._sock.bind(self._addr)

    def set_recv_timeout(self, timeout=0.5):
        self._eventQueue.set_wait_interval(timeout)

    def consume_rings(self, rings):
        for ring in rings:
            descrs = []
            for read_desc in ring.consume():
                descrs.append(read_desc)
            if descrs:
                self._eventQueue.enqueueEv((ring, descrs))

    def close(self):
        pass
        # self._sock.close()
