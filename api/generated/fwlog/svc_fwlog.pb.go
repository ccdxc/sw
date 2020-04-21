// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: svc_fwlog.proto

package fwlog

import proto "github.com/gogo/protobuf/proto"
import fmt "fmt"
import math "math"
import _ "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"
import _ "github.com/pensando/sw/venice/utils/apigen/annotations"
import _ "github.com/gogo/protobuf/gogoproto"
import api "github.com/pensando/sw/api"

import (
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for FwLogV1 service

type FwLogV1Client interface {
	AutoWatchSvcFwLogV1(ctx context.Context, in *api.ListWatchOptions, opts ...grpc.CallOption) (FwLogV1_AutoWatchSvcFwLogV1Client, error)
	// fwlog/v1/tenants/default/objects/<objectName>
	DownloadFwLogFileContent(ctx context.Context, in *api.ListWatchOptions, opts ...grpc.CallOption) (*FwLogList, error)
	// Queries firewall logs
	GetLogs(ctx context.Context, in *FwLogQuery, opts ...grpc.CallOption) (*FwLogList, error)
}

type fwLogV1Client struct {
	cc *grpc.ClientConn
}

func NewFwLogV1Client(cc *grpc.ClientConn) FwLogV1Client {
	return &fwLogV1Client{cc}
}

func (c *fwLogV1Client) AutoWatchSvcFwLogV1(ctx context.Context, in *api.ListWatchOptions, opts ...grpc.CallOption) (FwLogV1_AutoWatchSvcFwLogV1Client, error) {
	stream, err := grpc.NewClientStream(ctx, &_FwLogV1_serviceDesc.Streams[0], c.cc, "/fwlog.FwLogV1/AutoWatchSvcFwLogV1", opts...)
	if err != nil {
		return nil, err
	}
	x := &fwLogV1AutoWatchSvcFwLogV1Client{stream}
	if err := x.ClientStream.SendMsg(in); err != nil {
		return nil, err
	}
	if err := x.ClientStream.CloseSend(); err != nil {
		return nil, err
	}
	return x, nil
}

type FwLogV1_AutoWatchSvcFwLogV1Client interface {
	Recv() (*api.WatchEventList, error)
	grpc.ClientStream
}

type fwLogV1AutoWatchSvcFwLogV1Client struct {
	grpc.ClientStream
}

func (x *fwLogV1AutoWatchSvcFwLogV1Client) Recv() (*api.WatchEventList, error) {
	m := new(api.WatchEventList)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

func (c *fwLogV1Client) DownloadFwLogFileContent(ctx context.Context, in *api.ListWatchOptions, opts ...grpc.CallOption) (*FwLogList, error) {
	out := new(FwLogList)
	err := grpc.Invoke(ctx, "/fwlog.FwLogV1/DownloadFwLogFileContent", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *fwLogV1Client) GetLogs(ctx context.Context, in *FwLogQuery, opts ...grpc.CallOption) (*FwLogList, error) {
	out := new(FwLogList)
	err := grpc.Invoke(ctx, "/fwlog.FwLogV1/GetLogs", in, out, c.cc, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// Server API for FwLogV1 service

type FwLogV1Server interface {
	AutoWatchSvcFwLogV1(*api.ListWatchOptions, FwLogV1_AutoWatchSvcFwLogV1Server) error
	// fwlog/v1/tenants/default/objects/<objectName>
	DownloadFwLogFileContent(context.Context, *api.ListWatchOptions) (*FwLogList, error)
	// Queries firewall logs
	GetLogs(context.Context, *FwLogQuery) (*FwLogList, error)
}

func RegisterFwLogV1Server(s *grpc.Server, srv FwLogV1Server) {
	s.RegisterService(&_FwLogV1_serviceDesc, srv)
}

func _FwLogV1_AutoWatchSvcFwLogV1_Handler(srv interface{}, stream grpc.ServerStream) error {
	m := new(api.ListWatchOptions)
	if err := stream.RecvMsg(m); err != nil {
		return err
	}
	return srv.(FwLogV1Server).AutoWatchSvcFwLogV1(m, &fwLogV1AutoWatchSvcFwLogV1Server{stream})
}

type FwLogV1_AutoWatchSvcFwLogV1Server interface {
	Send(*api.WatchEventList) error
	grpc.ServerStream
}

type fwLogV1AutoWatchSvcFwLogV1Server struct {
	grpc.ServerStream
}

func (x *fwLogV1AutoWatchSvcFwLogV1Server) Send(m *api.WatchEventList) error {
	return x.ServerStream.SendMsg(m)
}

func _FwLogV1_DownloadFwLogFileContent_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(api.ListWatchOptions)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(FwLogV1Server).DownloadFwLogFileContent(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/fwlog.FwLogV1/DownloadFwLogFileContent",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(FwLogV1Server).DownloadFwLogFileContent(ctx, req.(*api.ListWatchOptions))
	}
	return interceptor(ctx, in, info, handler)
}

func _FwLogV1_GetLogs_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(FwLogQuery)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(FwLogV1Server).GetLogs(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/fwlog.FwLogV1/GetLogs",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(FwLogV1Server).GetLogs(ctx, req.(*FwLogQuery))
	}
	return interceptor(ctx, in, info, handler)
}

var _FwLogV1_serviceDesc = grpc.ServiceDesc{
	ServiceName: "fwlog.FwLogV1",
	HandlerType: (*FwLogV1Server)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "DownloadFwLogFileContent",
			Handler:    _FwLogV1_DownloadFwLogFileContent_Handler,
		},
		{
			MethodName: "GetLogs",
			Handler:    _FwLogV1_GetLogs_Handler,
		},
	},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "AutoWatchSvcFwLogV1",
			Handler:       _FwLogV1_AutoWatchSvcFwLogV1_Handler,
			ServerStreams: true,
		},
	},
	Metadata: "svc_fwlog.proto",
}

func init() { proto.RegisterFile("svc_fwlog.proto", fileDescriptorSvcFwlog) }

var fileDescriptorSvcFwlog = []byte{
	// 414 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x74, 0x92, 0x31, 0x8b, 0x13, 0x41,
	0x14, 0xc7, 0x33, 0x81, 0xcb, 0xc9, 0x2a, 0x68, 0x26, 0x1c, 0xb8, 0xab, 0x6c, 0x71, 0x68, 0x23,
	0x64, 0xc7, 0xd3, 0x4e, 0xab, 0x53, 0xef, 0x2c, 0x0c, 0x46, 0x51, 0x22, 0xa4, 0x91, 0xd9, 0xcd,
	0xcb, 0x64, 0x64, 0x33, 0x6f, 0xcd, 0xcc, 0xee, 0x12, 0x24, 0xcd, 0x1a, 0xfc, 0x00, 0xda, 0x59,
	0x59, 0x5b, 0x5a, 0xd9, 0x69, 0x69, 0x29, 0xf8, 0x05, 0x64, 0xb1, 0xf0, 0x63, 0xc8, 0xce, 0x26,
	0xb0, 0xe0, 0xa5, 0x7b, 0xff, 0xdf, 0xfc, 0xdf, 0xff, 0xbd, 0x19, 0xc6, 0xb9, 0xa8, 0xb3, 0xe8,
	0xe5, 0x34, 0x8f, 0x51, 0x04, 0xc9, 0x02, 0x0d, 0xd2, 0x3d, 0x2b, 0xbc, 0xab, 0x02, 0x51, 0xc4,
	0xc0, 0x78, 0x22, 0x19, 0x57, 0x0a, 0x0d, 0x37, 0x12, 0x95, 0xae, 0x4d, 0xde, 0x89, 0x90, 0x66,
	0x96, 0x86, 0x41, 0x84, 0x73, 0x96, 0x80, 0xd2, 0x5c, 0x4d, 0x90, 0xe9, 0x9c, 0x65, 0xa0, 0x64,
	0x04, 0x2c, 0x35, 0x32, 0xd6, 0x55, 0xab, 0x00, 0xd5, 0xec, 0x66, 0x52, 0x45, 0x71, 0x3a, 0x81,
	0x6d, 0x4c, 0xbf, 0x11, 0x23, 0x50, 0x20, 0xb3, 0x38, 0x4c, 0xa7, 0x56, 0x59, 0x61, 0xab, 0x8d,
	0xfd, 0xfa, 0x8e, 0xa9, 0xd5, 0x8e, 0x73, 0x30, 0x7c, 0x63, 0x3b, 0xdf, 0xb8, 0xce, 0xad, 0x6f,
	0x6d, 0x67, 0xff, 0x34, 0x1f, 0xa0, 0x18, 0x1d, 0xd1, 0x91, 0xd3, 0x3b, 0x4e, 0x0d, 0xbe, 0xe0,
	0x26, 0x9a, 0x3d, 0xcb, 0xa2, 0x2d, 0x3e, 0x08, 0x78, 0x22, 0x83, 0x81, 0xd4, 0xc6, 0x9e, 0x0c,
	0x13, 0xbb, 0xab, 0xd7, 0xb3, 0xd8, 0xa2, 0x93, 0x0c, 0x94, 0xa9, 0x0c, 0x87, 0xdd, 0x2f, 0x6b,
	0x77, 0x2f, 0xaf, 0xd8, 0xd7, 0xb5, 0x4b, 0xbe, 0xaf, 0xdd, 0xd6, 0x4d, 0x42, 0xdf, 0x11, 0xe7,
	0xf2, 0x03, 0xcc, 0x55, 0x8c, 0x7c, 0x62, 0x43, 0x4f, 0x65, 0x0c, 0xf7, 0x51, 0x19, 0x50, 0x66,
	0x57, 0xfa, 0xa5, 0xa0, 0xde, 0xd2, 0xfa, 0x6d, 0xf4, 0x71, 0xf1, 0xeb, 0xcf, 0x87, 0xf6, 0x5d,
	0x7a, 0x8d, 0x19, 0x50, 0x5c, 0x19, 0xcd, 0xde, 0x0c, 0x83, 0xe7, 0xb6, 0x5c, 0x31, 0x0c, 0x5f,
	0x41, 0x54, 0xa3, 0xc7, 0x7c, 0x0e, 0xab, 0x71, 0x8f, 0x76, 0xff, 0x83, 0xf4, 0x91, 0xb3, 0xff,
	0x10, 0xcc, 0x00, 0x85, 0xa6, 0xdd, 0x66, 0xfe, 0xd3, 0x14, 0x16, 0xcb, 0x33, 0x46, 0x5e, 0xb1,
	0x23, 0x0f, 0x0e, 0x3b, 0xec, 0x75, 0xe5, 0xb8, 0x43, 0x6e, 0x8c, 0xcf, 0xd1, 0x8d, 0xf0, 0x3a,
	0xef, 0xdf, 0xba, 0xed, 0xec, 0xe8, 0x5e, 0xff, 0x63, 0xe1, 0x5e, 0x48, 0x40, 0xf5, 0x75, 0xb2,
	0x14, 0x31, 0xd7, 0xfa, 0x53, 0xe1, 0xb6, 0x3e, 0x17, 0x6e, 0xfd, 0x51, 0x7e, 0x94, 0x3e, 0xf9,
	0x59, 0xfa, 0xe4, 0x77, 0xe9, 0x93, 0xbf, 0xa5, 0xdf, 0x7a, 0x42, 0xc2, 0x8e, 0x7d, 0xf9, 0xdb,
	0xff, 0x02, 0x00, 0x00, 0xff, 0xff, 0xad, 0x64, 0xab, 0x3b, 0x5b, 0x02, 0x00, 0x00,
}
