/*
Package cmd is a auto generated package.
Input file: protos/cmd.proto
*/
package cmd

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

type grpcServerCmdV1 struct {
	GetNodeListHdlr grpctransport.Handler
	NodeOperHdlr    grpctransport.Handler
	ClusterOperHdlr grpctransport.Handler
}

func MakeGRPCServerCmdV1(ctx context.Context, endpoints Endpoints_CmdV1, logger log.Logger) CmdV1Server {
	options := []grpctransport.ServerOption{
		grpctransport.ServerErrorLogger(logger),
		grpctransport.ServerBefore(recoverVersion),
	}
	return &grpcServerCmdV1{

		GetNodeListHdlr: grpctransport.NewServer(
			endpoints.GetNodeListEndpoint,
			DecodeGrpcReqNodeList,
			EncodeGrpcRespNodeList,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "GetNodeList", logger)))...,
		),

		NodeOperHdlr: grpctransport.NewServer(
			endpoints.NodeOperEndpoint,
			DecodeGrpcReqNode,
			EncodeGrpcRespNode,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "NodeOper", logger)))...,
		),

		ClusterOperHdlr: grpctransport.NewServer(
			endpoints.ClusterOperEndpoint,
			DecodeGrpcReqCluster,
			EncodeGrpcRespCluster,
			append(options, grpctransport.ServerBefore(opentracing.FromGRPCRequest(stdopentracing.GlobalTracer(), "ClusterOper", logger)))...,
		),
	}
}

func (s *grpcServerCmdV1) GetNodeList(ctx oldcontext.Context, req *NodeList) (*NodeList, error) {
	_, resp, err := s.GetNodeListHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respCmdV1GetNodeList).V
	return &r, resp.(respCmdV1GetNodeList).Err
}

func decodeHttprespCmdV1GetNodeList(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respCmdV1GetNodeList
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerCmdV1) NodeOper(ctx oldcontext.Context, req *Node) (*Node, error) {
	_, resp, err := s.NodeOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respCmdV1NodeOper).V
	return &r, resp.(respCmdV1NodeOper).Err
}

func decodeHttprespCmdV1NodeOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respCmdV1NodeOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func (s *grpcServerCmdV1) ClusterOper(ctx oldcontext.Context, req *Cluster) (*Cluster, error) {
	_, resp, err := s.ClusterOperHdlr.ServeGRPC(ctx, req)
	if err != nil {
		return nil, err
	}
	r := resp.(respCmdV1ClusterOper).V
	return &r, resp.(respCmdV1ClusterOper).Err
}

func decodeHttprespCmdV1ClusterOper(_ context.Context, r *http.Response) (interface{}, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp respCmdV1ClusterOper
	err := json.NewDecoder(r.Body).Decode(&resp)
	return resp, err
}

func encodeHttpNodeSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNodeSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req NodeSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNodeSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(NodeSpec)
	return &req, nil
}

func DecodeGrpcReqNodeSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeSpec)
	return req, nil
}

func EncodeGrpcRespNodeSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNodeSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNodeStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNodeStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req NodeStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNodeStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(NodeStatus)
	return &req, nil
}

func DecodeGrpcReqNodeStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeStatus)
	return req, nil
}

func EncodeGrpcRespNodeStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNodeStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNode(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNode(_ context.Context, r *http.Request) (interface{}, error) {
	var req Node
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNode(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Node)
	return &req, nil
}

func DecodeGrpcReqNode(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Node)
	return req, nil
}

func EncodeGrpcRespNode(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNode(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpNodeList(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpNodeList(_ context.Context, r *http.Request) (interface{}, error) {
	var req NodeList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqNodeList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(NodeList)
	return &req, nil
}

func DecodeGrpcReqNodeList(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeList)
	return req, nil
}

func EncodeGrpcRespNodeList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespNodeList(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpClusterSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpClusterSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req ClusterSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqClusterSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(ClusterSpec)
	return &req, nil
}

func DecodeGrpcReqClusterSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*ClusterSpec)
	return req, nil
}

func EncodeGrpcRespClusterSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespClusterSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpClusterStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpClusterStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req ClusterStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqClusterStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(ClusterStatus)
	return &req, nil
}

func DecodeGrpcReqClusterStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*ClusterStatus)
	return req, nil
}

func EncodeGrpcRespClusterStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespClusterStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHttpCluster(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHttpRequest(ctx, req, request)
}

func decodeHttpCluster(_ context.Context, r *http.Request) (interface{}, error) {
	var req Cluster
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

func EndcodeGrpcReqCluster(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(Cluster)
	return &req, nil
}

func DecodeGrpcReqCluster(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*Cluster)
	return req, nil
}

func EncodeGrpcRespCluster(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func DecodeGrpcRespCluster(ctx context.Context, response interface{}) (interface{}, error) {
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
