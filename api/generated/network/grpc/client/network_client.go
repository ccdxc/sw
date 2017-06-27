package grpcclient

import (
	"context"

	"github.com/go-kit/kit/endpoint"
	"github.com/go-kit/kit/tracing/opentracing"
	grpctransport "github.com/go-kit/kit/transport/grpc"
	stdopentracing "github.com/opentracing/opentracing-go"
	network "github.com/pensando/sw/api/generated/network"
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

func NewTenantV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceTenantV1 {

	var l_GetTenantListEndpoint endpoint.Endpoint
	{
		l_GetTenantListEndpoint = grpctransport.NewClient(
			conn,
			"network.TenantV1",
			"GetTenantList",
			network.EndcodeGrpcReqTenantList,
			network.DecodeGrpcRespTenantList,
			network.TenantList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetTenantListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "TenantV1:GetTenantList")(l_GetTenantListEndpoint)
	}
	var l_TenantOperEndpoint endpoint.Endpoint
	{
		l_TenantOperEndpoint = grpctransport.NewClient(
			conn,
			"network.TenantV1",
			"TenantOper",
			network.EndcodeGrpcReqTenant,
			network.DecodeGrpcRespTenant,
			network.Tenant{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_TenantOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "TenantV1:TenantOper")(l_TenantOperEndpoint)
	}
	return network.Endpoints_TenantV1{

		GetTenantListEndpoint: l_GetTenantListEndpoint,
		TenantOperEndpoint:    l_TenantOperEndpoint,
	}
}

func NewTenantV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceTenantV1 {
	cl := NewTenantV1(conn, logger)
	cl = network.LoggingTenantV1Middleware(logger)(cl)
	return cl
}

func NewNetworkV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceNetworkV1 {

	var l_GetNetworkListEndpoint endpoint.Endpoint
	{
		l_GetNetworkListEndpoint = grpctransport.NewClient(
			conn,
			"network.NetworkV1",
			"GetNetworkList",
			network.EndcodeGrpcReqNetworkList,
			network.DecodeGrpcRespNetworkList,
			network.NetworkList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetNetworkListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "NetworkV1:GetNetworkList")(l_GetNetworkListEndpoint)
	}
	var l_NetworkOperEndpoint endpoint.Endpoint
	{
		l_NetworkOperEndpoint = grpctransport.NewClient(
			conn,
			"network.NetworkV1",
			"NetworkOper",
			network.EndcodeGrpcReqNetwork,
			network.DecodeGrpcRespNetwork,
			network.Network{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_NetworkOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "NetworkV1:NetworkOper")(l_NetworkOperEndpoint)
	}
	return network.Endpoints_NetworkV1{

		GetNetworkListEndpoint: l_GetNetworkListEndpoint,
		NetworkOperEndpoint:    l_NetworkOperEndpoint,
	}
}

func NewNetworkV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceNetworkV1 {
	cl := NewNetworkV1(conn, logger)
	cl = network.LoggingNetworkV1Middleware(logger)(cl)
	return cl
}

func NewSecurityGroupV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceSecurityGroupV1 {

	var l_GetSecurityGroupListEndpoint endpoint.Endpoint
	{
		l_GetSecurityGroupListEndpoint = grpctransport.NewClient(
			conn,
			"network.SecurityGroupV1",
			"GetSecurityGroupList",
			network.EndcodeGrpcReqSecurityGroupList,
			network.DecodeGrpcRespSecurityGroupList,
			network.SecurityGroupList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetSecurityGroupListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "SecurityGroupV1:GetSecurityGroupList")(l_GetSecurityGroupListEndpoint)
	}
	var l_SecurityGroupOperEndpoint endpoint.Endpoint
	{
		l_SecurityGroupOperEndpoint = grpctransport.NewClient(
			conn,
			"network.SecurityGroupV1",
			"SecurityGroupOper",
			network.EndcodeGrpcReqSecurityGroup,
			network.DecodeGrpcRespSecurityGroup,
			network.SecurityGroup{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_SecurityGroupOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "SecurityGroupV1:SecurityGroupOper")(l_SecurityGroupOperEndpoint)
	}
	return network.Endpoints_SecurityGroupV1{

		GetSecurityGroupListEndpoint: l_GetSecurityGroupListEndpoint,
		SecurityGroupOperEndpoint:    l_SecurityGroupOperEndpoint,
	}
}

func NewSecurityGroupV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceSecurityGroupV1 {
	cl := NewSecurityGroupV1(conn, logger)
	cl = network.LoggingSecurityGroupV1Middleware(logger)(cl)
	return cl
}

func NewSgpolicyV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceSgpolicyV1 {

	var l_GetSgpolicyListEndpoint endpoint.Endpoint
	{
		l_GetSgpolicyListEndpoint = grpctransport.NewClient(
			conn,
			"network.SgpolicyV1",
			"GetSgpolicyList",
			network.EndcodeGrpcReqSgpolicyList,
			network.DecodeGrpcRespSgpolicyList,
			network.SgpolicyList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetSgpolicyListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "SgpolicyV1:GetSgpolicyList")(l_GetSgpolicyListEndpoint)
	}
	var l_SgpolicyOperEndpoint endpoint.Endpoint
	{
		l_SgpolicyOperEndpoint = grpctransport.NewClient(
			conn,
			"network.SgpolicyV1",
			"SgpolicyOper",
			network.EndcodeGrpcReqSgpolicy,
			network.DecodeGrpcRespSgpolicy,
			network.Sgpolicy{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_SgpolicyOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "SgpolicyV1:SgpolicyOper")(l_SgpolicyOperEndpoint)
	}
	return network.Endpoints_SgpolicyV1{

		GetSgpolicyListEndpoint: l_GetSgpolicyListEndpoint,
		SgpolicyOperEndpoint:    l_SgpolicyOperEndpoint,
	}
}

func NewSgpolicyV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceSgpolicyV1 {
	cl := NewSgpolicyV1(conn, logger)
	cl = network.LoggingSgpolicyV1Middleware(logger)(cl)
	return cl
}

func NewServiceV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceServiceV1 {

	var l_GetServiceListEndpoint endpoint.Endpoint
	{
		l_GetServiceListEndpoint = grpctransport.NewClient(
			conn,
			"network.ServiceV1",
			"GetServiceList",
			network.EndcodeGrpcReqServiceList,
			network.DecodeGrpcRespServiceList,
			network.ServiceList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetServiceListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "ServiceV1:GetServiceList")(l_GetServiceListEndpoint)
	}
	var l_ServiceOperEndpoint endpoint.Endpoint
	{
		l_ServiceOperEndpoint = grpctransport.NewClient(
			conn,
			"network.ServiceV1",
			"ServiceOper",
			network.EndcodeGrpcReqService,
			network.DecodeGrpcRespService,
			network.Service{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_ServiceOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "ServiceV1:ServiceOper")(l_ServiceOperEndpoint)
	}
	return network.Endpoints_ServiceV1{

		GetServiceListEndpoint: l_GetServiceListEndpoint,
		ServiceOperEndpoint:    l_ServiceOperEndpoint,
	}
}

func NewServiceV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceServiceV1 {
	cl := NewServiceV1(conn, logger)
	cl = network.LoggingServiceV1Middleware(logger)(cl)
	return cl
}

func NewLbPolicyV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceLbPolicyV1 {

	var l_GetLbPolicyListEndpoint endpoint.Endpoint
	{
		l_GetLbPolicyListEndpoint = grpctransport.NewClient(
			conn,
			"network.LbPolicyV1",
			"GetLbPolicyList",
			network.EndcodeGrpcReqLbPolicyList,
			network.DecodeGrpcRespLbPolicyList,
			network.LbPolicyList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetLbPolicyListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "LbPolicyV1:GetLbPolicyList")(l_GetLbPolicyListEndpoint)
	}
	var l_LbPolicyOperEndpoint endpoint.Endpoint
	{
		l_LbPolicyOperEndpoint = grpctransport.NewClient(
			conn,
			"network.LbPolicyV1",
			"LbPolicyOper",
			network.EndcodeGrpcReqLbPolicy,
			network.DecodeGrpcRespLbPolicy,
			network.LbPolicy{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_LbPolicyOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "LbPolicyV1:LbPolicyOper")(l_LbPolicyOperEndpoint)
	}
	return network.Endpoints_LbPolicyV1{

		GetLbPolicyListEndpoint: l_GetLbPolicyListEndpoint,
		LbPolicyOperEndpoint:    l_LbPolicyOperEndpoint,
	}
}

func NewLbPolicyV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceLbPolicyV1 {
	cl := NewLbPolicyV1(conn, logger)
	cl = network.LoggingLbPolicyV1Middleware(logger)(cl)
	return cl
}

func NewEndpointV1(conn *grpc.ClientConn, logger log.Logger) network.ServiceEndpointV1 {

	var l_GetEndpointListEndpoint endpoint.Endpoint
	{
		l_GetEndpointListEndpoint = grpctransport.NewClient(
			conn,
			"network.EndpointV1",
			"GetEndpointList",
			network.EndcodeGrpcReqEndpointList,
			network.DecodeGrpcRespEndpointList,
			network.EndpointList{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_GetEndpointListEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "EndpointV1:GetEndpointList")(l_GetEndpointListEndpoint)
	}
	var l_EndpointOperEndpoint endpoint.Endpoint
	{
		l_EndpointOperEndpoint = grpctransport.NewClient(
			conn,
			"network.EndpointV1",
			"EndpointOper",
			network.EndcodeGrpcReqEndpoint,
			network.DecodeGrpcRespEndpoint,
			network.Endpoint{},
			grpctransport.ClientBefore(opentracing.ToGRPCRequest(stdopentracing.GlobalTracer(), logger)),
			grpctransport.ClientBefore(dummyBefore),
		).Endpoint()
		l_EndpointOperEndpoint = opentracing.TraceClient(stdopentracing.GlobalTracer(), "EndpointV1:EndpointOper")(l_EndpointOperEndpoint)
	}
	return network.Endpoints_EndpointV1{

		GetEndpointListEndpoint: l_GetEndpointListEndpoint,
		EndpointOperEndpoint:    l_EndpointOperEndpoint,
	}
}

func NewEndpointV1Backend(conn *grpc.ClientConn, logger log.Logger) network.ServiceEndpointV1 {
	cl := NewEndpointV1(conn, logger)
	cl = network.LoggingEndpointV1Middleware(logger)(cl)
	return cl
}
