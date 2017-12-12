#! /usr/bin/python3
import zmq
import os
import sys

ws_top = os.path.dirname(sys.argv[0]) + '/../'
ws_top = os.path.abspath(ws_top)
os.environ['WS_TOP'] = ws_top

class SignalingClientHelper():
    def Connect(self, ):
        context = zmq.Context()
        print( "Connecting to signaling server at %s/zmqsockcv" %(ws_top))
        self.socket = context.socket(zmq.REQ)
        self.socket.connect("ipc://%s/zmqsockcv" % ws_top)

    def SendSignalingData(self, string):
        self.socket.send_string(string)

    def Wait(self, ):
        # This is a blocking call which does a wait.
        message = self.socket.recv()
        assert message.decode("utf-8") == "Proceed"
SignalingClient = SignalingClientHelper()
