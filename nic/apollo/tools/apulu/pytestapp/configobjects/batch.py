#! /usr/bin/python3
import pdb

import batch_pb2 as batch_pb2
import types_pb2 as types_pb2
import ipaddress

class BatchObject():
    def __init__(self, epoch = 1, rollback_on_failure = False):
        self.epoch = epoch
        self.rollback_on_failure = rollback_on_failure
        return

    def GetGrpcMessage(self):
        grpcmsg = batch_pb2.BatchSpec()
        grpcmsg.epoch = self.epoch
        #grpcmsg.rollback_on_failure = self.rollback_on_failure
        return grpcmsg


