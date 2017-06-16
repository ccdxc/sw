/*
Package cmd is a auto generated package.
Input file: protos/cmd.proto
*/

package cmd

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

type MiddlewareCmdV1 func(ServiceCmdV1) ServiceCmdV1
type Endpoints_CmdV1 struct {
	GetNodeListEndpoint endpoint.Endpoint
	NodeOperEndpoint    endpoint.Endpoint
	ClusterOperEndpoint endpoint.Endpoint
}

func (e Endpoints_CmdV1) GetNodeList(ctx context.Context, in NodeList) (NodeList, error) {
	resp, err := e.GetNodeListEndpoint(ctx, in)
	if err != nil {
		return NodeList{}, err
	}
	return *resp.(*NodeList), nil
}

type respCmdV1GetNodeList struct {
	V   NodeList
	Err error
}

func MakeCmdV1GetNodeListEndpoint(s ServiceCmdV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*NodeList)
		v, err := s.GetNodeList(ctx, *req)
		return respCmdV1GetNodeList{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "CmdV1:GetNodeList")(f)
}
func (e Endpoints_CmdV1) NodeOper(ctx context.Context, in Node) (Node, error) {
	resp, err := e.NodeOperEndpoint(ctx, in)
	if err != nil {
		return Node{}, err
	}
	return *resp.(*Node), nil
}

type respCmdV1NodeOper struct {
	V   Node
	Err error
}

func MakeCmdV1NodeOperEndpoint(s ServiceCmdV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Node)
		v, err := s.NodeOper(ctx, *req)
		return respCmdV1NodeOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "CmdV1:NodeOper")(f)
}
func (e Endpoints_CmdV1) ClusterOper(ctx context.Context, in Cluster) (Cluster, error) {
	resp, err := e.ClusterOperEndpoint(ctx, in)
	if err != nil {
		return Cluster{}, err
	}
	return *resp.(*Cluster), nil
}

type respCmdV1ClusterOper struct {
	V   Cluster
	Err error
}

func MakeCmdV1ClusterOperEndpoint(s ServiceCmdV1, logger log.Logger) endpoint.Endpoint {
	f := func(ctx context.Context, request interface{}) (response interface{}, err error) {
		req := request.(*Cluster)
		v, err := s.ClusterOper(ctx, *req)
		return respCmdV1ClusterOper{
			V:   v,
			Err: err,
		}, nil
	}
	return opentracing.TraceServer(stdopentracing.GlobalTracer(), "CmdV1:ClusterOper")(f)
}

func MakeCmdV1ServerEndpoints(s ServiceCmdV1, logger log.Logger) Endpoints_CmdV1 {
	return Endpoints_CmdV1{
		GetNodeListEndpoint: MakeCmdV1GetNodeListEndpoint(s, logger),
		NodeOperEndpoint:    MakeCmdV1NodeOperEndpoint(s, logger),
		ClusterOperEndpoint: MakeCmdV1ClusterOperEndpoint(s, logger),
	}
}

func LoggingCmdV1Middleware(logger log.Logger) MiddlewareCmdV1 {
	return func(next ServiceCmdV1) ServiceCmdV1 {
		return loggingCmdV1Middleware{
			logger: logger,
			next:   next,
		}
	}
}

type loggingCmdV1Middleware struct {
	logger log.Logger
	next   ServiceCmdV1
}

func (m loggingCmdV1Middleware) GetNodeList(ctx context.Context, in NodeList) (resp NodeList, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "CmdV1", "method", "GetNodeList", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.GetNodeList(ctx, in)
	return
}

func (m loggingCmdV1Middleware) NodeOper(ctx context.Context, in Node) (resp Node, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "CmdV1", "method", "NodeOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.NodeOper(ctx, in)
	return
}

func (m loggingCmdV1Middleware) ClusterOper(ctx context.Context, in Cluster) (resp Cluster, err error) {
	defer func(begin time.Time) {
		var rslt string
		if err == nil {
			rslt = "Success"
		} else {
			rslt = err.Error()
		}
		m.logger.Audit(ctx, "service", "CmdV1", "method", "ClusterOper", "result", rslt, "duration", time.Since(begin))
	}(time.Now())
	resp, err = m.next.ClusterOper(ctx, in)
	return
}

func MakeCmdV1RestClientEndpoints(instance string) (Endpoints_CmdV1, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	tgt, err := url.Parse(instance)
	if err != nil {
		return Endpoints_CmdV1{}, err
	}
	tgt.Path = ""
	options := []httptransport.ClientOption{}

	return Endpoints_CmdV1{
		GetNodeListEndpoint: httptransport.NewClient("POST", tgt, encodeHttpNodeList, decodeHttprespCmdV1GetNodeList, options...).Endpoint(),
		NodeOperEndpoint:    httptransport.NewClient("POST", tgt, encodeHttpNode, decodeHttprespCmdV1NodeOper, options...).Endpoint(),
		ClusterOperEndpoint: httptransport.NewClient("POST", tgt, encodeHttpCluster, decodeHttprespCmdV1ClusterOper, options...).Endpoint(),
	}, nil
}
