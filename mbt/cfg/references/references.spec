objects:
    - object : 
        service     : Interface
        name        : Interface
        key_handle  : InterfaceKeyHandle
        constraints : "venice:constraints={intf.InterfaceSpec.type=interface_pb2.IF_TYPE_UPLINK}"
    - object : 
        service     : Interface
        name        : Lif
        key_handle  : LifKeyHandle
        constraints : None
