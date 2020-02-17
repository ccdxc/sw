# Protocol Documentation
<a name="top"/>

## Table of Contents

- [descriptor.proto](#descriptor.proto)
    - [DescriptorProto](#google.protobuf.DescriptorProto)
    - [DescriptorProto.ExtensionRange](#google.protobuf.DescriptorProto.ExtensionRange)
    - [DescriptorProto.ReservedRange](#google.protobuf.DescriptorProto.ReservedRange)
    - [EnumDescriptorProto](#google.protobuf.EnumDescriptorProto)
    - [EnumDescriptorProto.EnumReservedRange](#google.protobuf.EnumDescriptorProto.EnumReservedRange)
    - [EnumOptions](#google.protobuf.EnumOptions)
    - [EnumValueDescriptorProto](#google.protobuf.EnumValueDescriptorProto)
    - [EnumValueOptions](#google.protobuf.EnumValueOptions)
    - [ExtensionRangeOptions](#google.protobuf.ExtensionRangeOptions)
    - [FieldDescriptorProto](#google.protobuf.FieldDescriptorProto)
    - [FieldOptions](#google.protobuf.FieldOptions)
    - [FileDescriptorProto](#google.protobuf.FileDescriptorProto)
    - [FileDescriptorSet](#google.protobuf.FileDescriptorSet)
    - [FileOptions](#google.protobuf.FileOptions)
    - [GeneratedCodeInfo](#google.protobuf.GeneratedCodeInfo)
    - [GeneratedCodeInfo.Annotation](#google.protobuf.GeneratedCodeInfo.Annotation)
    - [MessageOptions](#google.protobuf.MessageOptions)
    - [MethodDescriptorProto](#google.protobuf.MethodDescriptorProto)
    - [MethodOptions](#google.protobuf.MethodOptions)
    - [OneofDescriptorProto](#google.protobuf.OneofDescriptorProto)
    - [OneofOptions](#google.protobuf.OneofOptions)
    - [ServiceDescriptorProto](#google.protobuf.ServiceDescriptorProto)
    - [ServiceOptions](#google.protobuf.ServiceOptions)
    - [SourceCodeInfo](#google.protobuf.SourceCodeInfo)
    - [SourceCodeInfo.Location](#google.protobuf.SourceCodeInfo.Location)
    - [UninterpretedOption](#google.protobuf.UninterpretedOption)
    - [UninterpretedOption.NamePart](#google.protobuf.UninterpretedOption.NamePart)
  
    - [FieldDescriptorProto.Label](#google.protobuf.FieldDescriptorProto.Label)
    - [FieldDescriptorProto.Type](#google.protobuf.FieldDescriptorProto.Type)
    - [FieldOptions.CType](#google.protobuf.FieldOptions.CType)
    - [FieldOptions.JSType](#google.protobuf.FieldOptions.JSType)
    - [FileOptions.OptimizeMode](#google.protobuf.FileOptions.OptimizeMode)
    - [MethodOptions.IdempotencyLevel](#google.protobuf.MethodOptions.IdempotencyLevel)
  
  
  

- [gogo.proto](#gogo.proto)
  
  
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
    - [File-level Extensions](#gogo.proto-extensions)
  
  

- [pdsa.proto](#pdsa.proto)
    - [GlobalOpts](#venice.GlobalOpts)
    - [PdsaFields](#venice.PdsaFields)
  
  
    - [File-level Extensions](#pdsa.proto-extensions)
    - [File-level Extensions](#pdsa.proto-extensions)
    - [File-level Extensions](#pdsa.proto-extensions)
  
  

- [timestamp.proto](#timestamp.proto)
    - [Timestamp](#google.protobuf.Timestamp)
  
  
  
  

- [meta.proto](#meta.proto)
    - [ObjMeta](#meta.ObjMeta)
    - [Timestamp](#meta.Timestamp)
    - [TypeMeta](#meta.TypeMeta)
  
  
  
  

- [types.proto](#types.proto)
    - [Address](#types.Address)
    - [AddressRange](#types.AddressRange)
    - [BatchCtxt](#types.BatchCtxt)
    - [Empty](#types.Empty)
    - [Encap](#types.Encap)
    - [EncapVal](#types.EncapVal)
    - [FlowKey](#types.FlowKey)
    - [FlowL4Info](#types.FlowL4Info)
    - [FlowL4Info.ICMPInfo](#types.FlowL4Info.ICMPInfo)
    - [FlowL4Info.TCPUDPInfo](#types.FlowL4Info.TCPUDPInfo)
    - [ICMPMatch](#types.ICMPMatch)
    - [IPAddress](#types.IPAddress)
    - [IPFlowKey](#types.IPFlowKey)
    - [IPPrefix](#types.IPPrefix)
    - [IPRange](#types.IPRange)
    - [IPSubnet](#types.IPSubnet)
    - [IPv4Prefix](#types.IPv4Prefix)
    - [IPv6Prefix](#types.IPv6Prefix)
    - [MACFlowKey](#types.MACFlowKey)
    - [PortMatch](#types.PortMatch)
    - [PortRange](#types.PortRange)
    - [QinQTag](#types.QinQTag)
    - [RuleL3Match](#types.RuleL3Match)
    - [RuleL4Match](#types.RuleL4Match)
    - [RuleMatch](#types.RuleMatch)
  
    - [AddressType](#types.AddressType)
    - [ApiStatus](#types.ApiStatus)
    - [EncapType](#types.EncapType)
    - [IPAF](#types.IPAF)
    - [LifType](#types.LifType)
    - [NatAction](#types.NatAction)
    - [RuleDir](#types.RuleDir)
    - [SecurityRuleAction](#types.SecurityRuleAction)
  
  
  

- [common.proto](#common.proto)
  
    - [AdminState](#pds.AdminState)
  
  
  

- [bgp.proto](#bgp.proto)
    - [BGP](#pds.BGP)
    - [BGPGetResponse](#pds.BGPGetResponse)
    - [BGPNLRIPrefix](#pds.BGPNLRIPrefix)
    - [BGPNLRIPrefixGetResponse](#pds.BGPNLRIPrefixGetResponse)
    - [BGPNLRIPrefixRequest](#pds.BGPNLRIPrefixRequest)
    - [BGPNLRIPrefixStatus](#pds.BGPNLRIPrefixStatus)
    - [BGPPeer](#pds.BGPPeer)
    - [BGPPeerAf](#pds.BGPPeerAf)
    - [BGPPeerAfGetResponse](#pds.BGPPeerAfGetResponse)
    - [BGPPeerAfRequest](#pds.BGPPeerAfRequest)
    - [BGPPeerAfResponse](#pds.BGPPeerAfResponse)
    - [BGPPeerAfSpec](#pds.BGPPeerAfSpec)
    - [BGPPeerAfStatus](#pds.BGPPeerAfStatus)
    - [BGPPeerGetResponse](#pds.BGPPeerGetResponse)
    - [BGPPeerRequest](#pds.BGPPeerRequest)
    - [BGPPeerResponse](#pds.BGPPeerResponse)
    - [BGPPeerSpec](#pds.BGPPeerSpec)
    - [BGPPeerStatus](#pds.BGPPeerStatus)
    - [BGPRequest](#pds.BGPRequest)
    - [BGPResponse](#pds.BGPResponse)
    - [BGPSpec](#pds.BGPSpec)
    - [BGPStatus](#pds.BGPStatus)
  
    - [BGPAfi](#pds.BGPAfi)
    - [BGPPeerRRClient](#pds.BGPPeerRRClient)
    - [BGPPeerState](#pds.BGPPeerState)
    - [BGPSafi](#pds.BGPSafi)
    - [NLRISrc](#pds.NLRISrc)
  
  
    - [BGPSvc](#pds.BGPSvc)
  

- [cp_route.proto](#cp_route.proto)
    - [CPRoute](#pds.CPRoute)
    - [CPRouteGetRequest](#pds.CPRouteGetRequest)
    - [CPRouteGetResponse](#pds.CPRouteGetResponse)
    - [CPRouteStatus](#pds.CPRouteStatus)
    - [CPStaticRoute](#pds.CPStaticRoute)
    - [CPStaticRouteGetResponse](#pds.CPStaticRouteGetResponse)
    - [CPStaticRouteRequest](#pds.CPStaticRouteRequest)
    - [CPStaticRouteResponse](#pds.CPStaticRouteResponse)
    - [CPStaticRouteSpec](#pds.CPStaticRouteSpec)
    - [CPStaticRouteStatus](#pds.CPStaticRouteStatus)
  
    - [RouteProtocol](#pds.RouteProtocol)
    - [RouteType](#pds.RouteType)
  
  
    - [CPRouteSvc](#pds.CPRouteSvc)
  

- [cp_test.proto](#cp_test.proto)
    - [CPL2fTestCreateSpec](#pds.CPL2fTestCreateSpec)
    - [CPL2fTestDeleteSpec](#pds.CPL2fTestDeleteSpec)
    - [CPL2fTestResponse](#pds.CPL2fTestResponse)
  
  
  
    - [CPTestSvc](#pds.CPTestSvc)
  

- [evpn.proto](#evpn.proto)
    - [EvpnEvi](#pds.EvpnEvi)
    - [EvpnEviGetResponse](#pds.EvpnEviGetResponse)
    - [EvpnEviRequest](#pds.EvpnEviRequest)
    - [EvpnEviRt](#pds.EvpnEviRt)
    - [EvpnEviRtGetResponse](#pds.EvpnEviRtGetResponse)
    - [EvpnEviRtRequest](#pds.EvpnEviRtRequest)
    - [EvpnEviRtSpec](#pds.EvpnEviRtSpec)
    - [EvpnEviSpec](#pds.EvpnEviSpec)
    - [EvpnIpVrf](#pds.EvpnIpVrf)
    - [EvpnIpVrfGetResponse](#pds.EvpnIpVrfGetResponse)
    - [EvpnIpVrfRequest](#pds.EvpnIpVrfRequest)
    - [EvpnIpVrfRt](#pds.EvpnIpVrfRt)
    - [EvpnIpVrfRtGetResponse](#pds.EvpnIpVrfRtGetResponse)
    - [EvpnIpVrfRtRequest](#pds.EvpnIpVrfRtRequest)
    - [EvpnIpVrfRtSpec](#pds.EvpnIpVrfRtSpec)
    - [EvpnIpVrfSpec](#pds.EvpnIpVrfSpec)
    - [EvpnMacIp](#pds.EvpnMacIp)
    - [EvpnMacIpGetResponse](#pds.EvpnMacIpGetResponse)
    - [EvpnMacIpSpec](#pds.EvpnMacIpSpec)
    - [EvpnMacIpSpecRequest](#pds.EvpnMacIpSpecRequest)
    - [EvpnResponse](#pds.EvpnResponse)
    - [EvpnStatus](#pds.EvpnStatus)
  
    - [EvpnCfg](#pds.EvpnCfg)
    - [EvpnEncaps](#pds.EvpnEncaps)
    - [EvpnRtType](#pds.EvpnRtType)
    - [EvpnSource](#pds.EvpnSource)
  
  
    - [EvpnSvc](#pds.EvpnSvc)
  

- [internal.proto](#internal.proto)
    - [BgpRouteMapSpec](#pds.BgpRouteMapSpec)
    - [EvpnBdSpec](#pds.EvpnBdSpec)
    - [EvpnIfBindCfgSpec](#pds.EvpnIfBindCfgSpec)
    - [LimGenIrbIfSpec](#pds.LimGenIrbIfSpec)
    - [LimInterfaceAddrSpec](#pds.LimInterfaceAddrSpec)
    - [LimInterfaceCfgSpec](#pds.LimInterfaceCfgSpec)
    - [LimInterfaceSpec](#pds.LimInterfaceSpec)
    - [LimVrfSpec](#pds.LimVrfSpec)
  
    - [LimIntfType](#pds.LimIntfType)
  
  
  

- [Scalar Value Types](#scalar-value-types)



<a name="descriptor.proto"/>
<p align="right"><a href="#top">Top</a></p>

## descriptor.proto



<a name="google.protobuf.DescriptorProto"/>

### DescriptorProto
Describes a message type.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| field | [FieldDescriptorProto](#google.protobuf.FieldDescriptorProto) | repeated |  |
| extension | [FieldDescriptorProto](#google.protobuf.FieldDescriptorProto) | repeated |  |
| nested_type | [DescriptorProto](#google.protobuf.DescriptorProto) | repeated |  |
| enum_type | [EnumDescriptorProto](#google.protobuf.EnumDescriptorProto) | repeated |  |
| extension_range | [DescriptorProto.ExtensionRange](#google.protobuf.DescriptorProto.ExtensionRange) | repeated |  |
| oneof_decl | [OneofDescriptorProto](#google.protobuf.OneofDescriptorProto) | repeated |  |
| options | [MessageOptions](#google.protobuf.MessageOptions) | optional |  |
| reserved_range | [DescriptorProto.ReservedRange](#google.protobuf.DescriptorProto.ReservedRange) | repeated |  |
| reserved_name | [string](#string) | repeated | Reserved field names, which may not be used by fields in the same message. A given name may only be reserved once. |






<a name="google.protobuf.DescriptorProto.ExtensionRange"/>

### DescriptorProto.ExtensionRange



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| start | [int32](#int32) | optional |  |
| end | [int32](#int32) | optional |  |
| options | [ExtensionRangeOptions](#google.protobuf.ExtensionRangeOptions) | optional |  |






<a name="google.protobuf.DescriptorProto.ReservedRange"/>

### DescriptorProto.ReservedRange
Range of reserved tag numbers. Reserved tag numbers may not be used by
fields or extension ranges in the same message. Reserved ranges may
not overlap.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| start | [int32](#int32) | optional | Inclusive. |
| end | [int32](#int32) | optional | Exclusive. |






<a name="google.protobuf.EnumDescriptorProto"/>

### EnumDescriptorProto
Describes an enum type.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| value | [EnumValueDescriptorProto](#google.protobuf.EnumValueDescriptorProto) | repeated |  |
| options | [EnumOptions](#google.protobuf.EnumOptions) | optional |  |
| reserved_range | [EnumDescriptorProto.EnumReservedRange](#google.protobuf.EnumDescriptorProto.EnumReservedRange) | repeated | Range of reserved numeric values. Reserved numeric values may not be used by enum values in the same enum declaration. Reserved ranges may not overlap. |
| reserved_name | [string](#string) | repeated | Reserved enum value names, which may not be reused. A given name may only be reserved once. |






<a name="google.protobuf.EnumDescriptorProto.EnumReservedRange"/>

### EnumDescriptorProto.EnumReservedRange
Range of reserved numeric values. Reserved values may not be used by
entries in the same enum. Reserved ranges may not overlap.

Note that this is distinct from DescriptorProto.ReservedRange in that it
is inclusive such that it can appropriately represent the entire int32
domain.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| start | [int32](#int32) | optional | Inclusive. |
| end | [int32](#int32) | optional | Inclusive. |






<a name="google.protobuf.EnumOptions"/>

### EnumOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| allow_alias | [bool](#bool) | optional | Set this option to true to allow mapping different tag names to the same value. |
| deprecated | [bool](#bool) | optional | Is this enum deprecated? Depending on the target platform, this can emit Deprecated annotations for the enum, or it will be completely ignored; in the very least, this is a formalization for deprecating enums. |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.EnumValueDescriptorProto"/>

### EnumValueDescriptorProto
Describes a value within an enum.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| number | [int32](#int32) | optional |  |
| options | [EnumValueOptions](#google.protobuf.EnumValueOptions) | optional |  |






<a name="google.protobuf.EnumValueOptions"/>

### EnumValueOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| deprecated | [bool](#bool) | optional | Is this enum value deprecated? Depending on the target platform, this can emit Deprecated annotations for the enum value, or it will be completely ignored; in the very least, this is a formalization for deprecating enum values. |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.ExtensionRangeOptions"/>

### ExtensionRangeOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.FieldDescriptorProto"/>

### FieldDescriptorProto
Describes a field within a message.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| number | [int32](#int32) | optional |  |
| label | [FieldDescriptorProto.Label](#google.protobuf.FieldDescriptorProto.Label) | optional |  |
| type | [FieldDescriptorProto.Type](#google.protobuf.FieldDescriptorProto.Type) | optional | If type_name is set, this need not be set. If both this and type_name are set, this must be one of TYPE_ENUM, TYPE_MESSAGE or TYPE_GROUP. |
| type_name | [string](#string) | optional | For message and enum types, this is the name of the type. If the name starts with a &#39;.&#39;, it is fully-qualified. Otherwise, C&#43;&#43;-like scoping rules are used to find the type (i.e. first the nested types within this message are searched, then within the parent, on up to the root namespace). |
| extendee | [string](#string) | optional | For extensions, this is the name of the type being extended. It is resolved in the same manner as type_name. |
| default_value | [string](#string) | optional | For numeric types, contains the original text representation of the value. For booleans, &#34;true&#34; or &#34;false&#34;. For strings, contains the default text contents (not escaped in any way). For bytes, contains the C escaped value. All bytes &gt;= 128 are escaped. TODO(kenton): Base-64 encode? |
| oneof_index | [int32](#int32) | optional | If set, gives the index of a oneof in the containing type&#39;s oneof_decl list. This field is a member of that oneof. |
| json_name | [string](#string) | optional | JSON name of this field. The value is set by protocol compiler. If the user has set a &#34;json_name&#34; option on this field, that option&#39;s value will be used. Otherwise, it&#39;s deduced from the field&#39;s name by converting it to camelCase. |
| options | [FieldOptions](#google.protobuf.FieldOptions) | optional |  |






<a name="google.protobuf.FieldOptions"/>

### FieldOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ctype | [FieldOptions.CType](#google.protobuf.FieldOptions.CType) | optional | The ctype option instructs the C&#43;&#43; code generator to use a different representation of the field than it normally would. See the specific options below. This option is not yet implemented in the open source release -- sorry, we&#39;ll try to include it in a future version! |
| packed | [bool](#bool) | optional | The packed option can be enabled for repeated primitive fields to enable a more efficient representation on the wire. Rather than repeatedly writing the tag and type for each element, the entire array is encoded as a single length-delimited blob. In proto3, only explicit setting it to false will avoid using packed encoding. |
| jstype | [FieldOptions.JSType](#google.protobuf.FieldOptions.JSType) | optional | The jstype option determines the JavaScript type used for values of the field. The option is permitted only for 64 bit integral and fixed types (int64, uint64, sint64, fixed64, sfixed64). A field with jstype JS_STRING is represented as JavaScript string, which avoids loss of precision that can happen when a large value is converted to a floating point JavaScript. Specifying JS_NUMBER for the jstype causes the generated JavaScript code to use the JavaScript &#34;number&#34; type. The behavior of the default option JS_NORMAL is implementation dependent. This option is an enum to permit additional types to be added, e.g. goog.math.Integer. |
| lazy | [bool](#bool) | optional | Should this field be parsed lazily? Lazy applies only to message-type fields. It means that when the outer message is initially parsed, the inner message&#39;s contents will not be parsed but instead stored in encoded form. The inner message will actually be parsed when it is first accessed. This is only a hint. Implementations are free to choose whether to use eager or lazy parsing regardless of the value of this option. However, setting this option true suggests that the protocol author believes that using lazy parsing on this field is worth the additional bookkeeping overhead typically needed to implement it. This option does not affect the public interface of any generated code; all method signatures remain the same. Furthermore, thread-safety of the interface is not affected by this option; const methods remain safe to call from multiple threads concurrently, while non-const methods continue to require exclusive access. Note that implementations may choose not to check required fields within a lazy sub-message. That is, calling IsInitialized() on the outer message may return true even if the inner message has missing required fields. This is necessary because otherwise the inner message would have to be parsed in order to perform the check, defeating the purpose of lazy parsing. An implementation which chooses not to check required fields must be consistent about it. That is, for any particular sub-message, the implementation must either *always* check its required fields, or *never check its required fields, regardless of whether or not the message has been parsed. |
| deprecated | [bool](#bool) | optional | Is this field deprecated? Depending on the target platform, this can emit Deprecated annotations for accessors, or it will be completely ignored; in the very least, this is a formalization for deprecating fields. |
| weak | [bool](#bool) | optional | For Google-internal migration only. Do not use. |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.FileDescriptorProto"/>

### FileDescriptorProto
Describes a complete .proto file.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional | file name, relative to root of source tree |
| package | [string](#string) | optional | e.g. &#34;foo&#34;, &#34;foo.bar&#34;, etc. |
| dependency | [string](#string) | repeated | Names of files imported by this file. |
| public_dependency | [int32](#int32) | repeated | Indexes of the public imported files in the dependency list above. |
| weak_dependency | [int32](#int32) | repeated | Indexes of the weak imported files in the dependency list. For Google-internal migration only. Do not use. |
| message_type | [DescriptorProto](#google.protobuf.DescriptorProto) | repeated | All top-level definitions in this file. |
| enum_type | [EnumDescriptorProto](#google.protobuf.EnumDescriptorProto) | repeated |  |
| service | [ServiceDescriptorProto](#google.protobuf.ServiceDescriptorProto) | repeated |  |
| extension | [FieldDescriptorProto](#google.protobuf.FieldDescriptorProto) | repeated |  |
| options | [FileOptions](#google.protobuf.FileOptions) | optional |  |
| source_code_info | [SourceCodeInfo](#google.protobuf.SourceCodeInfo) | optional | This field contains optional information about the original source code. You may safely remove this entire field without harming runtime functionality of the descriptors -- the information is needed only by development tools. |
| syntax | [string](#string) | optional | The syntax of the proto file. The supported values are &#34;proto2&#34; and &#34;proto3&#34;. |






<a name="google.protobuf.FileDescriptorSet"/>

### FileDescriptorSet
The protocol compiler can output a FileDescriptorSet containing the .proto
files it parses.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| file | [FileDescriptorProto](#google.protobuf.FileDescriptorProto) | repeated |  |






<a name="google.protobuf.FileOptions"/>

### FileOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| java_package | [string](#string) | optional | Sets the Java package where classes generated from this .proto will be placed. By default, the proto package is used, but this is often inappropriate because proto packages do not normally start with backwards domain names. |
| java_outer_classname | [string](#string) | optional | If set, all the classes from the .proto file are wrapped in a single outer class with the given name. This applies to both Proto1 (equivalent to the old &#34;--one_java_file&#34; option) and Proto2 (where a .proto always translates to a single class, but you may want to explicitly choose the class name). |
| java_multiple_files | [bool](#bool) | optional | If set true, then the Java code generator will generate a separate .java file for each top-level message, enum, and service defined in the .proto file. Thus, these types will *not* be nested inside the outer class named by java_outer_classname. However, the outer class will still be generated to contain the file&#39;s getDescriptor() method as well as any top-level extensions defined in the file. |
| java_generate_equals_and_hash | [bool](#bool) | optional | This option does nothing. |
| java_string_check_utf8 | [bool](#bool) | optional | If set true, then the Java2 code generator will generate code that throws an exception whenever an attempt is made to assign a non-UTF-8 byte sequence to a string field. Message reflection will do the same. However, an extension field still accepts non-UTF-8 byte sequences. This option has no effect on when used with the lite runtime. |
| optimize_for | [FileOptions.OptimizeMode](#google.protobuf.FileOptions.OptimizeMode) | optional |  |
| go_package | [string](#string) | optional | Sets the Go package where structs generated from this .proto will be placed. If omitted, the Go package will be derived from the following: - The basename of the package import path, if provided. - Otherwise, the package statement in the .proto file, if present. - Otherwise, the basename of the .proto file, without extension. |
| cc_generic_services | [bool](#bool) | optional | Should generic services be generated in each language? &#34;Generic&#34; services are not specific to any particular RPC system. They are generated by the main code generators in each language (without additional plugins). Generic services were the only kind of service generation supported by early versions of google.protobuf. Generic services are now considered deprecated in favor of using plugins that generate code specific to your particular RPC system. Therefore, these default to false. Old code which depends on generic services should explicitly set them to true. |
| java_generic_services | [bool](#bool) | optional |  |
| py_generic_services | [bool](#bool) | optional |  |
| php_generic_services | [bool](#bool) | optional |  |
| deprecated | [bool](#bool) | optional | Is this file deprecated? Depending on the target platform, this can emit Deprecated annotations for everything in the file, or it will be completely ignored; in the very least, this is a formalization for deprecating files. |
| cc_enable_arenas | [bool](#bool) | optional | Enables the use of arenas for the proto messages in this file. This applies only to generated classes for C&#43;&#43;. |
| objc_class_prefix | [string](#string) | optional | Sets the objective c class prefix which is prepended to all objective c generated classes from this .proto. There is no default. |
| csharp_namespace | [string](#string) | optional | Namespace for generated classes; defaults to the package. |
| swift_prefix | [string](#string) | optional | By default Swift generators will take the proto package and CamelCase it replacing &#39;.&#39; with underscore and use that to prefix the types/symbols defined. When this options is provided, they will use this value instead to prefix the types/symbols defined. |
| php_class_prefix | [string](#string) | optional | Sets the php class prefix which is prepended to all php generated classes from this .proto. Default is empty. |
| php_namespace | [string](#string) | optional | Use this option to change the namespace of php generated classes. Default is empty. When this option is empty, the package name will be used for determining the namespace. |
| php_metadata_namespace | [string](#string) | optional | Use this option to change the namespace of php generated metadata classes. Default is empty. When this option is empty, the proto file name will be used for determining the namespace. |
| ruby_package | [string](#string) | optional | Use this option to change the package of ruby generated classes. Default is empty. When this option is not set, the package name will be used for determining the ruby package. |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See the documentation for the &#34;Options&#34; section above. |






<a name="google.protobuf.GeneratedCodeInfo"/>

### GeneratedCodeInfo
Describes the relationship between generated code and its original source
file. A GeneratedCodeInfo message is associated with only one generated
source file, but may contain references to different source .proto files.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| annotation | [GeneratedCodeInfo.Annotation](#google.protobuf.GeneratedCodeInfo.Annotation) | repeated | An Annotation connects some span of text in generated code to an element of its generating .proto file. |






<a name="google.protobuf.GeneratedCodeInfo.Annotation"/>

### GeneratedCodeInfo.Annotation



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| path | [int32](#int32) | repeated | Identifies the element in the original source .proto file. This field is formatted the same as SourceCodeInfo.Location.path. |
| source_file | [string](#string) | optional | Identifies the filesystem path to the original source .proto. |
| begin | [int32](#int32) | optional | Identifies the starting offset in bytes in the generated code that relates to the identified object. |
| end | [int32](#int32) | optional | Identifies the ending offset in bytes in the generated code that relates to the identified offset. The end offset should be one past the last relevant byte (so the length of the text = end - begin). |






<a name="google.protobuf.MessageOptions"/>

### MessageOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| message_set_wire_format | [bool](#bool) | optional | Set true to use the old proto1 MessageSet wire format for extensions. This is provided for backwards-compatibility with the MessageSet wire format. You should not use this for any other reason: It&#39;s less efficient, has fewer features, and is more complicated. The message must be defined exactly as follows: message Foo { option message_set_wire_format = true; extensions 4 to max; } Note that the message cannot have any defined fields; MessageSets only have extensions. All extensions of your type must be singular messages; e.g. they cannot be int32s, enums, or repeated messages. Because this is an option, the above two restrictions are not enforced by the protocol compiler. |
| no_standard_descriptor_accessor | [bool](#bool) | optional | Disables the generation of the standard &#34;descriptor()&#34; accessor, which can conflict with a field of the same name. This is meant to make migration from proto1 easier; new code should avoid fields named &#34;descriptor&#34;. |
| deprecated | [bool](#bool) | optional | Is this message deprecated? Depending on the target platform, this can emit Deprecated annotations for the message, or it will be completely ignored; in the very least, this is a formalization for deprecating messages. |
| map_entry | [bool](#bool) | optional | Whether the message is an automatically generated map entry type for the maps field. For maps fields: map&lt;KeyType, ValueType&gt; map_field = 1; The parsed descriptor looks like: message MapFieldEntry { option map_entry = true; optional KeyType key = 1; optional ValueType value = 2; } repeated MapFieldEntry map_field = 1; Implementations may choose not to generate the map_entry=true message, but use a native map in the target language to hold the keys and values. The reflection APIs in such implementions still need to work as if the field is a repeated message field. NOTE: Do not set the option in .proto files. Always use the maps syntax instead. The option should only be implicitly set by the proto compiler parser. |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.MethodDescriptorProto"/>

### MethodDescriptorProto
Describes a method of a service.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| input_type | [string](#string) | optional | Input and output type names. These are resolved in the same way as FieldDescriptorProto.type_name, but must refer to a message type. |
| output_type | [string](#string) | optional |  |
| options | [MethodOptions](#google.protobuf.MethodOptions) | optional |  |
| client_streaming | [bool](#bool) | optional | Identifies if client streams multiple client messages |
| server_streaming | [bool](#bool) | optional | Identifies if server streams multiple server messages |






<a name="google.protobuf.MethodOptions"/>

### MethodOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| deprecated | [bool](#bool) | optional | Is this method deprecated? Depending on the target platform, this can emit Deprecated annotations for the method, or it will be completely ignored; in the very least, this is a formalization for deprecating methods. |
| idempotency_level | [MethodOptions.IdempotencyLevel](#google.protobuf.MethodOptions.IdempotencyLevel) | optional |  |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.OneofDescriptorProto"/>

### OneofDescriptorProto
Describes a oneof.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| options | [OneofOptions](#google.protobuf.OneofOptions) | optional |  |






<a name="google.protobuf.OneofOptions"/>

### OneofOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.ServiceDescriptorProto"/>

### ServiceDescriptorProto
Describes a service.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |
| method | [MethodDescriptorProto](#google.protobuf.MethodDescriptorProto) | repeated |  |
| options | [ServiceOptions](#google.protobuf.ServiceOptions) | optional |  |






<a name="google.protobuf.ServiceOptions"/>

### ServiceOptions



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| deprecated | [bool](#bool) | optional | Is this service deprecated? Depending on the target platform, this can emit Deprecated annotations for the service, or it will be completely ignored; in the very least, this is a formalization for deprecating services. |
| uninterpreted_option | [UninterpretedOption](#google.protobuf.UninterpretedOption) | repeated | The parser stores options it doesn&#39;t recognize here. See above. |






<a name="google.protobuf.SourceCodeInfo"/>

### SourceCodeInfo
Encapsulates information about the original source file from which a
FileDescriptorProto was generated.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| location | [SourceCodeInfo.Location](#google.protobuf.SourceCodeInfo.Location) | repeated | A Location identifies a piece of source code in a .proto file which corresponds to a particular definition. This information is intended to be useful to IDEs, code indexers, documentation generators, and similar tools. For example, say we have a file like: message Foo { optional string foo = 1; } Let&#39;s look at just the field definition: optional string foo = 1; ^ ^^ ^^ ^ ^^^ a bc de f ghi We have the following locations: span path represents [a,i) [ 4, 0, 2, 0 ] The whole field definition. [a,b) [ 4, 0, 2, 0, 4 ] The label (optional). [c,d) [ 4, 0, 2, 0, 5 ] The type (string). [e,f) [ 4, 0, 2, 0, 1 ] The name (foo). [g,h) [ 4, 0, 2, 0, 3 ] The number (1). Notes: - A location may refer to a repeated field itself (i.e. not to any particular index within it). This is used whenever a set of elements are logically enclosed in a single code segment. For example, an entire extend block (possibly containing multiple extension definitions) will have an outer location whose path refers to the &#34;extensions&#34; repeated field without an index. - Multiple locations may have the same path. This happens when a single logical declaration is spread out across multiple places. The most obvious example is the &#34;extend&#34; block again -- there may be multiple extend blocks in the same scope, each of which will have the same path. - A location&#39;s span is not always a subset of its parent&#39;s span. For example, the &#34;extendee&#34; of an extension declaration appears at the beginning of the &#34;extend&#34; block and is shared by all extensions within the block. - Just because a location&#39;s span is a subset of some other location&#39;s span does not mean that it is a descendent. For example, a &#34;group&#34; defines both a type and a field in a single declaration. Thus, the locations corresponding to the type and field and their components will overlap. - Code which tries to interpret locations should probably be designed to ignore those that it doesn&#39;t understand, as more types of locations could be recorded in the future. |






<a name="google.protobuf.SourceCodeInfo.Location"/>

### SourceCodeInfo.Location



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| path | [int32](#int32) | repeated | Identifies which part of the FileDescriptorProto was defined at this location. Each element is a field number or an index. They form a path from the root FileDescriptorProto to the place where the definition. For example, this path: [ 4, 3, 2, 7, 1 ] refers to: file.message_type(3) // 4, 3 .field(7) // 2, 7 .name() // 1 This is because FileDescriptorProto.message_type has field number 4: repeated DescriptorProto message_type = 4; and DescriptorProto.field has field number 2: repeated FieldDescriptorProto field = 2; and FieldDescriptorProto.name has field number 1: optional string name = 1; Thus, the above path gives the location of a field name. If we removed the last element: [ 4, 3, 2, 7 ] this path refers to the whole field declaration (from the beginning of the label to the terminating semicolon). |
| span | [int32](#int32) | repeated | Always has exactly three or four elements: start line, start column, end line (optional, otherwise assumed same as start line), end column. These are packed into a single field for efficiency. Note that line and column numbers are zero-based -- typically you will want to add 1 to each before displaying to a user. |
| leading_comments | [string](#string) | optional | If this SourceCodeInfo represents a complete declaration, these are any comments appearing before and after the declaration which appear to be attached to the declaration. A series of line comments appearing on consecutive lines, with no other tokens appearing on those lines, will be treated as a single comment. leading_detached_comments will keep paragraphs of comments that appear before (but not connected to) the current element. Each paragraph, separated by empty lines, will be one comment element in the repeated field. Only the comment content is provided; comment markers (e.g. //) are stripped out. For block comments, leading whitespace and an asterisk will be stripped from the beginning of each line other than the first. Newlines are included in the output. Examples: optional int32 foo = 1; // Comment attached to foo. Comment attached to bar. optional int32 bar = 2; optional string baz = 3; Comment attached to baz. Another line attached to baz. Comment attached to qux. Another line attached to qux. optional double qux = 4; Detached comment for corge. This is not leading or trailing comments to qux or corge because there are blank lines separating it from both. Detached comment for corge paragraph 2. optional string corge = 5; Block comment attached to corge. Leading asterisks will be removed. Block comment attached to grault. optional int32 grault = 6; ignored detached comments. |
| trailing_comments | [string](#string) | optional |  |
| leading_detached_comments | [string](#string) | repeated |  |






<a name="google.protobuf.UninterpretedOption"/>

### UninterpretedOption
A message representing a option the parser does not recognize. This only
appears in options protos created by the compiler::Parser class.
DescriptorPool resolves these when building Descriptor objects. Therefore,
options protos in descriptor objects (e.g. returned by Descriptor::options(),
or produced by Descriptor::CopyTo()) will never have UninterpretedOptions
in them.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [UninterpretedOption.NamePart](#google.protobuf.UninterpretedOption.NamePart) | repeated |  |
| identifier_value | [string](#string) | optional | The value of the uninterpreted option, in whatever type the tokenizer identified it as during parsing. Exactly one of these should be set. |
| positive_int_value | [uint64](#uint64) | optional |  |
| negative_int_value | [int64](#int64) | optional |  |
| double_value | [double](#double) | optional |  |
| string_value | [bytes](#bytes) | optional |  |
| aggregate_value | [string](#string) | optional |  |






<a name="google.protobuf.UninterpretedOption.NamePart"/>

### UninterpretedOption.NamePart
The name of the uninterpreted option.  Each string represents a segment in
a dot-separated name.  is_extension is true iff a segment represents an
extension (denoted with parentheses in options specs in .proto files).
E.g.,{ [&#34;foo&#34;, false], [&#34;bar.baz&#34;, true], [&#34;qux&#34;, false] } represents
&#34;foo.(bar.baz).qux&#34;.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name_part | [string](#string) | required |  |
| is_extension | [bool](#bool) | required |  |





 


<a name="google.protobuf.FieldDescriptorProto.Label"/>

### FieldDescriptorProto.Label


| Name | Number | Description |
| ---- | ------ | ----------- |
| LABEL_OPTIONAL | 1 | 0 is reserved for errors |
| LABEL_REQUIRED | 2 |  |
| LABEL_REPEATED | 3 |  |



<a name="google.protobuf.FieldDescriptorProto.Type"/>

### FieldDescriptorProto.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| TYPE_DOUBLE | 1 | 0 is reserved for errors. Order is weird for historical reasons. |
| TYPE_FLOAT | 2 |  |
| TYPE_INT64 | 3 | Not ZigZag encoded. Negative numbers take 10 bytes. Use TYPE_SINT64 if negative values are likely. |
| TYPE_UINT64 | 4 |  |
| TYPE_INT32 | 5 | Not ZigZag encoded. Negative numbers take 10 bytes. Use TYPE_SINT32 if negative values are likely. |
| TYPE_FIXED64 | 6 |  |
| TYPE_FIXED32 | 7 |  |
| TYPE_BOOL | 8 |  |
| TYPE_STRING | 9 |  |
| TYPE_GROUP | 10 | Tag-delimited aggregate. Group type is deprecated and not supported in proto3. However, Proto3 implementations should still be able to parse the group wire format and treat group fields as unknown fields. |
| TYPE_MESSAGE | 11 | Length-delimited aggregate. |
| TYPE_BYTES | 12 | New in version 2. |
| TYPE_UINT32 | 13 |  |
| TYPE_ENUM | 14 |  |
| TYPE_SFIXED32 | 15 |  |
| TYPE_SFIXED64 | 16 |  |
| TYPE_SINT32 | 17 | Uses ZigZag encoding. |
| TYPE_SINT64 | 18 | Uses ZigZag encoding. |



<a name="google.protobuf.FieldOptions.CType"/>

### FieldOptions.CType


| Name | Number | Description |
| ---- | ------ | ----------- |
| STRING | 0 | Default mode. |
| CORD | 1 |  |
| STRING_PIECE | 2 |  |



<a name="google.protobuf.FieldOptions.JSType"/>

### FieldOptions.JSType


| Name | Number | Description |
| ---- | ------ | ----------- |
| JS_NORMAL | 0 | Use the default type. |
| JS_STRING | 1 | Use JavaScript strings. |
| JS_NUMBER | 2 | Use JavaScript numbers. |



<a name="google.protobuf.FileOptions.OptimizeMode"/>

### FileOptions.OptimizeMode
Generated classes can be optimized for speed or code size.

| Name | Number | Description |
| ---- | ------ | ----------- |
| SPEED | 1 | Generate complete code for parsing, serialization, |
| CODE_SIZE | 2 | etc. Use ReflectionOps to implement these methods. |
| LITE_RUNTIME | 3 | Generate code using MessageLite and the lite runtime. |



<a name="google.protobuf.MethodOptions.IdempotencyLevel"/>

### MethodOptions.IdempotencyLevel
Is this method side-effect-free (or safe in HTTP parlance), or idempotent,
or neither? HTTP based RPC implementation may choose GET verb for safe
methods, and PUT verb for idempotent methods instead of the default POST.

| Name | Number | Description |
| ---- | ------ | ----------- |
| IDEMPOTENCY_UNKNOWN | 0 |  |
| NO_SIDE_EFFECTS | 1 | implies idempotent |
| IDEMPOTENT | 2 | idempotent, but may have side effects |


 

 

 



<a name="gogo.proto"/>
<p align="right"><a href="#top">Top</a></p>

## gogo.proto


 

 


<a name="gogo.proto-extensions"/>

### File-level Extensions
| Extension | Type | Base | Number | Description |
| --------- | ---- | ---- | ------ | ----------- |
| enum_customname | string | google.protobuf.EnumOptions | 62023 |  |
| enum_stringer | bool | google.protobuf.EnumOptions | 62022 |  |
| enumdecl | bool | google.protobuf.EnumOptions | 62024 |  |
| goproto_enum_prefix | bool | google.protobuf.EnumOptions | 62001 |  |
| goproto_enum_stringer | bool | google.protobuf.EnumOptions | 62021 |  |
| enumvalue_customname | string | google.protobuf.EnumValueOptions | 66001 |  |
| castkey | string | google.protobuf.FieldOptions | 65008 |  |
| casttype | string | google.protobuf.FieldOptions | 65007 |  |
| castvalue | string | google.protobuf.FieldOptions | 65009 |  |
| customname | string | google.protobuf.FieldOptions | 65004 |  |
| customtype | string | google.protobuf.FieldOptions | 65003 |  |
| embed | bool | google.protobuf.FieldOptions | 65002 |  |
| jsontag | string | google.protobuf.FieldOptions | 65005 |  |
| moretags | string | google.protobuf.FieldOptions | 65006 |  |
| nullable | bool | google.protobuf.FieldOptions | 65001 |  |
| stdduration | bool | google.protobuf.FieldOptions | 65011 |  |
| stdtime | bool | google.protobuf.FieldOptions | 65010 |  |
| benchgen_all | bool | google.protobuf.FileOptions | 63016 |  |
| compare_all | bool | google.protobuf.FileOptions | 63029 |  |
| description_all | bool | google.protobuf.FileOptions | 63014 |  |
| enum_stringer_all | bool | google.protobuf.FileOptions | 63022 |  |
| enumdecl_all | bool | google.protobuf.FileOptions | 63031 |  |
| equal_all | bool | google.protobuf.FileOptions | 63013 |  |
| face_all | bool | google.protobuf.FileOptions | 63005 |  |
| gogoproto_import | bool | google.protobuf.FileOptions | 63027 |  |
| goproto_enum_prefix_all | bool | google.protobuf.FileOptions | 63002 |  |
| goproto_enum_stringer_all | bool | google.protobuf.FileOptions | 63021 |  |
| goproto_extensions_map_all | bool | google.protobuf.FileOptions | 63025 |  |
| goproto_getters_all | bool | google.protobuf.FileOptions | 63001 |  |
| goproto_registration | bool | google.protobuf.FileOptions | 63032 |  |
| goproto_stringer_all | bool | google.protobuf.FileOptions | 63003 |  |
| goproto_unrecognized_all | bool | google.protobuf.FileOptions | 63026 |  |
| gostring_all | bool | google.protobuf.FileOptions | 63006 |  |
| marshaler_all | bool | google.protobuf.FileOptions | 63017 |  |
| onlyone_all | bool | google.protobuf.FileOptions | 63009 |  |
| populate_all | bool | google.protobuf.FileOptions | 63007 |  |
| protosizer_all | bool | google.protobuf.FileOptions | 63028 |  |
| sizer_all | bool | google.protobuf.FileOptions | 63020 |  |
| stable_marshaler_all | bool | google.protobuf.FileOptions | 63019 |  |
| stringer_all | bool | google.protobuf.FileOptions | 63008 |  |
| testgen_all | bool | google.protobuf.FileOptions | 63015 |  |
| typedecl_all | bool | google.protobuf.FileOptions | 63030 |  |
| unmarshaler_all | bool | google.protobuf.FileOptions | 63018 |  |
| unsafe_marshaler_all | bool | google.protobuf.FileOptions | 63023 |  |
| unsafe_unmarshaler_all | bool | google.protobuf.FileOptions | 63024 |  |
| verbose_equal_all | bool | google.protobuf.FileOptions | 63004 |  |
| benchgen | bool | google.protobuf.MessageOptions | 64016 |  |
| compare | bool | google.protobuf.MessageOptions | 64029 |  |
| description | bool | google.protobuf.MessageOptions | 64014 |  |
| equal | bool | google.protobuf.MessageOptions | 64013 |  |
| face | bool | google.protobuf.MessageOptions | 64005 |  |
| goproto_extensions_map | bool | google.protobuf.MessageOptions | 64025 |  |
| goproto_getters | bool | google.protobuf.MessageOptions | 64001 |  |
| goproto_stringer | bool | google.protobuf.MessageOptions | 64003 |  |
| goproto_unrecognized | bool | google.protobuf.MessageOptions | 64026 |  |
| gostring | bool | google.protobuf.MessageOptions | 64006 |  |
| marshaler | bool | google.protobuf.MessageOptions | 64017 |  |
| onlyone | bool | google.protobuf.MessageOptions | 64009 |  |
| populate | bool | google.protobuf.MessageOptions | 64007 |  |
| protosizer | bool | google.protobuf.MessageOptions | 64028 |  |
| sizer | bool | google.protobuf.MessageOptions | 64020 |  |
| stable_marshaler | bool | google.protobuf.MessageOptions | 64019 |  |
| stringer | bool | google.protobuf.MessageOptions | 67008 |  |
| testgen | bool | google.protobuf.MessageOptions | 64015 |  |
| typedecl | bool | google.protobuf.MessageOptions | 64030 |  |
| unmarshaler | bool | google.protobuf.MessageOptions | 64018 |  |
| unsafe_marshaler | bool | google.protobuf.MessageOptions | 64023 |  |
| unsafe_unmarshaler | bool | google.protobuf.MessageOptions | 64024 |  |
| verbose_equal | bool | google.protobuf.MessageOptions | 64004 |  |

 

 



<a name="pdsa.proto"/>
<p align="right"><a href="#top">Top</a></p>

## pdsa.proto



<a name="venice.GlobalOpts"/>

### GlobalOpts



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| OidLen | [string](#string) |  |  |
| Mib | [string](#string) |  |  |
| FillFn | [string](#string) |  |  |
| ActionFn | [string](#string) |  |  |
| UpdateForDelete | [bool](#bool) |  |  |






<a name="venice.PdsaFields"/>

### PdsaFields



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Field | [string](#string) |  |  |
| SetFieldFn | [string](#string) |  |  |
| GetFieldFn | [string](#string) |  |  |
| SetKeyOidIndex | [string](#string) |  |  |
| GetKeyOidIndex | [string](#string) |  |  |
| SetKeyOidFn | [string](#string) |  |  |
| GetKeyOidFn | [string](#string) |  |  |
| SetKeyOidLenIndex | [string](#string) |  |  |
| GetKeyOidLenIndex | [string](#string) |  |  |
| IgnoreIfEmpty | [bool](#bool) |  |  |
| IsZeroIPValid | [bool](#bool) |  |  |
| SetKeyOidAddrIndex | [string](#string) |  |  |
| SetKeyOidAddrTypeIndex | [string](#string) |  |  |
| AddrTypeFieldName | [string](#string) |  |  |
| AddrLenFieldName | [string](#string) |  |  |





 

 


<a name="pdsa.proto-extensions"/>

### File-level Extensions
| Extension | Type | Base | Number | Description |
| --------- | ---- | ---- | ------ | ----------- |
| pdsaFields | PdsaFields | google.protobuf.FieldOptions | 83400 |  |
| pdsaGetGlobOpts | GlobalOpts | google.protobuf.MessageOptions | 82401 |  |
| pdsaSetGlobOpts | GlobalOpts | google.protobuf.MessageOptions | 82400 |  |

 

 



<a name="timestamp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## timestamp.proto



<a name="google.protobuf.Timestamp"/>

### Timestamp
A Timestamp represents a point in time independent of any time zone
or calendar, represented as seconds and fractions of seconds at
nanosecond resolution in UTC Epoch time. It is encoded using the
Proleptic Gregorian Calendar which extends the Gregorian calendar
backwards to year one. It is encoded assuming all minutes are 60
seconds long, i.e. leap seconds are &#34;smeared&#34; so that no leap second
table is needed for interpretation. Range is from
0001-01-01T00:00:00Z to 9999-12-31T23:59:59.999999999Z.
By restricting to that range, we ensure that we can convert to
and from  RFC 3339 date strings.
See [https://www.ietf.org/rfc/rfc3339.txt](https://www.ietf.org/rfc/rfc3339.txt).

# Examples

Example 1: Compute Timestamp from POSIX `time()`.

Timestamp timestamp;
timestamp.set_seconds(time(NULL));
timestamp.set_nanos(0);

Example 2: Compute Timestamp from POSIX `gettimeofday()`.

struct timeval tv;
gettimeofday(&amp;tv, NULL);

Timestamp timestamp;
timestamp.set_seconds(tv.tv_sec);
timestamp.set_nanos(tv.tv_usec * 1000);

Example 3: Compute Timestamp from Win32 `GetSystemTimeAsFileTime()`.

FILETIME ft;
GetSystemTimeAsFileTime(&amp;ft);
UINT64 ticks = (((UINT64)ft.dwHighDateTime) &lt;&lt; 32) | ft.dwLowDateTime;

A Windows tick is 100 nanoseconds. Windows epoch 1601-01-01T00:00:00Z
is 11644473600 seconds before Unix epoch 1970-01-01T00:00:00Z.
Timestamp timestamp;
timestamp.set_seconds((INT64) ((ticks / 10000000) - 11644473600LL));
timestamp.set_nanos((INT32) ((ticks % 10000000) * 100));

Example 4: Compute Timestamp from Java `System.currentTimeMillis()`.

long millis = System.currentTimeMillis();

Timestamp timestamp = Timestamp.newBuilder().setSeconds(millis / 1000)
.setNanos((int) ((millis % 1000) * 1000000)).build();


Example 5: Compute Timestamp from current time in Python.

timestamp = Timestamp()
timestamp.GetCurrentTime()

# JSON Mapping

In JSON format, the Timestamp type is encoded as a string in the
[RFC 3339](https://www.ietf.org/rfc/rfc3339.txt) format. That is, the
format is &#34;{year}-{month}-{day}T{hour}:{min}:{sec}[.{frac_sec}]Z&#34;
where {year} is always expressed using four digits while {month}, {day},
{hour}, {min}, and {sec} are zero-padded to two digits each. The fractional
seconds, which can go up to 9 digits (i.e. up to 1 nanosecond resolution),
are optional. The &#34;Z&#34; suffix indicates the timezone (&#34;UTC&#34;); the timezone
is required. A proto3 JSON serializer should always use UTC (as indicated by
&#34;Z&#34;) when printing the Timestamp type and a proto3 JSON parser should be
able to accept both UTC and other timezones (as indicated by an offset).

For example, &#34;2017-01-15T01:30:15.01Z&#34; encodes 15.01 seconds past
01:30 UTC on January 15, 2017.

In JavaScript, one can convert a Date object to this format using the
standard [toISOString()](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date/toISOString]
method. In Python, a standard `datetime.datetime` object can be converted
to this format using [`strftime`](https://docs.python.org/2/library/time.html#time.strftime)
with the time format spec &#39;%Y-%m-%dT%H:%M:%S.%fZ&#39;. Likewise, in Java, one
can use the Joda Time&#39;s [`ISODateTimeFormat.dateTime()`](
http://www.joda.org/joda-time/apidocs/org/joda/time/format/ISODateTimeFormat.html#dateTime--
) to obtain a formatter capable of generating timestamps in this format.


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| seconds | [int64](#int64) |  | Represents seconds of UTC time since Unix epoch 1970-01-01T00:00:00Z. Must be from 0001-01-01T00:00:00Z to 9999-12-31T23:59:59Z inclusive. |
| nanos | [int32](#int32) |  | Non-negative fractions of a second at nanosecond resolution. Negative second values with fractions must still have non-negative nanos values that count forward in time. Must be from 0 to 999,999,999 inclusive. |





 

 

 

 



<a name="meta.proto"/>
<p align="right"><a href="#top">Top</a></p>

## meta.proto



<a name="meta.ObjMeta"/>

### ObjMeta
ObjectMeta contains metadata that all objects stored in kvstore must have


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Name | [string](#string) |  | Name of the object, unique within a Namespace for scoped objects. Should start and end in an alphanumeric character and can contain alphanumner or ._-: characters minimum length is 2 and maximum length is 64 characters |
| UUID | [string](#string) |  | UUID is the unique identifier for the object. This is generated on creation of the object. System generated, not updatable by user. |
| CreationTime | [Timestamp](#meta.Timestamp) |  | CreationTime is the creation time of the object System generated and updated, not updatable by user. |
| ModTime | [Timestamp](#meta.Timestamp) |  | ModTime is the Last Modification time of the object System generated and updated, not updatable by user. |






<a name="meta.Timestamp"/>

### Timestamp



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Time | [.google.protobuf.Timestamp](#meta..google.protobuf.Timestamp) |  |  |






<a name="meta.TypeMeta"/>

### TypeMeta
TypeMeta contains the metadata about kind and version for all API objects


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Kind | [string](#string) |  | Kind represents the type of the API object. |





 

 

 

 



<a name="types.proto"/>
<p align="right"><a href="#top">Top</a></p>

## types.proto



<a name="types.Address"/>

### Address
Address object is the prefix or range


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Prefix | [IPSubnet](#types.IPSubnet) |  | IPv4 or IPv6 prefix |
| Range | [AddressRange](#types.AddressRange) |  | IPv4 or IPv6 address range |






<a name="types.AddressRange"/>

### AddressRange
AddressRange represents an IPv4 or IPv6 address range


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| IPv4Range | [IPRange](#types.IPRange) |  | IPv4 address range |
| IPv6Range | [IPRange](#types.IPRange) |  | IPv6 address range |






<a name="types.BatchCtxt"/>

### BatchCtxt
BatchCtxt is opaque to application(s) and is expected to be passed
back (to identify the batch) in every API call of this batch until
BatchCommit() or BatchAbort()


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| BatchCookie | [uint64](#uint64) |  | opaque cookie |






<a name="types.Empty"/>

### Empty
Empty message






<a name="types.Encap"/>

### Encap
fabric encap


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [EncapType](#types.EncapType) |  | encap type |
| value | [EncapVal](#types.EncapVal) |  | encap value |






<a name="types.EncapVal"/>

### EncapVal
based on the encap type in the fabric, one can carry per vnic information
for example, in case of MPLSoUDP, MPLS tag (along with source tunnel IP)
can uniquely identify the vnic from where the packet is originating and
another MPLS tag (along with the destination tunnel IP) can identify the
destination VNIC


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| VlanId | [uint32](#uint32) |  | MPLS tag corresponding to the VNIC or mapping VLAN tag |
| MPLSTag | [uint32](#uint32) |  | MPLS tag/slot |
| Vnid | [uint32](#uint32) |  | VxLAN vnid |
| QinQTag | [QinQTag](#types.QinQTag) |  | QinQ tag |






<a name="types.FlowKey"/>

### FlowKey
flow key is either IP flow key or non-IP flow key


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| IPFlowKey | [IPFlowKey](#types.IPFlowKey) |  | flow key for the IPv4 and IPv6 flows |
| MACFlowKey | [MACFlowKey](#types.MACFlowKey) |  | flow key for the non-IP flows |






<a name="types.FlowL4Info"/>

### FlowL4Info
L4 portion of flow key tuple


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| TcpUdpInfo | [FlowL4Info.TCPUDPInfo](#types.FlowL4Info.TCPUDPInfo) |  |  |
| IcmpInfo | [FlowL4Info.ICMPInfo](#types.FlowL4Info.ICMPInfo) |  |  |






<a name="types.FlowL4Info.ICMPInfo"/>

### FlowL4Info.ICMPInfo
key fields for ICMP flows


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Type | [uint32](#uint32) |  |  |
| Code | [uint32](#uint32) |  |  |
| Id | [uint32](#uint32) |  |  |






<a name="types.FlowL4Info.TCPUDPInfo"/>

### FlowL4Info.TCPUDPInfo
key fields for TCP/UDP flows


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| SrcPort | [uint32](#uint32) |  |  |
| DstPort | [uint32](#uint32) |  |  |






<a name="types.ICMPMatch"/>

### ICMPMatch
ICMPv4/ICMPv6 rule match criteria


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [uint32](#uint32) |  | ICMP type |
| code | [uint32](#uint32) |  | ICMP code |






<a name="types.IPAddress"/>

### IPAddress
IP Address object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Af | [IPAF](#types.IPAF) |  |  |
| V4Addr | [fixed32](#fixed32) |  | IPv4 address |
| V6Addr | [bytes](#bytes) |  | IPv6 address |






<a name="types.IPFlowKey"/>

### IPFlowKey
flow key for IP flows


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| VPCId | [bytes](#bytes) |  | VPC identifier |
| SrcIP | [IPAddress](#types.IPAddress) |  | source IP seen in the packet |
| DstIP | [IPAddress](#types.IPAddress) |  | destination IP seen in the packet |
| IPProtocol | [uint32](#uint32) |  | IP protocol |
| L4Info | [FlowL4Info](#types.FlowL4Info) |  | L4 information of the flow key |






<a name="types.IPPrefix"/>

### IPPrefix
IP Prefix object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Addr | [IPAddress](#types.IPAddress) |  | IP prefix address |
| Len | [uint32](#uint32) |  | IP Prefix length |






<a name="types.IPRange"/>

### IPRange
IP Range


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Low | [IPAddress](#types.IPAddress) |  | starting IP address |
| High | [IPAddress](#types.IPAddress) |  | ending IP address |






<a name="types.IPSubnet"/>

### IPSubnet
IPSubnet represents an IPv4 or IPv6 subnet


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| IPv4Subnet | [IPPrefix](#types.IPPrefix) |  | IPv4 subnet |
| IPv6Subnet | [IPPrefix](#types.IPPrefix) |  | IPv6 subnet |






<a name="types.IPv4Prefix"/>

### IPv4Prefix
IPv4 Prefix


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Addr | [fixed32](#fixed32) |  | IPv4 address portion |
| Len | [uint32](#uint32) |  | prefix length |






<a name="types.IPv6Prefix"/>

### IPv6Prefix
IPv6 Prefix


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Addr | [bytes](#bytes) |  | IPv6 address bytes |
| Len | [uint32](#uint32) |  | prefix length |






<a name="types.MACFlowKey"/>

### MACFlowKey
flow key for non-IP flows


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Subnet | [bytes](#bytes) |  | subnet identifier |
| SrcMAC | [uint64](#uint64) |  | source MAC address |
| DstMAC | [uint64](#uint64) |  | destination MAC address |






<a name="types.PortMatch"/>

### PortMatch
TCP/UDP rule match criteria


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| SrcPortRange | [PortRange](#types.PortRange) |  | source port range |
| DstPortRange | [PortRange](#types.PortRange) |  | destination port range |






<a name="types.PortRange"/>

### PortRange
PortRange object has low and high end of the port ranges


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| PortLow | [uint32](#uint32) |  |  |
| PortHigh | [uint32](#uint32) |  |  |






<a name="types.QinQTag"/>

### QinQTag
QinQ vlan tag


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| cTag | [uint32](#uint32) |  | Customer VLAN tag |
| sTag | [uint32](#uint32) |  | Service VLAN tag |






<a name="types.RuleL3Match"/>

### RuleL3Match
L3 rule match criteria


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Protocol | [uint32](#uint32) |  | IP Protocol |
| SrcPrefix | [IPPrefix](#types.IPPrefix) |  | Src IP Prefix |
| SrcRange | [AddressRange](#types.AddressRange) |  | Src IP Range |
| SrcTag | [uint32](#uint32) |  | Src Tag |
| DstPrefix | [IPPrefix](#types.IPPrefix) |  | Dst IP Prefix |
| DstRange | [AddressRange](#types.AddressRange) |  | Dst IP Range |
| DstTag | [uint32](#uint32) |  | Dst Tag |






<a name="types.RuleL4Match"/>

### RuleL4Match
L4 rule match criteria


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Ports | [PortMatch](#types.PortMatch) |  | port match criteria |
| TypeCode | [ICMPMatch](#types.ICMPMatch) |  | ICMP match criteria |






<a name="types.RuleMatch"/>

### RuleMatch
rule match criteria


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| L3Match | [RuleL3Match](#types.RuleL3Match) |  | Layer 3 match criteria |
| L4Match | [RuleL4Match](#types.RuleL4Match) |  | Layer 4 match criteria |





 


<a name="types.AddressType"/>

### AddressType
Public/Internet or Service/Infra address

| Name | Number | Description |
| ---- | ------ | ----------- |
| ADDR_TYPE_NONE | 0 |  |
| ADDR_TYPE_PUBLIC | 1 |  |
| ADDR_TYPE_SERVICE | 2 |  |



<a name="types.ApiStatus"/>

### ApiStatus
API return codes (more descriptive than protobuf enums)

| Name | Number | Description |
| ---- | ------ | ----------- |
| API_STATUS_OK | 0 | success |
| API_STATUS_ERR | 1 | catch-all error |
| API_STATUS_INVALID_ARG | 2 | invalid argument |
| API_STATUS_EXISTS_ALREADY | 3 | object already exists |
| API_STATUS_OUT_OF_MEM | 4 | out of memory |
| API_STATUS_NOT_FOUND | 5 | object not found |
| API_STATUS_OUT_OF_RESOURCE | 6 | out of resource |
| API_STATUS_RETRY | 7 | if operation can be retried |
| API_STATUS_OPERATION_NOT_ALLOWED | 8 | if operation is not allowed on |



<a name="types.EncapType"/>

### EncapType
encap types in the network

| Name | Number | Description |
| ---- | ------ | ----------- |
| ENCAP_TYPE_NONE | 0 |  |
| ENCAP_TYPE_DOT1Q | 1 |  |
| ENCAP_TYPE_QINQ | 2 |  |
| ENCAP_TYPE_MPLSoUDP | 3 |  |
| ENCAP_TYPE_VXLAN | 4 |  |



<a name="types.IPAF"/>

### IPAF
IP address families

| Name | Number | Description |
| ---- | ------ | ----------- |
| IP_AF_NONE | 0 |  |
| IP_AF_INET | 1 | IPv4 |
| IP_AF_INET6 | 2 | IPv6 |



<a name="types.LifType"/>

### LifType
Types of lif

| Name | Number | Description |
| ---- | ------ | ----------- |
| LIF_TYPE_NONE | 0 |  |
| LIF_TYPE_HOST | 1 |  |
| LIF_TYPE_HOST_MGMT | 2 |  |
| LIF_TYPE_OOB_MGMT | 3 |  |
| LIF_TYPE_INBAND_MGMT | 4 |  |
| LIF_TYPE_INTERNAL_MGMT | 5 |  |
| LIF_TYPE_DATAPATH | 6 |  |
| LIF_TYPE_LEARN | 7 |  |



<a name="types.NatAction"/>

### NatAction
types of NAT actions supported

| Name | Number | Description |
| ---- | ------ | ----------- |
| NAT_ACTION_NONE | 0 |  |
| NAT_ACTION_STATIC | 1 | Address only static NAT |
| NAT_ACTION_NAPT_PUBLIC | 2 | address and port translation to the public (aka. Internet) address space |
| NAT_ACTION_NAPT_SVC | 3 | address and port translation to internal service address space |



<a name="types.RuleDir"/>

### RuleDir
direction in which policy is enforced
INGRESS/EGRESS is w.r.t vnic (i.e., traffic leaving vnic is marked as
EGRESS and traffic going to vnic is marked as INGRESS

| Name | Number | Description |
| ---- | ------ | ----------- |
| RULE_DIR_NONE | 0 |  |
| RULE_DIR_INGRESS | 1 |  |
| RULE_DIR_EGRESS | 2 |  |



<a name="types.SecurityRuleAction"/>

### SecurityRuleAction
Security rule action is one of the below

| Name | Number | Description |
| ---- | ------ | ----------- |
| SECURITY_RULE_ACTION_NONE | 0 |  |
| SECURITY_RULE_ACTION_ALLOW | 1 |  |
| SECURITY_RULE_ACTION_DENY | 2 |  |


 

 

 



<a name="common.proto"/>
<p align="right"><a href="#top">Top</a></p>

## common.proto


 


<a name="pds.AdminState"/>

### AdminState
Admin State

| Name | Number | Description |
| ---- | ------ | ----------- |
| ADMIN_STATE_NONE | 0 |  |
| ADMIN_STATE_ENABLE | 1 |  |
| ADMIN_STATE_DISABLE | 2 |  |


 

 

 



<a name="bgp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## bgp.proto



<a name="pds.BGP"/>

### BGP
BGP object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [BGPSpec](#pds.BGPSpec) |  |  |
| Status | [BGPStatus](#pds.BGPStatus) |  |  |






<a name="pds.BGPGetResponse"/>

### BGPGetResponse
BGP spec get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [BGP](#pds.BGP) |  |  |






<a name="pds.BGPNLRIPrefix"/>

### BGPNLRIPrefix
BGP NLRI prefix object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Status | [BGPNLRIPrefixStatus](#pds.BGPNLRIPrefixStatus) |  |  |






<a name="pds.BGPNLRIPrefixGetResponse"/>

### BGPNLRIPrefixGetResponse
BGP NLRI prefix get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [BGPNLRIPrefix](#pds.BGPNLRIPrefix) | repeated |  |






<a name="pds.BGPNLRIPrefixRequest"/>

### BGPNLRIPrefixRequest
BGP NLRI prefix request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [BGPNLRIPrefixStatus](#pds.BGPNLRIPrefixStatus) |  | request |






<a name="pds.BGPNLRIPrefixStatus"/>

### BGPNLRIPrefixStatus
BGP NLRI prefix table


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Afi | [BGPAfi](#pds.BGPAfi) |  | BGP address family |
| Safi | [BGPSafi](#pds.BGPSafi) |  | BGP sub-address family |
| Prefix | [bytes](#bytes) |  | prefix in NLRI |
| PrefixLen | [uint32](#uint32) |  | prefix length |
| RouteSource | [NLRISrc](#pds.NLRISrc) |  | route source (learned from BGP peer or AFM) |
| RouteSourceIndex | [uint32](#uint32) |  | route source index |
| PathID | [uint32](#uint32) |  | path ID |
| BestRoute | [bool](#bool) |  | chosen as BGP best route or not |
| ASPathStr | [bytes](#bytes) |  | AS path string |
| PathOrigId | [bytes](#bytes) |  | originator ID |
| NextHopAddr | [bytes](#bytes) |  | next-hop address |






<a name="pds.BGPPeer"/>

### BGPPeer
BGP peer object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [BGPPeerSpec](#pds.BGPPeerSpec) |  |  |
| Status | [BGPPeerStatus](#pds.BGPPeerStatus) |  |  |






<a name="pds.BGPPeerAf"/>

### BGPPeerAf
BGP peer AF object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [BGPPeerAfSpec](#pds.BGPPeerAfSpec) |  |  |
| Status | [BGPPeerAfStatus](#pds.BGPPeerAfStatus) |  |  |






<a name="pds.BGPPeerAfGetResponse"/>

### BGPPeerAfGetResponse
BGP peer AF get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [BGPPeerAf](#pds.BGPPeerAf) | repeated |  |






<a name="pds.BGPPeerAfRequest"/>

### BGPPeerAfRequest
BGP peer AF create and update request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [BGPPeerAfSpec](#pds.BGPPeerAfSpec) | repeated | requests |






<a name="pds.BGPPeerAfResponse"/>

### BGPPeerAfResponse
BGP peer AF create and update response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |






<a name="pds.BGPPeerAfSpec"/>

### BGPPeerAfSpec
BGP peer AF configurations


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | unique key/identifier of peer AF config |
| LocalAddr | [.types.IPAddress](#pds..types.IPAddress) |  | BGP local IP address |
| PeerAddr | [.types.IPAddress](#pds..types.IPAddress) |  | BGP peer IP address |
| Afi | [BGPAfi](#pds.BGPAfi) |  | BGP address family |
| Safi | [BGPSafi](#pds.BGPSafi) |  | BGP sub-address family |
| Disable | [bool](#bool) |  | disable adress family. IPv4-unicast and l2vpn-evpn address families are enabled by default TODO: Disable field should become &#34;internal-only&#34; field once auto-gen support for this special case is added create/update should set Disable field to false delete should set Disable field to true |
| NexthopSelf | [bool](#bool) |  | enforce this router to set self as next-hop for advertised routes |
| DefaultOrig | [bool](#bool) |  | originate a default route to this peer |






<a name="pds.BGPPeerAfStatus"/>

### BGPPeerAfStatus
BGP peer AF status and stats, if any






<a name="pds.BGPPeerGetResponse"/>

### BGPPeerGetResponse
BGP peer get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [BGPPeer](#pds.BGPPeer) | repeated |  |






<a name="pds.BGPPeerRequest"/>

### BGPPeerRequest
BGP peer create and update request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [BGPPeerSpec](#pds.BGPPeerSpec) | repeated | requests |






<a name="pds.BGPPeerResponse"/>

### BGPPeerResponse
BGP peer create and update response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |






<a name="pds.BGPPeerSpec"/>

### BGPPeerSpec
BGP peer configurations


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | unique key/identifier of peer |
| State | [AdminState](#pds.AdminState) |  | peer enable/disable admin state. if peer is not enabled then local router must not initiate connections to the neighbor and must not respond to TCP connections attempts from neighbor |
| LocalAddr | [.types.IPAddress](#pds..types.IPAddress) |  | BGP local IP address. control plane chooses the local IP address of the session if an all 0 IP address is provided |
| PeerAddr | [.types.IPAddress](#pds..types.IPAddress) |  | BGP peer IP address |
| RemoteASN | [uint32](#uint32) |  | remote 4-byte AS number |
| SendComm | [bool](#bool) |  | send regular community attributes to neighbor |
| SendExtComm | [bool](#bool) |  | send extended community attributes to neighbor |
| RRClient | [BGPPeerRRClient](#pds.BGPPeerRRClient) |  | peer is a route reflector client |
| ConnectRetry | [uint32](#uint32) |  | BGP session connect-retry timer in seconds |
| HoldTime | [uint32](#uint32) |  | BGP session holdtime timer in seconds |
| KeepAlive | [uint32](#uint32) |  | BGP session keepalive timer in seconds |
| Password | [bytes](#bytes) |  | MD5 authentication |






<a name="pds.BGPPeerStatus"/>

### BGPPeerStatus
BGP peer status


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | unique key/identifier of peer |
| Status | [BGPPeerState](#pds.BGPPeerState) |  | BGP session status |
| PrevStatus | [BGPPeerState](#pds.BGPPeerState) |  | BGP session previous status |
| LastErrorRcvd | [bytes](#bytes) |  | last error received |
| LastErrorSent | [bytes](#bytes) |  | Last error sent |
| LocalAddr | [.types.IPAddress](#pds..types.IPAddress) |  | internal only BGP local IP address |
| PeerAddr | [.types.IPAddress](#pds..types.IPAddress) |  | BGP peer IP address |






<a name="pds.BGPRequest"/>

### BGPRequest
BGP spec create and update request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [BGPSpec](#pds.BGPSpec) |  | requests |






<a name="pds.BGPResponse"/>

### BGPResponse
BGP spec create and update response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [BGPStatus](#pds.BGPStatus) |  |  |






<a name="pds.BGPSpec"/>

### BGPSpec
BGP configuration


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | unique key/identifier of BGP config |
| LocalASN | [uint32](#uint32) |  | the local autonomous system number |
| RouterId | [fixed32](#fixed32) |  | router ID for this bgp instance |
| ClusterId | [fixed32](#fixed32) |  | cluster ID of the local router. if not configured, router ID will be used as cluster ID |






<a name="pds.BGPStatus"/>

### BGPStatus
operational status of BGP spec, if any





 


<a name="pds.BGPAfi"/>

### BGPAfi
BGP address family identifier

| Name | Number | Description |
| ---- | ------ | ----------- |
| BGP_AFI_NONE | 0 |  |
| BGP_AFI_IPV4 | 1 |  |
| BGP_AFI_IPV6 | 2 |  |
| BGP_AFI_L2VPN | 25 |  |



<a name="pds.BGPPeerRRClient"/>

### BGPPeerRRClient
BGP peer RR client

| Name | Number | Description |
| ---- | ------ | ----------- |
| BGP_PEER_RR_NONE | 0 |  |
| BGP_PEER_RR_CLIENT | 1 |  |
| BGP_PEER_RR_MESHED_CLIENT | 2 |  |



<a name="pds.BGPPeerState"/>

### BGPPeerState
BGP peer last_state

| Name | Number | Description |
| ---- | ------ | ----------- |
| BGP_PEER_STATE_NONE | 0 |  |
| BGP_PEER_STATE_IDLE | 1 |  |
| BGP_PEER_STATE_CONNECT | 2 |  |
| BGP_PEER_STATE_ACTIVE | 3 |  |
| BGP_PEER_STATE_OPENSENT | 4 |  |
| BGP_PEER_STATE_OPENCONFIRM | 5 |  |
| BGP_PEER_STATE_ESTABLISHED | 6 |  |



<a name="pds.BGPSafi"/>

### BGPSafi
BGP sub-address family identitfier

| Name | Number | Description |
| ---- | ------ | ----------- |
| BGP_SAFI_NONE | 0 |  |
| BGP_SAFI_UNICAST | 1 |  |
| BGP_SAFI_MULTICAST | 2 |  |
| BGP_SAFI_BOTH | 3 |  |
| BGP_SAFI_LABEL | 4 |  |
| BGP_SAFI_VPLS | 65 |  |
| BGP_SAFI_EVPN | 70 |  |
| BGP_SAFI_MPLS_BGP_VPN | 128 |  |
| BGP_SAFI_PRIVATE | 241 |  |



<a name="pds.NLRISrc"/>

### NLRISrc
NLRI source

| Name | Number | Description |
| ---- | ------ | ----------- |
| NLRI_NONE | 0 |  |
| NLRI_PEER | 1 | learned from peer |
| NLRI_AFM | 2 | learned from AFM |
| NLRI_SELF | 3 | created by itself |


 

 


<a name="pds.BGPSvc"/>

### BGPSvc
bgp service definition

| Method Name | Request Type | Response Type | Description |
| ----------- | ------------ | ------------- | ------------|
| BGPCreate | [BGPRequest](#pds.BGPRequest) | [BGPResponse](#pds.BGPRequest) |  |
| BGPUpdate | [BGPRequest](#pds.BGPRequest) | [BGPResponse](#pds.BGPRequest) |  |
| BGPDelete | [BGPRequest](#pds.BGPRequest) | [BGPResponse](#pds.BGPRequest) |  |
| BGPGet | [BGPRequest](#pds.BGPRequest) | [BGPGetResponse](#pds.BGPRequest) |  |
| BGPPeerCreate | [BGPPeerRequest](#pds.BGPPeerRequest) | [BGPPeerResponse](#pds.BGPPeerRequest) |  |
| BGPPeerUpdate | [BGPPeerRequest](#pds.BGPPeerRequest) | [BGPPeerResponse](#pds.BGPPeerRequest) |  |
| BGPPeerDelete | [BGPPeerRequest](#pds.BGPPeerRequest) | [BGPPeerResponse](#pds.BGPPeerRequest) |  |
| BGPPeerGet | [BGPPeerRequest](#pds.BGPPeerRequest) | [BGPPeerGetResponse](#pds.BGPPeerRequest) |  |
| BGPPeerAfCreate | [BGPPeerAfRequest](#pds.BGPPeerAfRequest) | [BGPPeerAfResponse](#pds.BGPPeerAfRequest) |  |
| BGPPeerAfUpdate | [BGPPeerAfRequest](#pds.BGPPeerAfRequest) | [BGPPeerAfResponse](#pds.BGPPeerAfRequest) |  |
| BGPPeerAfDelete | [BGPPeerAfRequest](#pds.BGPPeerAfRequest) | [BGPPeerAfResponse](#pds.BGPPeerAfRequest) |  |
| BGPPeerAfGet | [BGPPeerAfRequest](#pds.BGPPeerAfRequest) | [BGPPeerAfGetResponse](#pds.BGPPeerAfRequest) |  |
| BGPNLRIPrefixGet | [BGPNLRIPrefixRequest](#pds.BGPNLRIPrefixRequest) | [BGPNLRIPrefixGetResponse](#pds.BGPNLRIPrefixRequest) |  |

 



<a name="cp_route.proto"/>
<p align="right"><a href="#top">Top</a></p>

## cp_route.proto
------------------------------------------------------------------------------
{C} Copyright 2019 Pensando Systems Inc. All rights reserved
protobuf specification for Control Plane Route Table
------------------------------------------------------------------------------


<a name="pds.CPRoute"/>

### CPRoute
control plane route object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Status | [CPRouteStatus](#pds.CPRouteStatus) |  |  |






<a name="pds.CPRouteGetRequest"/>

### CPRouteGetRequest
control plane route table get request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [CPRouteStatus](#pds.CPRouteStatus) | repeated | requests |






<a name="pds.CPRouteGetResponse"/>

### CPRouteGetResponse



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [CPRoute](#pds.CPRoute) | repeated |  |






<a name="pds.CPRouteStatus"/>

### CPRouteStatus
control plane route table


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| RouteTableId | [uint32](#uint32) |  | route table id this route belongs to |
| DestAddr | [.types.IPAddress](#pds..types.IPAddress) |  | destination address |
| DestPrefixLen | [uint32](#uint32) |  | destination address prefix |
| NHAddr | [.types.IPAddress](#pds..types.IPAddress) |  | next-hop address |
| IfIndex | [uint32](#uint32) |  | interface index, if the next-hop is an interface |
| Type | [RouteType](#pds.RouteType) |  | type of route |
| Proto | [RouteProtocol](#pds.RouteProtocol) |  | protocol via which the route is learned |






<a name="pds.CPStaticRoute"/>

### CPStaticRoute
control plane static route object


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [CPStaticRouteSpec](#pds.CPStaticRouteSpec) |  |  |






<a name="pds.CPStaticRouteGetResponse"/>

### CPStaticRouteGetResponse



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [CPStaticRoute](#pds.CPStaticRoute) | repeated |  |






<a name="pds.CPStaticRouteRequest"/>

### CPStaticRouteRequest
control plane static route create, delete and update message


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| request | [CPStaticRouteSpec](#pds.CPStaticRouteSpec) | repeated |  |






<a name="pds.CPStaticRouteResponse"/>

### CPStaticRouteResponse
response messages


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [CPStaticRouteStatus](#pds.CPStaticRouteStatus) |  |  |






<a name="pds.CPStaticRouteSpec"/>

### CPStaticRouteSpec
control plane static route configuration


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| RouteTableId | [bytes](#bytes) |  | route table id this route belongs to |
| DestAddr | [.types.IPAddress](#pds..types.IPAddress) |  | destination address |
| PrefixLen | [uint32](#uint32) |  | destination address prefix length |
| NextHopAddr | [.types.IPAddress](#pds..types.IPAddress) |  | next-hop address |
| State | [AdminState](#pds.AdminState) |  | route is enabled or not |
| Override | [bool](#bool) |  | override admin distance |
| AdminDist | [uint32](#uint32) |  | admin distance |
| InterfaceId | [bytes](#bytes) |  | internal only outgoing interface UUID needed on pegasus |
| IfIndex | [uint32](#uint32) |  |  |






<a name="pds.CPStaticRouteStatus"/>

### CPStaticRouteStatus
status of control plane static route, if any





 


<a name="pds.RouteProtocol"/>

### RouteProtocol
route protocols

| Name | Number | Description |
| ---- | ------ | ----------- |
| ROUTE_PROTO_NONE | 0 |  |
| ROUTE_PROTO_LOCAL | 2 |  |
| ROUTE_PROTO_STATIC | 3 |  |
| ROUTE_PROTO_BGP | 14 |  |



<a name="pds.RouteType"/>

### RouteType
route type

| Name | Number | Description |
| ---- | ------ | ----------- |
| ROUTE_TYPE_NONE | 0 |  |
| ROUTE_TYPE_OTHER | 1 |  |
| ROUTE_TYPE_REJECT | 2 |  |
| ROUTE_TYPE_LOCAL | 3 |  |
| ROUTE_TYPE_REMOTE | 4 |  |


 

 


<a name="pds.CPRouteSvc"/>

### CPRouteSvc
Control Plane Route service definition

| Method Name | Request Type | Response Type | Description |
| ----------- | ------------ | ------------- | ------------|
| CPStaticRouteCreate | [CPStaticRouteRequest](#pds.CPStaticRouteRequest) | [CPStaticRouteResponse](#pds.CPStaticRouteRequest) |  |
| CPStaticRouteUpdate | [CPStaticRouteRequest](#pds.CPStaticRouteRequest) | [CPStaticRouteResponse](#pds.CPStaticRouteRequest) |  |
| CPStaticRouteDelete | [CPStaticRouteRequest](#pds.CPStaticRouteRequest) | [CPStaticRouteResponse](#pds.CPStaticRouteRequest) |  |
| CPStaticRouteGet | [CPStaticRouteRequest](#pds.CPStaticRouteRequest) | [CPStaticRouteGetResponse](#pds.CPStaticRouteRequest) |  |
| CPRouteGet | [CPRouteGetRequest](#pds.CPRouteGetRequest) | [CPRouteGetResponse](#pds.CPRouteGetRequest) |  |

 



<a name="cp_test.proto"/>
<p align="right"><a href="#top">Top</a></p>

## cp_test.proto



<a name="pds.CPL2fTestCreateSpec"/>

### CPL2fTestCreateSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| SubnetId | [bytes](#bytes) |  |  |
| IPAddr | [.types.IPAddress](#pds..types.IPAddress) |  |  |
| MACAddr | [bytes](#bytes) |  |  |
| IfId | [uint32](#uint32) |  |  |






<a name="pds.CPL2fTestDeleteSpec"/>

### CPL2fTestDeleteSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| SubnetId | [bytes](#bytes) |  |  |
| IPAddr | [.types.IPAddress](#pds..types.IPAddress) |  |  |
| MACAddr | [bytes](#bytes) |  |  |






<a name="pds.CPL2fTestResponse"/>

### CPL2fTestResponse



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |





 

 

 


<a name="pds.CPTestSvc"/>

### CPTestSvc
service definition

| Method Name | Request Type | Response Type | Description |
| ----------- | ------------ | ------------- | ------------|
| CPL2fTestCreate | [CPL2fTestCreateSpec](#pds.CPL2fTestCreateSpec) | [CPL2fTestResponse](#pds.CPL2fTestCreateSpec) |  |
| CPL2fTestDelete | [CPL2fTestDeleteSpec](#pds.CPL2fTestDeleteSpec) | [CPL2fTestResponse](#pds.CPL2fTestDeleteSpec) |  |

 



<a name="evpn.proto"/>
<p align="right"><a href="#top">Top</a></p>

## evpn.proto



<a name="pds.EvpnEvi"/>

### EvpnEvi



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [EvpnEviSpec](#pds.EvpnEviSpec) |  |  |






<a name="pds.EvpnEviGetResponse"/>

### EvpnEviGetResponse
EvpnEviSpec get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [EvpnEvi](#pds.EvpnEvi) | repeated |  |






<a name="pds.EvpnEviRequest"/>

### EvpnEviRequest
EvpnEvi create and update request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [EvpnEviSpec](#pds.EvpnEviSpec) | repeated | requests |






<a name="pds.EvpnEviRt"/>

### EvpnEviRt



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [EvpnEviRtSpec](#pds.EvpnEviRtSpec) |  |  |






<a name="pds.EvpnEviRtGetResponse"/>

### EvpnEviRtGetResponse
EvpnEviRtSpec get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [EvpnEviRt](#pds.EvpnEviRt) | repeated |  |






<a name="pds.EvpnEviRtRequest"/>

### EvpnEviRtRequest
EvpnEviRtSpec create and update request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [EvpnEviRtSpec](#pds.EvpnEviRtSpec) | repeated | requests |






<a name="pds.EvpnEviRtSpec"/>

### EvpnEviRtSpec
EVPN EVI RT Configuration


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | spec UUID key |
| SubnetId | [bytes](#bytes) |  | reference to subnet spec |
| RT | [bytes](#bytes) |  |  |
| RTType | [EvpnRtType](#pds.EvpnRtType) |  |  |
| EVIId | [uint32](#uint32) |  | Internal only |






<a name="pds.EvpnEviSpec"/>

### EvpnEviSpec
EVPN EVI configuration


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | spec UUID key |
| SubnetId | [bytes](#bytes) |  | reference to subnet spec |
| AutoRD | [EvpnCfg](#pds.EvpnCfg) |  |  |
| RD | [bytes](#bytes) |  | Not mandatory when AutoRD is set to EVPN_CFG_AUTO |
| AutoRT | [EvpnCfg](#pds.EvpnCfg) |  |  |
| RTType | [EvpnRtType](#pds.EvpnRtType) |  |  |
| Encap | [EvpnEncaps](#pds.EvpnEncaps) |  |  |
| EVIId | [uint32](#uint32) |  | Internal only |






<a name="pds.EvpnIpVrf"/>

### EvpnIpVrf



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [EvpnIpVrfSpec](#pds.EvpnIpVrfSpec) |  |  |






<a name="pds.EvpnIpVrfGetResponse"/>

### EvpnIpVrfGetResponse
EvpnIpVrfSpec get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [EvpnIpVrf](#pds.EvpnIpVrf) | repeated |  |






<a name="pds.EvpnIpVrfRequest"/>

### EvpnIpVrfRequest
EvpnIPVrf create and update request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [EvpnIpVrfSpec](#pds.EvpnIpVrfSpec) | repeated | requests |






<a name="pds.EvpnIpVrfRt"/>

### EvpnIpVrfRt



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [EvpnIpVrfRtSpec](#pds.EvpnIpVrfRtSpec) |  |  |






<a name="pds.EvpnIpVrfRtGetResponse"/>

### EvpnIpVrfRtGetResponse
EvpnIpVrfRtSpec get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [EvpnIpVrfRt](#pds.EvpnIpVrfRt) | repeated |  |






<a name="pds.EvpnIpVrfRtRequest"/>

### EvpnIpVrfRtRequest



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [EvpnIpVrfRtSpec](#pds.EvpnIpVrfRtSpec) | repeated | requests |






<a name="pds.EvpnIpVrfRtSpec"/>

### EvpnIpVrfRtSpec
EVPN IP VRF RT Configuration


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | spec UUID key |
| VPCId | [bytes](#bytes) |  | reference to VPS spec |
| RT | [bytes](#bytes) |  |  |
| RTType | [EvpnRtType](#pds.EvpnRtType) |  |  |
| VRFName | [string](#string) |  | Internal only |






<a name="pds.EvpnIpVrfSpec"/>

### EvpnIpVrfSpec
EVPN IP VRF configuration


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Id | [bytes](#bytes) |  | spec UUID key |
| VPCId | [bytes](#bytes) |  | reference to VPC spec |
| VNI | [uint32](#uint32) |  |  |
| AutoRD | [EvpnCfg](#pds.EvpnCfg) |  |  |
| RD | [bytes](#bytes) |  |  |
| VRFName | [string](#string) |  | Internal only |






<a name="pds.EvpnMacIp"/>

### EvpnMacIp



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Spec | [EvpnMacIpSpec](#pds.EvpnMacIpSpec) |  |  |






<a name="pds.EvpnMacIpGetResponse"/>

### EvpnMacIpGetResponse
EVPN MAC IP get response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [EvpnMacIp](#pds.EvpnMacIp) | repeated |  |






<a name="pds.EvpnMacIpSpec"/>

### EvpnMacIpSpec
EVPN MAC IP Table Get


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| EVIId | [uint32](#uint32) |  |  |
| EthTagID | [uint32](#uint32) |  |  |
| MACAddress | [bytes](#bytes) |  |  |
| IPAddress | [.types.IPAddress](#pds..types.IPAddress) |  |  |
| PathID | [uint32](#uint32) |  |  |
| Source | [EvpnSource](#pds.EvpnSource) |  |  |
| NHAddress | [.types.IPAddress](#pds..types.IPAddress) |  |  |
| LocalIfId | [uint32](#uint32) |  |  |






<a name="pds.EvpnMacIpSpecRequest"/>

### EvpnMacIpSpecRequest



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Request | [EvpnMacIpSpec](#pds.EvpnMacIpSpec) | repeated | requests |






<a name="pds.EvpnResponse"/>

### EvpnResponse
EVPN specs create and update response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ApiStatus | [.types.ApiStatus](#pds..types.ApiStatus) |  |  |
| Response | [EvpnStatus](#pds.EvpnStatus) |  |  |






<a name="pds.EvpnStatus"/>

### EvpnStatus
operational status of EVPN, if any





 


<a name="pds.EvpnCfg"/>

### EvpnCfg
EVPN Configured or Auto Values

| Name | Number | Description |
| ---- | ------ | ----------- |
| EVPN_CFG_INVALID | 0 |  |
| EVPN_CFG_MANUAL | 2 |  |
| EVPN_CFG_AUTO | 3 |  |



<a name="pds.EvpnEncaps"/>

### EvpnEncaps


| Name | Number | Description |
| ---- | ------ | ----------- |
| EVPN_ENCAP_INVALID | 0 |  |
| EVPN_ENCAP_MPLS | 1 |  |
| EVPN_ENCAP_VXLAN | 2 |  |



<a name="pds.EvpnRtType"/>

### EvpnRtType


| Name | Number | Description |
| ---- | ------ | ----------- |
| EVPN_RT_INVALID | 0 |  |
| EVPN_RT_IMPORT | 1 |  |
| EVPN_RT_EXPORT | 2 |  |
| EVPN_RT_IMPORT_EXPORT | 3 |  |
| EVPN_RT_NONE | 4 |  |



<a name="pds.EvpnSource"/>

### EvpnSource


| Name | Number | Description |
| ---- | ------ | ----------- |
| EVPN_SOURCE_NONE | 0 |  |
| EVPN_SOURCE_LOCAL | 1 |  |
| EVPN_SOURCE_REMOTE | 2 |  |


 

 


<a name="pds.EvpnSvc"/>

### EvpnSvc
Evpn service definition

| Method Name | Request Type | Response Type | Description |
| ----------- | ------------ | ------------- | ------------|
| EvpnEviSpecCreate | [EvpnEviRequest](#pds.EvpnEviRequest) | [EvpnResponse](#pds.EvpnEviRequest) |  |
| EvpnEviSpecUpdate | [EvpnEviRequest](#pds.EvpnEviRequest) | [EvpnResponse](#pds.EvpnEviRequest) |  |
| EvpnEviSpecDelete | [EvpnEviRequest](#pds.EvpnEviRequest) | [EvpnResponse](#pds.EvpnEviRequest) |  |
| EvpnEviSpecGet | [EvpnEviRequest](#pds.EvpnEviRequest) | [EvpnEviGetResponse](#pds.EvpnEviRequest) |  |
| EvpnEviRtSpecCreate | [EvpnEviRtRequest](#pds.EvpnEviRtRequest) | [EvpnResponse](#pds.EvpnEviRtRequest) |  |
| EvpnEviRtSpecUpdate | [EvpnEviRtRequest](#pds.EvpnEviRtRequest) | [EvpnResponse](#pds.EvpnEviRtRequest) |  |
| EvpnEviRtSpecDelete | [EvpnEviRtRequest](#pds.EvpnEviRtRequest) | [EvpnResponse](#pds.EvpnEviRtRequest) |  |
| EvpnEviRtSpecGet | [EvpnEviRtRequest](#pds.EvpnEviRtRequest) | [EvpnEviRtGetResponse](#pds.EvpnEviRtRequest) |  |
| EvpnIpVrfSpecCreate | [EvpnIpVrfRequest](#pds.EvpnIpVrfRequest) | [EvpnResponse](#pds.EvpnIpVrfRequest) |  |
| EvpnIpVrfSpecUpdate | [EvpnIpVrfRequest](#pds.EvpnIpVrfRequest) | [EvpnResponse](#pds.EvpnIpVrfRequest) |  |
| EvpnIpVrfSpecDelete | [EvpnIpVrfRequest](#pds.EvpnIpVrfRequest) | [EvpnResponse](#pds.EvpnIpVrfRequest) |  |
| EvpnIpVrfSpecGet | [EvpnIpVrfRequest](#pds.EvpnIpVrfRequest) | [EvpnIpVrfGetResponse](#pds.EvpnIpVrfRequest) |  |
| EvpnIpVrfRtSpecCreate | [EvpnIpVrfRtRequest](#pds.EvpnIpVrfRtRequest) | [EvpnResponse](#pds.EvpnIpVrfRtRequest) |  |
| EvpnIpVrfRtSpecUpdate | [EvpnIpVrfRtRequest](#pds.EvpnIpVrfRtRequest) | [EvpnResponse](#pds.EvpnIpVrfRtRequest) |  |
| EvpnIpVrfRtSpecDelete | [EvpnIpVrfRtRequest](#pds.EvpnIpVrfRtRequest) | [EvpnResponse](#pds.EvpnIpVrfRtRequest) |  |
| EvpnIpVrfRtSpecGet | [EvpnIpVrfRtRequest](#pds.EvpnIpVrfRtRequest) | [EvpnIpVrfRtGetResponse](#pds.EvpnIpVrfRtRequest) |  |
| EvpnMacIpSpecGet | [EvpnMacIpSpecRequest](#pds.EvpnMacIpSpecRequest) | [EvpnMacIpGetResponse](#pds.EvpnMacIpSpecRequest) |  |

 



<a name="internal.proto"/>
<p align="right"><a href="#top">Top</a></p>

## internal.proto



<a name="pds.BgpRouteMapSpec"/>

### BgpRouteMapSpec
spec to configure ext-comm ORF route map table


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| RmIndex | [uint32](#uint32) |  |  |
| Index | [uint32](#uint32) |  |  |
| Number | [uint32](#uint32) |  |  |
| Afi | [uint32](#uint32) |  |  |
| AfiDefined | [uint32](#uint32) |  |  |
| Safi | [uint32](#uint32) |  |  |
| SafiDefined | [uint32](#uint32) |  |  |
| MatchExtComm | [bytes](#bytes) |  |  |
| OrfAssociation | [uint32](#uint32) |  |  |






<a name="pds.EvpnBdSpec"/>

### EvpnBdSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| EntityIndex | [uint32](#uint32) |  |  |
| EviIndex | [uint32](#uint32) |  |  |
| VNI | [uint32](#uint32) |  |  |






<a name="pds.EvpnIfBindCfgSpec"/>

### EvpnIfBindCfgSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| EntityIndex | [uint32](#uint32) |  |  |
| IfIndex | [uint32](#uint32) |  |  |
| EviIndex | [uint32](#uint32) |  |  |






<a name="pds.LimGenIrbIfSpec"/>

### LimGenIrbIfSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| EntityIndex | [uint32](#uint32) |  |  |
| BDType | [uint32](#uint32) |  |  |
| BDIndex | [uint32](#uint32) |  |  |






<a name="pds.LimInterfaceAddrSpec"/>

### LimInterfaceAddrSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| IfType | [LimIntfType](#pds.LimIntfType) |  | Interface Type should be converted to MS interface type/subtype, cannot be autogen. |
| IfId | [uint32](#uint32) |  | Interface ID should be converted to MS interface Index, cannot be Autogen. Filling if_index as part of FillFn |
| IPAddr | [.types.IPAddress](#pds..types.IPAddress) |  |  |
| PrefixLen | [uint32](#uint32) |  |  |






<a name="pds.LimInterfaceCfgSpec"/>

### LimInterfaceCfgSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| EntityIndex | [uint32](#uint32) |  |  |
| IfIndex | [uint32](#uint32) |  |  |
| IfEnable | [uint32](#uint32) |  |  |
| IPv4Enabled | [uint32](#uint32) |  |  |
| IPv4Fwding | [uint32](#uint32) |  |  |
| IPv6Enabled | [uint32](#uint32) |  |  |
| IPv6Fwding | [uint32](#uint32) |  |  |
| FwdingMode | [uint32](#uint32) |  |  |
| VrfName | [string](#string) |  |  |






<a name="pds.LimInterfaceSpec"/>

### LimInterfaceSpec
This spec should be used to create only LIF and Loopback interfaces.
If at all there is a need to create other types of interfaces, another spec should be defined with relevent Mib


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| IfType | [LimIntfType](#pds.LimIntfType) |  | interface Type should be converted to MS interface type/subtype, cannot be autogen |
| IfId | [uint32](#uint32) |  |  |






<a name="pds.LimVrfSpec"/>

### LimVrfSpec



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| EntityIndex | [uint32](#uint32) |  |  |
| VrfName | [string](#string) |  |  |
| VrfNameLen | [uint32](#uint32) |  |  |





 


<a name="pds.LimIntfType"/>

### LimIntfType


| Name | Number | Description |
| ---- | ------ | ----------- |
| LIM_IF_TYPE_NONE | 0 |  |
| LIM_IF_TYPE_ETH | 1 |  |
| LIM_IF_TYPE_LIF | 2 |  |
| LIM_IF_TYPE_LOOPBACK | 3 |  |
| LIM_IF_TYPE_IRB | 4 |  |


 

 

 



## Scalar Value Types

| .proto Type | Notes | C++ Type | Java Type | Python Type |
| ----------- | ----- | -------- | --------- | ----------- |
| <a name="double" /> double |  | double | double | float |
| <a name="float" /> float |  | float | float | float |
| <a name="int32" /> int32 | Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint32 instead. | int32 | int | int |
| <a name="int64" /> int64 | Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint64 instead. | int64 | long | int/long |
| <a name="uint32" /> uint32 | Uses variable-length encoding. | uint32 | int | int/long |
| <a name="uint64" /> uint64 | Uses variable-length encoding. | uint64 | long | int/long |
| <a name="sint32" /> sint32 | Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int32s. | int32 | int | int |
| <a name="sint64" /> sint64 | Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int64s. | int64 | long | int/long |
| <a name="fixed32" /> fixed32 | Always four bytes. More efficient than uint32 if values are often greater than 2^28. | uint32 | int | int |
| <a name="fixed64" /> fixed64 | Always eight bytes. More efficient than uint64 if values are often greater than 2^56. | uint64 | long | int/long |
| <a name="sfixed32" /> sfixed32 | Always four bytes. | int32 | int | int |
| <a name="sfixed64" /> sfixed64 | Always eight bytes. | int64 | long | int/long |
| <a name="bool" /> bool |  | bool | boolean | boolean |
| <a name="string" /> string | A string must always contain UTF-8 encoded or 7-bit ASCII text. | string | String | str/unicode |
| <a name="bytes" /> bytes | May contain any arbitrary sequence of bytes. | string | ByteString | str |

