/*
Package network is a auto generated package.
Input file: protos/network.proto
*/
package network

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"io/ioutil"
	"net/http"

	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc/metadata"

	"github.com/go-kit/kit/tracing/opentracing"
	grpctransport "github.com/go-kit/kit/transport/grpc"
	stdopentracing "github.com/opentracing/opentracing-go"
	"github.com/pensando/sw/utils/log"
)

var (
	ErrInconsistentIDs = errors.New("inconsistent IDs")
	ErrAlreadyExists   = errors.New("already exists")
	ErrNotFound        = errors.New("not found")
)

// FIXME: add HTTP handler here.
func recoverVersion(ctx context.Context, md metadata.MD) context.Context {
	var pairs []string
	xmd := md
	v, ok := xmd["req-version"]
	if ok {
		pairs = append(pairs, "req-version", v[0])
	}
	if v, ok = xmd["req-uri"]; ok {
		pairs = append(pairs, "req-uri", v[0])
	}
	if v, ok = xmd["req-method"]; ok {
		pairs = append(pairs, "req-method", v[0])
	}
	nmd := metadata.Pairs(pairs...)
	nmd = metadata.Join(nmd, md)
	ctx = metadata.NewContext(ctx, nmd)
	return ctx
}

type grpcServerTenantV1 struct {
	GetTenantListHdlr grpctransport.Handler
	TenantOperHdlr    grpctransport.Handler
}

func MakeGRPCServerTenantV1(ctx context.Context, endpoints Endpoints_TenantV1, logger log.Logger) TenantV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerTenantV1{

		GetTenantListHdlr: grpctransport.NewServer(
			endpoints.GetTenantListEndpoint,
			DecodeGrpcReqTenantList,
			EncodeGrpcRespTenantList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetTenantList", logger)))...,
		),

		TenantOperHdlr: grpctransport.NewServer(
			endpoints.TenantOperEndpoint,
			DecodeGrpcReqTenant,
			EncodeGrpcRespTenant,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "TenantOper", logger)))...,
		),
	}
}

func (s *grpcServerTenantV1) GetTenantList(ctx oldcontext.Context, req *TenantList) (*TenantList, error) {
	_, resp, err := s.GetTenantListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respTenantV1GetTenantList).V
	return &r, resp.(respTenantV1GetTenantList).Err
}

func decodeHttprespTenantV1GetTenantList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respTenantV1GetTenantList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerTenantV1) TenantOper(ctx oldcontext.Context, req *Tenant) (*Tenant, error) {
	_, resp, err := s.TenantOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respTenantV1TenantOper).V
	return &r, resp.(respTenantV1TenantOper).Err
}

func decodeHttprespTenantV1TenantOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respTenantV1TenantOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

type grpcServerNetworkV1 struct {
	GetNetworkListHdlr grpctransport.Handler
	NetworkOperHdlr    grpctransport.Handler
}

func MakeGRPCServerNetworkV1(ctx context.Context, endpoints Endpoints_NetworkV1, logger log.Logger) NetworkV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerNetworkV1{

		GetNetworkListHdlr: grpctransport.NewServer(
			endpoints.GetNetworkListEndpoint,
			DecodeGrpcReqNetworkList,
			EncodeGrpcRespNetworkList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetNetworkList", logger)))...,
		),

		NetworkOperHdlr: grpctransport.NewServer(
			endpoints.NetworkOperEndpoint,
			DecodeGrpcReqNetwork,
			EncodeGrpcRespNetwork,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "NetworkOper", logger)))...,
		),
	}
}

func (s *grpcServerNetworkV1) GetNetworkList(ctx oldcontext.Context, req *NetworkList) (*NetworkList, error) {
	_, resp, err := s.GetNetworkListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respNetworkV1GetNetworkList).V
	return &r, resp.(respNetworkV1GetNetworkList).Err
}

func decodeHttprespNetworkV1GetNetworkList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respNetworkV1GetNetworkList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerNetworkV1) NetworkOper(ctx oldcontext.Context, req *Network) (*Network, error) {
	_, resp, err := s.NetworkOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respNetworkV1NetworkOper).V
	return &r, resp.(respNetworkV1NetworkOper).Err
}

func decodeHttprespNetworkV1NetworkOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respNetworkV1NetworkOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

type grpcServerSecurityGroupV1 struct {
	GetSecurityGroupListHdlr grpctransport.Handler
	SecurityGroupOperHdlr    grpctransport.Handler
}

func MakeGRPCServerSecurityGroupV1(ctx context.Context, endpoints Endpoints_SecurityGroupV1, logger log.Logger) SecurityGroupV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerSecurityGroupV1{

		GetSecurityGroupListHdlr: grpctransport.NewServer(
			endpoints.GetSecurityGroupListEndpoint,
			DecodeGrpcReqSecurityGroupList,
			EncodeGrpcRespSecurityGroupList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetSecurityGroupList", logger)))...,
		),

		SecurityGroupOperHdlr: grpctransport.NewServer(
			endpoints.SecurityGroupOperEndpoint,
			DecodeGrpcReqSecurityGroup,
			EncodeGrpcRespSecurityGroup,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "SecurityGroupOper", logger)))...,
		),
	}
}

func (s *grpcServerSecurityGroupV1) GetSecurityGroupList(ctx oldcontext.Context, req *SecurityGroupList) (*SecurityGroupList, error) {
	_, resp, err := s.GetSecurityGroupListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respSecurityGroupV1GetSecurityGroupList).V
	return &r, resp.(respSecurityGroupV1GetSecurityGroupList).Err
}

func decodeHttprespSecurityGroupV1GetSecurityGroupList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respSecurityGroupV1GetSecurityGroupList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerSecurityGroupV1) SecurityGroupOper(ctx oldcontext.Context, req *SecurityGroup) (*SecurityGroup, error) {
	_, resp, err := s.SecurityGroupOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respSecurityGroupV1SecurityGroupOper).V
	return &r, resp.(respSecurityGroupV1SecurityGroupOper).Err
}

func decodeHttprespSecurityGroupV1SecurityGroupOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respSecurityGroupV1SecurityGroupOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

type grpcServerSgpolicyV1 struct {
	GetSgpolicyListHdlr grpctransport.Handler
	SgpolicyOperHdlr    grpctransport.Handler
}

func MakeGRPCServerSgpolicyV1(ctx context.Context, endpoints Endpoints_SgpolicyV1, logger log.Logger) SgpolicyV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerSgpolicyV1{

		GetSgpolicyListHdlr: grpctransport.NewServer(
			endpoints.GetSgpolicyListEndpoint,
			DecodeGrpcReqSgpolicyList,
			EncodeGrpcRespSgpolicyList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetSgpolicyList", logger)))...,
		),

		SgpolicyOperHdlr: grpctransport.NewServer(
			endpoints.SgpolicyOperEndpoint,
			DecodeGrpcReqSgpolicy,
			EncodeGrpcRespSgpolicy,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "SgpolicyOper", logger)))...,
		),
	}
}

func (s *grpcServerSgpolicyV1) GetSgpolicyList(ctx oldcontext.Context, req *SgpolicyList) (*SgpolicyList, error) {
	_, resp, err := s.GetSgpolicyListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respSgpolicyV1GetSgpolicyList).V
	return &r, resp.(respSgpolicyV1GetSgpolicyList).Err
}

func decodeHttprespSgpolicyV1GetSgpolicyList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respSgpolicyV1GetSgpolicyList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerSgpolicyV1) SgpolicyOper(ctx oldcontext.Context, req *Sgpolicy) (*Sgpolicy, error) {
	_, resp, err := s.SgpolicyOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respSgpolicyV1SgpolicyOper).V
	return &r, resp.(respSgpolicyV1SgpolicyOper).Err
}

func decodeHttprespSgpolicyV1SgpolicyOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respSgpolicyV1SgpolicyOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

type grpcServerServiceV1 struct {
	GetServiceListHdlr grpctransport.Handler
	ServiceOperHdlr    grpctransport.Handler
}

func MakeGRPCServerServiceV1(ctx context.Context, endpoints Endpoints_ServiceV1, logger log.Logger) ServiceV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerServiceV1{

		GetServiceListHdlr: grpctransport.NewServer(
			endpoints.GetServiceListEndpoint,
			DecodeGrpcReqServiceList,
			EncodeGrpcRespServiceList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetServiceList", logger)))...,
		),

		ServiceOperHdlr: grpctransport.NewServer(
			endpoints.ServiceOperEndpoint,
			DecodeGrpcReqService,
			EncodeGrpcRespService,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "ServiceOper", logger)))...,
		),
	}
}

func (s *grpcServerServiceV1) GetServiceList(ctx oldcontext.Context, req *ServiceList) (*ServiceList, error) {
	_, resp, err := s.GetServiceListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respServiceV1GetServiceList).V
	return &r, resp.(respServiceV1GetServiceList).Err
}

func decodeHttprespServiceV1GetServiceList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respServiceV1GetServiceList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerServiceV1) ServiceOper(ctx oldcontext.Context, req *Service) (*Service, error) {
	_, resp, err := s.ServiceOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respServiceV1ServiceOper).V
	return &r, resp.(respServiceV1ServiceOper).Err
}

func decodeHttprespServiceV1ServiceOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respServiceV1ServiceOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

type grpcServerLbPolicyV1 struct {
	GetLbPolicyListHdlr grpctransport.Handler
	LbPolicyOperHdlr    grpctransport.Handler
}

func MakeGRPCServerLbPolicyV1(ctx context.Context, endpoints Endpoints_LbPolicyV1, logger log.Logger) LbPolicyV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerLbPolicyV1{

		GetLbPolicyListHdlr: grpctransport.NewServer(
			endpoints.GetLbPolicyListEndpoint,
			DecodeGrpcReqLbPolicyList,
			EncodeGrpcRespLbPolicyList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetLbPolicyList", logger)))...,
		),

		LbPolicyOperHdlr: grpctransport.NewServer(
			endpoints.LbPolicyOperEndpoint,
			DecodeGrpcReqLbPolicy,
			EncodeGrpcRespLbPolicy,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "LbPolicyOper", logger)))...,
		),
	}
}

func (s *grpcServerLbPolicyV1) GetLbPolicyList(ctx oldcontext.Context, req *LbPolicyList) (*LbPolicyList, error) {
	_, resp, err := s.GetLbPolicyListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respLbPolicyV1GetLbPolicyList).V
	return &r, resp.(respLbPolicyV1GetLbPolicyList).Err
}

func decodeHttprespLbPolicyV1GetLbPolicyList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respLbPolicyV1GetLbPolicyList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerLbPolicyV1) LbPolicyOper(ctx oldcontext.Context, req *LbPolicy) (*LbPolicy, error) {
	_, resp, err := s.LbPolicyOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respLbPolicyV1LbPolicyOper).V
	return &r, resp.(respLbPolicyV1LbPolicyOper).Err
}

func decodeHttprespLbPolicyV1LbPolicyOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respLbPolicyV1LbPolicyOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

type grpcServerEndpointV1 struct {
	GetEndpointListHdlr grpctransport.Handler
	EndpointOperHdlr    grpctransport.Handler
}

func MakeGRPCServerEndpointV1(ctx context.Context, endpoints Endpoints_EndpointV1, logger log.Logger) EndpointV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerEndpointV1{

		GetEndpointListHdlr: grpctransport.NewServer(
			endpoints.GetEndpointListEndpoint,
			DecodeGrpcReqEndpointList,
			EncodeGrpcRespEndpointList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetEndpointList", logger)))...,
		),

		EndpointOperHdlr: grpctransport.NewServer(
			endpoints.EndpointOperEndpoint,
			DecodeGrpcReqEndpoint,
			EncodeGrpcRespEndpoint,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "EndpointOper", logger)))...,
		),
	}
}

func (s *grpcServerEndpointV1) GetEndpointList(ctx oldcontext.Context, req *EndpointList) (*EndpointList, error) {
	_, resp, err := s.GetEndpointListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEndpointV1GetEndpointList).V
	return &r, resp.(respEndpointV1GetEndpointList).Err
}

func decodeHttprespEndpointV1GetEndpointList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respEndpointV1GetEndpointList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerEndpointV1) EndpointOper(ctx oldcontext.Context, req *Endpoint) (*Endpoint, error) {
	_, resp, err := s.EndpointOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respEndpointV1EndpointOper).V
	return &r, resp.(respEndpointV1EndpointOper).Err
}

func decodeHttprespEndpointV1EndpointOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respEndpointV1EndpointOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func encodeHttpTenantSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpTenantSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req TenantSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqTenantSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(TenantSpec)
	return &req, nil
}

func DecodeGrpcReqTenantSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TenantSpec)
	return req, nil
}

func EncodeGrpcRespTenantSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespTenantSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpTenantStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpTenantStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req TenantStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqTenantStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(TenantStatus)
	return &req, nil
}

func DecodeGrpcReqTenantStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TenantStatus)
	return req, nil
}

func EncodeGrpcRespTenantStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespTenantStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpTenant(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpTenant(_ context.Context, r *http.Request) (interface{}, error) {
	var req Tenant
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqTenant(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Tenant)
	return &req, nil
}

func DecodeGrpcReqTenant(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Tenant)
	return req, nil
}

func EncodeGrpcRespTenant(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespTenant(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpTenantList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpTenantList(_ context.Context, r *http.Request) (interface{}, error) {
	var req TenantList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqTenantList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(TenantList)
	return &req, nil
}

func DecodeGrpcReqTenantList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TenantList)
	return req, nil
}

func EncodeGrpcRespTenantList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespTenantList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNetworkSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNetworkSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req NetworkSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNetworkSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(NetworkSpec)
	return &req, nil
}

func DecodeGrpcReqNetworkSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NetworkSpec)
	return req, nil
}

func EncodeGrpcRespNetworkSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNetworkSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNetworkStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNetworkStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req NetworkStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNetworkStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(NetworkStatus)
	return &req, nil
}

func DecodeGrpcReqNetworkStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NetworkStatus)
	return req, nil
}

func EncodeGrpcRespNetworkStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNetworkStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNetwork(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNetwork(_ context.Context, r *http.Request) (interface{}, error) {
	var req Network
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNetwork(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Network)
	return &req, nil
}

func DecodeGrpcReqNetwork(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Network)
	return req, nil
}

func EncodeGrpcRespNetwork(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNetwork(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNetworkList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNetworkList(_ context.Context, r *http.Request) (interface{}, error) {
	var req NetworkList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNetworkList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(NetworkList)
	return &req, nil
}

func DecodeGrpcReqNetworkList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NetworkList)
	return req, nil
}

func EncodeGrpcRespNetworkList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNetworkList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSecurityGroupSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSecurityGroupSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req SecurityGroupSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSecurityGroupSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SecurityGroupSpec)
	return &req, nil
}

func DecodeGrpcReqSecurityGroupSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SecurityGroupSpec)
	return req, nil
}

func EncodeGrpcRespSecurityGroupSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSecurityGroupSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSecurityGroupStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSecurityGroupStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req SecurityGroupStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSecurityGroupStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SecurityGroupStatus)
	return &req, nil
}

func DecodeGrpcReqSecurityGroupStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SecurityGroupStatus)
	return req, nil
}

func EncodeGrpcRespSecurityGroupStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSecurityGroupStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSecurityGroup(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSecurityGroup(_ context.Context, r *http.Request) (interface{}, error) {
	var req SecurityGroup
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSecurityGroup(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SecurityGroup)
	return &req, nil
}

func DecodeGrpcReqSecurityGroup(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SecurityGroup)
	return req, nil
}

func EncodeGrpcRespSecurityGroup(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSecurityGroup(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSecurityGroupList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSecurityGroupList(_ context.Context, r *http.Request) (interface{}, error) {
	var req SecurityGroupList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSecurityGroupList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SecurityGroupList)
	return &req, nil
}

func DecodeGrpcReqSecurityGroupList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SecurityGroupList)
	return req, nil
}

func EncodeGrpcRespSecurityGroupList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSecurityGroupList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSGRule(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSGRule(_ context.Context, r *http.Request) (interface{}, error) {
	var req SGRule
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSGRule(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SGRule)
	return &req, nil
}

func DecodeGrpcReqSGRule(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SGRule)
	return req, nil
}

func EncodeGrpcRespSGRule(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSGRule(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSgpolicySpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSgpolicySpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req SgpolicySpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSgpolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SgpolicySpec)
	return &req, nil
}

func DecodeGrpcReqSgpolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SgpolicySpec)
	return req, nil
}

func EncodeGrpcRespSgpolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSgpolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSgpolicyStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSgpolicyStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req SgpolicyStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSgpolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SgpolicyStatus)
	return &req, nil
}

func DecodeGrpcReqSgpolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SgpolicyStatus)
	return req, nil
}

func EncodeGrpcRespSgpolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSgpolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSgpolicy(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSgpolicy(_ context.Context, r *http.Request) (interface{}, error) {
	var req Sgpolicy
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSgpolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Sgpolicy)
	return &req, nil
}

func DecodeGrpcReqSgpolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Sgpolicy)
	return req, nil
}

func EncodeGrpcRespSgpolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSgpolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpSgpolicyList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpSgpolicyList(_ context.Context, r *http.Request) (interface{}, error) {
	var req SgpolicyList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqSgpolicyList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(SgpolicyList)
	return &req, nil
}

func DecodeGrpcReqSgpolicyList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SgpolicyList)
	return req, nil
}

func EncodeGrpcRespSgpolicyList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespSgpolicyList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpServiceSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpServiceSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req ServiceSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqServiceSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(ServiceSpec)
	return &req, nil
}

func DecodeGrpcReqServiceSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*ServiceSpec)
	return req, nil
}

func EncodeGrpcRespServiceSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespServiceSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpServiceStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpServiceStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req ServiceStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqServiceStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(ServiceStatus)
	return &req, nil
}

func DecodeGrpcReqServiceStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*ServiceStatus)
	return req, nil
}

func EncodeGrpcRespServiceStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespServiceStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpService(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpService(_ context.Context, r *http.Request) (interface{}, error) {
	var req Service
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqService(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Service)
	return &req, nil
}

func DecodeGrpcReqService(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Service)
	return req, nil
}

func EncodeGrpcRespService(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespService(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpServiceList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpServiceList(_ context.Context, r *http.Request) (interface{}, error) {
	var req ServiceList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqServiceList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(ServiceList)
	return &req, nil
}

func DecodeGrpcReqServiceList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*ServiceList)
	return req, nil
}

func EncodeGrpcRespServiceList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespServiceList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpHealthCheckSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpHealthCheckSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req HealthCheckSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqHealthCheckSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(HealthCheckSpec)
	return &req, nil
}

func DecodeGrpcReqHealthCheckSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*HealthCheckSpec)
	return req, nil
}

func EncodeGrpcRespHealthCheckSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespHealthCheckSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpLbPolicySpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpLbPolicySpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req LbPolicySpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqLbPolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(LbPolicySpec)
	return &req, nil
}

func DecodeGrpcReqLbPolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*LbPolicySpec)
	return req, nil
}

func EncodeGrpcRespLbPolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespLbPolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpLbPolicyStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpLbPolicyStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req LbPolicyStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqLbPolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(LbPolicyStatus)
	return &req, nil
}

func DecodeGrpcReqLbPolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*LbPolicyStatus)
	return req, nil
}

func EncodeGrpcRespLbPolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespLbPolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpLbPolicy(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpLbPolicy(_ context.Context, r *http.Request) (interface{}, error) {
	var req LbPolicy
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqLbPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(LbPolicy)
	return &req, nil
}

func DecodeGrpcReqLbPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*LbPolicy)
	return req, nil
}

func EncodeGrpcRespLbPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespLbPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpLbPolicyList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpLbPolicyList(_ context.Context, r *http.Request) (interface{}, error) {
	var req LbPolicyList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqLbPolicyList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(LbPolicyList)
	return &req, nil
}

func DecodeGrpcReqLbPolicyList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*LbPolicyList)
	return req, nil
}

func EncodeGrpcRespLbPolicyList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespLbPolicyList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpEndpointSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpEndpointSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req EndpointSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqEndpointSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(EndpointSpec)
	return &req, nil
}

func DecodeGrpcReqEndpointSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EndpointSpec)
	return req, nil
}

func EncodeGrpcRespEndpointSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespEndpointSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpEndpointStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpEndpointStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req EndpointStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqEndpointStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(EndpointStatus)
	return &req, nil
}

func DecodeGrpcReqEndpointStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EndpointStatus)
	return req, nil
}

func EncodeGrpcRespEndpointStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespEndpointStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpEndpoint(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpEndpoint(_ context.Context, r *http.Request) (interface{}, error) {
	var req Endpoint
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqEndpoint(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Endpoint)
	return &req, nil
}

func DecodeGrpcReqEndpoint(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Endpoint)
	return req, nil
}

func EncodeGrpcRespEndpoint(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespEndpoint(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpEndpointList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpEndpointList(_ context.Context, r *http.Request) (interface{}, error) {
	var req EndpointList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqEndpointList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(EndpointList)
	return &req, nil
}

func DecodeGrpcReqEndpointList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*EndpointList)
	return req, nil
}

func EncodeGrpcRespEndpointList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespEndpointList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpResponse(ctx context.Context, w http.ResponseWriter, response interface{}) error {
	if e, ok := response.(errorer); ok && e.error() != nil {
		// Not a Go kit transport error, but a business-logic error.
		// Provide those as HTTP errors.
		encodeError(ctx, e.error(), w)
		return nil
	}
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	return json.NewEncoder(w).Encode(response)
}

func encodeHttpRequest(_ context.Context, req *http.Request, request interface{}) error {
	var buf bytes.Buffer
	err := json.NewEncoder(&buf).Encode(request)
	if err != nil {
		return err
	}
	req.Body = ioutil.NopCloser(&buf)
	return nil
}

type errorer interface {
	error() error
}

func encodeError(_ context.Context, err error, w http.ResponseWriter) {
	if err == nil {
		panic("encodeError with nil error")
	}
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(codeFrom(err))
	json.NewEncoder(w).Encode(map[string]interface{}{
		"error": err.Error(),
	})
}

func errorDecoder(r *http.Response) error {
	var w errorWrapper
	if err := json.NewDecoder(r.Body).Decode(&w); err != nil {
		return err
	}
	return errors.New(w.Error)
}

type errorWrapper struct {
	Error string `json:"error"`
}

func codeFrom(err error) int {
	switch err {
	case ErrNotFound:
		return http.StatusNotFound
	case ErrAlreadyExists, ErrInconsistentIDs:
		return http.StatusBadRequest
	default:
		return http.StatusInternalServerError
	}
}
