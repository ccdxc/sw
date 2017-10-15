# Flow generation configuration template.

meta:
    id: SESSION_UDP_RECIRC

proto: udp

entries:
    - entry:
        label     : recirc_21bit_flow_1
        tracking  : False
        initiator : 
            flowhash : range/0x8a123456/0x8affffff/1
            port     : const/0
        responder : 
            flowhash : range/0x8b123456/0x8bffffff/1
            port     : const/0

    - entry:
        label     : recirc_32bit_flow_1
        tracking  : False
        initiator : 
            flowhash : range/0x8c001122/0x8cffffff/1
            port     : const/0
        responder : 
            flowhash : range/0x8c001122/0x8cffffff/1
            port     : const/0
