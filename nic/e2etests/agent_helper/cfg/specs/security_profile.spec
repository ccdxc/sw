ProtoObject: nwsec_pb2
Service: NwSecurity
enabled : True
dolEnabled : True
objects: 
    - object : 
        name : SecurityProfile
        key_handle : SecurityProfileKeyHandle
        create:
            api      : SecurityProfileCreate
            request  : SecurityProfileRequestMsg
            response : SecurityProfileResponseMsg
