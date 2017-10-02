// Code generated by protoc-gen-go. DO NOT EDIT.
// source: nic/proto/hal/l2segment.proto

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

// BroadcastFwdPolicy defines various forwarding options to deal with
// broadcast traffic
type BroadcastFwdPolicy int32

const (
	BroadcastFwdPolicy_BROADCAST_FWD_POLICY_NONE  BroadcastFwdPolicy = 0
	BroadcastFwdPolicy_BROADCAST_FWD_POLICY_DROP  BroadcastFwdPolicy = 1
	BroadcastFwdPolicy_BROADCAST_FWD_POLICY_FLOOD BroadcastFwdPolicy = 2
)

var BroadcastFwdPolicy_name = map[int32]string{
	0: "BROADCAST_FWD_POLICY_NONE",
	1: "BROADCAST_FWD_POLICY_DROP",
	2: "BROADCAST_FWD_POLICY_FLOOD",
}
var BroadcastFwdPolicy_value = map[string]int32{
	"BROADCAST_FWD_POLICY_NONE":  0,
	"BROADCAST_FWD_POLICY_DROP":  1,
	"BROADCAST_FWD_POLICY_FLOOD": 2,
}

func (x BroadcastFwdPolicy) String() string {
	return proto.EnumName(BroadcastFwdPolicy_name, int32(x))
}
func (BroadcastFwdPolicy) EnumDescriptor() ([]byte, []int) { return fileDescriptor8, []int{0} }

// MulticastFwdPolicy defines various forwarding options to deal with
// multicast traffic
type MulticastFwdPolicy int32

const (
	MulticastFwdPolicy_MULTICAST_FWD_POLICY_NONE      MulticastFwdPolicy = 0
	MulticastFwdPolicy_MULTICAST_FWD_POLICY_FLOOD     MulticastFwdPolicy = 1
	MulticastFwdPolicy_MULTICAST_FWD_POLICY_REPLICATE MulticastFwdPolicy = 2
	MulticastFwdPolicy_MULTICAST_FWD_POLICY_DROP      MulticastFwdPolicy = 3
)

var MulticastFwdPolicy_name = map[int32]string{
	0: "MULTICAST_FWD_POLICY_NONE",
	1: "MULTICAST_FWD_POLICY_FLOOD",
	2: "MULTICAST_FWD_POLICY_REPLICATE",
	3: "MULTICAST_FWD_POLICY_DROP",
}
var MulticastFwdPolicy_value = map[string]int32{
	"MULTICAST_FWD_POLICY_NONE":      0,
	"MULTICAST_FWD_POLICY_FLOOD":     1,
	"MULTICAST_FWD_POLICY_REPLICATE": 2,
	"MULTICAST_FWD_POLICY_DROP":      3,
}

func (x MulticastFwdPolicy) String() string {
	return proto.EnumName(MulticastFwdPolicy_name, int32(x))
}
func (MulticastFwdPolicy) EnumDescriptor() ([]byte, []int) { return fileDescriptor8, []int{1} }

// L2SegmentKeyHandle is used to operate on a L2 segment either by its
// segment id or HAL allocated handle
type L2SegmentKeyHandle struct {
	// Types that are valid to be assigned to KeyOrHandle:
	//	*L2SegmentKeyHandle_SegmentId
	//	*L2SegmentKeyHandle_L2SegmentHandle
	KeyOrHandle isL2SegmentKeyHandle_KeyOrHandle `protobuf_oneof:"key_or_handle"`
}

func (m *L2SegmentKeyHandle) Reset()                    { *m = L2SegmentKeyHandle{} }
func (m *L2SegmentKeyHandle) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentKeyHandle) ProtoMessage()               {}
func (*L2SegmentKeyHandle) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{0} }

type isL2SegmentKeyHandle_KeyOrHandle interface {
	isL2SegmentKeyHandle_KeyOrHandle()
}

type L2SegmentKeyHandle_SegmentId struct {
	SegmentId uint32 `protobuf:"varint,1,opt,name=segment_id,json=segmentId,oneof"`
}
type L2SegmentKeyHandle_L2SegmentHandle struct {
	L2SegmentHandle uint64 `protobuf:"fixed64,2,opt,name=l2segment_handle,json=l2segmentHandle,oneof"`
}

func (*L2SegmentKeyHandle_SegmentId) isL2SegmentKeyHandle_KeyOrHandle()       {}
func (*L2SegmentKeyHandle_L2SegmentHandle) isL2SegmentKeyHandle_KeyOrHandle() {}

func (m *L2SegmentKeyHandle) GetKeyOrHandle() isL2SegmentKeyHandle_KeyOrHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *L2SegmentKeyHandle) GetSegmentId() uint32 {
	if x, ok := m.GetKeyOrHandle().(*L2SegmentKeyHandle_SegmentId); ok {
		return x.SegmentId
	}
	return 0
}

func (m *L2SegmentKeyHandle) GetL2SegmentHandle() uint64 {
	if x, ok := m.GetKeyOrHandle().(*L2SegmentKeyHandle_L2SegmentHandle); ok {
		return x.L2SegmentHandle
	}
	return 0
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*L2SegmentKeyHandle) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _L2SegmentKeyHandle_OneofMarshaler, _L2SegmentKeyHandle_OneofUnmarshaler, _L2SegmentKeyHandle_OneofSizer, []interface{}{
		(*L2SegmentKeyHandle_SegmentId)(nil),
		(*L2SegmentKeyHandle_L2SegmentHandle)(nil),
	}
}

func _L2SegmentKeyHandle_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*L2SegmentKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *L2SegmentKeyHandle_SegmentId:
		b.EncodeVarint(1<<3 | proto.WireVarint)
		b.EncodeVarint(uint64(x.SegmentId))
	case *L2SegmentKeyHandle_L2SegmentHandle:
		b.EncodeVarint(2<<3 | proto.WireFixed64)
		b.EncodeFixed64(uint64(x.L2SegmentHandle))
	case nil:
	default:
		return fmt.Errorf("L2SegmentKeyHandle.KeyOrHandle has unexpected type %T", x)
	}
	return nil
}

func _L2SegmentKeyHandle_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*L2SegmentKeyHandle)
	switch tag {
	case 1: // key_or_handle.segment_id
		if wire != proto.WireVarint {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeVarint()
		m.KeyOrHandle = &L2SegmentKeyHandle_SegmentId{uint32(x)}
		return true, err
	case 2: // key_or_handle.l2segment_handle
		if wire != proto.WireFixed64 {
			return true, proto.ErrInternalBadWireType
		}
		x, err := b.DecodeFixed64()
		m.KeyOrHandle = &L2SegmentKeyHandle_L2SegmentHandle{x}
		return true, err
	default:
		return false, nil
	}
}

func _L2SegmentKeyHandle_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*L2SegmentKeyHandle)
	// key_or_handle
	switch x := m.KeyOrHandle.(type) {
	case *L2SegmentKeyHandle_SegmentId:
		n += proto.SizeVarint(1<<3 | proto.WireVarint)
		n += proto.SizeVarint(uint64(x.SegmentId))
	case *L2SegmentKeyHandle_L2SegmentHandle:
		n += proto.SizeVarint(2<<3 | proto.WireFixed64)
		n += 8
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// L2SegmentSpec message is used to do add/mod of vlan
type L2SegmentSpec struct {
	Meta           *ObjectMeta         `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle    *L2SegmentKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
	NetworkHandle  []uint64            `protobuf:"fixed64,3,rep,packed,name=network_handle,json=networkHandle" json:"network_handle,omitempty"`
	SegmentType    L2SegmentType       `protobuf:"varint,4,opt,name=segment_type,json=segmentType,enum=types.L2SegmentType" json:"segment_type,omitempty"`
	McastFwdPolicy MulticastFwdPolicy  `protobuf:"varint,5,opt,name=mcast_fwd_policy,json=mcastFwdPolicy,enum=l2segment.MulticastFwdPolicy" json:"mcast_fwd_policy,omitempty"`
	BcastFwdPolicy BroadcastFwdPolicy  `protobuf:"varint,6,opt,name=bcast_fwd_policy,json=bcastFwdPolicy,enum=l2segment.BroadcastFwdPolicy" json:"bcast_fwd_policy,omitempty"`
	AccessEncap    *EncapInfo          `protobuf:"bytes,7,opt,name=access_encap,json=accessEncap" json:"access_encap,omitempty"`
	FabricEncap    *EncapInfo          `protobuf:"bytes,8,opt,name=fabric_encap,json=fabricEncap" json:"fabric_encap,omitempty"`
}

func (m *L2SegmentSpec) Reset()                    { *m = L2SegmentSpec{} }
func (m *L2SegmentSpec) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentSpec) ProtoMessage()               {}
func (*L2SegmentSpec) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{1} }

func (m *L2SegmentSpec) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *L2SegmentSpec) GetKeyOrHandle() *L2SegmentKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

func (m *L2SegmentSpec) GetNetworkHandle() []uint64 {
	if m != nil {
		return m.NetworkHandle
	}
	return nil
}

func (m *L2SegmentSpec) GetSegmentType() L2SegmentType {
	if m != nil {
		return m.SegmentType
	}
	return L2SegmentType_L2_SEGMENT_TYPE_NONE
}

func (m *L2SegmentSpec) GetMcastFwdPolicy() MulticastFwdPolicy {
	if m != nil {
		return m.McastFwdPolicy
	}
	return MulticastFwdPolicy_MULTICAST_FWD_POLICY_NONE
}

func (m *L2SegmentSpec) GetBcastFwdPolicy() BroadcastFwdPolicy {
	if m != nil {
		return m.BcastFwdPolicy
	}
	return BroadcastFwdPolicy_BROADCAST_FWD_POLICY_NONE
}

func (m *L2SegmentSpec) GetAccessEncap() *EncapInfo {
	if m != nil {
		return m.AccessEncap
	}
	return nil
}

func (m *L2SegmentSpec) GetFabricEncap() *EncapInfo {
	if m != nil {
		return m.FabricEncap
	}
	return nil
}

// L2SegmentRequestMsg is batched add or modify L2 segment request
type L2SegmentRequestMsg struct {
	Request []*L2SegmentSpec `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *L2SegmentRequestMsg) Reset()                    { *m = L2SegmentRequestMsg{} }
func (m *L2SegmentRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentRequestMsg) ProtoMessage()               {}
func (*L2SegmentRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{2} }

func (m *L2SegmentRequestMsg) GetRequest() []*L2SegmentSpec {
	if m != nil {
		return m.Request
	}
	return nil
}

// L2SegmentStatus represents the current status of the L2 segment
type L2SegmentStatus struct {
	L2SegmentHandle uint64 `protobuf:"fixed64,1,opt,name=l2segment_handle,json=l2segmentHandle" json:"l2segment_handle,omitempty"`
}

func (m *L2SegmentStatus) Reset()                    { *m = L2SegmentStatus{} }
func (m *L2SegmentStatus) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentStatus) ProtoMessage()               {}
func (*L2SegmentStatus) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{3} }

func (m *L2SegmentStatus) GetL2SegmentHandle() uint64 {
	if m != nil {
		return m.L2SegmentHandle
	}
	return 0
}

// L2SegmentResponse is response to L2SegmentSpec
type L2SegmentResponse struct {
	ApiStatus       ApiStatus        `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	L2SegmentStatus *L2SegmentStatus `protobuf:"bytes,2,opt,name=l2segment_status,json=l2segmentStatus" json:"l2segment_status,omitempty"`
}

func (m *L2SegmentResponse) Reset()                    { *m = L2SegmentResponse{} }
func (m *L2SegmentResponse) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentResponse) ProtoMessage()               {}
func (*L2SegmentResponse) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{4} }

func (m *L2SegmentResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *L2SegmentResponse) GetL2SegmentStatus() *L2SegmentStatus {
	if m != nil {
		return m.L2SegmentStatus
	}
	return nil
}

// L2SegmentResponseMsg is batched response to L2SegmentRequestMsg
type L2SegmentResponseMsg struct {
	Response []*L2SegmentResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *L2SegmentResponseMsg) Reset()                    { *m = L2SegmentResponseMsg{} }
func (m *L2SegmentResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentResponseMsg) ProtoMessage()               {}
func (*L2SegmentResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{5} }

func (m *L2SegmentResponseMsg) GetResponse() []*L2SegmentResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

// L2SegmentDeleteRequest is used to delete a L2 segment
type L2SegmentDeleteRequest struct {
	Meta        *ObjectMeta         `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *L2SegmentKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *L2SegmentDeleteRequest) Reset()                    { *m = L2SegmentDeleteRequest{} }
func (m *L2SegmentDeleteRequest) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentDeleteRequest) ProtoMessage()               {}
func (*L2SegmentDeleteRequest) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{6} }

func (m *L2SegmentDeleteRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *L2SegmentDeleteRequest) GetKeyOrHandle() *L2SegmentKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// L2SegmentDeleteRequestMsg is used to delete a batch of L2 segments
type L2SegmentDeleteRequestMsg struct {
	Request []*L2SegmentDeleteRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *L2SegmentDeleteRequestMsg) Reset()                    { *m = L2SegmentDeleteRequestMsg{} }
func (m *L2SegmentDeleteRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentDeleteRequestMsg) ProtoMessage()               {}
func (*L2SegmentDeleteRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{7} }

func (m *L2SegmentDeleteRequestMsg) GetRequest() []*L2SegmentDeleteRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// L2SegmentDeleteResponseMsg is batched response to L2SegmentDeleteRequestMsg
type L2SegmentDeleteResponseMsg struct {
	ApiStatus []ApiStatus `protobuf:"varint,1,rep,packed,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
}

func (m *L2SegmentDeleteResponseMsg) Reset()                    { *m = L2SegmentDeleteResponseMsg{} }
func (m *L2SegmentDeleteResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentDeleteResponseMsg) ProtoMessage()               {}
func (*L2SegmentDeleteResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{8} }

func (m *L2SegmentDeleteResponseMsg) GetApiStatus() []ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return nil
}

// L2SegmentGetRequest is used to get information about a L2 Segment
type L2SegmentGetRequest struct {
	Meta        *ObjectMeta         `protobuf:"bytes,1,opt,name=meta" json:"meta,omitempty"`
	KeyOrHandle *L2SegmentKeyHandle `protobuf:"bytes,2,opt,name=key_or_handle,json=keyOrHandle" json:"key_or_handle,omitempty"`
}

func (m *L2SegmentGetRequest) Reset()                    { *m = L2SegmentGetRequest{} }
func (m *L2SegmentGetRequest) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentGetRequest) ProtoMessage()               {}
func (*L2SegmentGetRequest) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{9} }

func (m *L2SegmentGetRequest) GetMeta() *ObjectMeta {
	if m != nil {
		return m.Meta
	}
	return nil
}

func (m *L2SegmentGetRequest) GetKeyOrHandle() *L2SegmentKeyHandle {
	if m != nil {
		return m.KeyOrHandle
	}
	return nil
}

// L2SegmentGetRequestMsg is batched GET request for L2 segments
type L2SegmentGetRequestMsg struct {
	Request []*L2SegmentGetRequest `protobuf:"bytes,1,rep,name=request" json:"request,omitempty"`
}

func (m *L2SegmentGetRequestMsg) Reset()                    { *m = L2SegmentGetRequestMsg{} }
func (m *L2SegmentGetRequestMsg) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentGetRequestMsg) ProtoMessage()               {}
func (*L2SegmentGetRequestMsg) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{10} }

func (m *L2SegmentGetRequestMsg) GetRequest() []*L2SegmentGetRequest {
	if m != nil {
		return m.Request
	}
	return nil
}

// L2SegmentStats is the statistics object for each L2 segment
type L2SegmentStats struct {
	NumEndpoints uint32 `protobuf:"varint,1,opt,name=num_endpoints,json=numEndpoints" json:"num_endpoints,omitempty"`
}

func (m *L2SegmentStats) Reset()                    { *m = L2SegmentStats{} }
func (m *L2SegmentStats) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentStats) ProtoMessage()               {}
func (*L2SegmentStats) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{11} }

func (m *L2SegmentStats) GetNumEndpoints() uint32 {
	if m != nil {
		return m.NumEndpoints
	}
	return 0
}

// L2SegmentGetResponse captures all the information about a L2 segment
// only if api_status indicates success, other fields are valid
type L2SegmentGetResponse struct {
	ApiStatus ApiStatus        `protobuf:"varint,1,opt,name=api_status,json=apiStatus,enum=types.ApiStatus" json:"api_status,omitempty"`
	Spec      *L2SegmentSpec   `protobuf:"bytes,2,opt,name=spec" json:"spec,omitempty"`
	Status    *L2SegmentStatus `protobuf:"bytes,3,opt,name=status" json:"status,omitempty"`
	Stats     *L2SegmentStats  `protobuf:"bytes,4,opt,name=stats" json:"stats,omitempty"`
}

func (m *L2SegmentGetResponse) Reset()                    { *m = L2SegmentGetResponse{} }
func (m *L2SegmentGetResponse) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentGetResponse) ProtoMessage()               {}
func (*L2SegmentGetResponse) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{12} }

func (m *L2SegmentGetResponse) GetApiStatus() ApiStatus {
	if m != nil {
		return m.ApiStatus
	}
	return ApiStatus_API_STATUS_OK
}

func (m *L2SegmentGetResponse) GetSpec() *L2SegmentSpec {
	if m != nil {
		return m.Spec
	}
	return nil
}

func (m *L2SegmentGetResponse) GetStatus() *L2SegmentStatus {
	if m != nil {
		return m.Status
	}
	return nil
}

func (m *L2SegmentGetResponse) GetStats() *L2SegmentStats {
	if m != nil {
		return m.Stats
	}
	return nil
}

// L2SegmentGetResponseMsg is the batched response to L2SegmentGetRequestMsg
type L2SegmentGetResponseMsg struct {
	Response []*L2SegmentGetResponse `protobuf:"bytes,1,rep,name=response" json:"response,omitempty"`
}

func (m *L2SegmentGetResponseMsg) Reset()                    { *m = L2SegmentGetResponseMsg{} }
func (m *L2SegmentGetResponseMsg) String() string            { return proto.CompactTextString(m) }
func (*L2SegmentGetResponseMsg) ProtoMessage()               {}
func (*L2SegmentGetResponseMsg) Descriptor() ([]byte, []int) { return fileDescriptor8, []int{13} }

func (m *L2SegmentGetResponseMsg) GetResponse() []*L2SegmentGetResponse {
	if m != nil {
		return m.Response
	}
	return nil
}

func init() {
	proto.RegisterType((*L2SegmentKeyHandle)(nil), "l2segment.L2SegmentKeyHandle")
	proto.RegisterType((*L2SegmentSpec)(nil), "l2segment.L2SegmentSpec")
	proto.RegisterType((*L2SegmentRequestMsg)(nil), "l2segment.L2SegmentRequestMsg")
	proto.RegisterType((*L2SegmentStatus)(nil), "l2segment.L2SegmentStatus")
	proto.RegisterType((*L2SegmentResponse)(nil), "l2segment.L2SegmentResponse")
	proto.RegisterType((*L2SegmentResponseMsg)(nil), "l2segment.L2SegmentResponseMsg")
	proto.RegisterType((*L2SegmentDeleteRequest)(nil), "l2segment.L2SegmentDeleteRequest")
	proto.RegisterType((*L2SegmentDeleteRequestMsg)(nil), "l2segment.L2SegmentDeleteRequestMsg")
	proto.RegisterType((*L2SegmentDeleteResponseMsg)(nil), "l2segment.L2SegmentDeleteResponseMsg")
	proto.RegisterType((*L2SegmentGetRequest)(nil), "l2segment.L2SegmentGetRequest")
	proto.RegisterType((*L2SegmentGetRequestMsg)(nil), "l2segment.L2SegmentGetRequestMsg")
	proto.RegisterType((*L2SegmentStats)(nil), "l2segment.L2SegmentStats")
	proto.RegisterType((*L2SegmentGetResponse)(nil), "l2segment.L2SegmentGetResponse")
	proto.RegisterType((*L2SegmentGetResponseMsg)(nil), "l2segment.L2SegmentGetResponseMsg")
	proto.RegisterEnum("l2segment.BroadcastFwdPolicy", BroadcastFwdPolicy_name, BroadcastFwdPolicy_value)
	proto.RegisterEnum("l2segment.MulticastFwdPolicy", MulticastFwdPolicy_name, MulticastFwdPolicy_value)
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for L2Segment service

type L2SegmentClient interface {
	L2SegmentCreate(ctx context.Context, in *L2SegmentRequestMsg, opts ...grpc.CallOption) (*L2SegmentResponseMsg, error)
	L2SegmentUpdate(ctx context.Context, in *L2SegmentRequestMsg, opts ...grpc.CallOption) (*L2SegmentResponseMsg, error)
	L2SegmentDelete(ctx context.Context, in *L2SegmentDeleteRequestMsg, opts ...grpc.CallOption) (*L2SegmentDeleteResponseMsg, error)
	L2SegmentGet(ctx context.Context, in *L2SegmentGetRequestMsg, opts ...grpc.CallOption) (*L2SegmentGetResponseMsg, error)
}

type l2SegmentClient struct {
	cc *grpc.ClientConn
}

func NewL2SegmentClient(cc *grpc.ClientConn) L2SegmentClient {
	return &l2SegmentClient{cc}
}

func (c *l2SegmentClient) L2SegmentCreate(ctx context.Context, in *L2SegmentRequestMsg, opts ...grpc.CallOption) (*L2SegmentResponseMsg, error) {
	out := new(L2SegmentResponseMsg)
	err := grpc.Invoke(ctx, "/l2segment.L2Segment/L2SegmentCreate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *l2SegmentClient) L2SegmentUpdate(ctx context.Context, in *L2SegmentRequestMsg, opts ...grpc.CallOption) (*L2SegmentResponseMsg, error) {
	out := new(L2SegmentResponseMsg)
	err := grpc.Invoke(ctx, "/l2segment.L2Segment/L2SegmentUpdate", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *l2SegmentClient) L2SegmentDelete(ctx context.Context, in *L2SegmentDeleteRequestMsg, opts ...grpc.CallOption) (*L2SegmentDeleteResponseMsg, error) {
	out := new(L2SegmentDeleteResponseMsg)
	err := grpc.Invoke(ctx, "/l2segment.L2Segment/L2SegmentDelete", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *l2SegmentClient) L2SegmentGet(ctx context.Context, in *L2SegmentGetRequestMsg, opts ...grpc.CallOption) (*L2SegmentGetResponseMsg, error) {
	out := new(L2SegmentGetResponseMsg)
	err := grpc.Invoke(ctx, "/l2segment.L2Segment/L2SegmentGet", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for L2Segment service

type L2SegmentServer interface {
	L2SegmentCreate(context.Context, *L2SegmentRequestMsg) (*L2SegmentResponseMsg, error)
	L2SegmentUpdate(context.Context, *L2SegmentRequestMsg) (*L2SegmentResponseMsg, error)
	L2SegmentDelete(context.Context, *L2SegmentDeleteRequestMsg) (*L2SegmentDeleteResponseMsg, error)
	L2SegmentGet(context.Context, *L2SegmentGetRequestMsg) (*L2SegmentGetResponseMsg, error)
}

func RegisterL2SegmentServer(s *grpc.Server, srv L2SegmentServer) {
	s.RegisterService(&_L2Segment_serviceDesc, srv)
}

func _L2Segment_L2SegmentCreate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(L2SegmentRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(L2SegmentServer).L2SegmentCreate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/l2segment.L2Segment/L2SegmentCreate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(L2SegmentServer).L2SegmentCreate(ctx, req.(*L2SegmentRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _L2Segment_L2SegmentUpdate_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(L2SegmentRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(L2SegmentServer).L2SegmentUpdate(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/l2segment.L2Segment/L2SegmentUpdate",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(L2SegmentServer).L2SegmentUpdate(ctx, req.(*L2SegmentRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _L2Segment_L2SegmentDelete_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(L2SegmentDeleteRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(L2SegmentServer).L2SegmentDelete(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/l2segment.L2Segment/L2SegmentDelete",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(L2SegmentServer).L2SegmentDelete(ctx, req.(*L2SegmentDeleteRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

func _L2Segment_L2SegmentGet_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(L2SegmentGetRequestMsg)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(L2SegmentServer).L2SegmentGet(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/l2segment.L2Segment/L2SegmentGet",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(L2SegmentServer).L2SegmentGet(ctx, req.(*L2SegmentGetRequestMsg))
	}
	return interceptor(ctx, in, info, handler)
}

var _L2Segment_serviceDesc = grpc.ServiceDesc{
	ServiceName: "l2segment.L2Segment",
	HandlerType: (*L2SegmentServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "L2SegmentCreate",
			Handler:    _L2Segment_L2SegmentCreate_Handler,
		},
		{
			MethodName: "L2SegmentUpdate",
			Handler:    _L2Segment_L2SegmentUpdate_Handler,
		},
		{
			MethodName: "L2SegmentDelete",
			Handler:    _L2Segment_L2SegmentDelete_Handler,
		},
		{
			MethodName: "L2SegmentGet",
			Handler:    _L2Segment_L2SegmentGet_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "nic/proto/hal/l2segment.proto",
}

func init() { proto.RegisterFile("nic/proto/hal/l2segment.proto", fileDescriptor8) }

var fileDescriptor8 = []byte{
	// 851 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xbc, 0x56, 0xdd, 0x6e, 0xe3, 0x44,
	0x14, 0xae, 0x9b, 0x6e, 0xb6, 0x3d, 0x69, 0xd2, 0xec, 0xb0, 0x40, 0x1a, 0xd1, 0x34, 0x6b, 0xa8,
	0x14, 0x16, 0xd4, 0x48, 0x5e, 0x21, 0x90, 0xe0, 0x26, 0x7f, 0xdd, 0x46, 0x24, 0xeb, 0x68, 0x9a,
	0x65, 0x81, 0x1b, 0xe3, 0xd8, 0xd3, 0xd6, 0x34, 0xb1, 0x8d, 0x67, 0xa2, 0x2a, 0x57, 0x48, 0xdc,
	0x72, 0xcf, 0x9b, 0x71, 0xc3, 0x63, 0xf0, 0x04, 0xc8, 0x33, 0x13, 0xdb, 0x71, 0x9c, 0x46, 0x42,
	0xab, 0x5e, 0xd5, 0x73, 0xce, 0x77, 0xbe, 0xf3, 0x33, 0xdf, 0x9c, 0x06, 0x4e, 0x5c, 0xc7, 0x6a,
	0xfa, 0x81, 0xc7, 0xbc, 0xe6, 0xad, 0x39, 0x6d, 0x4e, 0x35, 0x4a, 0x6e, 0x66, 0xc4, 0x65, 0xe7,
	0xdc, 0x86, 0x0e, 0x22, 0x43, 0xf5, 0xc3, 0x18, 0xc9, 0x16, 0x3e, 0xa1, 0x02, 0xa1, 0x52, 0x40,
	0x03, 0xed, 0x4a, 0x60, 0xbe, 0x27, 0x8b, 0x4b, 0xd3, 0xb5, 0xa7, 0x04, 0x9d, 0x02, 0xc8, 0x38,
	0xc3, 0xb1, 0x2b, 0x4a, 0x5d, 0x69, 0x14, 0x2f, 0x77, 0xf0, 0x81, 0xb4, 0xf5, 0x6d, 0xf4, 0x05,
	0x94, 0x23, 0x6a, 0xe3, 0x96, 0x07, 0x55, 0x76, 0xeb, 0x4a, 0x23, 0x7f, 0xb9, 0x83, 0x8f, 0x22,
	0x8f, 0x60, 0x6b, 0x1f, 0x41, 0xf1, 0x8e, 0x2c, 0x0c, 0x2f, 0x90, 0x48, 0xf5, 0xef, 0x1c, 0x14,
	0xa3, 0xac, 0x57, 0x3e, 0xb1, 0xd0, 0x19, 0xec, 0xcd, 0x08, 0x33, 0x79, 0xaa, 0x82, 0xf6, 0xec,
	0x5c, 0x94, 0xa8, 0x4f, 0x7e, 0x25, 0x16, 0x1b, 0x12, 0x66, 0x62, 0xee, 0x46, 0xad, 0x14, 0x13,
	0xcf, 0x59, 0xd0, 0x4e, 0xce, 0xe3, 0xc6, 0xd7, 0xbb, 0xc1, 0x85, 0x3b, 0xb2, 0xd0, 0x03, 0xd9,
	0xda, 0x19, 0x94, 0x5c, 0xc2, 0xee, 0xbd, 0xe0, 0x6e, 0xc9, 0x91, 0xab, 0xe7, 0x1a, 0x79, 0x5c,
	0x94, 0x56, 0x09, 0xfb, 0x1a, 0x0e, 0x97, 0xed, 0x85, 0xb5, 0x54, 0xf6, 0xea, 0x4a, 0xa3, 0xa4,
	0x3d, 0x97, 0x85, 0x45, 0x49, 0xc6, 0x0b, 0x9f, 0xe0, 0x02, 0x8d, 0x0f, 0xe8, 0x35, 0x94, 0x67,
	0x96, 0x49, 0x99, 0x71, 0x7d, 0x6f, 0x1b, 0xbe, 0x37, 0x75, 0xac, 0x45, 0xe5, 0x09, 0x0f, 0x4e,
	0x56, 0x39, 0x9c, 0x4f, 0x99, 0x13, 0xc2, 0x2e, 0xee, 0xed, 0x11, 0x07, 0xe1, 0xd2, 0x6c, 0xe5,
	0x1c, 0x12, 0x4d, 0xd2, 0x44, 0xf9, 0x35, 0xa2, 0x76, 0xe0, 0x99, 0x76, 0x8a, 0x68, 0xb2, 0x4a,
	0xf4, 0x0a, 0x0e, 0x4d, 0xcb, 0x22, 0x94, 0x1a, 0xc4, 0xb5, 0x4c, 0xbf, 0xf2, 0x94, 0xcf, 0xac,
	0x2c, 0x5b, 0xe9, 0x85, 0xb6, 0xbe, 0x7b, 0xed, 0xe1, 0x82, 0x40, 0x71, 0x43, 0x18, 0x74, 0x6d,
	0x4e, 0x02, 0xc7, 0x92, 0x41, 0xfb, 0x9b, 0x82, 0x04, 0x8a, 0x1b, 0xd4, 0x3e, 0x7c, 0x10, 0x4d,
	0x06, 0x93, 0xdf, 0xe6, 0x84, 0xb2, 0x21, 0xbd, 0x41, 0x1a, 0x3c, 0x0d, 0xc4, 0xa9, 0xa2, 0xd4,
	0x73, 0x8d, 0x82, 0x56, 0xc9, 0xba, 0xaf, 0x50, 0x07, 0x78, 0x09, 0x54, 0xbf, 0x83, 0xa3, 0xd8,
	0xc3, 0x4c, 0x36, 0xa7, 0xe8, 0xf3, 0x0c, 0xcd, 0x85, 0x7a, 0xc9, 0xaf, 0x29, 0x4e, 0xfd, 0x53,
	0x81, 0x67, 0x89, 0x4a, 0xa8, 0xef, 0xb9, 0x94, 0xa0, 0x26, 0x80, 0xe9, 0x3b, 0x06, 0xe5, 0x74,
	0x3c, 0xb4, 0x14, 0x75, 0xd4, 0xf2, 0x1d, 0x91, 0x06, 0x1f, 0x98, 0xcb, 0x4f, 0xd4, 0x4b, 0x66,
	0x94, 0x61, 0x42, 0x71, 0xd5, 0xcc, 0x0e, 0x04, 0x41, 0x5c, 0x8d, 0x30, 0xa8, 0x23, 0x78, 0xbe,
	0x56, 0x4c, 0x38, 0x97, 0x6f, 0x60, 0x3f, 0x90, 0x47, 0x39, 0x98, 0x4f, 0xb2, 0x68, 0x97, 0x21,
	0x38, 0x42, 0xab, 0x7f, 0x28, 0xf0, 0x51, 0xe4, 0xef, 0x92, 0x29, 0x61, 0x44, 0xce, 0xfb, 0xf1,
	0x5e, 0x92, 0xfa, 0x23, 0x1c, 0x67, 0xd7, 0x10, 0xf6, 0xf6, 0x6d, 0xfa, 0xce, 0x5f, 0x64, 0x31,
	0xaf, 0x84, 0xc5, 0x97, 0x3f, 0x84, 0xea, 0x1a, 0x24, 0x1e, 0x5b, 0xfa, 0x1a, 0x73, 0x5b, 0xae,
	0x51, 0xfd, 0x3d, 0x21, 0xcb, 0xd7, 0x84, 0x3d, 0xfe, 0xa4, 0x70, 0xe2, 0xb6, 0xe2, 0x02, 0x84,
	0x04, 0x52, 0x63, 0xaa, 0x65, 0xd1, 0xc6, 0x31, 0xf1, 0x8c, 0xbe, 0x82, 0xd2, 0x8a, 0xf0, 0x28,
	0xfa, 0x14, 0x8a, 0xee, 0x7c, 0x66, 0x10, 0xd7, 0xf6, 0x3d, 0xc7, 0x65, 0x42, 0xe1, 0x45, 0x7c,
	0xe8, 0xce, 0x67, 0xbd, 0xa5, 0x4d, 0xfd, 0x47, 0x49, 0x88, 0x91, 0xf3, 0xfe, 0xdf, 0xc7, 0xf1,
	0x25, 0xec, 0x51, 0x9f, 0x58, 0x72, 0x1c, 0x9b, 0x9f, 0x34, 0x47, 0x21, 0x0d, 0xf2, 0x92, 0x3a,
	0xb7, 0xf5, 0x01, 0x49, 0x24, 0x6a, 0xc2, 0x93, 0xf0, 0x8b, 0xf2, 0xe5, 0x5b, 0xd0, 0x8e, 0x37,
	0x85, 0x50, 0x2c, 0x70, 0xea, 0x0f, 0xf0, 0x71, 0x56, 0x6f, 0x42, 0x8f, 0xe9, 0xb7, 0x76, 0xba,
	0x71, 0xd2, 0xe9, 0xe7, 0xf6, 0x32, 0x00, 0xb4, 0xbe, 0x67, 0xd1, 0x09, 0x1c, 0xb7, 0xb1, 0xde,
	0xea, 0x76, 0x5a, 0x57, 0x63, 0xe3, 0xe2, 0x5d, 0xd7, 0x18, 0xe9, 0x83, 0x7e, 0xe7, 0x27, 0xe3,
	0x8d, 0xfe, 0xa6, 0x57, 0xde, 0xd9, 0xe8, 0xee, 0x62, 0x7d, 0x54, 0x56, 0x50, 0x0d, 0xaa, 0x99,
	0xee, 0x8b, 0x81, 0xae, 0x77, 0xcb, 0xbb, 0x2f, 0xff, 0x52, 0x00, 0xad, 0xff, 0x97, 0x08, 0x59,
	0x87, 0x6f, 0x07, 0xe3, 0xfe, 0x86, 0xa4, 0x35, 0xa8, 0x66, 0xba, 0x05, 0xab, 0x82, 0x54, 0xa8,
	0x65, 0xfa, 0x71, 0x6f, 0x34, 0xe8, 0x77, 0x5a, 0xe3, 0x5e, 0x79, 0x77, 0x63, 0x0a, 0x5e, 0x78,
	0x4e, 0xfb, 0x77, 0x17, 0x0e, 0xa2, 0x79, 0xa1, 0x71, 0x62, 0x4f, 0x77, 0x02, 0x62, 0x32, 0x82,
	0x6a, 0xd9, 0x4b, 0x6c, 0xa9, 0xf9, 0xea, 0xe9, 0x43, 0x4b, 0x6e, 0x48, 0x6f, 0xd4, 0x9d, 0x15,
	0xd6, 0xb7, 0xbe, 0xfd, 0x9e, 0x58, 0x7f, 0x49, 0xb0, 0x8a, 0xb5, 0x82, 0x3e, 0xdb, 0xba, 0x95,
	0x42, 0xee, 0xb3, 0x87, 0x50, 0xc9, 0x0c, 0xef, 0xe0, 0x30, 0x29, 0x25, 0xf4, 0xe2, 0xe1, 0xd7,
	0x1c, 0x72, 0xab, 0x5b, 0x64, 0xc8, 0x89, 0xdb, 0xf0, 0xf3, 0xfe, 0xad, 0x39, 0xe5, 0x3f, 0xd9,
	0x26, 0x79, 0xfe, 0xe7, 0xd5, 0x7f, 0x01, 0x00, 0x00, 0xff, 0xff, 0xd3, 0xe9, 0x86, 0x2f, 0xfc,
	0x09, 0x00, 0x00,
}
