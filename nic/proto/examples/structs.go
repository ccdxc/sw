// ---------------------------------- .proto ---------------------------------
//
// Key:
// 	l2_segment_handle, mac_address
// Handle field:
//		ep_handle
// Mac field:
// 		mac_address
// IPv4 field:
//		ip_address
// IPv6 field:
//		ip_address
// List field:
//		ip_address, security_group
// Enum field:
// 		ip_af
// bool field:
//		learn_source_config
// Range field:
//   	useg_vlan. The min and max tags are not present today. I just added them as an example.

// EndpointSpec is used to add or update an endpoint
// Key: l2_segment_handle, mac_address
// Handle: ep_handle
message EndpointSpec {
    ObjectMeta                  meta                 = 1;    // common object meta
    fixed64                     l2_segment_handle    = 2;    // L2 segment endpoint is in
    MacAddress                  mac_address          = 3;    // MAC address of endpoint
    fixed64                     interface_handle     = 4;    // interface endpoint is reachable on
    uint32                      useg_vlan            = 5;    // [(gogoproto.min) = 0, (gogoproto.max) = 4095];    // microseg vlan assigned to this EP
    repeated IPAddress          ip_address           = 6;    // IP address(es) of endpoint
    repeated uint32             security_group       = 7;    // security group list(s) endpoint is in
    fixed64                     ep_handle            = 8;    // endpoint's handle
    bool                        learn_source_config  = 9;    // true if learnt through configuration
}

// common meta object that is part of all other top level objects
message ObjectMeta {
  fixed32        tenant_id = 1;     // tenant identifier
}

// mac address structure
message MacAddress {
    uint64       mac = 1;
}


// IP address structure
message IPAddress {
  IPAddressFamily    ip_af = 1;        // IP address family
  oneof v4_or_v6 {
    fixed32          v4_addr = 2;      // IPv4 address
    bytes            v6_addr = 3;      // IPv6 address  (TODO: need better representation here)
  }
}

// IP address families
enum IPAddressFamily {
  IP_AF_NONE     = 0;
  IP_AF_INET     = 1;    // IPv4
  IP_AF_INET6    = 2;    // IPv6
}



// ------------------------------- Generated go code --------------------------
// EndpointSpec is used to add or update an endpoint
type EndpointSpec struct {
	Meta              *ObjectMeta  `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	L2SegmentHandle   uint64       `protobuf:"fixed64,2,opt,name=l2_segment_handle,json=l2SegmentHandle" json:"l2_segment_handle,omitempty"`
	MacAddress        *MacAddress  `protobuf:"bytes,3,opt,name=mac_address,json=macAddress" json:"mac_address,omitempty"`
	InterfaceHandle   uint64       `protobuf:"fixed64,4,opt,name=interface_handle,json=interfaceHandle" json:"interface_handle,omitempty"`
	UsegVlan          uint32       `protobuf:"varint,5,opt,name=useg_vlan,json=usegVlan" json:"useg_vlan,omitempty"`
	IpAddress         []*IPAddress `protobuf:"bytes,6,rep,name=ip_address,json=ipAddress" json:"ip_address,omitempty"`
	SecurityGroup     []uint32     `protobuf:"varint,7,rep,packed,name=security_group,json=securityGroup" json:"security_group,omitempty"`
	EpHandle          uint64       `protobuf:"fixed64,8,opt,name=ep_handle,json=epHandle" json:"ep_handle,omitempty"`
	LearnSourceConfig bool         `protobuf:"varint,9,opt,name=learn_source_config,json=learnSourceConfig" json:"learn_source_config,omitempty"`
}

// common meta object that is part of all other top level objects
type ObjectMeta struct {
    TenantId uint32 `protobuf:"fixed32,1,opt,name=tenant_id,json=tenantId" json:"tenant_id,omitempty"`
}

// mac address structure
type MacAddress struct {
	Mac uint64 `protobuf:"varint,1,opt,name=mac" json:"mac,omitempty"`
}

// IP Address Struct
type IPAddress struct {
    IpAf IPAddressFamily `protobuf:"varint,1,opt,name=ip_af,json=ipAf,enum=types.IPAddressFamily" json:"ip_af,omitempty"`
    // Types that are valid to be assigned to V4OrV6:
    //  *IPAddress_V4Addr
    //  *IPAddress_V6Addr
    V4OrV6 isIPAddress_V4OrV6 `protobuf_oneof:"v4_or_v6"`
}

// IP address families
type IPAddressFamily int32
const (
    IPAddressFamily_IP_AF_NONE  IPAddressFamily = 0
    IPAddressFamily_IP_AF_INET  IPAddressFamily = 1
    IPAddressFamily_IP_AF_INET6 IPAddressFamily = 2
)

type IPAddress_V4Addr struct {
    V4Addr uint32 `protobuf:"fixed32,2,opt,name=v4_addr,json=v4Addr,oneof"`
}
type IPAddress_V6Addr struct {
    V6Addr []byte `protobuf:"bytes,3,opt,name=v6_addr,json=v6Addr,proto3,oneof"`
}












