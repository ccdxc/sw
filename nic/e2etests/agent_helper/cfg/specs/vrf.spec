ProtoObject: vrf_pb2
Service: Vrf
enabled : True
dolEnabled : True
objects:
    - object :
        name : Vrf
        key_handle : VrfKeyHandle
        ignore:
        create:
            api      : VrfCreate
            request  : VrfRequestMsg
            response : VrfResponseMsg