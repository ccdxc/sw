// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package monitoring is a auto generated package.
Input file: auditpolicy.proto
*/
package monitoring

import (
	"context"
	"encoding/json"
	"net/http"

	"github.com/pensando/sw/api"
)

// Dummy definitions to suppress nonused warnings
var _ api.ObjectMeta

func encodeHTTPAuditPolicy(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPAuditPolicy(_ context.Context, r *http.Request) (interface{}, error) {
	var req AuditPolicy
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqAuditPolicy encodes GRPC request
func EncodeGrpcReqAuditPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*AuditPolicy)
	return req, nil
}

// DecodeGrpcReqAuditPolicy decodes GRPC request
func DecodeGrpcReqAuditPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*AuditPolicy)
	return req, nil
}

// EncodeGrpcRespAuditPolicy encodes GRC response
func EncodeGrpcRespAuditPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespAuditPolicy decodes GRPC response
func DecodeGrpcRespAuditPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPAuditPolicySpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPAuditPolicySpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req AuditPolicySpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqAuditPolicySpec encodes GRPC request
func EncodeGrpcReqAuditPolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*AuditPolicySpec)
	return req, nil
}

// DecodeGrpcReqAuditPolicySpec decodes GRPC request
func DecodeGrpcReqAuditPolicySpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*AuditPolicySpec)
	return req, nil
}

// EncodeGrpcRespAuditPolicySpec encodes GRC response
func EncodeGrpcRespAuditPolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespAuditPolicySpec decodes GRPC response
func DecodeGrpcRespAuditPolicySpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPAuditPolicyStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPAuditPolicyStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req AuditPolicyStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqAuditPolicyStatus encodes GRPC request
func EncodeGrpcReqAuditPolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*AuditPolicyStatus)
	return req, nil
}

// DecodeGrpcReqAuditPolicyStatus decodes GRPC request
func DecodeGrpcReqAuditPolicyStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*AuditPolicyStatus)
	return req, nil
}

// EncodeGrpcRespAuditPolicyStatus encodes GRC response
func EncodeGrpcRespAuditPolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespAuditPolicyStatus decodes GRPC response
func DecodeGrpcRespAuditPolicyStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPSyslogAuditor(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPSyslogAuditor(_ context.Context, r *http.Request) (interface{}, error) {
	var req SyslogAuditor
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqSyslogAuditor encodes GRPC request
func EncodeGrpcReqSyslogAuditor(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SyslogAuditor)
	return req, nil
}

// DecodeGrpcReqSyslogAuditor decodes GRPC request
func DecodeGrpcReqSyslogAuditor(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*SyslogAuditor)
	return req, nil
}

// EncodeGrpcRespSyslogAuditor encodes GRC response
func EncodeGrpcRespSyslogAuditor(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespSyslogAuditor decodes GRPC response
func DecodeGrpcRespSyslogAuditor(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}
