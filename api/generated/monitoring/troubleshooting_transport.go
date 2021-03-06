// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package monitoring is a auto generated package.
Input file: troubleshooting.proto
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

func encodeHTTPPingPktStats(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPPingPktStats(_ context.Context, r *http.Request) (interface{}, error) {
	var req PingPktStats
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqPingPktStats encodes GRPC request
func EncodeGrpcReqPingPktStats(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*PingPktStats)
	return req, nil
}

// DecodeGrpcReqPingPktStats decodes GRPC request
func DecodeGrpcReqPingPktStats(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*PingPktStats)
	return req, nil
}

// EncodeGrpcRespPingPktStats encodes GRC response
func EncodeGrpcRespPingPktStats(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespPingPktStats decodes GRPC response
func DecodeGrpcRespPingPktStats(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPPingStats(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPPingStats(_ context.Context, r *http.Request) (interface{}, error) {
	var req PingStats
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqPingStats encodes GRPC request
func EncodeGrpcReqPingStats(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*PingStats)
	return req, nil
}

// DecodeGrpcReqPingStats decodes GRPC request
func DecodeGrpcReqPingStats(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*PingStats)
	return req, nil
}

// EncodeGrpcRespPingStats encodes GRC response
func EncodeGrpcRespPingStats(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespPingStats decodes GRPC response
func DecodeGrpcRespPingStats(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTimeWindow(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTimeWindow(_ context.Context, r *http.Request) (interface{}, error) {
	var req TimeWindow
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTimeWindow encodes GRPC request
func EncodeGrpcReqTimeWindow(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TimeWindow)
	return req, nil
}

// DecodeGrpcReqTimeWindow decodes GRPC request
func DecodeGrpcReqTimeWindow(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TimeWindow)
	return req, nil
}

// EncodeGrpcRespTimeWindow encodes GRC response
func EncodeGrpcRespTimeWindow(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTimeWindow decodes GRPC response
func DecodeGrpcRespTimeWindow(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTraceRouteInfo(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTraceRouteInfo(_ context.Context, r *http.Request) (interface{}, error) {
	var req TraceRouteInfo
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTraceRouteInfo encodes GRPC request
func EncodeGrpcReqTraceRouteInfo(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TraceRouteInfo)
	return req, nil
}

// DecodeGrpcReqTraceRouteInfo decodes GRPC request
func DecodeGrpcReqTraceRouteInfo(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TraceRouteInfo)
	return req, nil
}

// EncodeGrpcRespTraceRouteInfo encodes GRC response
func EncodeGrpcRespTraceRouteInfo(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTraceRouteInfo decodes GRPC response
func DecodeGrpcRespTraceRouteInfo(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTroubleshootingSession(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTroubleshootingSession(_ context.Context, r *http.Request) (interface{}, error) {
	var req TroubleshootingSession
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTroubleshootingSession encodes GRPC request
func EncodeGrpcReqTroubleshootingSession(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TroubleshootingSession)
	return req, nil
}

// DecodeGrpcReqTroubleshootingSession decodes GRPC request
func DecodeGrpcReqTroubleshootingSession(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TroubleshootingSession)
	return req, nil
}

// EncodeGrpcRespTroubleshootingSession encodes GRC response
func EncodeGrpcRespTroubleshootingSession(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTroubleshootingSession decodes GRPC response
func DecodeGrpcRespTroubleshootingSession(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTroubleshootingSessionSpec(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTroubleshootingSessionSpec(_ context.Context, r *http.Request) (interface{}, error) {
	var req TroubleshootingSessionSpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTroubleshootingSessionSpec encodes GRPC request
func EncodeGrpcReqTroubleshootingSessionSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TroubleshootingSessionSpec)
	return req, nil
}

// DecodeGrpcReqTroubleshootingSessionSpec decodes GRPC request
func DecodeGrpcReqTroubleshootingSessionSpec(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TroubleshootingSessionSpec)
	return req, nil
}

// EncodeGrpcRespTroubleshootingSessionSpec encodes GRC response
func EncodeGrpcRespTroubleshootingSessionSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTroubleshootingSessionSpec decodes GRPC response
func DecodeGrpcRespTroubleshootingSessionSpec(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTroubleshootingSessionStatus(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTroubleshootingSessionStatus(_ context.Context, r *http.Request) (interface{}, error) {
	var req TroubleshootingSessionStatus
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTroubleshootingSessionStatus encodes GRPC request
func EncodeGrpcReqTroubleshootingSessionStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TroubleshootingSessionStatus)
	return req, nil
}

// DecodeGrpcReqTroubleshootingSessionStatus decodes GRPC request
func DecodeGrpcReqTroubleshootingSessionStatus(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TroubleshootingSessionStatus)
	return req, nil
}

// EncodeGrpcRespTroubleshootingSessionStatus encodes GRC response
func EncodeGrpcRespTroubleshootingSessionStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTroubleshootingSessionStatus decodes GRPC response
func DecodeGrpcRespTroubleshootingSessionStatus(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsAuditTrail(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsAuditTrail(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsAuditTrail
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsAuditTrail encodes GRPC request
func EncodeGrpcReqTsAuditTrail(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsAuditTrail)
	return req, nil
}

// DecodeGrpcReqTsAuditTrail decodes GRPC request
func DecodeGrpcReqTsAuditTrail(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsAuditTrail)
	return req, nil
}

// EncodeGrpcRespTsAuditTrail encodes GRC response
func EncodeGrpcRespTsAuditTrail(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsAuditTrail decodes GRPC response
func DecodeGrpcRespTsAuditTrail(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsFlowCounters(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsFlowCounters(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsFlowCounters
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsFlowCounters encodes GRPC request
func EncodeGrpcReqTsFlowCounters(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsFlowCounters)
	return req, nil
}

// DecodeGrpcReqTsFlowCounters decodes GRPC request
func DecodeGrpcReqTsFlowCounters(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsFlowCounters)
	return req, nil
}

// EncodeGrpcRespTsFlowCounters encodes GRC response
func EncodeGrpcRespTsFlowCounters(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsFlowCounters decodes GRPC response
func DecodeGrpcRespTsFlowCounters(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsFlowLogs(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsFlowLogs(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsFlowLogs
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsFlowLogs encodes GRPC request
func EncodeGrpcReqTsFlowLogs(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsFlowLogs)
	return req, nil
}

// DecodeGrpcReqTsFlowLogs decodes GRPC request
func DecodeGrpcReqTsFlowLogs(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsFlowLogs)
	return req, nil
}

// EncodeGrpcRespTsFlowLogs encodes GRC response
func EncodeGrpcRespTsFlowLogs(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsFlowLogs decodes GRPC response
func DecodeGrpcRespTsFlowLogs(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsPolicy(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsPolicy(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsPolicy
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsPolicy encodes GRPC request
func EncodeGrpcReqTsPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsPolicy)
	return req, nil
}

// DecodeGrpcReqTsPolicy decodes GRPC request
func DecodeGrpcReqTsPolicy(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsPolicy)
	return req, nil
}

// EncodeGrpcRespTsPolicy encodes GRC response
func EncodeGrpcRespTsPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsPolicy decodes GRPC response
func DecodeGrpcRespTsPolicy(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsReport(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsReport(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsReport
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsReport encodes GRPC request
func EncodeGrpcReqTsReport(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsReport)
	return req, nil
}

// DecodeGrpcReqTsReport decodes GRPC request
func DecodeGrpcReqTsReport(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsReport)
	return req, nil
}

// EncodeGrpcRespTsReport encodes GRC response
func EncodeGrpcRespTsReport(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsReport decodes GRPC response
func DecodeGrpcRespTsReport(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsResult(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsResult(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsResult
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsResult encodes GRPC request
func EncodeGrpcReqTsResult(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsResult)
	return req, nil
}

// DecodeGrpcReqTsResult decodes GRPC request
func DecodeGrpcReqTsResult(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsResult)
	return req, nil
}

// EncodeGrpcRespTsResult encodes GRC response
func EncodeGrpcRespTsResult(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsResult decodes GRPC response
func DecodeGrpcRespTsResult(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

func encodeHTTPTsStats(ctx context.Context, req *http.Request, request interface{}) error {
	return encodeHTTPRequest(ctx, req, request)
}

func decodeHTTPTsStats(_ context.Context, r *http.Request) (interface{}, error) {
	var req TsStats
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return req, nil
}

// EncodeGrpcReqTsStats encodes GRPC request
func EncodeGrpcReqTsStats(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsStats)
	return req, nil
}

// DecodeGrpcReqTsStats decodes GRPC request
func DecodeGrpcReqTsStats(ctx context.Context, request interface{}) (interface{}, error) {
	req := request.(*TsStats)
	return req, nil
}

// EncodeGrpcRespTsStats encodes GRC response
func EncodeGrpcRespTsStats(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}

// DecodeGrpcRespTsStats decodes GRPC response
func DecodeGrpcRespTsStats(ctx context.Context, response interface{}) (interface{}, error) {
	return response, nil
}
