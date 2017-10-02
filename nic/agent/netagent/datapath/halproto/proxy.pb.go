// Code generated by protoc-gen-go. DO NOT EDIT.
// source: nic/proto/hal/proxy.proto

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

// ProxyKeyHandle is used to operate on a proxy either by its key or handle
type ProxyKeyHandle struct {
	// Types that are valid to be assigned to KeyOrHandle:
	//	*ProxyKeyHandle_ProxyId
	//	*ProxyKeyHandle_ProxyHandle
	KeyOrHandle isProxyKeyHandle_KeyOrHandle `protobuf_oneof:"key_or_handle"`
}

func (m *ProxyKeyHandle) Reset()                    { *m = ProxyKeyHandle{} }
func (m *ProxyKeyHandle) String() string            { return proto.CompactTextString(m) }
func (*ProxyKeyHandle) ProtoMessage()               {}
func (*ProxyKeyHandle) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{0} }

type isProxyKeyHandle_KeyOrHandle interface {
	isProxyKeyHandle_KeyOrHandle()
}

type ProxyKeyHandle_ProxyId struct {
	ProxyId uint32 `protobuf:"varint,1,opt,name=proxy_id,json=proxyId,oneof"`
}
type ProxyKeyHandle_ProxyHandle struct {
	ProxyHandle uint64 `protobuf:"fixed64,2,opt,name=proxy_handle,json=proxyHandle,oneof"`
}

func (*ProxyKeyHandle_ProxyId) isProxyKeyHandle_KeyOrHandle()     {}
func (*ProxyKeyHandle_ProxyHandle) isProxyKeyHandle_KeyOrHandle() {}

func (m *ProxyKeyHandle) GetKeyOrHandle() isProxyKeyHandle_KeyOrHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *ProxyKeyHandle) GetProxyId() uint32 {
	if x, ok := m.GetKeyOrHandle().(*ProxyKeyHandle_ProxyId); ok {
		return x.ProxyId
	}
	return 0
}

func (m *ProxyKeyHandle) GetProxyHandle() uint64 {
	if x, ok := m.GetKeyOrHandle().(*ProxyKeyHandle_ProxyHandle); ok {
		return x.ProxyHandle
	}
	return 0
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*ProxyKeyHandle) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _ProxyKeyHandle_OneofMarshaler, _ProxyKeyHandle_OneofUnmarshaler, _ProxyKeyHandle_OneofSizer, []interface{}{
		(*ProxyKeyHandle_ProxyId)(nil),
		(*ProxyKeyHandle_ProxyHandle)(nil),
	}
}

func _ProxyKeyHandle_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*ProxyKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *ProxyKeyHandle_ProxyId:
		b.EncodeVarint(1<<3 | proto.WireVarint)
		b.EncodeVarint(uint64(x.ProxyId))
	case *ProxyKeyHandle_ProxyHandle:
		b.EncodeVarint(2<<3 | proto.WireFixed64)
		b.EncodeFixed64(uint64(x.ProxyHandle))
	case nil:
	default:
		return fmt.Errorf("ProxyKeyHandle.KeyOrHandle has unexpected type %T", x)
	}
	return nil
}

func _ProxyKeyHandle_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*ProxyKeyHandle)
	switch tag {
	case 1: // key_or_handle.proxy_id
		if wire != proto.WireVarint {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeVarint()
		m.KeyOrHandle = &ProxyKeyHandle_ProxyId{uint32(x)}
		return true, err
	case 2: // key_or_handle.proxy_handle
		if wire != proto.WireFixed64 {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeFixed64()
		m.KeyOrHandle = &ProxyKeyHandle_ProxyHandle{x}
		return true, err
	default:
		return false, nil
	}
}

func _ProxyKeyHandle_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*ProxyKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *ProxyKeyHandle_ProxyId:
		n += proto.SizeVarint(1<<3 | proto.WireVarint)
		n += proto.SizeVarint(uint64(x.ProxyId))
	case *ProxyKeyHandle_ProxyHandle:
		n += proto.SizeVarint(2<<3 | proto.WireFixed64)
		n += 8
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// ProxySpec captures all the proxy level configuration
type ProxySpec struct {
	Meta        *ObjectMeta     `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *ProxyKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
	ProxyType   ProxyType       `protobuf:"varint,3,opt,name=proxy_type,json=proxyType,enum=types.ProxyType" json:"proxy_type,omitempty"`
	ProxyQtype  uint32          `protobuf:"varint,4,opt,name=proxy_qtype,json=proxyQtype" json:"proxy_qtype,omitempty"`
}

func (m *ProxySpec) Reset()                    { *m = ProxySpec{} }
func (m *ProxySpec) String() string            { return proto.CompactTextString(m) }
func (*ProxySpec) ProtoMessage()               {}
func (*ProxySpec) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{1} }

func (m *ProxySpec) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *ProxySpec) GetKeyOrHandle() *ProxyKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *ProxySpec) GetProxyType() ProxyType {
	if m != nil {
		return m.ProxyType
	}
	return ProxyType_PROXY_TYPE_NONE
}

func (m *ProxySpec) GetProxyQtype() uint32 {
	if m != nil {
		return m.ProxyQtype
	}
	return 0
}

// ProxyRequestMsg is batched add or modify proxy request
type ProxyRequestMsg struct {
	Request []*ProxySpec `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *ProxyRequestMsg) Reset()                    { *m = ProxyRequestMsg{} }
func (m *ProxyRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyRequestMsg) ProtoMessage()               {}
func (*ProxyRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{2} }

func (m *ProxyRequestMsg) GetRequest() []*ProxySpec {
	if m != nil {
		return m.Request
	}
	return nil
}

// ProxyStatus is the operational status of a given proxy
type ProxyStatus struct {
	ProxyHandle uint64 `protobuf:"fixed64,1,opt,name=proxy_handle,json=proxyHandle" json:"proxy_handle,omitempty"`
}

func (m *ProxyStatus) Reset()                    { *m = ProxyStatus{} }
func (m *ProxyStatus) String() string            { return proto.CompactTextString(m) }
func (*ProxyStatus) ProtoMessage()               {}
func (*ProxyStatus) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{3} }

func (m *ProxyStatus) GetProxyHandle() uint64 {
	if m != nil {
		return m.ProxyHandle
	}
	return 0
}

// ProxyResponse is response to ProxySpec
type ProxyResponse struct {
	ApiStatus   ApiStatus    `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	ProxyStatus *ProxyStatus `protobuf:"bytes,2,opt,name=proxy_status,json=proxyStatus" json:"proxy_status,omitempty"`
}

func (m *ProxyResponse) Reset()                    { *m = ProxyResponse{} }
func (m *ProxyResponse) String() string            { return proto.CompactTextString(m) }
func (*ProxyResponse) ProtoMessage()               {}
func (*ProxyResponse) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{4} }

func (m *ProxyResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *ProxyResponse) GetProxyStatus() *ProxyStatus {
	if m != nil {
		return m.ProxyStatus
	}
	return nil
}

// ProxyResponseMsg is batched response to ProxyRequestMsg
type ProxyResponseMsg struct {
	Response []*ProxyResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *ProxyResponseMsg) Reset()                    { *m = ProxyResponseMsg{} }
func (m *ProxyResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyResponseMsg) ProtoMessage()               {}
func (*ProxyResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{5} }

func (m *ProxyResponseMsg) GetResponse() []*ProxyResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

// ProxyDisableRequest is used to delete a proxy
type ProxyDisableRequest struct {
	Meta        *ObjectMeta     `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *ProxyKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *ProxyDisableRequest) Reset()                    { *m = ProxyDisableRequest{} }
func (m *ProxyDisableRequest) String() string            { return proto.CompactTextString(m) }
func (*ProxyDisableRequest) ProtoMessage()               {}
func (*ProxyDisableRequest) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{6} }

func (m *ProxyDisableRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *ProxyDisableRequest) GetKeyOrHandle() *ProxyKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// ProxyDisableRequestMsg is used to delete a batch of proxy
type ProxyDisableRequestMsg struct {
	Request []*ProxyDisableRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *ProxyDisableRequestMsg) Reset()                    { *m = ProxyDisableRequestMsg{} }
func (m *ProxyDisableRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyDisableRequestMsg) ProtoMessage()               {}
func (*ProxyDisableRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{7} }

func (m *ProxyDisableRequestMsg) GetRequest() []*ProxyDisableRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// ProxyDisableResponseMsg is batched response to ProxyDisableRequestMsg
type ProxyDisableResponseMsg struct {
	ApiStatus []ApiStatus `protobuf:"varint,1,rep,packed,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
}

func (m *ProxyDisableResponseMsg) Reset()                    { *m = ProxyDisableResponseMsg{} }
func (m *ProxyDisableResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyDisableResponseMsg) ProtoMessage()               {}
func (*ProxyDisableResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{8} }

func (m *ProxyDisableResponseMsg) GetApiStatus() []ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return nil
}

// ProxyGetRequest is used to get information about a proxy
type ProxyGetRequest struct {
	Meta        *ObjectMeta     `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *ProxyKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *ProxyGetRequest) Reset()                    { *m = ProxyGetRequest{} }
func (m *ProxyGetRequest) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetRequest) ProtoMessage()               {}
func (*ProxyGetRequest) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{9} }

func (m *ProxyGetRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *ProxyGetRequest) GetKeyOrHandle() *ProxyKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// ProxyGetRequestMsg is batched GET requests for proxys
type ProxyGetRequestMsg struct {
	Request []*ProxyGetRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *ProxyGetRequestMsg) Reset()                    { *m = ProxyGetRequestMsg{} }
func (m *ProxyGetRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetRequestMsg) ProtoMessage()               {}
func (*ProxyGetRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{10} }

func (m *ProxyGetRequestMsg) GetRequest() []*ProxyGetRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// ProxyStats is the statistics object for each proxy
type ProxyStats struct {
}

func (m *ProxyStats) Reset()                    { *m = ProxyStats{} }
func (m *ProxyStats) String() string            { return proto.CompactTextString(m) }
func (*ProxyStats) ProtoMessage()               {}
func (*ProxyStats) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{11} }

// ProxyGetResponse captures all the information about a proxy
// only if api_status indicates success, other fields are valid
type ProxyGetResponse struct {
	ApiStatus ApiStatus    `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	Spec      *ProxySpec   `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	Status    *ProxyStatus `protobuf:"bytes,3,opt,name=status" json:"status,omitempty"`
	Stats     *ProxyStats  `protobuf:"bytes,4,opt,name=stats" json:"stats,omitempty"`
}

func (m *ProxyGetResponse) Reset()                    { *m = ProxyGetResponse{} }
func (m *ProxyGetResponse) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetResponse) ProtoMessage()               {}
func (*ProxyGetResponse) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{12} }

func (m *ProxyGetResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *ProxyGetResponse) GetSpec() *ProxySpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *ProxyGetResponse) GetStatus() *ProxyStatus {
	if m != nil {
		return m.Status
	}
	return nil
}

func (m *ProxyGetResponse) GetStats() *ProxyStats {
	if m != nil {
		return m.Stats
	}
	return nil
}

// ProxyGetResponseMsg is batched response to ProxyGetRequestMsg
type ProxyGetResponseMsg struct {
	Response []*ProxyGetResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *ProxyGetResponseMsg) Reset()                    { *m = ProxyGetResponseMsg{} }
func (m *ProxyGetResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetResponseMsg) ProtoMessage()               {}
func (*ProxyGetResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{13} }

func (m *ProxyGetResponseMsg) GetResponse() []*ProxyGetResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

type ProxyFlowConfigRequest struct {
	Meta     *ObjectMeta `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	Spec     *ProxySpec  `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	FlowKey  *FlowKey    `protobuf:"bytes,3,opt,name=flow_key,json=flowKey" json:"flow_key,omitempty"`
	ProxyEn  bool        `protobuf:"varint,4,opt,name=proxy_en,json=proxyEn" json:"proxy_en,omitempty"`
	FlowData *FlowData   `protobuf:"bytes,5,opt,name=flow_data,json=flowData" json:"flow_data,omitempty"`
}

func (m *ProxyFlowConfigRequest) Reset()                    { *m = ProxyFlowConfigRequest{} }
func (m *ProxyFlowConfigRequest) String() string            { return proto.CompactTextString(m) }
func (*ProxyFlowConfigRequest) ProtoMessage()               {}
func (*ProxyFlowConfigRequest) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{14} }

func (m *ProxyFlowConfigRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *ProxyFlowConfigRequest) GetSpec() *ProxySpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *ProxyFlowConfigRequest) GetFlowKey() *FlowKey {
	if m != nil {
		return m.FlowKey
	}
	return nil
}

func (m *ProxyFlowConfigRequest) GetProxyEn() bool {
	if m != nil {
		return m.ProxyEn
	}
	return false
}

func (m *ProxyFlowConfigRequest) GetFlowData() *FlowData {
	if m != nil {
		return m.FlowData
	}
	return nil
}

type ProxyFlowConfigRequestMsg struct {
	Request []*ProxyFlowConfigRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *ProxyFlowConfigRequestMsg) Reset()                    { *m = ProxyFlowConfigRequestMsg{} }
func (m *ProxyFlowConfigRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyFlowConfigRequestMsg) ProtoMessage()               {}
func (*ProxyFlowConfigRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{15} }

func (m *ProxyFlowConfigRequestMsg) GetRequest() []*ProxyFlowConfigRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

type ProxyGetFlowInfoRequest struct {
	Meta     *ObjectMeta `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	Spec     *ProxySpec  `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	FlowKey  *FlowKey    `protobuf:"bytes,3,opt,name=flow_key,json=flowKey" json:"flow_key,omitempty"`
	FlowData *FlowData   `protobuf:"bytes,5,opt,name=flow_data,json=flowData" json:"flow_data,omitempty"`
}

func (m *ProxyGetFlowInfoRequest) Reset()                    { *m = ProxyGetFlowInfoRequest{} }
func (m *ProxyGetFlowInfoRequest) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetFlowInfoRequest) ProtoMessage()               {}
func (*ProxyGetFlowInfoRequest) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{16} }

func (m *ProxyGetFlowInfoRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *ProxyGetFlowInfoRequest) GetSpec() *ProxySpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *ProxyGetFlowInfoRequest) GetFlowKey() *FlowKey {
	if m != nil {
		return m.FlowKey
	}
	return nil
}

func (m *ProxyGetFlowInfoRequest) GetFlowData() *FlowData {
	if m != nil {
		return m.FlowData
	}
	return nil
}

type ProxyGetFlowInfoRequestMsg struct {
	Request []*ProxyGetFlowInfoRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *ProxyGetFlowInfoRequestMsg) Reset()                    { *m = ProxyGetFlowInfoRequestMsg{} }
func (m *ProxyGetFlowInfoRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetFlowInfoRequestMsg) ProtoMessage()               {}
func (*ProxyGetFlowInfoRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{17} }

func (m *ProxyGetFlowInfoRequestMsg) GetRequest() []*ProxyGetFlowInfoRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

type ProxyGetFlowInfoResponse struct {
	ApiStatus ApiStatus   `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	Meta      *ObjectMeta `protobuf:"bytes,2,opt,name=meta" json:"meta,omitempty"`
	ProxyType ProxyType   `protobuf:"varint,3,opt,name=proxy_type,json=proxyType,enum=types.ProxyType" json:"proxy_type,omitempty"`
	FlowKey   *FlowKey    `protobuf:"bytes,4,opt,name=flow_key,json=flowKey" json:"flow_key,omitempty"`
	LifId     uint32      `protobuf:"varint,5,opt,name=lif_id,json=lifId" json:"lif_id,omitempty"`
	Qtype     uint32      `protobuf:"varint,6,opt,name=qtype" json:"qtype,omitempty"`
	Qid1      uint32      `protobuf:"varint,7,opt,name=qid1" json:"qid1,omitempty"`
	Qid2      uint32      `protobuf:"varint,8,opt,name=qid2" json:"qid2,omitempty"`
}

func (m *ProxyGetFlowInfoResponse) Reset()                    { *m = ProxyGetFlowInfoResponse{} }
func (m *ProxyGetFlowInfoResponse) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetFlowInfoResponse) ProtoMessage()               {}
func (*ProxyGetFlowInfoResponse) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{18} }

func (m *ProxyGetFlowInfoResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *ProxyGetFlowInfoResponse) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *ProxyGetFlowInfoResponse) GetProxyType() ProxyType {
	if m != nil {
		return m.ProxyType
	}
	return ProxyType_PROXY_TYPE_NONE
}

func (m *ProxyGetFlowInfoResponse) GetFlowKey() *FlowKey {
	if m != nil {
		return m.FlowKey
	}
	return nil
}

func (m *ProxyGetFlowInfoResponse) GetLifId() uint32 {
	if m != nil {
		return m.LifId
	}
	return 0
}

func (m *ProxyGetFlowInfoResponse) GetQtype() uint32 {
	if m != nil {
		return m.Qtype
	}
	return 0
}

func (m *ProxyGetFlowInfoResponse) GetQid1() uint32 {
	if m != nil {
		return m.Qid1
	}
	return 0
}

func (m *ProxyGetFlowInfoResponse) GetQid2() uint32 {
	if m != nil {
		return m.Qid2
	}
	return 0
}

type ProxyGetFlowInfoResponseMsg struct {
	Response []*ProxyGetFlowInfoResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *ProxyGetFlowInfoResponseMsg) Reset()                    { *m = ProxyGetFlowInfoResponseMsg{} }
func (m *ProxyGetFlowInfoResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*ProxyGetFlowInfoResponseMsg) ProtoMessage()               {}
func (*ProxyGetFlowInfoResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor14, []int{19} }

func (m *ProxyGetFlowInfoResponseMsg) GetResponse() []*ProxyGetFlowInfoResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

func init() {
	proto.RegisterType((*ProxyKeyHandle)(nil), "proxy.ProxyKeyHandle")
	proto.RegisterType((*ProxySpec)(nil), "proxy.ProxySpec")
	proto.RegisterType((*ProxyRequestMsg)(nil), "proxy.ProxyRequestMsg")
	proto.RegisterType((*ProxyStatus)(nil), "proxy.ProxyStatus")
	proto.RegisterType((*ProxyResponse)(nil), "proxy.ProxyResponse")
	proto.RegisterType((*ProxyResponseMsg)(nil), "proxy.ProxyResponseMsg")
	proto.RegisterType((*ProxyDisableRequest)(nil), "proxy.ProxyDisableRequest")
	proto.RegisterType((*ProxyDisableRequestMsg)(nil), "proxy.ProxyDisableRequestMsg")
	proto.RegisterType((*ProxyDisableResponseMsg)(nil), "proxy.ProxyDisableResponseMsg")
	proto.RegisterType((*ProxyGetRequest)(nil), "proxy.ProxyGetRequest")
	proto.RegisterType((*ProxyGetRequestMsg)(nil), "proxy.ProxyGetRequestMsg")
	proto.RegisterType((*ProxyStats)(nil), "proxy.ProxyStats")
	proto.RegisterType((*ProxyGetResponse)(nil), "proxy.ProxyGetResponse")
	proto.RegisterType((*ProxyGetResponseMsg)(nil), "proxy.ProxyGetResponseMsg")
	proto.RegisterType((*ProxyFlowConfigRequest)(nil), "proxy.ProxyFlowConfigRequest")
	proto.RegisterType((*ProxyFlowConfigRequestMsg)(nil), "proxy.ProxyFlowConfigRequestMsg")
	proto.RegisterType((*ProxyGetFlowInfoRequest)(nil), "proxy.ProxyGetFlowInfoRequest")
	proto.RegisterType((*ProxyGetFlowInfoRequestMsg)(nil), "proxy.ProxyGetFlowInfoRequestMsg")
	proto.RegisterType((*ProxyGetFlowInfoResponse)(nil), "proxy.ProxyGetFlowInfoResponse")
	proto.RegisterType((*ProxyGetFlowInfoResponseMsg)(nil), "proxy.ProxyGetFlowInfoResponseMsg")
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for Proxy service

type ProxyClient interface {
	ProxyEnable(ctx context.Context, in *ProxyRequestMsg, opts ...grpc.CallOption) (*ProxyResponseMsg, error)
	ProxyUpdate(ctx context.Context, in *ProxyRequestMsg, opts ...grpc.CallOption) (*ProxyResponseMsg, error)
	ProxyDisable(ctx context.Context, in *ProxyDisableRequestMsg, opts ...grpc.CallOption) (*ProxyDisableResponseMsg, error)
	ProxyGet(ctx context.Context, in *ProxyGetRequestMsg, opts ...grpc.CallOption) (*ProxyGetResponseMsg, error)
	ProxyFlowConfig(ctx context.Context, in *ProxyFlowConfigRequestMsg, opts ...grpc.CallOption) (*ProxyResponseMsg, error)
	ProxyGetFlowInfo(ctx context.Context, in *ProxyGetFlowInfoRequestMsg, opts ...grpc.CallOption) (*ProxyGetFlowInfoResponseMsg, error)
}

type proxyClient struct {
	cc *grpc.ClientConn
}

func NewProxyClient(cc *grpc.ClientConn) ProxyClient {
	return &proxyClient{cc}
}

func (c *proxyClient) ProxyEnable(ctx context.Context, in *ProxyRequestMsg, opts ...grpc.CallOption) (*ProxyResponseMsg, error) {
	out := new(ProxyResponseMsg)
	err := grpc.Invoke(ctx, "/proxy.Proxy/ProxyEnable", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *proxyClient) ProxyUpdate(ctx context.Context, in *ProxyRequestMsg, opts ...grpc.CallOption) (*ProxyResponseMsg, error) {
	out := new(ProxyResponseMsg)
	err := grpc.Invoke(ctx, "/proxy.Proxy/ProxyUpdate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *proxyClient) ProxyDisable(ctx context.Context, in *ProxyDisableRequestMsg, opts ...grpc.CallOption) (*ProxyDisableResponseMsg, error) {
	out := new(ProxyDisableResponseMsg)
	err := grpc.Invoke(ctx, "/proxy.Proxy/ProxyDisable", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *proxyClient) ProxyGet(ctx context.Context, in *ProxyGetRequestMsg, opts ...grpc.CallOption) (*ProxyGetResponseMsg, error) {
	out := new(ProxyGetResponseMsg)
	err := grpc.Invoke(ctx, "/proxy.Proxy/ProxyGet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *proxyClient) ProxyFlowConfig(ctx context.Context, in *ProxyFlowConfigRequestMsg, opts ...grpc.CallOption) (*ProxyResponseMsg, error) {
	out := new(ProxyResponseMsg)
	err := grpc.Invoke(ctx, "/proxy.Proxy/ProxyFlowConfig", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *proxyClient) ProxyGetFlowInfo(ctx context.Context, in *ProxyGetFlowInfoRequestMsg, opts ...grpc.CallOption) (*ProxyGetFlowInfoResponseMsg, error) {
	out := new(ProxyGetFlowInfoResponseMsg)
	err := grpc.Invoke(ctx, "/proxy.Proxy/ProxyGetFlowInfo", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for Proxy service

type ProxyServer interface {
	ProxyEnable(context.Context, *ProxyRequestMsg) (*ProxyResponseMsg, error)
	ProxyUpdate(context.Context, *ProxyRequestMsg) (*ProxyResponseMsg, error)
	ProxyDisable(context.Context, *ProxyDisableRequestMsg) (*ProxyDisableResponseMsg, error)
	ProxyGet(context.Context, *ProxyGetRequestMsg) (*ProxyGetResponseMsg, error)
	ProxyFlowConfig(context.Context, *ProxyFlowConfigRequestMsg) (*ProxyResponseMsg, error)
	ProxyGetFlowInfo(context.Context, *ProxyGetFlowInfoRequestMsg) (*ProxyGetFlowInfoResponseMsg, error)
}

func RegisterProxyServer(s *grpc.Server, srv ProxyServer) {
	s.RegisterService(&_Proxy_serviceDesc, srv)
}

func _Proxy_ProxyEnable_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ProxyRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(ProxyServer).ProxyEnable(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/proxy.Proxy/ProxyEnable",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(ProxyServer).ProxyEnable(ctx, req.(*ProxyRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Proxy_ProxyUpdate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ProxyRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(ProxyServer).ProxyUpdate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/proxy.Proxy/ProxyUpdate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(ProxyServer).ProxyUpdate(ctx, req.(*ProxyRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Proxy_ProxyDisable_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ProxyDisableRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(ProxyServer).ProxyDisable(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/proxy.Proxy/ProxyDisable",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(ProxyServer).ProxyDisable(ctx, req.(*ProxyDisableRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Proxy_ProxyGet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ProxyGetRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(ProxyServer).ProxyGet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/proxy.Proxy/ProxyGet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(ProxyServer).ProxyGet(ctx, req.(*ProxyGetRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Proxy_ProxyFlowConfig_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ProxyFlowConfigRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(ProxyServer).ProxyFlowConfig(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/proxy.Proxy/ProxyFlowConfig",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(ProxyServer).ProxyFlowConfig(ctx, req.(*ProxyFlowConfigRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _Proxy_ProxyGetFlowInfo_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ProxyGetFlowInfoRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(ProxyServer).ProxyGetFlowInfo(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/proxy.Proxy/ProxyGetFlowInfo",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(ProxyServer).ProxyGetFlowInfo(ctx, req.(*ProxyGetFlowInfoRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

var _Proxy_serviceDesc = grpc.ServiceDesc{
	ServiceName: "proxy.Proxy",
	HandlerType: (*ProxyServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "ProxyEnable",
			Handler:    _Proxy_ProxyEnable_Handler,
		},
		{
			MethodName: "ProxyUpdate",
			Handler:    _Proxy_ProxyUpdate_Handler,
		},
		{
			MethodName: "ProxyDisable",
			Handler:    _Proxy_ProxyDisable_Handler,
		},
		{
			MethodName: "ProxyGet",
			Handler:    _Proxy_ProxyGet_Handler,
		},
		{
			MethodName: "ProxyFlowConfig",
			Handler:    _Proxy_ProxyFlowConfig_Handler,
		},
		{
			MethodName: "ProxyGetFlowInfo",
			Handler:    _Proxy_ProxyGetFlowInfo_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "nic/proto/hal/proxy.proto",
}

func init() { proto.RegisterFile("nic/proto/hal/proxy.proto", fileDescriptor14) }

var fileDescriptor14 = []byte{
	// 863 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xc4, 0x56, 0x59, 0x6f, 0xdb, 0x46,
	0x10, 0x36, 0xad, 0x7b, 0xe4, 0x43, 0xdd, 0xfa, 0xa0, 0x64, 0xd8, 0x96, 0xd9, 0x16, 0x15, 0x5c,
	0x40, 0x76, 0xe5, 0x16, 0x6d, 0x51, 0x14, 0x68, 0x7d, 0x1f, 0xb0, 0xdd, 0xb2, 0x6e, 0x1f, 0x9c,
	0x07, 0x81, 0x96, 0x56, 0x36, 0x63, 0x86, 0xa4, 0xb5, 0x34, 0x14, 0xfe, 0xa0, 0xfc, 0x8d, 0xe4,
	0x21, 0xef, 0xf9, 0x07, 0xf9, 0x2f, 0xc1, 0xce, 0x2e, 0xc5, 0x43, 0x92, 0x15, 0x1b, 0x48, 0xf2,
	0xc4, 0xdd, 0x9d, 0x99, 0x6f, 0x76, 0xbe, 0x99, 0xd9, 0x21, 0x94, 0x6d, 0xb3, 0xb5, 0xe1, 0x76,
	0x1d, 0xcf, 0xd9, 0xb8, 0x31, 0x2c, 0xbe, 0x7a, 0xe9, 0xd7, 0x71, 0x4f, 0x32, 0xb8, 0xa9, 0xcc,
	0x87, 0x1a, 0x9e, 0xef, 0x52, 0x26, 0xa4, 0x95, 0xa5, 0xb8, 0x21, 0xa3, 0x8c, 0x99, 0x8e, 0x2d,
	0x84, 0x5a, 0x0b, 0x66, 0xfe, 0xe6, 0xc6, 0x27, 0xd4, 0x3f, 0x34, 0xec, 0xb6, 0x45, 0xc9, 0x12,
	0xe4, 0x11, 0xae, 0x69, 0xb6, 0x55, 0xa5, 0xaa, 0xd4, 0xa6, 0x0f, 0x27, 0xf4, 0x1c, 0x9e, 0x1c,
	0xb5, 0xc9, 0x37, 0x30, 0x25, 0x84, 0x37, 0xa8, 0xac, 0x4e, 0x56, 0x95, 0x5a, 0xf6, 0x70, 0x42,
	0x2f, 0xe2, 0xa9, 0x40, 0xd8, 0x9e, 0x85, 0xe9, 0x5b, 0xea, 0x37, 0x9d, 0xae, 0xd4, 0xd2, 0xde,
	0x28, 0x50, 0x40, 0x2f, 0xff, 0xba, 0xb4, 0x45, 0xbe, 0x83, 0xf4, 0x0b, 0xea, 0x19, 0x08, 0x5e,
	0x6c, 0x7c, 0x55, 0x17, 0x77, 0x3d, 0xbf, 0x7a, 0x4e, 0x5b, 0xde, 0x29, 0xf5, 0x0c, 0x1d, 0xc5,
	0xe4, 0xb7, 0x04, 0x0a, 0xfa, 0x2a, 0x36, 0xe6, 0xeb, 0x22, 0xf2, 0xf8, 0xad, 0xf5, 0xe2, 0x2d,
	0xf5, 0xcf, 0xbb, 0x32, 0x84, 0x0d, 0x00, 0x71, 0x4b, 0x0e, 0xad, 0xa6, 0xaa, 0x4a, 0x6d, 0xa6,
	0x51, 0x92, 0x7e, 0xd0, 0xee, 0xc2, 0x77, 0xa9, 0x5e, 0x70, 0x83, 0x25, 0x59, 0x05, 0x11, 0x40,
	0xf3, 0x0e, 0x2d, 0xd2, 0x3c, 0x6c, 0x5d, 0x60, 0xfc, 0xc3, 0x4f, 0xb4, 0x3f, 0x60, 0x16, 0x0d,
	0x75, 0x7a, 0x77, 0x4f, 0x99, 0x77, 0xca, 0xae, 0xc9, 0x3a, 0xe4, 0xba, 0x62, 0xa7, 0x2a, 0xd5,
	0x54, 0xad, 0xd8, 0x28, 0x45, 0x6f, 0xc6, 0x23, 0xd5, 0x03, 0x05, 0x6d, 0x13, 0x8a, 0xe2, 0xd4,
	0x33, 0xbc, 0x7b, 0x46, 0xd6, 0x12, 0x2c, 0x72, 0x26, 0xb2, 0x31, 0x0e, 0xb5, 0x1e, 0x4c, 0x4b,
	0x87, 0xcc, 0x75, 0x6c, 0x86, 0x31, 0x19, 0xae, 0xd9, 0x64, 0x88, 0x80, 0x16, 0x61, 0x4c, 0x7f,
	0xb9, 0xa6, 0x40, 0xd6, 0x0b, 0x46, 0xb0, 0x24, 0x3f, 0x07, 0x4e, 0xa4, 0x89, 0xa0, 0x8f, 0xc4,
	0x2e, 0x29, 0x8c, 0x84, 0x63, 0xb1, 0xd1, 0x76, 0xa1, 0x14, 0x73, 0xcc, 0x43, 0xdd, 0x84, 0x7c,
	0x57, 0x6e, 0x65, 0xac, 0x73, 0x51, 0x98, 0x40, 0x55, 0xef, 0x6b, 0x69, 0x3d, 0xf8, 0x1a, 0x45,
	0xbb, 0x26, 0x33, 0xae, 0x2c, 0x2a, 0x69, 0xfb, 0xf4, 0xa9, 0xd7, 0xce, 0x60, 0x61, 0x88, 0x63,
	0x1e, 0xc4, 0x4f, 0xc9, 0x7c, 0x55, 0xa2, 0x70, 0x71, 0xfd, 0x30, 0x73, 0xc7, 0xb0, 0x18, 0x97,
	0x87, 0xac, 0x24, 0x33, 0x92, 0x1a, 0x93, 0x11, 0x8d, 0xc9, 0x22, 0x3a, 0xa0, 0xde, 0xe7, 0x23,
	0x64, 0x1f, 0x48, 0xc2, 0xa9, 0xc8, 0x68, 0x82, 0x8c, 0x85, 0x28, 0x54, 0xa8, 0x1b, 0x12, 0x31,
	0x05, 0xd0, 0xaf, 0x19, 0xa6, 0xbd, 0x56, 0x64, 0x99, 0xa0, 0xea, 0x53, 0x4b, 0xf4, 0x5b, 0x48,
	0x33, 0x97, 0xb6, 0x64, 0x34, 0x83, 0xfd, 0x83, 0x52, 0xb2, 0x0e, 0x59, 0x09, 0x99, 0x1a, 0x59,
	0xc2, 0x52, 0x83, 0x7c, 0x0f, 0x19, 0xbe, 0x62, 0xd8, 0xc2, 0x9c, 0xd0, 0x84, 0x2a, 0xd3, 0x85,
	0x5c, 0x3b, 0x96, 0x05, 0x1a, 0xb9, 0x3f, 0xe7, 0x65, 0x6b, 0xa0, 0xd2, 0x17, 0x07, 0x88, 0x19,
	0x28, 0xf6, 0xf7, 0x8a, 0x2c, 0xba, 0x7d, 0xcb, 0xe9, 0xed, 0x38, 0x76, 0xc7, 0xbc, 0x7e, 0x64,
	0x7e, 0x3f, 0x8e, 0x88, 0x1f, 0x20, 0xdf, 0xb1, 0x9c, 0x5e, 0xf3, 0x96, 0xfa, 0x92, 0x8a, 0x52,
	0x3d, 0x78, 0xcd, 0xb9, 0xeb, 0x13, 0xea, 0xeb, 0xb9, 0x8e, 0x58, 0x90, 0x72, 0xf0, 0x8c, 0x53,
	0x1b, 0xc9, 0xc8, 0xcb, 0x47, 0x7c, 0xcf, 0x26, 0x75, 0x28, 0x20, 0x4e, 0xdb, 0xf0, 0x0c, 0x35,
	0x23, 0x6f, 0x16, 0x05, 0xda, 0x35, 0x3c, 0x43, 0x47, 0x5f, 0x7c, 0xa5, 0x5d, 0x40, 0x79, 0x78,
	0x78, 0x9c, 0xb1, 0x5f, 0x92, 0x95, 0xb4, 0x1c, 0xbd, 0xfd, 0x80, 0x49, 0x58, 0x50, 0x6f, 0x15,
	0xd9, 0x5a, 0x07, 0xd4, 0xe3, 0x6a, 0x47, 0x76, 0xc7, 0xf9, 0xf2, 0xb4, 0x3d, 0x96, 0x9b, 0xff,
	0xa1, 0x32, 0x22, 0x08, 0x4e, 0xce, 0xaf, 0x49, 0x72, 0x56, 0x12, 0xd5, 0x94, 0xb0, 0x09, 0xd9,
	0x79, 0x35, 0x09, 0xea, 0xa0, 0xd2, 0x53, 0x1b, 0x2d, 0xe0, 0x73, 0xf2, 0x61, 0x3e, 0x1f, 0x3d,
	0x37, 0xa3, 0xd4, 0xa6, 0xc7, 0x51, 0x3b, 0x0f, 0x59, 0xcb, 0xec, 0xf0, 0xdf, 0x8a, 0x0c, 0xce,
	0xd7, 0x8c, 0x65, 0x76, 0x8e, 0xda, 0x64, 0x0e, 0x32, 0x62, 0xea, 0x66, 0xc5, 0x29, 0x6e, 0x08,
	0x81, 0xf4, 0x9d, 0xd9, 0xfe, 0x51, 0xcd, 0xe1, 0x21, 0xae, 0xe5, 0x59, 0x43, 0xcd, 0xf7, 0xcf,
	0x1a, 0xda, 0x25, 0x2c, 0x8d, 0xa2, 0x89, 0x27, 0xe0, 0xf7, 0x81, 0x7e, 0x5e, 0x1d, 0x99, 0x81,
	0x64, 0x5f, 0x37, 0xde, 0xa5, 0x20, 0x83, 0x6a, 0xe4, 0x4f, 0x39, 0xbf, 0xf7, 0x6c, 0x3e, 0x04,
	0xc8, 0x42, 0x7c, 0xfa, 0x05, 0xe9, 0xae, 0x2c, 0x0e, 0x9b, 0x8a, 0xa7, 0xec, 0x5a, 0x9b, 0xe8,
	0x23, 0xfc, 0xe7, 0xb6, 0x0d, 0xef, 0x49, 0x08, 0xe7, 0x30, 0x15, 0x9d, 0x44, 0x64, 0x79, 0xf4,
	0xf8, 0xe2, 0x48, 0x2b, 0x43, 0xc5, 0x51, 0xc0, 0x1d, 0xc8, 0x07, 0x24, 0x90, 0xf2, 0xf0, 0xe7,
	0x9f, 0x03, 0x55, 0x46, 0x3c, 0x80, 0x02, 0xe4, 0x4c, 0xce, 0xb4, 0xb0, 0xd1, 0x49, 0xf5, 0xc1,
	0x07, 0x60, 0x4c, 0x94, 0xcf, 0xc2, 0xb9, 0x12, 0x64, 0x86, 0xac, 0x3d, 0xdc, 0x34, 0x1c, 0x51,
	0x1b, 0x93, 0x55, 0x04, 0xdf, 0x86, 0xcb, 0xfc, 0x8d, 0x61, 0xe1, 0x8f, 0xef, 0x55, 0x16, 0x3f,
	0x5b, 0x1f, 0x02, 0x00, 0x00, 0xff, 0xff, 0x1c, 0xe7, 0xe4, 0x05, 0x57, 0x0b, 0x00, 0x00,
}
