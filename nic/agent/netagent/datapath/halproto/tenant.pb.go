// Code generated by protoc-gen-go. DO NOT EDIT.
// source: nic/proto/hal/tenant.proto

package halproto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// TenantKeyHandle is used to operate on a tenant either by its key or handle
type TenantKeyHandle struct {
	// Types that are valid to be assigned to KeyOrHandle:
	//	*TenantKeyHandle_TenantId
	//	*TenantKeyHandle_TenantHandle
	KeyOrHandle isTenantKeyHandle_KeyOrHandle `protobuf_oneof:"key_or_handle"`
}

func (m *TenantKeyHandle) Reset()                    { *m = TenantKeyHandle{} }
func (m *TenantKeyHandle) String() string            { return proto.CompactTextString(m) }
func (*TenantKeyHandle) ProtoMessage()               {}
func (*TenantKeyHandle) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{0} }

type isTenantKeyHandle_KeyOrHandle interface {
	isTenantKeyHandle_KeyOrHandle()
}

type TenantKeyHandle_TenantId struct {
	TenantId uint32 `protobuf:"varint,1,opt,name=tenant_id,json=tenantId,oneof"`
}
type TenantKeyHandle_TenantHandle struct {
	TenantHandle uint64 `protobuf:"fixed64,2,opt,name=tenant_handle,json=tenantHandle,oneof"`
}

func (*TenantKeyHandle_TenantId) isTenantKeyHandle_KeyOrHandle()     {}
func (*TenantKeyHandle_TenantHandle) isTenantKeyHandle_KeyOrHandle() {}

func (m *TenantKeyHandle) GetKeyOrHandle() isTenantKeyHandle_KeyOrHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *TenantKeyHandle) GetTenantId() uint32 {
	if x, ok := m.GetKeyOrHandle().(*TenantKeyHandle_TenantId); ok {
		return x.TenantId
	}
	return 0
}

func (m *TenantKeyHandle) GetTenantHandle() uint64 {
	if x, ok := m.GetKeyOrHandle().(*TenantKeyHandle_TenantHandle); ok {
		return x.TenantHandle
	}
	return 0
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*TenantKeyHandle) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _TenantKeyHandle_OneofMarshaler, _TenantKeyHandle_OneofUnmarshaler, _TenantKeyHandle_OneofSizer, []interface{}{
		(*TenantKeyHandle_TenantId)(nil),
		(*TenantKeyHandle_TenantHandle)(nil),
	}
}

func _TenantKeyHandle_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*TenantKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *TenantKeyHandle_TenantId:
		b.EncodeVarint(1<<3 | proto.WireVarint)
		b.EncodeVarint(uint64(x.TenantId))
	case *TenantKeyHandle_TenantHandle:
		b.EncodeVarint(2<<3 | proto.WireFixed64)
		b.EncodeFixed64(uint64(x.TenantHandle))
	case nil:
	default:
		return fmt.Errorf("TenantKeyHandle.KeyOrHandle has unexpected type %T", x)
	}
	return nil
}

func _TenantKeyHandle_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*TenantKeyHandle)
	switch tag {
	case 1: // key_or_handle.tenant_id
		if wire != proto.WireVarint {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeVarint()
		m.KeyOrHandle = &TenantKeyHandle_TenantId{uint32(x)}
		return true, err
	case 2: // key_or_handle.tenant_handle
		if wire != proto.WireFixed64 {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeFixed64()
		m.KeyOrHandle = &TenantKeyHandle_TenantHandle{x}
		return true, err
	default:
		return false, nil
	}
}

func _TenantKeyHandle_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*TenantKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *TenantKeyHandle_TenantId:
		n += proto.SizeVarint(1<<3 | proto.WireVarint)
		n += proto.SizeVarint(uint64(x.TenantId))
	case *TenantKeyHandle_TenantHandle:
		n += proto.SizeVarint(2<<3 | proto.WireFixed64)
		n += 8
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// TenantSpec captures all the tenant level configuration
type TenantSpec struct {
	Meta                  *ObjectMeta      `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle           *TenantKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
	SecurityProfileHandle uint64           `protobuf:"fixed64,3,opt,name=security_profile_handle,json=securityProfileHandle" json:"security_profile_handle,omitempty"`
}

func (m *TenantSpec) Reset()                    { *m = TenantSpec{} }
func (m *TenantSpec) String() string            { return proto.CompactTextString(m) }
func (*TenantSpec) ProtoMessage()               {}
func (*TenantSpec) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{1} }

func (m *TenantSpec) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *TenantSpec) GetKeyOrHandle() *TenantKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *TenantSpec) GetSecurityProfileHandle() uint64 {
	if m != nil {
		return m.SecurityProfileHandle
	}
	return 0
}

// TenantRequestMsg is batched add or modify tenant request
type TenantRequestMsg struct {
	Request []*TenantSpec `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *TenantRequestMsg) Reset()                    { *m = TenantRequestMsg{} }
func (m *TenantRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*TenantRequestMsg) ProtoMessage()               {}
func (*TenantRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{2} }

func (m *TenantRequestMsg) GetRequest() []*TenantSpec {
	if m != nil {
		return m.Request
	}
	return nil
}

// TenantStatus is the operational status of a given tenant
type TenantStatus struct {
	TenantHandle uint64 `protobuf:"fixed64,1,opt,name=tenant_handle,json=tenantHandle" json:"tenant_handle,omitempty"`
}

func (m *TenantStatus) Reset()                    { *m = TenantStatus{} }
func (m *TenantStatus) String() string            { return proto.CompactTextString(m) }
func (*TenantStatus) ProtoMessage()               {}
func (*TenantStatus) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{3} }

func (m *TenantStatus) GetTenantHandle() uint64 {
	if m != nil {
		return m.TenantHandle
	}
	return 0
}

// TenantResponse is response to TenantSpec
type TenantResponse struct {
	ApiStatus    ApiStatus     `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	TenantStatus *TenantStatus `protobuf:"bytes,2,opt,name=tenant_status,json=tenantStatus" json:"tenant_status,omitempty"`
}

func (m *TenantResponse) Reset()                    { *m = TenantResponse{} }
func (m *TenantResponse) String() string            { return proto.CompactTextString(m) }
func (*TenantResponse) ProtoMessage()               {}
func (*TenantResponse) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{4} }

func (m *TenantResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *TenantResponse) GetTenantStatus() *TenantStatus {
	if m != nil {
		return m.TenantStatus
	}
	return nil
}

// TenantResponseMsg is batched response to TenantRequestMsg
type TenantResponseMsg struct {
	Response []*TenantResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *TenantResponseMsg) Reset()                    { *m = TenantResponseMsg{} }
func (m *TenantResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*TenantResponseMsg) ProtoMessage()               {}
func (*TenantResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{5} }

func (m *TenantResponseMsg) GetResponse() []*TenantResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

// TenantDeleteRequest is used to delete a tenant
type TenantDeleteRequest struct {
	Meta        *ObjectMeta      `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *TenantKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *TenantDeleteRequest) Reset()                    { *m = TenantDeleteRequest{} }
func (m *TenantDeleteRequest) String() string            { return proto.CompactTextString(m) }
func (*TenantDeleteRequest) ProtoMessage()               {}
func (*TenantDeleteRequest) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{6} }

func (m *TenantDeleteRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *TenantDeleteRequest) GetKeyOrHandle() *TenantKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// TenantDeleteRequestMsg is used to delete a batch of tenants
type TenantDeleteRequestMsg struct {
	Request []*TenantDeleteRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *TenantDeleteRequestMsg) Reset()                    { *m = TenantDeleteRequestMsg{} }
func (m *TenantDeleteRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*TenantDeleteRequestMsg) ProtoMessage()               {}
func (*TenantDeleteRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{7} }

func (m *TenantDeleteRequestMsg) GetRequest() []*TenantDeleteRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// TenantDeleteResponseMsg is batched response to TenantDeleteRequestMsg
type TenantDeleteResponseMsg struct {
	ApiStatus []ApiStatus `protobuf:"varint,1,rep,packed,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
}

func (m *TenantDeleteResponseMsg) Reset()                    { *m = TenantDeleteResponseMsg{} }
func (m *TenantDeleteResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*TenantDeleteResponseMsg) ProtoMessage()               {}
func (*TenantDeleteResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{8} }

func (m *TenantDeleteResponseMsg) GetApiStatus() []ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return nil
}

// TenantGetRequest is used to get information about a tenant
type TenantGetRequest struct {
	Meta        *ObjectMeta      `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *TenantKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *TenantGetRequest) Reset()                    { *m = TenantGetRequest{} }
func (m *TenantGetRequest) String() string            { return proto.CompactTextString(m) }
func (*TenantGetRequest) ProtoMessage()               {}
func (*TenantGetRequest) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{9} }

func (m *TenantGetRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *TenantGetRequest) GetKeyOrHandle() *TenantKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// TenantGetRequestMsg is batched GET requests for tenants
type TenantGetRequestMsg struct {
	Request []*TenantGetRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *TenantGetRequestMsg) Reset()                    { *m = TenantGetRequestMsg{} }
func (m *TenantGetRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*TenantGetRequestMsg) ProtoMessage()               {}
func (*TenantGetRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{10} }

func (m *TenantGetRequestMsg) GetRequest() []*TenantGetRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// TenantStats is the statistics object for each tenant
type TenantStats struct {
	NumL2Segments     uint32 `protobuf:"varint,1,opt,name=num_l2_segments,json=numL2Segments" json:"num_l2_segments,omitempty"`
	NumSecurityGroups uint32 `protobuf:"varint,2,opt,name=num_security_groups,json=numSecurityGroups" json:"num_security_groups,omitempty"`
	NumL4LbServices   uint32 `protobuf:"varint,3,opt,name=num_l4lb_services,json=numL4lbServices" json:"num_l4lb_services,omitempty"`
	NumEndpoints      uint32 `protobuf:"varint,4,opt,name=num_endpoints,json=numEndpoints" json:"num_endpoints,omitempty"`
}

func (m *TenantStats) Reset()                    { *m = TenantStats{} }
func (m *TenantStats) String() string            { return proto.CompactTextString(m) }
func (*TenantStats) ProtoMessage()               {}
func (*TenantStats) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{11} }

func (m *TenantStats) GetNumL2Segments() uint32 {
	if m != nil {
		return m.NumL2Segments
	}
	return 0
}

func (m *TenantStats) GetNumSecurityGroups() uint32 {
	if m != nil {
		return m.NumSecurityGroups
	}
	return 0
}

func (m *TenantStats) GetNumL4LbServices() uint32 {
	if m != nil {
		return m.NumL4LbServices
	}
	return 0
}

func (m *TenantStats) GetNumEndpoints() uint32 {
	if m != nil {
		return m.NumEndpoints
	}
	return 0
}

// TenantGetResponse captures all the information about a tenant
// only if api_status indicates success, other fields are valid
type TenantGetResponse struct {
	ApiStatus ApiStatus     `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	Spec      *TenantSpec   `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	Status    *TenantStatus `protobuf:"bytes,3,opt,name=status" json:"status,omitempty"`
	Stats     *TenantStats  `protobuf:"bytes,4,opt,name=stats" json:"stats,omitempty"`
}

func (m *TenantGetResponse) Reset()                    { *m = TenantGetResponse{} }
func (m *TenantGetResponse) String() string            { return proto.CompactTextString(m) }
func (*TenantGetResponse) ProtoMessage()               {}
func (*TenantGetResponse) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{12} }

func (m *TenantGetResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *TenantGetResponse) GetSpec() *TenantSpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *TenantGetResponse) GetStatus() *TenantStatus {
	if m != nil {
		return m.Status
	}
	return nil
}

func (m *TenantGetResponse) GetStats() *TenantStats {
	if m != nil {
		return m.Stats
	}
	return nil
}

// TenantGetResponseMsg is batched response to TenantGetRequestMsg
type TenantGetResponseMsg struct {
	Response []*TenantGetResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *TenantGetResponseMsg) Reset()                    { *m = TenantGetResponseMsg{} }
func (m *TenantGetResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*TenantGetResponseMsg) ProtoMessage()               {}
func (*TenantGetResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor21, []int{13} }

func (m *TenantGetResponseMsg) GetResponse() []*TenantGetResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

func init() {
	proto.RegisterType((*TenantKeyHandle)(nil), "tenant.TenantKeyHandle")
	proto.RegisterType((*TenantSpec)(nil), "tenant.TenantSpec")
	proto.RegisterType((*TenantRequestMsg)(nil), "tenant.TenantRequestMsg")
	proto.RegisterType((*TenantStatus)(nil), "tenant.TenantStatus")
	proto.RegisterType((*TenantResponse)(nil), "tenant.TenantResponse")
	proto.RegisterType((*TenantResponseMsg)(nil), "tenant.TenantResponseMsg")
	proto.RegisterType((*TenantDeleteRequest)(nil), "tenant.TenantDeleteRequest")
	proto.RegisterType((*TenantDeleteRequestMsg)(nil), "tenant.TenantDeleteRequestMsg")
	proto.RegisterType((*TenantDeleteResponseMsg)(nil), "tenant.TenantDeleteResponseMsg")
	proto.RegisterType((*TenantGetRequest)(nil), "tenant.TenantGetRequest")
	proto.RegisterType((*TenantGetRequestMsg)(nil), "tenant.TenantGetRequestMsg")
	proto.RegisterType((*TenantStats)(nil), "tenant.TenantStats")
	proto.RegisterType((*TenantGetResponse)(nil), "tenant.TenantGetResponse")
	proto.RegisterType((*TenantGetResponseMsg)(nil), "tenant.TenantGetResponseMsg")
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for Tenant service

type TenantClient interface {
	TenantCreate(ctx context.Context, in *TenantRequestMsg, opts ...grpc.CallOption) (*TenantResponseMsg, error)
	TenantUpdate(ctx context.Context, in *TenantRequestMsg, opts ...grpc.CallOption) (*TenantResponseMsg, error)
	TenantDelete(ctx context.Context, in *TenantDeleteRequestMsg, opts ...grpc.CallOption) (*TenantDeleteResponseMsg, error)
	TenantGet(ctx context.Context, in *TenantGetRequestMsg, opts ...grpc.CallOption) (*TenantGetResponseMsg, error)
}

type tenantClient struct {
	cc *grpc.ClientConn
}

func NewTenantClient(cc *grpc.ClientConn) TenantClient {
	return &tenantClient{cc}
}

func (c *tenantClient) TenantCreate(ctx context.Context, in *TenantRequestMsg, opts ...grpc.CallOption) (*TenantResponseMsg, error) {
	out := new(TenantResponseMsg)
	err := grpc.Invoke(ctx, "/tenant.Tenant/TenantCreate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *tenantClient) TenantUpdate(ctx context.Context, in *TenantRequestMsg, opts ...grpc.CallOption) (*TenantResponseMsg, error) {
	out := new(TenantResponseMsg)
	err := grpc.Invoke(ctx, "/tenant.Tenant/TenantUpdate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *tenantClient) TenantDelete(ctx context.Context, in *TenantDeleteRequestMsg, opts ...grpc.CallOption) (*TenantDeleteResponseMsg, error) {
	out := new(TenantDeleteResponseMsg)
	err := grpc.Invoke(ctx, "/tenant.Tenant/TenantDelete", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *tenantClient) TenantGet(ctx context.Context, in *TenantGetRequestMsg, opts ...grpc.CallOption) (*TenantGetResponseMsg, error) {
	out := new(TenantGetResponseMsg)
	err := grpc.Invoke(ctx, "/tenant.Tenant/TenantGet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for Tenant service

type TenantServer interface {
	TenantCreate(context.Context, *TenantRequestMsg) (*TenantResponseMsg, error)
	TenantUpdate(context.Context, *TenantRequestMsg) (*TenantResponseMsg, error)
	TenantDelete(context.Context, *TenantDeleteRequestMsg) (*TenantDeleteResponseMsg, error)
	TenantGet(context.Context, *TenantGetRequestMsg) (*TenantGetResponseMsg, error)
}

func RegisterTenantServer(s *grpc.Server, srv TenantServer) {
	s.RegisterService(&_Tenant_serviceDesc, srv)
}

func _Tenant_TenantCreate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(TenantRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(TenantServer).TenantCreate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/tenant.Tenant/TenantCreate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(TenantServer).TenantCreate(ctx, req.(*TenantRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Tenant_TenantUpdate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(TenantRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(TenantServer).TenantUpdate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/tenant.Tenant/TenantUpdate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(TenantServer).TenantUpdate(ctx, req.(*TenantRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Tenant_TenantDelete_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(TenantDeleteRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(TenantServer).TenantDelete(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/tenant.Tenant/TenantDelete",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(TenantServer).TenantDelete(ctx, req.(*TenantDeleteRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Tenant_TenantGet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(TenantGetRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(TenantServer).TenantGet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/tenant.Tenant/TenantGet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(TenantServer).TenantGet(ctx, req.(*TenantGetRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

var _Tenant_serviceDesc = grpc.ServiceDesc{
	ServiceName: "tenant.Tenant",
	HandlerType: (*TenantServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "TenantCreate",
			Handler:    _Tenant_TenantCreate_Handler,
		},
		{
			MethodName: "TenantUpdate",
			Handler:    _Tenant_TenantUpdate_Handler,
		},
		{
			MethodName: "TenantDelete",
			Handler:    _Tenant_TenantDelete_Handler,
		},
		{
			MethodName: "TenantGet",
			Handler:    _Tenant_TenantGet_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "nic/proto/hal/tenant.proto",
}

func init() { proto.RegisterFile("nic/proto/hal/tenant.proto", fileDescriptor21) }

var fileDescriptor21 = []byte{
	// 690 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xbc, 0x55, 0x5d, 0x4f, 0x13, 0x5d,
	0x10, 0x66, 0x29, 0x6f, 0x5f, 0x98, 0xb2, 0x40, 0x0f, 0x5f, 0x05, 0xfc, 0x20, 0x6b, 0x20, 0x68,
	0x48, 0x9b, 0x2c, 0x68, 0x62, 0xbc, 0x51, 0x94, 0x50, 0x94, 0x06, 0x3d, 0xd5, 0x1b, 0x6f, 0x36,
	0xdb, 0x76, 0x6c, 0x57, 0xb6, 0xbb, 0xeb, 0x9e, 0xb3, 0x24, 0x4d, 0xfc, 0x41, 0xde, 0x9b, 0xf8,
	0x0f, 0xfc, 0x5f, 0x66, 0xcf, 0x47, 0xd7, 0x5d, 0x0a, 0x26, 0x9a, 0x70, 0xd5, 0x3d, 0x33, 0xcf,
	0x79, 0x66, 0xe6, 0x99, 0x99, 0x53, 0xd8, 0x0c, 0xbc, 0x6e, 0x23, 0x8a, 0x43, 0x1e, 0x36, 0x06,
	0xae, 0xdf, 0xe0, 0x18, 0xb8, 0x01, 0xaf, 0x0b, 0x03, 0x29, 0xcb, 0xd3, 0xe6, 0x6a, 0x86, 0xe1,
	0xa3, 0x08, 0x99, 0x74, 0x5b, 0x03, 0x58, 0x7c, 0x2f, 0x00, 0x6f, 0x70, 0xd4, 0x74, 0x83, 0x9e,
	0x8f, 0xe4, 0x2e, 0xcc, 0xc9, 0x3b, 0x8e, 0xd7, 0xab, 0x19, 0xdb, 0xc6, 0x9e, 0xd9, 0x9c, 0xa2,
	0xb3, 0xd2, 0x74, 0xda, 0x23, 0x3b, 0x60, 0x2a, 0xf7, 0x40, 0xe0, 0x6b, 0xd3, 0xdb, 0xc6, 0x5e,
	0xb9, 0x39, 0x45, 0xe7, 0xa5, 0x59, 0xb2, 0x1c, 0x2d, 0x82, 0x79, 0x81, 0x23, 0x27, 0x8c, 0x15,
	0xcc, 0xfa, 0x66, 0x00, 0xc8, 0x50, 0xed, 0x08, 0xbb, 0x64, 0x07, 0x66, 0x86, 0xc8, 0x5d, 0x11,
	0xa0, 0x62, 0x57, 0xeb, 0x32, 0xa9, 0xf3, 0xce, 0x67, 0xec, 0xf2, 0x16, 0x72, 0x97, 0x0a, 0x37,
	0x79, 0x56, 0xa0, 0x11, 0xd1, 0x2a, 0xf6, 0x7a, 0x5d, 0x15, 0x59, 0x48, 0x9e, 0x56, 0x2e, 0x70,
	0x74, 0x1e, 0xab, 0x4a, 0x9e, 0xc0, 0x3a, 0xc3, 0x6e, 0x12, 0x7b, 0x7c, 0xe4, 0x44, 0x71, 0xf8,
	0xc9, 0xf3, 0x51, 0xd3, 0x94, 0xd2, 0xa4, 0xe9, 0xaa, 0x76, 0xbf, 0x95, 0x5e, 0x79, 0xcf, 0x7a,
	0x0e, 0x4b, 0x92, 0x97, 0xe2, 0x97, 0x04, 0x19, 0x6f, 0xb1, 0x3e, 0xd9, 0x87, 0xff, 0x63, 0x79,
	0xaa, 0x19, 0xdb, 0xa5, 0xbd, 0x8a, 0x4d, 0xf2, 0x29, 0xa4, 0x45, 0x51, 0x0d, 0xb1, 0x0e, 0x60,
	0x5e, 0x99, 0xb9, 0xcb, 0x13, 0x46, 0x1e, 0x14, 0x45, 0x33, 0x44, 0xfc, 0x9c, 0x64, 0xd6, 0x57,
	0x58, 0xd0, 0x61, 0x59, 0x14, 0x06, 0x0c, 0x49, 0x03, 0xc0, 0x8d, 0x3c, 0x87, 0x09, 0x12, 0x71,
	0x67, 0xc1, 0x5e, 0x52, 0x52, 0xbd, 0x88, 0x3c, 0x49, 0x4e, 0xe7, 0x5c, 0xfd, 0x49, 0x9e, 0x8e,
	0xe3, 0xa8, 0x3b, 0x52, 0xae, 0x95, 0x42, 0xae, 0xf2, 0x9e, 0x8a, 0x2e, 0x4f, 0xd6, 0x09, 0x54,
	0xf3, 0xd1, 0xd3, 0xaa, 0x6d, 0x98, 0x8d, 0xd5, 0x51, 0x95, 0xbd, 0x96, 0xa7, 0xd2, 0x60, 0x3a,
	0xc6, 0x59, 0x23, 0x58, 0x96, 0xbe, 0x57, 0xe8, 0x23, 0x47, 0xa5, 0xe1, 0x6d, 0x34, 0xdc, 0x3a,
	0x87, 0xb5, 0x09, 0xa1, 0xd3, 0x42, 0x1e, 0x17, 0xdb, 0xb7, 0x95, 0x27, 0xcc, 0x5d, 0xc8, 0xfa,
	0xf8, 0x1a, 0xd6, 0xf3, 0xfe, 0x4c, 0x9a, 0x62, 0x6f, 0x4a, 0x7f, 0xe8, 0x8d, 0x75, 0xa9, 0xa7,
	0xea, 0x04, 0xf9, 0x6d, 0x8a, 0x72, 0xaa, 0xfb, 0x91, 0xc5, 0x95, 0xad, 0x2d, 0x28, 0x52, 0xcb,
	0xb3, 0x65, 0xe8, 0x4c, 0x8e, 0x1f, 0x06, 0x54, 0xb2, 0x11, 0x62, 0x64, 0x17, 0x16, 0x83, 0x64,
	0xe8, 0xf8, 0xb6, 0xc3, 0xb0, 0x3f, 0xc4, 0x80, 0xcb, 0x21, 0x35, 0xa9, 0x19, 0x24, 0xc3, 0x33,
	0xbb, 0xad, 0x8c, 0xa4, 0x0e, 0xcb, 0x29, 0x6e, 0xbc, 0x8c, 0xfd, 0x38, 0x4c, 0x22, 0x39, 0x9c,
	0x26, 0xad, 0x06, 0xc9, 0xb0, 0xad, 0x3c, 0x27, 0xc2, 0x41, 0x1e, 0x41, 0x55, 0xf0, 0x1e, 0xfa,
	0x1d, 0x87, 0x61, 0x7c, 0xe9, 0x75, 0x91, 0x89, 0x95, 0x35, 0x69, 0x1a, 0xf0, 0xec, 0xd0, 0xef,
	0xb4, 0x95, 0x39, 0x5d, 0xad, 0x14, 0x8b, 0x41, 0x2f, 0x0a, 0xbd, 0x34, 0x83, 0x19, 0x81, 0x9b,
	0x0f, 0x92, 0xe1, 0xb1, 0xb6, 0x59, 0x3f, 0x0d, 0x3d, 0xdd, 0xa2, 0xac, 0xbf, 0x5d, 0xaf, 0x5d,
	0x98, 0x61, 0x11, 0x76, 0x95, 0xfc, 0x93, 0x5e, 0x00, 0xe1, 0x27, 0xfb, 0x50, 0x56, 0xa4, 0xa5,
	0x1b, 0xf6, 0x4f, 0x61, 0xc8, 0x43, 0xf8, 0x2f, 0xfd, 0x92, 0x99, 0x57, 0xec, 0xe5, 0xab, 0x60,
	0x46, 0x25, 0xc2, 0x6a, 0xc1, 0xca, 0x95, 0x32, 0xe4, 0x78, 0x17, 0xf7, 0x74, 0x63, 0x42, 0x37,
	0x8b, 0xab, 0x6a, 0x7f, 0x9f, 0x86, 0xb2, 0xf4, 0x93, 0x63, 0xfd, 0x62, 0xbd, 0x8c, 0xd1, 0xe5,
	0x48, 0x6a, 0xc5, 0x3d, 0xd7, 0x83, 0xb3, 0xb9, 0x31, 0xf9, 0x05, 0x68, 0xb1, 0xbe, 0x35, 0x95,
	0xd1, 0x7c, 0x88, 0x7a, 0xff, 0x40, 0xf3, 0x4e, 0xd3, 0xc8, 0xbd, 0x23, 0xf7, 0x6e, 0xd8, 0xd6,
	0x94, 0xec, 0xfe, 0x64, 0xff, 0xef, 0x94, 0x4d, 0x98, 0x1b, 0x4b, 0x41, 0xb6, 0xae, 0x9b, 0xf5,
	0x94, 0xec, 0xce, 0xb5, 0xd2, 0x09, 0xa6, 0x23, 0xf8, 0x38, 0x3b, 0x70, 0x7d, 0xf1, 0xff, 0xd9,
	0x29, 0x8b, 0x9f, 0x83, 0x5f, 0x01, 0x00, 0x00, 0xff, 0xff, 0xe7, 0xd3, 0xd9, 0x9e, 0x83, 0x07,
	0x00, 0x00,
}
