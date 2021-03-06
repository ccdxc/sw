// Code generated by protoc-gen-gogo. DO NOT EDIT.
// source: diagnostics.proto

/*
	Package tsproto is a generated protocol buffer package.

	It is generated from these files:
		diagnostics.proto

	It has these top-level messages:
*/
package tsproto

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import _ "google.golang.org/genproto/googleapis/api/annotations"
import _ "github.com/pensando/sw/venice/utils/apigen/annotations"
import _ "github.com/gogo/protobuf/gogoproto"
import api "github.com/pensando/sw/api"
import diagnostics "github.com/pensando/sw/api/generated/diagnostics"

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
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

// Reference imports to suppress errors if they are not otherwise used.
var _ context.Context
var _ grpc.ClientConn

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
const _ = grpc.SupportPackageIsVersion4

// Client API for DiagnosticsApi service

type DiagnosticsApiClient interface {
	WatchModule(ctx context.Context, in *api.ListWatchOptions, opts ...grpc.CallOption) (DiagnosticsApi_WatchModuleClient, error)
}

type diagnosticsApiClient struct {
	cc *grpc.ClientConn
}

func NewDiagnosticsApiClient(cc *grpc.ClientConn) DiagnosticsApiClient {
	return &diagnosticsApiClient{cc}
}

func (c *diagnosticsApiClient) WatchModule(ctx context.Context, in *api.ListWatchOptions, opts ...grpc.CallOption) (DiagnosticsApi_WatchModuleClient, error) {
	stream, err := grpc.NewClientStream(ctx, &_DiagnosticsApi_serviceDesc.Streams[0], c.cc, "/tsproto.DiagnosticsApi/WatchModule", opts...)
	if err != nil {
		return nil, err
	}
	x := &diagnosticsApiWatchModuleClient{stream}
	if err := x.ClientStream.SendMsg(in); err != nil {
		return nil, err
	}
	if err := x.ClientStream.CloseSend(); err != nil {
		return nil, err
	}
	return x, nil
}

type DiagnosticsApi_WatchModuleClient interface {
	Recv() (*diagnostics.Module, error)
	grpc.ClientStream
}

type diagnosticsApiWatchModuleClient struct {
	grpc.ClientStream
}

func (x *diagnosticsApiWatchModuleClient) Recv() (*diagnostics.Module, error) {
	m := new(diagnostics.Module)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

// Server API for DiagnosticsApi service

type DiagnosticsApiServer interface {
	WatchModule(*api.ListWatchOptions, DiagnosticsApi_WatchModuleServer) error
}

func RegisterDiagnosticsApiServer(s *grpc.Server, srv DiagnosticsApiServer) {
	s.RegisterService(&_DiagnosticsApi_serviceDesc, srv)
}

func _DiagnosticsApi_WatchModule_Handler(srv interface{}, stream grpc.ServerStream) error {
	m := new(api.ListWatchOptions)
	if err := stream.RecvMsg(m); err != nil {
		return err
	}
	return srv.(DiagnosticsApiServer).WatchModule(m, &diagnosticsApiWatchModuleServer{stream})
}

type DiagnosticsApi_WatchModuleServer interface {
	Send(*diagnostics.Module) error
	grpc.ServerStream
}

type diagnosticsApiWatchModuleServer struct {
	grpc.ServerStream
}

func (x *diagnosticsApiWatchModuleServer) Send(m *diagnostics.Module) error {
	return x.ServerStream.SendMsg(m)
}

var _DiagnosticsApi_serviceDesc = grpc.ServiceDesc{
	ServiceName: "tsproto.DiagnosticsApi",
	HandlerType: (*DiagnosticsApiServer)(nil),
	Methods:     []grpc.MethodDesc{},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "WatchModule",
			Handler:       _DiagnosticsApi_WatchModule_Handler,
			ServerStreams: true,
		},
	},
	Metadata: "diagnostics.proto",
}

func init() { proto.RegisterFile("diagnostics.proto", fileDescriptorDiagnostics) }

var fileDescriptorDiagnostics = []byte{
	// 244 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x7c, 0xd0, 0x31, 0x4b, 0xc5, 0x40,
	0x0c, 0x00, 0xe0, 0xd7, 0x45, 0xa1, 0x82, 0x68, 0xc5, 0xa5, 0x48, 0x07, 0xc1, 0xd1, 0x8b, 0xe8,
	0xec, 0xa0, 0xe8, 0xa6, 0x3c, 0x37, 0xe7, 0xeb, 0xf5, 0xbc, 0x17, 0x68, 0x93, 0xc3, 0xcb, 0xe9,
	0x5f, 0xf1, 0x27, 0x39, 0xfa, 0x13, 0xa4, 0xfe, 0x11, 0x79, 0x79, 0x15, 0xbb, 0xe8, 0x96, 0x84,
	0x2f, 0xc9, 0x5d, 0xca, 0xfd, 0x0e, 0x6d, 0x20, 0x4e, 0x82, 0x2e, 0x99, 0xf8, 0xcc, 0xc2, 0xd5,
	0xb6, 0x24, 0x0d, 0xea, 0xa3, 0xc0, 0x1c, 0x7a, 0x0f, 0x36, 0x22, 0x58, 0x22, 0x16, 0x2b, 0xc8,
	0x34, 0xb1, 0xfa, 0x36, 0xa0, 0xac, 0x72, 0x6b, 0x1c, 0x0f, 0x10, 0x3d, 0x25, 0x4b, 0x1d, 0x43,
	0x7a, 0x85, 0x17, 0x4f, 0xe8, 0x3c, 0x64, 0xc1, 0x3e, 0xad, 0x5b, 0x83, 0xa7, 0x79, 0x37, 0x20,
	0xb9, 0x3e, 0x77, 0xfe, 0x67, 0xcc, 0xe9, 0x6c, 0x4c, 0xe0, 0xc0, 0xa0, 0xe5, 0x36, 0x3f, 0x69,
	0xa6, 0x89, 0x46, 0x13, 0x3f, 0xf9, 0x63, 0xeb, 0xfa, 0x8d, 0x83, 0x17, 0x3b, 0x31, 0xf3, 0x0f,
	0x53, 0x91, 0x60, 0xe0, 0x2e, 0xf7, 0x7e, 0xe3, 0xcf, 0x97, 0xe5, 0xee, 0xcd, 0xef, 0x21, 0xae,
	0x22, 0x56, 0x97, 0xe5, 0xce, 0xa3, 0x15, 0xb7, 0xba, 0x57, 0x56, 0x1d, 0x1a, 0x1b, 0xd1, 0xdc,
	0x61, 0x12, 0xad, 0x2e, 0xa3, 0x7e, 0xa6, 0x3e, 0x30, 0xf3, 0xfb, 0x6d, 0xec, 0xf1, 0xe2, 0xac,
	0xb8, 0xde, 0x7b, 0x1f, 0x9b, 0xe2, 0x63, 0x6c, 0x8a, 0xcf, 0xb1, 0x29, 0xde, 0xbe, 0x9a, 0xc5,
	0x43, 0xd1, 0x6e, 0xe9, 0xae, 0x8b, 0xef, 0x00, 0x00, 0x00, 0xff, 0xff, 0xee, 0xb9, 0xa7, 0x04,
	0x74, 0x01, 0x00, 0x00,
}
