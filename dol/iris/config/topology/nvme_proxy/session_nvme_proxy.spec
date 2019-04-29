# Flow generation configuration template.
meta:
    id: SESSION_NVME_PROXY

proto: tcp

entries:

#every session has a well-known destination port number 4420 and
#varying source port number starting from 50001.
#50001 corresponds to NS1, 50002 corresponds to NS2 and so on.
    - entry:
        label: nvme-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/4420
        initiator: 
            port : const/50001
    - entry:
        label: nvme-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/4420
        initiator: 
            port : const/50002
    - entry:
        label: nvme-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/4420
        initiator: 
            port : const/50003
    - entry:
        label: nvme-proxy
        tracking: False
        timestamp: False
        responder: 
            port : const/4420
        initiator: 
            port : const/50004
