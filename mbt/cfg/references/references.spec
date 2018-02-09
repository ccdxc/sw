objects:
    - object :
        service     : NwSecurity
        name        : SecurityProfile
        key_handle  : SecurityProfileKeyHandle
        constraints : None
    - object : 
        service     : NwSecurity
        name        : SecurityGroup
        key_handle  : SecurityGroupKeyHandle
        constraints : None
    - object : 
        service     : Vrf
        name        : Vrf
        key_handle  : VrfKeyHandle
        constraints : None
    - object : 
        service     : Network 
        name        : Network
        key_handle  : NetworkKeyHandle
        constraints : None
    - object : 
        service     : Interface
        name        : Interface
        key_handle  : InterfaceKeyHandle
        constraints : "venice:constraints={intf.InterfaceSpec.type=interface_pb2.IF_TYPE_UPLINK}"
    - object : 
        service     : L2Segment
        name        : L2Segment
        key_handle  : L2SegmentKeyHandle
        constraints : None
    - object : 
        service     : Interface
        name        : Interface
        key_handle  : InterfaceKeyHandle
        constraints : "venice:constraints={intf.InterfaceSpec.type=interface_pb2.IF_TYPE_UPLINK_PC}"
    - object : 
        service     : Interface
        name        : Lif
        key_handle  : LifKeyHandle
        constraints : None
    - object : 
        service     : Acl
        name        : Acl
        key_handle  : AclKeyHandle
        constraints : None
    - object : 
        service     : Interface
        name        : Interface
        key_handle  : InterfaceKeyHandle
        constraints : "venice:constraints={intf.InterfaceSpec.type=interface_pb2.IF_TYPE_ENIC}"
#     - object : 
#         service     : Endpoint
#         name        : Endpoint
#         key_handle  : EndpointKeyHandle
#         constraints : None
