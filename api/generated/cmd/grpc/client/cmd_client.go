package grpcclient

import (
	"context"

	"github.com/go-kit/kit/endpoint"
	"github.com/go-kit/kit/tracing/opentracing"
	grpctransport "github.com/go-kit/kit/transport/grpc"
	stdopentracing "github.com/opentracing/opentracing-go"
	cmd "github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/utils/log"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"
)

func dummyBefore(ctx context.Context, md *metadata.MD) context.Context {
	xmd, ok := metadata.FromContext(ctx)
	if ok {
		cmd := metadata.Join(*md, xmd)
		*md = cmd
	}
	return ctx
}

func NewCmdV1(conn *grpc.ClientConn, logger log.Logger) cmd.ServiceCmdV1 {

	var l_GetNodeListEndpoint endpoint.Endpoint
	{
		l_GetNodeListEndpoint = grpctransport.NewClient(
			conn,
			"cmd.CmdV1",
			"GetNodeList",
			cmd.EndcodeGrpcReqNodeList,
			cmd.DecodeGrpcRespNodeList,
			cmd.NodeList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetNodeListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "CmdV1:GetNodeList")(l_GetNodeListEndpoint)
	}
	var l_NodeOperEndpoint endpoint.Endpoint
	{
		l_NodeOperEndpoint = grpctransport.NewClient(
			conn,
			"cmd.CmdV1",
			"NodeOper",
			cmd.EndcodeGrpcReqNode,
			cmd.DecodeGrpcRespNode,
			cmd.Node{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_NodeOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "CmdV1:NodeOper")(l_NodeOperEndpoint)
	}
	var l_ClusterOperEndpoint endpoint.Endpoint
	{
		l_ClusterOperEndpoint = grpctransport.NewClient(
			conn,
			"cmd.CmdV1",
			"ClusterOper",
			cmd.EndcodeGrpcReqCluster,
			cmd.DecodeGrpcRespCluster,
			cmd.Cluster{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_ClusterOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "CmdV1:ClusterOper")(l_ClusterOperEndpoint)
	}
	return cmd.Endpoints_CmdV1{

		GetNodeListEndpoint: l_GetNodeListEndpoint,
		NodeOperEndpoint:    l_NodeOperEndpoint,
		ClusterOperEndpoint: l_ClusterOperEndpoint,
	}
}

func NewCmdV1Backend(conn *grpc.ClientConn, logger log.Logger) cmd.ServiceCmdV1 {
	cl := NewCmdV1(conn, logger)
	cl = cmd.LoggingCmdV1Middleware(logger)(cl)
	return cl
}
