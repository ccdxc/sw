// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: tunnel.proto

package netproto

import proto "github.com/gogo/protobuf/proto"
import fmt "fmt"
import math "math"
import _ "google.golang.org/genproto/googleapis/api/annotations"
import _ "github.com/pensando/sw/venice/utils/apigen/annotations"
import _ "github.com/gogo/protobuf/gogoproto"
import api "github.com/pensando/sw/api"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

import io "io"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type TunnelSpec_Encap int32

const (
	TunnelSpec_NONE  TunnelSpec_Encap = 0
	TunnelSpec_VXLAN TunnelSpec_Encap = 1
	TunnelSpec_GRE   TunnelSpec_Encap = 2
)

var TunnelSpec_Encap_name = map[int32]string{
	0: "NONE",
	1: "VXLAN",
	2: "GRE",
}
var TunnelSpec_Encap_value = map[string]int32{
	"NONE":  0,
	"VXLAN": 1,
	"GRE":   2,
}

func (x TunnelSpec_Encap) String() string {
	return proto.EnumName(TunnelSpec_Encap_name, int32(x))
}
func (TunnelSpec_Encap) EnumDescriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{1, 0} }

type TunnelStatus_IFStatus int32

const (
	TunnelStatus_NONE TunnelStatus_IFStatus = 0
	TunnelStatus_UP   TunnelStatus_IFStatus = 1
	TunnelStatus_DOWN TunnelStatus_IFStatus = 2
)

var TunnelStatus_IFStatus_name = map[int32]string{
	0: "NONE",
	1: "UP",
	2: "DOWN",
}
var TunnelStatus_IFStatus_value = map[string]int32{
	"NONE": 0,
	"UP":   1,
	"DOWN": 2,
}

func (x TunnelStatus_IFStatus) String() string {
	return proto.EnumName(TunnelStatus_IFStatus_name, int32(x))
}
func (TunnelStatus_IFStatus) EnumDescriptor() ([]byte, []int) {
	return fileDescriptorTunnel, []int{2, 0}
}

// Tunnel object
type Tunnel struct {
	api.TypeMeta   `protobuf:"bytes,1,opt,name=TypeMeta,embedded=TypeMeta" json:",inline"`
	api.ObjectMeta `protobuf:"bytes,2,opt,name=ObjectMeta,embedded=ObjectMeta" json:"meta,omitempty"`
	Spec           TunnelSpec   `protobuf:"bytes,3,opt,name=Spec" json:"spec,omitempty"`
	Status         TunnelStatus `protobuf:"bytes,4,opt,name=Status" json:"status,omitempty"`
}

func (m *Tunnel) Reset()                    { *m = Tunnel{} }
func (m *Tunnel) String() string            { return proto.CompactTextString(m) }
func (*Tunnel) ProtoMessage()               {}
func (*Tunnel) Descriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{0} }

func (m *Tunnel) GetSpec() TunnelSpec {
	if m != nil {
		return m.Spec
	}
	return TunnelSpec{}
}

func (m *Tunnel) GetStatus() TunnelStatus {
	if m != nil {
		return m.Status
	}
	return TunnelStatus{}
}

// TunnelSpec captures all the tunnel level configuration
type TunnelSpec struct {
	// VrfName specifies the name of the VRF that the current Tunnel belongs to
	VrfName string `protobuf:"bytes,1,opt,name=VrfName,proto3" json:"vrf-name,omitempty"`
	// Overlay Tunnel Type. Required. Only VXLAN is currently supported
	Type string `protobuf:"bytes,2,opt,name=Type,proto3" json:"type,omitempty"`
	// AdminStatus of the overlay tunnel.
	AdminStatus string `protobuf:"bytes,3,opt,name=AdminStatus,proto3" json:"admin-status,omitempty"`
	// Local TEP IP Address
	Src string `protobuf:"bytes,4,opt,name=Src,proto3" json:"source,omitempty"`
	// Remote TEP IP Address
	Dst string `protobuf:"bytes,5,opt,name=Dst,proto3" json:"destination,omitempty"`
}

func (m *TunnelSpec) Reset()                    { *m = TunnelSpec{} }
func (m *TunnelSpec) String() string            { return proto.CompactTextString(m) }
func (*TunnelSpec) ProtoMessage()               {}
func (*TunnelSpec) Descriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{1} }

func (m *TunnelSpec) GetVrfName() string {
	if m != nil {
		return m.VrfName
	}
	return ""
}

func (m *TunnelSpec) GetType() string {
	if m != nil {
		return m.Type
	}
	return ""
}

func (m *TunnelSpec) GetAdminStatus() string {
	if m != nil {
		return m.AdminStatus
	}
	return ""
}

func (m *TunnelSpec) GetSrc() string {
	if m != nil {
		return m.Src
	}
	return ""
}

func (m *TunnelSpec) GetDst() string {
	if m != nil {
		return m.Dst
	}
	return ""
}

// Tunnel Status
type TunnelStatus struct {
	OperStatus string `protobuf:"bytes,1,opt,name=OperStatus,proto3" json:"oper-status,omitempty"`
	// TunnelID in the datapath
	TunnelID uint64 `protobuf:"varint,2,opt,name=TunnelID,proto3" json:"id,omitempty"`
}

func (m *TunnelStatus) Reset()                    { *m = TunnelStatus{} }
func (m *TunnelStatus) String() string            { return proto.CompactTextString(m) }
func (*TunnelStatus) ProtoMessage()               {}
func (*TunnelStatus) Descriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{2} }

func (m *TunnelStatus) GetOperStatus() string {
	if m != nil {
		return m.OperStatus
	}
	return ""
}

func (m *TunnelStatus) GetTunnelID() uint64 {
	if m != nil {
		return m.TunnelID
	}
	return 0
}

type TunnelList struct {
	Tunnels []*Tunnel `protobuf:"bytes,1,rep,name=tunnels" json:"tunnels,omitempty"`
}

func (m *TunnelList) Reset()                    { *m = TunnelList{} }
func (m *TunnelList) String() string            { return proto.CompactTextString(m) }
func (*TunnelList) ProtoMessage()               {}
func (*TunnelList) Descriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{3} }

func (m *TunnelList) GetTunnels() []*Tunnel {
	if m != nil {
		return m.Tunnels
	}
	return nil
}

// tunnel watch event
type TunnelEvent struct {
	EventType api.EventType `protobuf:"varint,1,opt,name=EventType,proto3,enum=api.EventType" json:"event-type,omitempty"`
	Tunnel    Tunnel        `protobuf:"bytes,2,opt,name=Tunnel" json:"tunnel,omitempty"`
}

func (m *TunnelEvent) Reset()                    { *m = TunnelEvent{} }
func (m *TunnelEvent) String() string            { return proto.CompactTextString(m) }
func (*TunnelEvent) ProtoMessage()               {}
func (*TunnelEvent) Descriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{4} }

func (m *TunnelEvent) GetEventType() api.EventType {
	if m != nil {
		return m.EventType
	}
	return api.EventType_CreateEvent
}

func (m *TunnelEvent) GetTunnel() Tunnel {
	if m != nil {
		return m.Tunnel
	}
	return Tunnel{}
}

// tunnel watch events batched
type TunnelEventList struct {
	TunnelEvents []*TunnelEvent `protobuf:"bytes,1,rep,name=TunnelEvents" json:"TunnelEvents,omitempty"`
}

func (m *TunnelEventList) Reset()                    { *m = TunnelEventList{} }
func (m *TunnelEventList) String() string            { return proto.CompactTextString(m) }
func (*TunnelEventList) ProtoMessage()               {}
func (*TunnelEventList) Descriptor() ([]byte, []int) { return fileDescriptorTunnel, []int{5} }

func (m *TunnelEventList) GetTunnelEvents() []*TunnelEvent {
	if m != nil {
		return m.TunnelEvents
	}
	return nil
}

func init() {
	proto.RegisterType((*Tunnel)(nil), "netproto.Tunnel")
	proto.RegisterType((*TunnelSpec)(nil), "netproto.TunnelSpec")
	proto.RegisterType((*TunnelStatus)(nil), "netproto.TunnelStatus")
	proto.RegisterType((*TunnelList)(nil), "netproto.TunnelList")
	proto.RegisterType((*TunnelEvent)(nil), "netproto.TunnelEvent")
	proto.RegisterType((*TunnelEventList)(nil), "netproto.TunnelEventList")
	proto.RegisterEnum("netproto.TunnelSpec_Encap", TunnelSpec_Encap_name, TunnelSpec_Encap_value)
	proto.RegisterEnum("netproto.TunnelStatus_IFStatus", TunnelStatus_IFStatus_name, TunnelStatus_IFStatus_value)
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for TunnelApiV1 service

type TunnelApiV1Client interface {
}

type tunnelApiV1Client struct {
	cc *grpc.ClientConn
}

func NewTunnelApiV1Client(cc *grpc.ClientConn) TunnelApiV1Client {
	return &tunnelApiV1Client{cc}
}

// Server API for TunnelApiV1 service

type TunnelApiV1Server interface {
}

func RegisterTunnelApiV1Server(s *grpc.Server, srv TunnelApiV1Server) {
	s.RegisterService(&_TunnelApiV1_serviceDesc, srv)
}

var _TunnelApiV1_serviceDesc = grpc.ServiceDesc{
	ServiceName: "netproto.TunnelApiV1",
	HandlerType: (*TunnelApiV1Server)(nil),
	Methods:     []grpc.MethodDesc{},
	Streams:     []grpc.StreamDesc{},
	Metadata:    "tunnel.proto",
}

func (m *Tunnel) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *Tunnel) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	dAtA[i] = 0xa
	i++
	i = encodeVarintTunnel(dAtA, i, uint64(m.TypeMeta.Size()))
	n1, err := m.TypeMeta.MarshalTo(dAtA[i:])
	if err != nil {
		return 0, err
	}
	i += n1
	dAtA[i] = 0x12
	i++
	i = encodeVarintTunnel(dAtA, i, uint64(m.ObjectMeta.Size()))
	n2, err := m.ObjectMeta.MarshalTo(dAtA[i:])
	if err != nil {
		return 0, err
	}
	i += n2
	dAtA[i] = 0x1a
	i++
	i = encodeVarintTunnel(dAtA, i, uint64(m.Spec.Size()))
	n3, err := m.Spec.MarshalTo(dAtA[i:])
	if err != nil {
		return 0, err
	}
	i += n3
	dAtA[i] = 0x22
	i++
	i = encodeVarintTunnel(dAtA, i, uint64(m.Status.Size()))
	n4, err := m.Status.MarshalTo(dAtA[i:])
	if err != nil {
		return 0, err
	}
	i += n4
	return i, nil
}

func (m *TunnelSpec) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *TunnelSpec) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.VrfName) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(len(m.VrfName)))
		i += copy(dAtA[i:], m.VrfName)
	}
	if len(m.Type) > 0 {
		dAtA[i] = 0x12
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(len(m.Type)))
		i += copy(dAtA[i:], m.Type)
	}
	if len(m.AdminStatus) > 0 {
		dAtA[i] = 0x1a
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(len(m.AdminStatus)))
		i += copy(dAtA[i:], m.AdminStatus)
	}
	if len(m.Src) > 0 {
		dAtA[i] = 0x22
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(len(m.Src)))
		i += copy(dAtA[i:], m.Src)
	}
	if len(m.Dst) > 0 {
		dAtA[i] = 0x2a
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(len(m.Dst)))
		i += copy(dAtA[i:], m.Dst)
	}
	return i, nil
}

func (m *TunnelStatus) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *TunnelStatus) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.OperStatus) > 0 {
		dAtA[i] = 0xa
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(len(m.OperStatus)))
		i += copy(dAtA[i:], m.OperStatus)
	}
	if m.TunnelID != 0 {
		dAtA[i] = 0x10
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(m.TunnelID))
	}
	return i, nil
}

func (m *TunnelList) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *TunnelList) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Tunnels) > 0 {
		for _, msg := range m.Tunnels {
			dAtA[i] = 0xa
			i++
			i = encodeVarintTunnel(dAtA, i, uint64(msg.Size()))
			n, err := msg.MarshalTo(dAtA[i:])
			if err != nil {
				return 0, err
			}
			i += n
		}
	}
	return i, nil
}

func (m *TunnelEvent) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *TunnelEvent) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if m.EventType != 0 {
		dAtA[i] = 0x8
		i++
		i = encodeVarintTunnel(dAtA, i, uint64(m.EventType))
	}
	dAtA[i] = 0x12
	i++
	i = encodeVarintTunnel(dAtA, i, uint64(m.Tunnel.Size()))
	n5, err := m.Tunnel.MarshalTo(dAtA[i:])
	if err != nil {
		return 0, err
	}
	i += n5
	return i, nil
}

func (m *TunnelEventList) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *TunnelEventList) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.TunnelEvents) > 0 {
		for _, msg := range m.TunnelEvents {
			dAtA[i] = 0xa
			i++
			i = encodeVarintTunnel(dAtA, i, uint64(msg.Size()))
			n, err := msg.MarshalTo(dAtA[i:])
			if err != nil {
				return 0, err
			}
			i += n
		}
	}
	return i, nil
}

func encodeVarintTunnel(dAtA []byte, offset int, v uint64) int {
	for v >= 1<<7 {
		dAtA[offset] = uint8(v&0x7f | 0x80)
		v >>= 7
		offset++
	}
	dAtA[offset] = uint8(v)
	return offset + 1
}
func (m *Tunnel) Size() (n int) {
	var l int
	_ = l
	l = m.TypeMeta.Size()
	n += 1 + l + sovTunnel(uint64(l))
	l = m.ObjectMeta.Size()
	n += 1 + l + sovTunnel(uint64(l))
	l = m.Spec.Size()
	n += 1 + l + sovTunnel(uint64(l))
	l = m.Status.Size()
	n += 1 + l + sovTunnel(uint64(l))
	return n
}

func (m *TunnelSpec) Size() (n int) {
	var l int
	_ = l
	l = len(m.VrfName)
	if l > 0 {
		n += 1 + l + sovTunnel(uint64(l))
	}
	l = len(m.Type)
	if l > 0 {
		n += 1 + l + sovTunnel(uint64(l))
	}
	l = len(m.AdminStatus)
	if l > 0 {
		n += 1 + l + sovTunnel(uint64(l))
	}
	l = len(m.Src)
	if l > 0 {
		n += 1 + l + sovTunnel(uint64(l))
	}
	l = len(m.Dst)
	if l > 0 {
		n += 1 + l + sovTunnel(uint64(l))
	}
	return n
}

func (m *TunnelStatus) Size() (n int) {
	var l int
	_ = l
	l = len(m.OperStatus)
	if l > 0 {
		n += 1 + l + sovTunnel(uint64(l))
	}
	if m.TunnelID != 0 {
		n += 1 + sovTunnel(uint64(m.TunnelID))
	}
	return n
}

func (m *TunnelList) Size() (n int) {
	var l int
	_ = l
	if len(m.Tunnels) > 0 {
		for _, e := range m.Tunnels {
			l = e.Size()
			n += 1 + l + sovTunnel(uint64(l))
		}
	}
	return n
}

func (m *TunnelEvent) Size() (n int) {
	var l int
	_ = l
	if m.EventType != 0 {
		n += 1 + sovTunnel(uint64(m.EventType))
	}
	l = m.Tunnel.Size()
	n += 1 + l + sovTunnel(uint64(l))
	return n
}

func (m *TunnelEventList) Size() (n int) {
	var l int
	_ = l
	if len(m.TunnelEvents) > 0 {
		for _, e := range m.TunnelEvents {
			l = e.Size()
			n += 1 + l + sovTunnel(uint64(l))
		}
	}
	return n
}

func sovTunnel(x uint64) (n int) {
	for {
		n++
		x >>= 7
		if x == 0 {
			break
		}
	}
	return n
}
func sozTunnel(x uint64) (n int) {
	return sovTunnel(uint64((x << 1) ^ uint64((int64(x) >> 63))))
}
func (m *Tunnel) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: Tunnel: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: Tunnel: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field TypeMeta", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			if err := m.TypeMeta.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field ObjectMeta", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			if err := m.ObjectMeta.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Spec", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			if err := m.Spec.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		case 4:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Status", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			if err := m.Status.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipTunnel(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthTunnel
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func (m *TunnelSpec) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: TunnelSpec: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: TunnelSpec: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field VrfName", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.VrfName = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Type", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Type = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field AdminStatus", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.AdminStatus = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 4:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Src", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Src = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 5:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Dst", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Dst = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipTunnel(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthTunnel
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func (m *TunnelStatus) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: TunnelStatus: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: TunnelStatus: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field OperStatus", wireType)
			}
			var stringLen uint64
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				stringLen |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			intStringLen := int(stringLen)
			if intStringLen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + intStringLen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.OperStatus = string(dAtA[iNdEx:postIndex])
			iNdEx = postIndex
		case 2:
			if wireType != 0 {
				return fmt.Errorf("proto: wrong wireType = %d for field TunnelID", wireType)
			}
			m.TunnelID = 0
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				m.TunnelID |= (uint64(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
		default:
			iNdEx = preIndex
			skippy, err := skipTunnel(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthTunnel
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func (m *TunnelList) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: TunnelList: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: TunnelList: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Tunnels", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Tunnels = append(m.Tunnels, &Tunnel{})
			if err := m.Tunnels[len(m.Tunnels)-1].Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipTunnel(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthTunnel
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func (m *TunnelEvent) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: TunnelEvent: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: TunnelEvent: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 0 {
				return fmt.Errorf("proto: wrong wireType = %d for field EventType", wireType)
			}
			m.EventType = 0
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				m.EventType |= (api.EventType(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
		case 2:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Tunnel", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			if err := m.Tunnel.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipTunnel(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthTunnel
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func (m *TunnelEventList) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		fieldNum := int32(wire >> 3)
		wireType := int(wire & 0x7)
		if wireType == 4 {
			return fmt.Errorf("proto: TunnelEventList: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: TunnelEventList: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field TunnelEvents", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				msglen |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			if msglen < 0 {
				return ErrInvalidLengthTunnel
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.TunnelEvents = append(m.TunnelEvents, &TunnelEvent{})
			if err := m.TunnelEvents[len(m.TunnelEvents)-1].Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipTunnel(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthTunnel
			}
			if (iNdEx + skippy) > l {
				return io.ErrUnexpectedEOF
			}
			iNdEx += skippy
		}
	}

	if iNdEx > l {
		return io.ErrUnexpectedEOF
	}
	return nil
}
func skipTunnel(dAtA []byte) (n int, err error) {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return 0, ErrIntOverflowTunnel
			}
			if iNdEx >= l {
				return 0, io.ErrUnexpectedEOF
			}
			b := dAtA[iNdEx]
			iNdEx++
			wire |= (uint64(b) & 0x7F) << shift
			if b < 0x80 {
				break
			}
		}
		wireType := int(wire & 0x7)
		switch wireType {
		case 0:
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return 0, ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return 0, io.ErrUnexpectedEOF
				}
				iNdEx++
				if dAtA[iNdEx-1] < 0x80 {
					break
				}
			}
			return iNdEx, nil
		case 1:
			iNdEx += 8
			return iNdEx, nil
		case 2:
			var length int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return 0, ErrIntOverflowTunnel
				}
				if iNdEx >= l {
					return 0, io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				length |= (int(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
			iNdEx += length
			if length < 0 {
				return 0, ErrInvalidLengthTunnel
			}
			return iNdEx, nil
		case 3:
			for {
				var innerWire uint64
				var start int = iNdEx
				for shift := uint(0); ; shift += 7 {
					if shift >= 64 {
						return 0, ErrIntOverflowTunnel
					}
					if iNdEx >= l {
						return 0, io.ErrUnexpectedEOF
					}
					b := dAtA[iNdEx]
					iNdEx++
					innerWire |= (uint64(b) & 0x7F) << shift
					if b < 0x80 {
						break
					}
				}
				innerWireType := int(innerWire & 0x7)
				if innerWireType == 4 {
					break
				}
				next, err := skipTunnel(dAtA[start:])
				if err != nil {
					return 0, err
				}
				iNdEx = start + next
			}
			return iNdEx, nil
		case 4:
			return iNdEx, nil
		case 5:
			iNdEx += 4
			return iNdEx, nil
		default:
			return 0, fmt.Errorf("proto: illegal wireType %d", wireType)
		}
	}
	panic("unreachable")
}

var (
	ErrInvalidLengthTunnel = fmt.Errorf("proto: negative length found during unmarshaling")
	ErrIntOverflowTunnel   = fmt.Errorf("proto: integer overflow")
)

func init() { proto.RegisterFile("tunnel.proto", fileDescriptorTunnel) }

var fileDescriptorTunnel = []byte{
	// 773 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x8c, 0x94, 0xdd, 0x6a, 0xdb, 0x48,
	0x14, 0xc7, 0x23, 0xdb, 0xf1, 0xc7, 0xd8, 0x9b, 0x88, 0xd9, 0xc4, 0xd8, 0x66, 0xb1, 0x17, 0x41,
	0x42, 0x76, 0x37, 0x96, 0x76, 0xbd, 0x37, 0xbb, 0xec, 0x07, 0x1b, 0xad, 0x9d, 0x36, 0x34, 0xb5,
	0x83, 0x9d, 0xa6, 0x85, 0x42, 0x41, 0x96, 0x27, 0xae, 0x8a, 0x35, 0x9a, 0x4a, 0xa3, 0x94, 0x50,
	0x02, 0x85, 0x42, 0xde, 0xa1, 0x97, 0xbd, 0xcd, 0x65, 0xdf, 0xa0, 0x77, 0xb9, 0xcc, 0x13, 0xb8,
	0x25, 0x97, 0x7e, 0x8a, 0x32, 0x47, 0x96, 0x3d, 0xb1, 0x29, 0xf4, 0x6e, 0xe6, 0xcc, 0xff, 0xfc,
	0x74, 0xfe, 0x47, 0x67, 0x06, 0x15, 0x78, 0x48, 0x29, 0x19, 0xe9, 0xcc, 0xf7, 0xb8, 0x87, 0xb3,
	0x94, 0x70, 0x58, 0x55, 0x7e, 0x18, 0x7a, 0xde, 0x70, 0x44, 0x0c, 0x8b, 0x39, 0x86, 0x45, 0xa9,
	0xc7, 0x2d, 0xee, 0x78, 0x34, 0x88, 0x74, 0x95, 0xd6, 0xd0, 0xe1, 0xcf, 0xc3, 0xbe, 0x6e, 0x7b,
	0xae, 0xc1, 0x08, 0x0d, 0x2c, 0x3a, 0xf0, 0x8c, 0xe0, 0x95, 0x71, 0x46, 0xa8, 0x63, 0x13, 0x23,
	0xe4, 0xce, 0x28, 0x10, 0xa9, 0x43, 0x42, 0xe5, 0x6c, 0xc3, 0xa1, 0xf6, 0x28, 0x1c, 0x90, 0x18,
	0x53, 0x97, 0x30, 0x43, 0x6f, 0xe8, 0x19, 0x10, 0xee, 0x87, 0xa7, 0xb0, 0x83, 0x0d, 0xac, 0xa6,
	0xf2, 0xad, 0xaf, 0x7c, 0x55, 0xd4, 0xe8, 0x12, 0x6e, 0x45, 0x32, 0xed, 0x7d, 0x02, 0xa5, 0x8f,
	0xc1, 0x15, 0xfe, 0x17, 0x65, 0x8f, 0xcf, 0x19, 0x79, 0x48, 0xb8, 0x55, 0x52, 0x7e, 0x54, 0x76,
	0xf2, 0x8d, 0xef, 0x74, 0x8b, 0x39, 0x7a, 0x1c, 0x34, 0xbf, 0xbf, 0x1e, 0xd7, 0x56, 0x6e, 0xc6,
	0x35, 0x65, 0x32, 0xae, 0x65, 0x76, 0x1d, 0x3a, 0x72, 0x28, 0xe9, 0xce, 0x72, 0xf0, 0x03, 0x84,
	0x3a, 0xfd, 0x17, 0xc4, 0xe6, 0x40, 0x48, 0x00, 0x61, 0x1d, 0x08, 0xf3, 0xb0, 0x59, 0x91, 0x18,
	0x6b, 0xa2, 0x8e, 0x5d, 0xcf, 0x75, 0x38, 0x71, 0x19, 0x3f, 0xef, 0x4a, 0xe9, 0xf8, 0x3f, 0x94,
	0xea, 0x31, 0x62, 0x97, 0x92, 0x80, 0xd9, 0xd0, 0xe3, 0x5e, 0xeb, 0x51, 0xb1, 0xe2, 0xcc, 0x2c,
	0x0a, 0x96, 0xe0, 0x04, 0x8c, 0xd8, 0x12, 0x07, 0x32, 0xf1, 0x7d, 0x94, 0xee, 0x71, 0x8b, 0x87,
	0x41, 0x29, 0x05, 0x8c, 0xe2, 0x12, 0x03, 0x4e, 0xcd, 0xd2, 0x94, 0xa2, 0x06, 0xb0, 0x97, 0x38,
	0xd3, 0x7c, 0xed, 0x53, 0x02, 0xa1, 0xf9, 0x67, 0xf1, 0xaf, 0x28, 0x73, 0xe2, 0x9f, 0xb6, 0x2d,
	0x97, 0x40, 0x9b, 0x72, 0x66, 0x71, 0x32, 0xae, 0xe1, 0x33, 0xff, 0xb4, 0x4e, 0x2d, 0x97, 0x48,
	0xf9, 0xb1, 0x0c, 0xff, 0x8f, 0x52, 0xa2, 0x4b, 0xd0, 0x93, 0x9c, 0x69, 0x5c, 0x5d, 0x96, 0xcb,
	0x3d, 0xee, 0xb7, 0x68, 0xe8, 0xee, 0xcc, 0xb9, 0x7a, 0x8b, 0xda, 0x16, 0xfb, 0x49, 0xf8, 0xe1,
	0xe7, 0x4c, 0xe6, 0x40, 0x32, 0x7e, 0x86, 0xf2, 0x7b, 0x03, 0xd7, 0xa1, 0x53, 0x53, 0x49, 0x60,
	0xfd, 0x7d, 0x75, 0x59, 0xae, 0x2e, 0xb0, 0xe0, 0x5c, 0x3f, 0xd8, 0x8f, 0x16, 0x02, 0x58, 0xb4,
	0x44, 0x62, 0x7d, 0xc9, 0xa0, 0x0c, 0xc4, 0xdb, 0x28, 0xd9, 0xf3, 0x6d, 0x68, 0x56, 0xce, 0xdc,
	0x80, 0x86, 0x78, 0xa1, 0x6f, 0xcb, 0x85, 0x08, 0x01, 0xfe, 0x05, 0x25, 0x9b, 0x01, 0x2f, 0xad,
	0x82, 0xae, 0x3c, 0x19, 0xd7, 0x36, 0x07, 0x24, 0xe0, 0x0e, 0x85, 0xa9, 0x95, 0xc5, 0xcd, 0x80,
	0x6b, 0x5b, 0x68, 0x15, 0x6c, 0xe1, 0x2c, 0x4a, 0xb5, 0x3b, 0xed, 0x96, 0xba, 0x82, 0x73, 0x68,
	0xf5, 0xe4, 0xc9, 0xe1, 0x5e, 0x5b, 0x55, 0x70, 0x06, 0x25, 0xef, 0x75, 0x5b, 0x6a, 0x42, 0xfb,
	0xa8, 0xa0, 0x82, 0x5c, 0x3d, 0x7e, 0x8a, 0x50, 0x87, 0x11, 0x7f, 0xea, 0x35, 0x6a, 0xf3, 0x5f,
	0xdf, 0xe4, 0x75, 0xd3, 0x63, 0xc4, 0x5f, 0xb6, 0x2a, 0xe1, 0xf0, 0x2e, 0xca, 0x46, 0xe9, 0x07,
	0x4d, 0xf8, 0x25, 0x29, 0x53, 0x9d, 0x8c, 0x6b, 0x05, 0x67, 0x20, 0xe9, 0x67, 0x0a, 0x6d, 0x1b,
	0x65, 0x63, 0xbe, 0xe4, 0x22, 0x8d, 0x12, 0x8f, 0x8e, 0x54, 0x45, 0x44, 0x9a, 0x9d, 0xc7, 0x6d,
	0x35, 0xa1, 0xfd, 0x11, 0x0f, 0xc9, 0xa1, 0x13, 0x70, 0xfc, 0x33, 0xca, 0x44, 0x8f, 0x85, 0xa8,
	0x3e, 0xb9, 0x93, 0x6f, 0xa8, 0x8b, 0xe3, 0xd7, 0x8d, 0x05, 0xda, 0x3b, 0x05, 0xe5, 0xa3, 0x58,
	0xeb, 0x8c, 0x50, 0x8e, 0xf7, 0x51, 0x0e, 0x16, 0x30, 0x33, 0xc2, 0xfb, 0x5a, 0x63, 0x0d, 0xee,
	0xd1, 0x2c, 0x6a, 0x96, 0x26, 0xe3, 0xda, 0x06, 0x11, 0xdb, 0xfa, 0xc2, 0xb0, 0xcc, 0x53, 0xb1,
	0x19, 0x5f, 0xed, 0xe9, 0x65, 0x5c, 0x2a, 0x61, 0x3e, 0xfb, 0x51, 0x29, 0xf2, 0xec, 0x47, 0x0a,
	0xed, 0x10, 0xad, 0x4b, 0xa5, 0x81, 0xb5, 0x3f, 0xe3, 0x7f, 0x05, 0xa1, 0xd8, 0xdf, 0xe6, 0x22,
	0x1c, 0x4e, 0xbb, 0x77, 0xa4, 0x8d, 0x37, 0x33, 0xa7, 0x7b, 0xcc, 0x39, 0xf9, 0xad, 0xf2, 0xf2,
	0xc3, 0xdb, 0xb2, 0x3b, 0x7b, 0x80, 0x52, 0x23, 0x81, 0x4f, 0x31, 0x2f, 0xe0, 0x38, 0x3d, 0x20,
	0x23, 0xc2, 0x09, 0x4e, 0xb2, 0x90, 0x57, 0xfe, 0x31, 0x5e, 0xcf, 0x9f, 0x06, 0xfd, 0x98, 0x50,
	0x8b, 0xf2, 0x8b, 0x3b, 0x31, 0x71, 0xcd, 0x02, 0x66, 0xd9, 0x64, 0x39, 0x7c, 0xa1, 0xe5, 0xc5,
	0x9b, 0x37, 0x6d, 0xb6, 0x59, 0xb8, 0xbe, 0xad, 0x2a, 0x37, 0xb7, 0x55, 0xe5, 0xf3, 0x6d, 0x55,
	0x39, 0x52, 0xfa, 0x69, 0xa8, 0xf9, 0xf7, 0x2f, 0x01, 0x00, 0x00, 0xff, 0xff, 0xeb, 0x41, 0x68,
	0x5b, 0xdc, 0x05, 0x00, 0x00,
}
