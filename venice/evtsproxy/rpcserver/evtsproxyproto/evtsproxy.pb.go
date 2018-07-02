// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: evtsproxy.proto

/*
	Package evtsproxyproto is a generated protocol buffer package.

	It is generated from these files:
		evtsproxy.proto

	It has these top-level messages:
*/
package evtsproxyproto

import proto "github.com/gogo/protobuf/proto"
import fmt "fmt"
import math "math"
import _ "google.golang.org/genproto/googleapis/api/annotations"
import _ "github.com/pensando/sw/venice/utils/apigen/annotations"
import _ "github.com/gogo/protobuf/gogoproto"
import events "github.com/pensando/sw/api/generated/events"
import api1 "github.com/pensando/sw/api"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.GoGoProtoPackageIsVersion2 // please upgrade the proto package

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for EventsProxyAPI service

type EventsProxyAPIClient interface {
	// recorder will forward the event to proxy which will be channeled to the
	// dispatcher where the event gets deduped and distributed to the writers
	// (events manager, events exporter, etc.). event recorder connects only
	// with proxy; proxy handles rest of the workflow.
	ForwardEvent(ctx context.Context, in *events.Event, opts ...grpc.CallOption) (*api1.Empty, error)
	// recorder can buffer events when the proxy is unavailable and send
	// all of them at once when the proxy comes back online.
	ForwardEvents(ctx context.Context, in *events.EventList, opts ...grpc.CallOption) (*api1.Empty, error)
}

type eventsProxyAPIClient struct {
	cc *grpc.ClientConn
}

func NewEventsProxyAPIClient(cc *grpc.ClientConn) EventsProxyAPIClient {
	return &eventsProxyAPIClient{cc}
}

func (c *eventsProxyAPIClient) ForwardEvent(ctx context.Context, in *events.Event, opts ...grpc.CallOption) (*api1.Empty, error) {
	out := new(api1.Empty)
	err := grpc.Invoke(ctx, "/evtsproxyproto.EventsProxyAPI/ForwardEvent", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *eventsProxyAPIClient) ForwardEvents(ctx context.Context, in *events.EventList, opts ...grpc.CallOption) (*api1.Empty, error) {
	out := new(api1.Empty)
	err := grpc.Invoke(ctx, "/evtsproxyproto.EventsProxyAPI/ForwardEvents", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for EventsProxyAPI service

type EventsProxyAPIServer interface {
	// recorder will forward the event to proxy which will be channeled to the
	// dispatcher where the event gets deduped and distributed to the writers
	// (events manager, events exporter, etc.). event recorder connects only
	// with proxy; proxy handles rest of the workflow.
	ForwardEvent(context.Context, *events.Event) (*api1.Empty, error)
	// recorder can buffer events when the proxy is unavailable and send
	// all of them at once when the proxy comes back online.
	ForwardEvents(context.Context, *events.EventList) (*api1.Empty, error)
}

func RegisterEventsProxyAPIServer(s *grpc.Server, srv EventsProxyAPIServer) {
	s.RegisterService(&_EventsProxyAPI_serviceDesc, srv)
}

func _EventsProxyAPI_ForwardEvent_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(events.Event)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(EventsProxyAPIServer).ForwardEvent(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/evtsproxyproto.EventsProxyAPI/ForwardEvent",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(EventsProxyAPIServer).ForwardEvent(ctx, req.(*events.Event))
	}
	return interceptor(ctx, in, info, handler)
}

func _EventsProxyAPI_ForwardEvents_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(events.EventList)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(EventsProxyAPIServer).ForwardEvents(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/evtsproxyproto.EventsProxyAPI/ForwardEvents",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(EventsProxyAPIServer).ForwardEvents(ctx, req.(*events.EventList))
	}
	return interceptor(ctx, in, info, handler)
}

var _EventsProxyAPI_serviceDesc = grpc.ServiceDesc{
	ServiceName: "evtsproxyproto.EventsProxyAPI",
	HandlerType: (*EventsProxyAPIServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "ForwardEvent",
			Handler:    _EventsProxyAPI_ForwardEvent_Handler,
		},
		{
			MethodName: "ForwardEvents",
			Handler:    _EventsProxyAPI_ForwardEvents_Handler,
		},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "evtsproxy.proto",
}

func init() { proto.RegisterFile("evtsproxy.proto", fileDescriptorEvtsproxy) }

var fileDescriptorEvtsproxy = []byte{
	// 257 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x7c, 0x90, 0x41, 0x4a, 0xc4, 0x30,
	0x14, 0x86, 0xa7, 0x1b, 0x17, 0xc1, 0x19, 0x35, 0xcb, 0x22, 0x5d, 0xb8, 0x71, 0x21, 0x26, 0xa2,
	0x27, 0x50, 0xa8, 0x20, 0xb8, 0xe8, 0x15, 0xd2, 0xf6, 0x19, 0x03, 0xed, 0x7b, 0xa1, 0x49, 0x3b,
	0xce, 0x4d, 0x3c, 0x92, 0x4b, 0x8f, 0x20, 0xf5, 0x22, 0xd2, 0x37, 0x33, 0x52, 0x05, 0xdd, 0xe5,
	0x7f, 0xf9, 0xfe, 0x2f, 0x8f, 0x88, 0x23, 0x18, 0x62, 0xf0, 0x1d, 0xbd, 0x6c, 0x94, 0xef, 0x28,
	0x92, 0x5c, 0x7d, 0x0f, 0x38, 0xa7, 0xa7, 0x96, 0xc8, 0x36, 0xa0, 0x8d, 0x77, 0xda, 0x20, 0x52,
	0x34, 0xd1, 0x11, 0x86, 0x2d, 0x9d, 0xe6, 0xd6, 0xc5, 0xe7, 0xbe, 0x54, 0x15, 0xb5, 0xda, 0x03,
	0x06, 0x83, 0x35, 0xe9, 0xb0, 0xd6, 0x03, 0xa0, 0xab, 0x40, 0xf7, 0xd1, 0x35, 0x61, 0xaa, 0x5a,
	0xc0, 0x79, 0x5b, 0x3b, 0xac, 0x9a, 0xbe, 0x86, 0xbd, 0xe6, 0x72, 0xa6, 0xb1, 0x64, 0x49, 0xf3,
	0xb8, 0xec, 0x9f, 0x38, 0x71, 0xe0, 0xd3, 0x0e, 0x57, 0x7f, 0xbc, 0x3a, 0xed, 0xc8, 0x44, 0xd0,
	0x30, 0x00, 0xc6, 0xbd, 0xfe, 0xfc, 0x1f, 0xbe, 0xa2, 0xb6, 0x25, 0xdc, 0x82, 0xd7, 0x24, 0x56,
	0x39, 0x17, 0x8b, 0xe9, 0x03, 0x6e, 0x8b, 0x07, 0x79, 0x21, 0x0e, 0xef, 0xa9, 0x5b, 0x9b, 0xae,
	0xe6, 0x0b, 0xb9, 0x54, 0x3b, 0x33, 0xc7, 0x54, 0x28, 0xe3, 0x9d, 0xca, 0x5b, 0x1f, 0x37, 0x67,
	0x0b, 0x79, 0x25, 0x96, 0x73, 0x38, 0xc8, 0x93, 0x1f, 0xf4, 0xa3, 0x0b, 0xbf, 0x1a, 0x77, 0xc7,
	0x6f, 0x63, 0x96, 0xbc, 0x8f, 0x59, 0xf2, 0x31, 0x66, 0xc9, 0xeb, 0x67, 0xb6, 0x28, 0x92, 0xf2,
	0x80, 0x77, 0xb9, 0xf9, 0x0a, 0x00, 0x00, 0xff, 0xff, 0x61, 0x56, 0xac, 0xa9, 0x9b, 0x01, 0x00,
	0x00,
}
