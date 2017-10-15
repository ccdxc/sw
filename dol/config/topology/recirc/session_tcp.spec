# Flow generation configuration template.

meta:
    id: SESSION_TCP_RECIRC

proto: tcp

entries:
    - entry:
        label     : recirc_21bit_flow_1
        tracking  : False
        initiator : 
            flowhash : range/0x0a123456/0x0affffff/1
            port     : const/0
        responder : 
            flowhash : range/0x0b123456/0x0bffffff/1
            port     : const/0

    - entry:
        label     : recirc_32bit_flow_1
        tracking  : False
        initiator : 
            flowhash : range/0x0c001122/0x0cffffff/1
            port     : const/0
        responder : 
            flowhash : range/0x0c001122/0x0cffffff/1
            port     : const/0
