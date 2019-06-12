// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package tokenauth is a auto generated package.
Input file: tokenauth.proto
*/
package tokenauth

import (
	"context"
	"encoding/json"
	"net/http"

	"github.com/pensando/sw/api"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

func encodeHTTPNodeTokenRequest(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPNodeTokenRequest(_ context.Context, r *http.Request) (interface{}, error) {
	var req NodeTokenRequest
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqNodeTokenRequest encodes GRPC request
func EncodeGrpcReqNodeTokenRequest(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeTokenRequest)
	return req, nil
}

// DecodeGrpcReqNodeTokenRequest decodes GRPC request
func DecodeGrpcReqNodeTokenRequest(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeTokenRequest)
	return req, nil
}

// EncodeGrpcRespNodeTokenRequest encodes GRC response
func EncodeGrpcRespNodeTokenRequest(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespNodeTokenRequest decodes GRPC response
func DecodeGrpcRespNodeTokenRequest(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPNodeTokenResponse(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPNodeTokenResponse(_ context.Context, r *http.Request) (interface{}, error) {
	var req NodeTokenResponse
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqNodeTokenResponse encodes GRPC request
func EncodeGrpcReqNodeTokenResponse(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeTokenResponse)
	return req, nil
}

// DecodeGrpcReqNodeTokenResponse decodes GRPC request
func DecodeGrpcReqNodeTokenResponse(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*NodeTokenResponse)
	return req, nil
}

// EncodeGrpcRespNodeTokenResponse encodes GRC response
func EncodeGrpcRespNodeTokenResponse(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespNodeTokenResponse decodes GRPC response
func DecodeGrpcRespNodeTokenResponse(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}
