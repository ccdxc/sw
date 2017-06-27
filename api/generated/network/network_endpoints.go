/*
Package network is a auto generated package.
Input file: protos/network.proto
*/

package network

import (
	"net/url"
	"strings"

	"context"
	"time"

	"github.com/go-kit/kit/endpoint"
	"github.com/go-kit/kit/tracing/opentracing"
	httptransport "github.com/go-kit/kit/transport/http"
	stdopentracing "github.com/opentracing/opentracing-go"
	"github.com/pensando/sw/utils/log"
)

type MiddlewareTenantV1 func(ServiceTenantV1) ServiceTenantV1
type Endpoints_TenantV1 struct {
	GetTenantListEndpoint endpoint.Endpoint
	TenantOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_TenantV1) GetTenantList(ctx context.Context, in TenantList) (TenantList, error) {
	resp, err := e.GetTenantListEndpoint(ctx, in)
	if err != nil {
		return TenantList{}, err
	}
	return *resp.(*TenantList), nil
}

type respTenantV1GetTenantList struct {
	V   TenantList
	Err error
}

func MakeTenantV1GetTenantListEndpoint(s ServiceTenantV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*TenantList)
		v, err := s.GetTenantList(ctx, *req)
		return respTenantV1GetTenantList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "TenantV1:GetTenantList")(f)
}
func (e Endpoints_TenantV1) TenantOper(ctx context.Context, in Tenant) (Tenant, error) {
	resp, err := e.TenantOperEndpoint(ctx, in)
	if err != nil {
		return Tenant{}, err
	}
	return *resp.(*Tenant), nil
}

type respTenantV1TenantOper struct {
	V   Tenant
	Err error
}

func MakeTenantV1TenantOperEndpoint(s ServiceTenantV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Tenant)
		v, err := s.TenantOper(ctx, *req)
		return respTenantV1TenantOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "TenantV1:TenantOper")(f)
}

func MakeTenantV1ServerEndpoints(s ServiceTenantV1, logger log.Logger) Endpoints_TenantV1 {
	return Endpoints_TenantV1{
		GetTenantListEndpoint: MakeTenantV1GetTenantListEndpoint(s, logger),
		TenantOperEndpoint:    MakeTenantV1TenantOperEndpoint(s, logger),
	}
}

func LoggingTenantV1Middleware(logger log.Logger) MiddlewareTenantV1 {
	return func(next ServiceTenantV1) ServiceTenantV1 {
		return loggingTenantV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingTenantV1Middleware struct {
	logger log.Logger
	next   ServiceTenantV1
}

func (m loggingTenantV1Middleware) GetTenantList(ctx context.Context, in TenantList) (resp TenantList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TenantV1", "method", "GetTenantList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetTenantList(ctx, in)
	return
}

func (m loggingTenantV1Middleware) TenantOper(ctx context.Context, in Tenant) (resp Tenant, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "TenantV1", "method", "TenantOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.TenantOper(ctx, in)
	return
}

func MakeTenantV1RestClientEndpoints(instance string) (Endpoints_TenantV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_TenantV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_TenantV1{
		GetTenantListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpTenantList, decodeHttprespTenantV1GetTenantList, options...).Endpoint(),
		TenantOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpTenant, decodeHttprespTenantV1TenantOper, options...).Endpoint(),
	}, nil
}

type MiddlewareNetworkV1 func(ServiceNetworkV1) ServiceNetworkV1
type Endpoints_NetworkV1 struct {
	GetNetworkListEndpoint endpoint.Endpoint
	NetworkOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_NetworkV1) GetNetworkList(ctx context.Context, in NetworkList) (NetworkList, error) {
	resp, err := e.GetNetworkListEndpoint(ctx, in)
	if err != nil {
		return NetworkList{}, err
	}
	return *resp.(*NetworkList), nil
}

type respNetworkV1GetNetworkList struct {
	V   NetworkList
	Err error
}

func MakeNetworkV1GetNetworkListEndpoint(s ServiceNetworkV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*NetworkList)
		v, err := s.GetNetworkList(ctx, *req)
		return respNetworkV1GetNetworkList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "NetworkV1:GetNetworkList")(f)
}
func (e Endpoints_NetworkV1) NetworkOper(ctx context.Context, in Network) (Network, error) {
	resp, err := e.NetworkOperEndpoint(ctx, in)
	if err != nil {
		return Network{}, err
	}
	return *resp.(*Network), nil
}

type respNetworkV1NetworkOper struct {
	V   Network
	Err error
}

func MakeNetworkV1NetworkOperEndpoint(s ServiceNetworkV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Network)
		v, err := s.NetworkOper(ctx, *req)
		return respNetworkV1NetworkOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "NetworkV1:NetworkOper")(f)
}

func MakeNetworkV1ServerEndpoints(s ServiceNetworkV1, logger log.Logger) Endpoints_NetworkV1 {
	return Endpoints_NetworkV1{
		GetNetworkListEndpoint: MakeNetworkV1GetNetworkListEndpoint(s, logger),
		NetworkOperEndpoint:    MakeNetworkV1NetworkOperEndpoint(s, logger),
	}
}

func LoggingNetworkV1Middleware(logger log.Logger) MiddlewareNetworkV1 {
	return func(next ServiceNetworkV1) ServiceNetworkV1 {
		return loggingNetworkV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingNetworkV1Middleware struct {
	logger log.Logger
	next   ServiceNetworkV1
}

func (m loggingNetworkV1Middleware) GetNetworkList(ctx context.Context, in NetworkList) (resp NetworkList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "NetworkV1", "method", "GetNetworkList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetNetworkList(ctx, in)
	return
}

func (m loggingNetworkV1Middleware) NetworkOper(ctx context.Context, in Network) (resp Network, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "NetworkV1", "method", "NetworkOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.NetworkOper(ctx, in)
	return
}

func MakeNetworkV1RestClientEndpoints(instance string) (Endpoints_NetworkV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_NetworkV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_NetworkV1{
		GetNetworkListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpNetworkList, decodeHttprespNetworkV1GetNetworkList, options...).Endpoint(),
		NetworkOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpNetwork, decodeHttprespNetworkV1NetworkOper, options...).Endpoint(),
	}, nil
}

type MiddlewareSecurityGroupV1 func(ServiceSecurityGroupV1) ServiceSecurityGroupV1
type Endpoints_SecurityGroupV1 struct {
	GetSecurityGroupListEndpoint endpoint.Endpoint
	SecurityGroupOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_SecurityGroupV1) GetSecurityGroupList(ctx context.Context, in SecurityGroupList) (SecurityGroupList, error) {
	resp, err := e.GetSecurityGroupListEndpoint(ctx, in)
	if err != nil {
		return SecurityGroupList{}, err
	}
	return *resp.(*SecurityGroupList), nil
}

type respSecurityGroupV1GetSecurityGroupList struct {
	V   SecurityGroupList
	Err error
}

func MakeSecurityGroupV1GetSecurityGroupListEndpoint(s ServiceSecurityGroupV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*SecurityGroupList)
		v, err := s.GetSecurityGroupList(ctx, *req)
		return respSecurityGroupV1GetSecurityGroupList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "SecurityGroupV1:GetSecurityGroupList")(f)
}
func (e Endpoints_SecurityGroupV1) SecurityGroupOper(ctx context.Context, in SecurityGroup) (SecurityGroup, error) {
	resp, err := e.SecurityGroupOperEndpoint(ctx, in)
	if err != nil {
		return SecurityGroup{}, err
	}
	return *resp.(*SecurityGroup), nil
}

type respSecurityGroupV1SecurityGroupOper struct {
	V   SecurityGroup
	Err error
}

func MakeSecurityGroupV1SecurityGroupOperEndpoint(s ServiceSecurityGroupV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*SecurityGroup)
		v, err := s.SecurityGroupOper(ctx, *req)
		return respSecurityGroupV1SecurityGroupOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "SecurityGroupV1:SecurityGroupOper")(f)
}

func MakeSecurityGroupV1ServerEndpoints(s ServiceSecurityGroupV1, logger log.Logger) Endpoints_SecurityGroupV1 {
	return Endpoints_SecurityGroupV1{
		GetSecurityGroupListEndpoint: MakeSecurityGroupV1GetSecurityGroupListEndpoint(s, logger),
		SecurityGroupOperEndpoint:    MakeSecurityGroupV1SecurityGroupOperEndpoint(s, logger),
	}
}

func LoggingSecurityGroupV1Middleware(logger log.Logger) MiddlewareSecurityGroupV1 {
	return func(next ServiceSecurityGroupV1) ServiceSecurityGroupV1 {
		return loggingSecurityGroupV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingSecurityGroupV1Middleware struct {
	logger log.Logger
	next   ServiceSecurityGroupV1
}

func (m loggingSecurityGroupV1Middleware) GetSecurityGroupList(ctx context.Context, in SecurityGroupList) (resp SecurityGroupList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "SecurityGroupV1", "method", "GetSecurityGroupList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetSecurityGroupList(ctx, in)
	return
}

func (m loggingSecurityGroupV1Middleware) SecurityGroupOper(ctx context.Context, in SecurityGroup) (resp SecurityGroup, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "SecurityGroupV1", "method", "SecurityGroupOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.SecurityGroupOper(ctx, in)
	return
}

func MakeSecurityGroupV1RestClientEndpoints(instance string) (Endpoints_SecurityGroupV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_SecurityGroupV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_SecurityGroupV1{
		GetSecurityGroupListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpSecurityGroupList, decodeHttprespSecurityGroupV1GetSecurityGroupList, options...).Endpoint(),
		SecurityGroupOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpSecurityGroup, decodeHttprespSecurityGroupV1SecurityGroupOper, options...).Endpoint(),
	}, nil
}

type MiddlewareSgpolicyV1 func(ServiceSgpolicyV1) ServiceSgpolicyV1
type Endpoints_SgpolicyV1 struct {
	GetSgpolicyListEndpoint endpoint.Endpoint
	SgpolicyOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_SgpolicyV1) GetSgpolicyList(ctx context.Context, in SgpolicyList) (SgpolicyList, error) {
	resp, err := e.GetSgpolicyListEndpoint(ctx, in)
	if err != nil {
		return SgpolicyList{}, err
	}
	return *resp.(*SgpolicyList), nil
}

type respSgpolicyV1GetSgpolicyList struct {
	V   SgpolicyList
	Err error
}

func MakeSgpolicyV1GetSgpolicyListEndpoint(s ServiceSgpolicyV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*SgpolicyList)
		v, err := s.GetSgpolicyList(ctx, *req)
		return respSgpolicyV1GetSgpolicyList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "SgpolicyV1:GetSgpolicyList")(f)
}
func (e Endpoints_SgpolicyV1) SgpolicyOper(ctx context.Context, in Sgpolicy) (Sgpolicy, error) {
	resp, err := e.SgpolicyOperEndpoint(ctx, in)
	if err != nil {
		return Sgpolicy{}, err
	}
	return *resp.(*Sgpolicy), nil
}

type respSgpolicyV1SgpolicyOper struct {
	V   Sgpolicy
	Err error
}

func MakeSgpolicyV1SgpolicyOperEndpoint(s ServiceSgpolicyV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Sgpolicy)
		v, err := s.SgpolicyOper(ctx, *req)
		return respSgpolicyV1SgpolicyOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "SgpolicyV1:SgpolicyOper")(f)
}

func MakeSgpolicyV1ServerEndpoints(s ServiceSgpolicyV1, logger log.Logger) Endpoints_SgpolicyV1 {
	return Endpoints_SgpolicyV1{
		GetSgpolicyListEndpoint: MakeSgpolicyV1GetSgpolicyListEndpoint(s, logger),
		SgpolicyOperEndpoint:    MakeSgpolicyV1SgpolicyOperEndpoint(s, logger),
	}
}

func LoggingSgpolicyV1Middleware(logger log.Logger) MiddlewareSgpolicyV1 {
	return func(next ServiceSgpolicyV1) ServiceSgpolicyV1 {
		return loggingSgpolicyV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingSgpolicyV1Middleware struct {
	logger log.Logger
	next   ServiceSgpolicyV1
}

func (m loggingSgpolicyV1Middleware) GetSgpolicyList(ctx context.Context, in SgpolicyList) (resp SgpolicyList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "SgpolicyV1", "method", "GetSgpolicyList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetSgpolicyList(ctx, in)
	return
}

func (m loggingSgpolicyV1Middleware) SgpolicyOper(ctx context.Context, in Sgpolicy) (resp Sgpolicy, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "SgpolicyV1", "method", "SgpolicyOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.SgpolicyOper(ctx, in)
	return
}

func MakeSgpolicyV1RestClientEndpoints(instance string) (Endpoints_SgpolicyV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_SgpolicyV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_SgpolicyV1{
		GetSgpolicyListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpSgpolicyList, decodeHttprespSgpolicyV1GetSgpolicyList, options...).Endpoint(),
		SgpolicyOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpSgpolicy, decodeHttprespSgpolicyV1SgpolicyOper, options...).Endpoint(),
	}, nil
}

type MiddlewareServiceV1 func(ServiceServiceV1) ServiceServiceV1
type Endpoints_ServiceV1 struct {
	GetServiceListEndpoint endpoint.Endpoint
	ServiceOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_ServiceV1) GetServiceList(ctx context.Context, in ServiceList) (ServiceList, error) {
	resp, err := e.GetServiceListEndpoint(ctx, in)
	if err != nil {
		return ServiceList{}, err
	}
	return *resp.(*ServiceList), nil
}

type respServiceV1GetServiceList struct {
	V   ServiceList
	Err error
}

func MakeServiceV1GetServiceListEndpoint(s ServiceServiceV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*ServiceList)
		v, err := s.GetServiceList(ctx, *req)
		return respServiceV1GetServiceList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "ServiceV1:GetServiceList")(f)
}
func (e Endpoints_ServiceV1) ServiceOper(ctx context.Context, in Service) (Service, error) {
	resp, err := e.ServiceOperEndpoint(ctx, in)
	if err != nil {
		return Service{}, err
	}
	return *resp.(*Service), nil
}

type respServiceV1ServiceOper struct {
	V   Service
	Err error
}

func MakeServiceV1ServiceOperEndpoint(s ServiceServiceV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Service)
		v, err := s.ServiceOper(ctx, *req)
		return respServiceV1ServiceOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "ServiceV1:ServiceOper")(f)
}

func MakeServiceV1ServerEndpoints(s ServiceServiceV1, logger log.Logger) Endpoints_ServiceV1 {
	return Endpoints_ServiceV1{
		GetServiceListEndpoint: MakeServiceV1GetServiceListEndpoint(s, logger),
		ServiceOperEndpoint:    MakeServiceV1ServiceOperEndpoint(s, logger),
	}
}

func LoggingServiceV1Middleware(logger log.Logger) MiddlewareServiceV1 {
	return func(next ServiceServiceV1) ServiceServiceV1 {
		return loggingServiceV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingServiceV1Middleware struct {
	logger log.Logger
	next   ServiceServiceV1
}

func (m loggingServiceV1Middleware) GetServiceList(ctx context.Context, in ServiceList) (resp ServiceList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "ServiceV1", "method", "GetServiceList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetServiceList(ctx, in)
	return
}

func (m loggingServiceV1Middleware) ServiceOper(ctx context.Context, in Service) (resp Service, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "ServiceV1", "method", "ServiceOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.ServiceOper(ctx, in)
	return
}

func MakeServiceV1RestClientEndpoints(instance string) (Endpoints_ServiceV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_ServiceV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_ServiceV1{
		GetServiceListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpServiceList, decodeHttprespServiceV1GetServiceList, options...).Endpoint(),
		ServiceOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpService, decodeHttprespServiceV1ServiceOper, options...).Endpoint(),
	}, nil
}

type MiddlewareLbPolicyV1 func(ServiceLbPolicyV1) ServiceLbPolicyV1
type Endpoints_LbPolicyV1 struct {
	GetLbPolicyListEndpoint endpoint.Endpoint
	LbPolicyOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_LbPolicyV1) GetLbPolicyList(ctx context.Context, in LbPolicyList) (LbPolicyList, error) {
	resp, err := e.GetLbPolicyListEndpoint(ctx, in)
	if err != nil {
		return LbPolicyList{}, err
	}
	return *resp.(*LbPolicyList), nil
}

type respLbPolicyV1GetLbPolicyList struct {
	V   LbPolicyList
	Err error
}

func MakeLbPolicyV1GetLbPolicyListEndpoint(s ServiceLbPolicyV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*LbPolicyList)
		v, err := s.GetLbPolicyList(ctx, *req)
		return respLbPolicyV1GetLbPolicyList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "LbPolicyV1:GetLbPolicyList")(f)
}
func (e Endpoints_LbPolicyV1) LbPolicyOper(ctx context.Context, in LbPolicy) (LbPolicy, error) {
	resp, err := e.LbPolicyOperEndpoint(ctx, in)
	if err != nil {
		return LbPolicy{}, err
	}
	return *resp.(*LbPolicy), nil
}

type respLbPolicyV1LbPolicyOper struct {
	V   LbPolicy
	Err error
}

func MakeLbPolicyV1LbPolicyOperEndpoint(s ServiceLbPolicyV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*LbPolicy)
		v, err := s.LbPolicyOper(ctx, *req)
		return respLbPolicyV1LbPolicyOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "LbPolicyV1:LbPolicyOper")(f)
}

func MakeLbPolicyV1ServerEndpoints(s ServiceLbPolicyV1, logger log.Logger) Endpoints_LbPolicyV1 {
	return Endpoints_LbPolicyV1{
		GetLbPolicyListEndpoint: MakeLbPolicyV1GetLbPolicyListEndpoint(s, logger),
		LbPolicyOperEndpoint:    MakeLbPolicyV1LbPolicyOperEndpoint(s, logger),
	}
}

func LoggingLbPolicyV1Middleware(logger log.Logger) MiddlewareLbPolicyV1 {
	return func(next ServiceLbPolicyV1) ServiceLbPolicyV1 {
		return loggingLbPolicyV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingLbPolicyV1Middleware struct {
	logger log.Logger
	next   ServiceLbPolicyV1
}

func (m loggingLbPolicyV1Middleware) GetLbPolicyList(ctx context.Context, in LbPolicyList) (resp LbPolicyList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "LbPolicyV1", "method", "GetLbPolicyList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetLbPolicyList(ctx, in)
	return
}

func (m loggingLbPolicyV1Middleware) LbPolicyOper(ctx context.Context, in LbPolicy) (resp LbPolicy, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "LbPolicyV1", "method", "LbPolicyOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.LbPolicyOper(ctx, in)
	return
}

func MakeLbPolicyV1RestClientEndpoints(instance string) (Endpoints_LbPolicyV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_LbPolicyV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_LbPolicyV1{
		GetLbPolicyListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpLbPolicyList, decodeHttprespLbPolicyV1GetLbPolicyList, options...).Endpoint(),
		LbPolicyOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpLbPolicy, decodeHttprespLbPolicyV1LbPolicyOper, options...).Endpoint(),
	}, nil
}

type MiddlewareEndpointV1 func(ServiceEndpointV1) ServiceEndpointV1
type Endpoints_EndpointV1 struct {
	GetEndpointListEndpoint endpoint.Endpoint
	EndpointOperEndpoint    endpoint.Endpoint
}

func (e Endpoints_EndpointV1) GetEndpointList(ctx context.Context, in EndpointList) (EndpointList, error) {
	resp, err := e.GetEndpointListEndpoint(ctx, in)
	if err != nil {
		return EndpointList{}, err
	}
	return *resp.(*EndpointList), nil
}

type respEndpointV1GetEndpointList struct {
	V   EndpointList
	Err error
}

func MakeEndpointV1GetEndpointListEndpoint(s ServiceEndpointV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*EndpointList)
		v, err := s.GetEndpointList(ctx, *req)
		return respEndpointV1GetEndpointList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "EndpointV1:GetEndpointList")(f)
}
func (e Endpoints_EndpointV1) EndpointOper(ctx context.Context, in Endpoint) (Endpoint, error) {
	resp, err := e.EndpointOperEndpoint(ctx, in)
	if err != nil {
		return Endpoint{}, err
	}
	return *resp.(*Endpoint), nil
}

type respEndpointV1EndpointOper struct {
	V   Endpoint
	Err error
}

func MakeEndpointV1EndpointOperEndpoint(s ServiceEndpointV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Endpoint)
		v, err := s.EndpointOper(ctx, *req)
		return respEndpointV1EndpointOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "EndpointV1:EndpointOper")(f)
}

func MakeEndpointV1ServerEndpoints(s ServiceEndpointV1, logger log.Logger) Endpoints_EndpointV1 {
	return Endpoints_EndpointV1{
		GetEndpointListEndpoint: MakeEndpointV1GetEndpointListEndpoint(s, logger),
		EndpointOperEndpoint:    MakeEndpointV1EndpointOperEndpoint(s, logger),
	}
}

func LoggingEndpointV1Middleware(logger log.Logger) MiddlewareEndpointV1 {
	return func(next ServiceEndpointV1) ServiceEndpointV1 {
		return loggingEndpointV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingEndpointV1Middleware struct {
	logger log.Logger
	next   ServiceEndpointV1
}

func (m loggingEndpointV1Middleware) GetEndpointList(ctx context.Context, in EndpointList) (resp EndpointList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "EndpointV1", "method", "GetEndpointList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetEndpointList(ctx, in)
	return
}

func (m loggingEndpointV1Middleware) EndpointOper(ctx context.Context, in Endpoint) (resp Endpoint, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "EndpointV1", "method", "EndpointOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.EndpointOper(ctx, in)
	return
}

func MakeEndpointV1RestClientEndpoints(instance string) (Endpoints_EndpointV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_EndpointV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_EndpointV1{
		GetEndpointListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpEndpointList, decodeHttprespEndpointV1GetEndpointList, options...).Endpoint(),
		EndpointOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpEndpoint, decodeHttprespEndpointV1EndpointOper, options...).Endpoint(),
	}, nil
}
