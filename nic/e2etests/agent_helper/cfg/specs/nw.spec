ProtoObject: nw_pb2
Service: Network
enabled : True
dolEnabled : True
objects:
    - object:
        name : Network
        key_handle : NetworkKeyHandle
        ignore:
        create:
            api      : NetworkCreate
            request  : NetworkRequestMsg
            response : NetworkResponseMsg
