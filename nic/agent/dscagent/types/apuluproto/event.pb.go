// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: event.proto

package pds

import proto "github.com/gogo/protobuf/proto"
import fmt "fmt"
import math "math"
import _ "github.com/pensando/sw/nic/agent/dscagent/types/apuluproto/meta/pds"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

import io "io"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// Events generated by PDS agent/HAL
type EventId int32

const (
	EventId_EVENT_ID_NONE        EventId = 0
	EventId_EVENT_ID_PORT_CREATE EventId = 1
	EventId_EVENT_ID_PORT_UP     EventId = 2
	EventId_EVENT_ID_PORT_DOWN   EventId = 3
	EventId_EVENT_ID_LIF_CREATE  EventId = 4
	EventId_EVENT_ID_LIF_UPDATE  EventId = 5
	EventId_EVENT_ID_LIF_UP      EventId = 6
	EventId_EVENT_ID_LIF_DOWN    EventId = 7
)

var EventId_name = map[int32]string{
	0: "EVENT_ID_NONE",
	1: "EVENT_ID_PORT_CREATE",
	2: "EVENT_ID_PORT_UP",
	3: "EVENT_ID_PORT_DOWN",
	4: "EVENT_ID_LIF_CREATE",
	5: "EVENT_ID_LIF_UPDATE",
	6: "EVENT_ID_LIF_UP",
	7: "EVENT_ID_LIF_DOWN",
}
var EventId_value = map[string]int32{
	"EVENT_ID_NONE":        0,
	"EVENT_ID_PORT_CREATE": 1,
	"EVENT_ID_PORT_UP":     2,
	"EVENT_ID_PORT_DOWN":   3,
	"EVENT_ID_LIF_CREATE":  4,
	"EVENT_ID_LIF_UPDATE":  5,
	"EVENT_ID_LIF_UP":      6,
	"EVENT_ID_LIF_DOWN":    7,
}

func (x EventId) String() string {
	return proto.EnumName(EventId_name, int32(x))
}
func (EventId) EnumDescriptor() ([]byte, []int) { return fileDescriptorEvent, []int{0} }

// EventOp indicates whether client is subscribing or unsubscribing
type EventOp int32

const (
	EventOp_EVENT_OP_NONE        EventOp = 0
	EventOp_EVENT_OP_SUBSCRIBE   EventOp = 1
	EventOp_EVENT_OP_UNSUBSCRIBE EventOp = 2
)

var EventOp_name = map[int32]string{
	0: "EVENT_OP_NONE",
	1: "EVENT_OP_SUBSCRIBE",
	2: "EVENT_OP_UNSUBSCRIBE",
}
var EventOp_value = map[string]int32{
	"EVENT_OP_NONE":        0,
	"EVENT_OP_SUBSCRIBE":   1,
	"EVENT_OP_UNSUBSCRIBE": 2,
}

func (x EventOp) String() string {
	return proto.EnumName(EventOp_name, int32(x))
}
func (EventOp) EnumDescriptor() ([]byte, []int) { return fileDescriptorEvent, []int{1} }

// EventRequest captures the event information for subscription/unsubscription
type EventRequest struct {
	Request []*EventRequest_EventSpec `protobuf:"bytes,1,rep,name=Request" json:"Request,omitempty"`
}

func (m *EventRequest) Reset()                    { *m = EventRequest{} }
func (m *EventRequest) String() string            { return proto.CompactTextString(m) }
func (*EventRequest) ProtoMessage()               {}
func (*EventRequest) Descriptor() ([]byte, []int) { return fileDescriptorEvent, []int{0} }

func (m *EventRequest) GetRequest() []*EventRequest_EventSpec {
	if m != nil {
		return m.Request
	}
	return nil
}

type EventRequest_EventSpec struct {
	// event of interest
	EventId EventId `protobuf:"varint,1,opt,name=EventId,proto3,enum=pds.EventId" json:"EventId,omitempty"`
	// action is either subscribe or unsubscribe
	Action EventOp `protobuf:"varint,2,opt,name=Action,proto3,enum=pds.EventOp" json:"Action,omitempty"`
}

func (m *EventRequest_EventSpec) Reset()                    { *m = EventRequest_EventSpec{} }
func (m *EventRequest_EventSpec) String() string            { return proto.CompactTextString(m) }
func (*EventRequest_EventSpec) ProtoMessage()               {}
func (*EventRequest_EventSpec) Descriptor() ([]byte, []int) { return fileDescriptorEvent, []int{0, 0} }

func (m *EventRequest_EventSpec) GetEventId() EventId {
	if m != nil {
		return m.EventId
	}
	return EventId_EVENT_ID_NONE
}

func (m *EventRequest_EventSpec) GetAction() EventOp {
	if m != nil {
		return m.Action
	}
	return EventOp_EVENT_OP_NONE
}

type EventResponse struct {
	Status ApiStatus `protobuf:"varint,1,opt,name=Status,proto3,enum=types.ApiStatus" json:"Status,omitempty"`
	// event id
	EventId EventId `protobuf:"varint,2,opt,name=EventId,proto3,enum=pds.EventId" json:"EventId,omitempty"`
	// event specific information
	//
	// Types that are valid to be assigned to EventInfo:
	//	*EventResponse_LifEventInfo
	//	*EventResponse_PortEventInfo
	EventInfo isEventResponse_EventInfo `protobuf_oneof:"event_info"`
}

func (m *EventResponse) Reset()                    { *m = EventResponse{} }
func (m *EventResponse) String() string            { return proto.CompactTextString(m) }
func (*EventResponse) ProtoMessage()               {}
func (*EventResponse) Descriptor() ([]byte, []int) { return fileDescriptorEvent, []int{1} }

type isEventResponse_EventInfo interface {
	isEventResponse_EventInfo()
	MarshalTo([]byte) (int, error)
	Size() int
}

type EventResponse_LifEventInfo struct {
	LifEventInfo *Lif `protobuf:"bytes,3,opt,name=LifEventInfo,oneof"`
}
type EventResponse_PortEventInfo struct {
	PortEventInfo *Port `protobuf:"bytes,4,opt,name=PortEventInfo,oneof"`
}

func (*EventResponse_LifEventInfo) isEventResponse_EventInfo()  {}
func (*EventResponse_PortEventInfo) isEventResponse_EventInfo() {}

func (m *EventResponse) GetEventInfo() isEventResponse_EventInfo {
	if m != nil {
		return m.EventInfo
	}
	return nil
}

func (m *EventResponse) GetStatus() ApiStatus {
	if m != nil {
		return m.Status
	}
	return ApiStatus_API_STATUS_OK
}

func (m *EventResponse) GetEventId() EventId {
	if m != nil {
		return m.EventId
	}
	return EventId_EVENT_ID_NONE
}

func (m *EventResponse) GetLifEventInfo() *Lif {
	if x, ok := m.GetEventInfo().(*EventResponse_LifEventInfo); ok {
		return x.LifEventInfo
	}
	return nil
}

func (m *EventResponse) GetPortEventInfo() *Port {
	if x, ok := m.GetEventInfo().(*EventResponse_PortEventInfo); ok {
		return x.PortEventInfo
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*EventResponse) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _EventResponse_OneofMarshaler, _EventResponse_OneofUnmarshaler, _EventResponse_OneofSizer, []interface{}{
		(*EventResponse_LifEventInfo)(nil),
		(*EventResponse_PortEventInfo)(nil),
	}
}

func _EventResponse_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*EventResponse)
	// event_info
	switch x := m.EventInfo.(type) {
	case *EventResponse_LifEventInfo:
		_ = b.EncodeVarint(3<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.LifEventInfo); err != nil {
			return err
		}
	case *EventResponse_PortEventInfo:
		_ = b.EncodeVarint(4<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.PortEventInfo); err != nil {
			return err
		}
	case nil:
	default:
		return fmt.Errorf("EventResponse.EventInfo has unexpected type %T", x)
	}
	return nil
}

func _EventResponse_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*EventResponse)
	switch tag {
	case 3: // event_info.LifEventInfo
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(Lif)
		err := b.DecodeMessage(msg)
		m.EventInfo = &EventResponse_LifEventInfo{msg}
		return true, err
	case 4: // event_info.PortEventInfo
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(Port)
		err := b.DecodeMessage(msg)
		m.EventInfo = &EventResponse_PortEventInfo{msg}
		return true, err
	default:
		return false, nil
	}
}

func _EventResponse_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*EventResponse)
	// event_info
	switch x := m.EventInfo.(type) {
	case *EventResponse_LifEventInfo:
		s := proto.Size(x.LifEventInfo)
		n += proto.SizeVarint(3<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *EventResponse_PortEventInfo:
		s := proto.Size(x.PortEventInfo)
		n += proto.SizeVarint(4<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

func init() {
	proto.RegisterType((*EventRequest)(nil), "pds.EventRequest")
	proto.RegisterType((*EventRequest_EventSpec)(nil), "pds.EventRequest.EventSpec")
	proto.RegisterType((*EventResponse)(nil), "pds.EventResponse")
	proto.RegisterEnum("pds.EventId", EventId_name, EventId_value)
	proto.RegisterEnum("pds.EventOp", EventOp_name, EventOp_value)
}

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for EventSvc service

type EventSvcClient interface {
	EventSubscribe(ctx context.Context, opts ...grpc.CallOption) (EventSvc_EventSubscribeClient, error)
}

type eventSvcClient struct {
	cc *grpc.ClientConn
}

func NewEventSvcClient(cc *grpc.ClientConn) EventSvcClient {
	return &eventSvcClient{cc}
}

func (c *eventSvcClient) EventSubscribe(ctx context.Context, opts ...grpc.CallOption) (EventSvc_EventSubscribeClient, error) {
	stream, err := grpc.NewClientStream(ctx, &_EventSvc_serviceDesc.Streams[0], c.cc, "/pds.EventSvc/EventSubscribe", opts...)
	if err != nil {
		return nil, err
	}
	x := &eventSvcEventSubscribeClient{stream}
	return x, nil
}

type EventSvc_EventSubscribeClient interface {
	Send(*EventRequest) error
	Recv() (*EventResponse, error)
	grpc.ClientStream
}

type eventSvcEventSubscribeClient struct {
	grpc.ClientStream
}

func (x *eventSvcEventSubscribeClient) Send(m *EventRequest) error {
	return x.ClientStream.SendMsg(m)
}

func (x *eventSvcEventSubscribeClient) Recv() (*EventResponse, error) {
	m := new(EventResponse)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

// Server API for EventSvc service

type EventSvcServer interface {
	EventSubscribe(EventSvc_EventSubscribeServer) error
}

func RegisterEventSvcServer(s *grpc.Server, srv EventSvcServer) {
	s.RegisterService(&_EventSvc_serviceDesc, srv)
}

func _EventSvc_EventSubscribe_Handler(srv interface{}, stream grpc.ServerStream) error {
	return srv.(EventSvcServer).EventSubscribe(&eventSvcEventSubscribeServer{stream})
}

type EventSvc_EventSubscribeServer interface {
	Send(*EventResponse) error
	Recv() (*EventRequest, error)
	grpc.ServerStream
}

type eventSvcEventSubscribeServer struct {
	grpc.ServerStream
}

func (x *eventSvcEventSubscribeServer) Send(m *EventResponse) error {
	return x.ServerStream.SendMsg(m)
}

func (x *eventSvcEventSubscribeServer) Recv() (*EventRequest, error) {
	m := new(EventRequest)
	if err := x.ServerStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

var _EventSvc_serviceDesc = grpc.ServiceDesc{
	ServiceName: "pds.EventSvc",
	HandlerType: (*EventSvcServer)(nil),
	Methods:     []grpc.MethodDesc{},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "EventSubscribe",
			Handler:       _EventSvc_EventSubscribe_Handler,
			ServerStreams: true,
			ClientStreams: true,
		},
	},
	Metadata: "event.proto",
}

func (m *EventRequest) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *EventRequest) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if len(m.Request) > 0 {
		for _, msg := range m.Request {
			dAtA[i] = 0xa
			i++
			i = encodeVarintEvent(dAtA, i, uint64(msg.Size()))
			n, err := msg.MarshalTo(dAtA[i:])
			if err != nil {
				return 0, err
			}
			i += n
		}
	}
	return i, nil
}

func (m *EventRequest_EventSpec) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *EventRequest_EventSpec) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if m.EventId != 0 {
		dAtA[i] = 0x8
		i++
		i = encodeVarintEvent(dAtA, i, uint64(m.EventId))
	}
	if m.Action != 0 {
		dAtA[i] = 0x10
		i++
		i = encodeVarintEvent(dAtA, i, uint64(m.Action))
	}
	return i, nil
}

func (m *EventResponse) Marshal() (dAtA []byte, err error) {
	size := m.Size()
	dAtA = make([]byte, size)
	n, err := m.MarshalTo(dAtA)
	if err != nil {
		return nil, err
	}
	return dAtA[:n], nil
}

func (m *EventResponse) MarshalTo(dAtA []byte) (int, error) {
	var i int
	_ = i
	var l int
	_ = l
	if m.Status != 0 {
		dAtA[i] = 0x8
		i++
		i = encodeVarintEvent(dAtA, i, uint64(m.Status))
	}
	if m.EventId != 0 {
		dAtA[i] = 0x10
		i++
		i = encodeVarintEvent(dAtA, i, uint64(m.EventId))
	}
	if m.EventInfo != nil {
		nn1, err := m.EventInfo.MarshalTo(dAtA[i:])
		if err != nil {
			return 0, err
		}
		i += nn1
	}
	return i, nil
}

func (m *EventResponse_LifEventInfo) MarshalTo(dAtA []byte) (int, error) {
	i := 0
	if m.LifEventInfo != nil {
		dAtA[i] = 0x1a
		i++
		i = encodeVarintEvent(dAtA, i, uint64(m.LifEventInfo.Size()))
		n2, err := m.LifEventInfo.MarshalTo(dAtA[i:])
		if err != nil {
			return 0, err
		}
		i += n2
	}
	return i, nil
}
func (m *EventResponse_PortEventInfo) MarshalTo(dAtA []byte) (int, error) {
	i := 0
	if m.PortEventInfo != nil {
		dAtA[i] = 0x22
		i++
		i = encodeVarintEvent(dAtA, i, uint64(m.PortEventInfo.Size()))
		n3, err := m.PortEventInfo.MarshalTo(dAtA[i:])
		if err != nil {
			return 0, err
		}
		i += n3
	}
	return i, nil
}
func encodeVarintEvent(dAtA []byte, offset int, v uint64) int {
	for v >= 1<<7 {
		dAtA[offset] = uint8(v&0x7f | 0x80)
		v >>= 7
		offset++
	}
	dAtA[offset] = uint8(v)
	return offset + 1
}
func (m *EventRequest) Size() (n int) {
	var l int
	_ = l
	if len(m.Request) > 0 {
		for _, e := range m.Request {
			l = e.Size()
			n += 1 + l + sovEvent(uint64(l))
		}
	}
	return n
}

func (m *EventRequest_EventSpec) Size() (n int) {
	var l int
	_ = l
	if m.EventId != 0 {
		n += 1 + sovEvent(uint64(m.EventId))
	}
	if m.Action != 0 {
		n += 1 + sovEvent(uint64(m.Action))
	}
	return n
}

func (m *EventResponse) Size() (n int) {
	var l int
	_ = l
	if m.Status != 0 {
		n += 1 + sovEvent(uint64(m.Status))
	}
	if m.EventId != 0 {
		n += 1 + sovEvent(uint64(m.EventId))
	}
	if m.EventInfo != nil {
		n += m.EventInfo.Size()
	}
	return n
}

func (m *EventResponse_LifEventInfo) Size() (n int) {
	var l int
	_ = l
	if m.LifEventInfo != nil {
		l = m.LifEventInfo.Size()
		n += 1 + l + sovEvent(uint64(l))
	}
	return n
}
func (m *EventResponse_PortEventInfo) Size() (n int) {
	var l int
	_ = l
	if m.PortEventInfo != nil {
		l = m.PortEventInfo.Size()
		n += 1 + l + sovEvent(uint64(l))
	}
	return n
}

func sovEvent(x uint64) (n int) {
	for {
		n++
		x >>= 7
		if x == 0 {
			break
		}
	}
	return n
}
func sozEvent(x uint64) (n int) {
	return sovEvent(uint64((x << 1) ^ uint64((int64(x) >> 63))))
}
func (m *EventRequest) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowEvent
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
			return fmt.Errorf("proto: EventRequest: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: EventRequest: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field Request", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
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
				return ErrInvalidLengthEvent
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			m.Request = append(m.Request, &EventRequest_EventSpec{})
			if err := m.Request[len(m.Request)-1].Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipEvent(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthEvent
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
func (m *EventRequest_EventSpec) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowEvent
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
			return fmt.Errorf("proto: EventSpec: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: EventSpec: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 0 {
				return fmt.Errorf("proto: wrong wireType = %d for field EventId", wireType)
			}
			m.EventId = 0
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				m.EventId |= (EventId(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
		case 2:
			if wireType != 0 {
				return fmt.Errorf("proto: wrong wireType = %d for field Action", wireType)
			}
			m.Action = 0
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				m.Action |= (EventOp(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
		default:
			iNdEx = preIndex
			skippy, err := skipEvent(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthEvent
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
func (m *EventResponse) Unmarshal(dAtA []byte) error {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		preIndex := iNdEx
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return ErrIntOverflowEvent
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
			return fmt.Errorf("proto: EventResponse: wiretype end group for non-group")
		}
		if fieldNum <= 0 {
			return fmt.Errorf("proto: EventResponse: illegal tag %d (wire type %d)", fieldNum, wire)
		}
		switch fieldNum {
		case 1:
			if wireType != 0 {
				return fmt.Errorf("proto: wrong wireType = %d for field Status", wireType)
			}
			m.Status = 0
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				m.Status |= (ApiStatus(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
		case 2:
			if wireType != 0 {
				return fmt.Errorf("proto: wrong wireType = %d for field EventId", wireType)
			}
			m.EventId = 0
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
				}
				if iNdEx >= l {
					return io.ErrUnexpectedEOF
				}
				b := dAtA[iNdEx]
				iNdEx++
				m.EventId |= (EventId(b) & 0x7F) << shift
				if b < 0x80 {
					break
				}
			}
		case 3:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field LifEventInfo", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
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
				return ErrInvalidLengthEvent
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			v := &Lif{}
			if err := v.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			m.EventInfo = &EventResponse_LifEventInfo{v}
			iNdEx = postIndex
		case 4:
			if wireType != 2 {
				return fmt.Errorf("proto: wrong wireType = %d for field PortEventInfo", wireType)
			}
			var msglen int
			for shift := uint(0); ; shift += 7 {
				if shift >= 64 {
					return ErrIntOverflowEvent
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
				return ErrInvalidLengthEvent
			}
			postIndex := iNdEx + msglen
			if postIndex > l {
				return io.ErrUnexpectedEOF
			}
			v := &Port{}
			if err := v.Unmarshal(dAtA[iNdEx:postIndex]); err != nil {
				return err
			}
			m.EventInfo = &EventResponse_PortEventInfo{v}
			iNdEx = postIndex
		default:
			iNdEx = preIndex
			skippy, err := skipEvent(dAtA[iNdEx:])
			if err != nil {
				return err
			}
			if skippy < 0 {
				return ErrInvalidLengthEvent
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
func skipEvent(dAtA []byte) (n int, err error) {
	l := len(dAtA)
	iNdEx := 0
	for iNdEx < l {
		var wire uint64
		for shift := uint(0); ; shift += 7 {
			if shift >= 64 {
				return 0, ErrIntOverflowEvent
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
					return 0, ErrIntOverflowEvent
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
					return 0, ErrIntOverflowEvent
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
				return 0, ErrInvalidLengthEvent
			}
			return iNdEx, nil
		case 3:
			for {
				var innerWire uint64
				var start int = iNdEx
				for shift := uint(0); ; shift += 7 {
					if shift >= 64 {
						return 0, ErrIntOverflowEvent
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
				next, err := skipEvent(dAtA[start:])
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
	ErrInvalidLengthEvent = fmt.Errorf("proto: negative length found during unmarshaling")
	ErrIntOverflowEvent   = fmt.Errorf("proto: integer overflow")
)

func init() { proto.RegisterFile("event.proto", fileDescriptorEvent) }

var fileDescriptorEvent = []byte{
	// 459 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x7c, 0x92, 0xc1, 0x8e, 0xd2, 0x50,
	0x14, 0x86, 0xb9, 0x30, 0xc2, 0xcc, 0xa1, 0x0c, 0xe5, 0xcc, 0xa8, 0x0d, 0x26, 0x84, 0x10, 0x63,
	0x9a, 0x59, 0x54, 0xc5, 0xb8, 0x74, 0x01, 0x43, 0xcd, 0x34, 0x21, 0x6d, 0x73, 0x01, 0x8d, 0xab,
	0x06, 0xca, 0x6d, 0xd2, 0x85, 0xed, 0xb5, 0xbd, 0x4c, 0xe2, 0xdb, 0xf8, 0x2c, 0xae, 0x5c, 0x1a,
	0x9f, 0xc0, 0xf0, 0x24, 0xa6, 0xed, 0xa5, 0xd0, 0x99, 0xc4, 0x0d, 0xb9, 0xe7, 0xff, 0xce, 0xf9,
	0x39, 0x7f, 0x7a, 0xa0, 0xcd, 0xee, 0x59, 0x24, 0x0c, 0x9e, 0xc4, 0x22, 0xc6, 0x06, 0xdf, 0xa6,
	0xfd, 0xee, 0x57, 0x26, 0xd6, 0xaf, 0xb3, 0x9f, 0x42, 0xed, 0xb7, 0xc5, 0x77, 0xce, 0x52, 0x59,
	0x74, 0xc3, 0x48, 0xb0, 0x24, 0x58, 0xfb, 0x4c, 0x0a, 0xc0, 0xe3, 0x44, 0xce, 0x8f, 0x7e, 0x10,
	0x50, 0xcc, 0xcc, 0x8f, 0xb2, 0x6f, 0x3b, 0x96, 0x0a, 0x7c, 0x0f, 0x2d, 0xf9, 0xd4, 0xc8, 0xb0,
	0xa1, 0xb7, 0xc7, 0x2f, 0x0c, 0xbe, 0x4d, 0x8d, 0xd3, 0x9e, 0xa2, 0x58, 0x70, 0xe6, 0xd3, 0x43,
	0x6f, 0xff, 0x0b, 0x5c, 0x94, 0x2a, 0xbe, 0x82, 0x56, 0x5e, 0x58, 0x5b, 0x8d, 0x0c, 0x89, 0x7e,
	0x39, 0x56, 0x8e, 0x1e, 0xd6, 0x96, 0x1e, 0x20, 0xbe, 0x84, 0xe6, 0xc4, 0x17, 0x61, 0x1c, 0x69,
	0xf5, 0x87, 0x6d, 0x0e, 0xa7, 0x92, 0x8d, 0xfe, 0x10, 0xe8, 0xc8, 0xbf, 0x4f, 0x79, 0x1c, 0xa5,
	0x0c, 0x75, 0x68, 0x2e, 0xc4, 0x5a, 0xec, 0x52, 0x69, 0xaf, 0x1a, 0x45, 0xde, 0x09, 0x0f, 0x0b,
	0x9d, 0x4a, 0x7e, 0xba, 0x49, 0xfd, 0x7f, 0x9b, 0x18, 0xa0, 0xcc, 0xc3, 0xa0, 0xa8, 0xa2, 0x20,
	0xd6, 0x1a, 0x43, 0xa2, 0xb7, 0xc7, 0xe7, 0x79, 0xf3, 0x3c, 0x0c, 0xee, 0x6a, 0xb4, 0xc2, 0xf1,
	0x2d, 0x74, 0xdc, 0x38, 0x11, 0xc7, 0x81, 0xb3, 0x7c, 0xe0, 0x22, 0x1f, 0xc8, 0xc8, 0x5d, 0x8d,
	0x56, 0x3b, 0xa6, 0x0a, 0x40, 0xfe, 0xe1, 0xbc, 0x30, 0x0a, 0xe2, 0x9b, 0x9f, 0xa4, 0xdc, 0x0c,
	0x7b, 0xd0, 0x31, 0x3f, 0x99, 0xf6, 0xd2, 0xb3, 0x66, 0x9e, 0xed, 0xd8, 0xa6, 0x5a, 0x43, 0x0d,
	0xae, 0x4b, 0xc9, 0x75, 0xe8, 0xd2, 0xbb, 0xa5, 0xe6, 0x64, 0x69, 0xaa, 0x04, 0xaf, 0x41, 0xad,
	0x92, 0x95, 0xab, 0xd6, 0xf1, 0x19, 0x60, 0x55, 0x9d, 0x39, 0x9f, 0x6d, 0xb5, 0x81, 0xcf, 0xe1,
	0xaa, 0xd4, 0xe7, 0xd6, 0xc7, 0x83, 0xcd, 0xd9, 0x23, 0xb0, 0x72, 0x67, 0x19, 0x78, 0x82, 0x57,
	0xd0, 0x7d, 0x00, 0xd4, 0x26, 0x3e, 0x85, 0x5e, 0x45, 0xcc, 0xdd, 0x5b, 0x37, 0xb6, 0xcc, 0xe0,
	0xf0, 0x63, 0x06, 0xc7, 0x3d, 0x64, 0x28, 0x77, 0x72, 0x5c, 0x6f, 0xb1, 0x9a, 0x2e, 0x6e, 0xa9,
	0x35, 0xcd, 0x12, 0x94, 0xd9, 0x1c, 0xd7, 0x5b, 0xd9, 0x47, 0x52, 0x1f, 0x5b, 0x70, 0x5e, 0x1c,
	0xd1, 0xbd, 0x8f, 0x1f, 0xe0, 0xb2, 0x78, 0xef, 0x36, 0xa9, 0x9f, 0x84, 0x1b, 0x86, 0xbd, 0x47,
	0x87, 0xd8, 0xc7, 0x53, 0xa9, 0x38, 0x8e, 0x51, 0x4d, 0x27, 0x6f, 0xc8, 0x54, 0xf9, 0xb5, 0x1f,
	0x90, 0xdf, 0xfb, 0x01, 0xf9, 0xbb, 0x1f, 0x90, 0x4d, 0x33, 0x3f, 0xf6, 0x77, 0xff, 0x02, 0x00,
	0x00, 0xff, 0xff, 0xc2, 0x79, 0xfc, 0xd4, 0x3b, 0x03, 0x00, 0x00,
}
