#! /usr/bin/python3
import zmq
import os
import sys
import pdb

class SignalingClientHelper():
    def Connect(self, ):
        context = zmq.Context()
        print( "Connecting to signaling server at %s/zmqsockmbt" %(ws_top))
        self.socket = context.socket(zmq.REQ)
        self.socket.connect("ipc://%s/zmqsockmbt" % ws_top)

    def SendSignalingData(self, string):
        self.socket.send_string(string)

    def Wait(self, ):
        # This is a blocking call which does a wait.
        message = self.socket.recv()
        assert message.decode("utf-8") == "Proceed"
SignalingClient = SignalingClientHelper()
