package datapath

import (
	"context"
	"fmt"

	"google.golang.org/grpc"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
)

// Mock is the HAL mock
type Mock struct {
}

// MockHal implements a dummy HAL
func MockHal() *Mock {
	return &Mock{}
}

// Collector operations CRUD operations

// CollectorCreate mock
func (m *Mock) CollectorCreate(ctx context.Context, in *halproto.CollectorRequestMsg, opts ...grpc.CallOption) (*halproto.CollectorResponseMsg, error) {
	return &halproto.CollectorResponseMsg{
		Response: []*halproto.CollectorResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// CollectorUpdate mock
func (m *Mock) CollectorUpdate(ctx context.Context, in *halproto.CollectorRequestMsg, opts ...grpc.CallOption) (*halproto.CollectorResponseMsg, error) {
	return &halproto.CollectorResponseMsg{
		Response: []*halproto.CollectorResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// CollectorDelete mock
func (m *Mock) CollectorDelete(ctx context.Context, in *halproto.CollectorDeleteRequestMsg, opts ...grpc.CallOption) (*halproto.CollectorDeleteResponseMsg, error) {
	return &halproto.CollectorDeleteResponseMsg{
		Response: []*halproto.CollectorDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK},
		},
	}, nil
}

// CollectorGet mock
func (m *Mock) CollectorGet(ctx context.Context, in *halproto.CollectorGetRequestMsg, opts ...grpc.CallOption) (*halproto.CollectorGetResponseMsg, error) {
	return &halproto.CollectorGetResponseMsg{
		Response: []*halproto.CollectorGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// FlowMonitorRules CRUD Operations

// FlowMonitorRuleCreate mock
func (m *Mock) FlowMonitorRuleCreate(ctx context.Context, in *halproto.FlowMonitorRuleRequestMsg, opts ...grpc.CallOption) (*halproto.FlowMonitorRuleResponseMsg, error) {

	if len(in.Request) != 1 {
		return nil, fmt.Errorf("invalid number(%d) of requests ", len(in.Request))
	}

	for _, req := range in.Request {
		for _, m := range req.Match.SrcMacAddress {
			if m == 0 {
				return nil, fmt.Errorf("invalid mac address in request")
			}
		}

		for _, m := range req.Match.DstMacAddress {
			if m == 0 {
				return nil, fmt.Errorf("invalid mac address in request")
			}
		}

		for _, m := range req.Match.DstAddress {
			if m == nil {
				return nil, fmt.Errorf("invalid ip address in request")
			}
		}

		for _, m := range req.Match.SrcAddress {
			if m == nil {
				return nil, fmt.Errorf("invalid ip address in request")
			}
		}
	}

	return &halproto.FlowMonitorRuleResponseMsg{
		Response: []*halproto.FlowMonitorRuleResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// FlowMonitorRuleUpdate mock
func (m *Mock) FlowMonitorRuleUpdate(ctx context.Context, in *halproto.FlowMonitorRuleRequestMsg, opts ...grpc.CallOption) (*halproto.FlowMonitorRuleResponseMsg, error) {
	return &halproto.FlowMonitorRuleResponseMsg{
		Response: []*halproto.FlowMonitorRuleResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// FlowMonitorRuleDelete mock
func (m *Mock) FlowMonitorRuleDelete(ctx context.Context, in *halproto.FlowMonitorRuleDeleteRequestMsg, opts ...grpc.CallOption) (*halproto.FlowMonitorRuleDeleteResponseMsg, error) {
	return &halproto.FlowMonitorRuleDeleteResponseMsg{
		Response: []*halproto.FlowMonitorRuleDeleteResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// FlowMonitorRuleGet mock
func (m *Mock) FlowMonitorRuleGet(ctx context.Context, in *halproto.FlowMonitorRuleGetRequestMsg, opts ...grpc.CallOption) (*halproto.FlowMonitorRuleGetResponseMsg, error) {
	return &halproto.FlowMonitorRuleGetResponseMsg{
		Response: []*halproto.FlowMonitorRuleGetResponse{
			{
				ApiStatus: halproto.ApiStatus_API_STATUS_OK,
			},
		},
	}, nil
}

// DropMonitorRules CRUD Operations

//DropMonitorRuleCreate mock
func (m *Mock) DropMonitorRuleCreate(ctx context.Context, in *halproto.DropMonitorRuleRequestMsg, opts ...grpc.CallOption) (*halproto.DropMonitorRuleResponseMsg, error) {
	return nil, nil
}

// DropMonitorRuleUpdate mock
func (m *Mock) DropMonitorRuleUpdate(ctx context.Context, in *halproto.DropMonitorRuleRequestMsg, opts ...grpc.CallOption) (*halproto.DropMonitorRuleResponseMsg, error) {
	return nil, nil
}

// DropMonitorRuleDelete mock
func (m *Mock) DropMonitorRuleDelete(ctx context.Context, in *halproto.DropMonitorRuleDeleteRequestMsg, opts ...grpc.CallOption) (*halproto.DropMonitorRuleDeleteResponseMsg, error) {
	return nil, nil
}

//DropMonitorRuleGet mock
func (m *Mock) DropMonitorRuleGet(ctx context.Context, in *halproto.DropMonitorRuleGetRequestMsg, opts ...grpc.CallOption) (*halproto.DropMonitorRuleGetResponseMsg, error) {
	return nil, nil
}

// MirrorSessionCreate mock
func (m *Mock) MirrorSessionCreate(ctx context.Context, in *halproto.MirrorSessionRequestMsg, opts ...grpc.CallOption) (*halproto.MirrorSessionResponseMsg, error) {
	return nil, nil
}

//MirrorSessionUpdate mock
func (m *Mock) MirrorSessionUpdate(ctx context.Context, in *halproto.MirrorSessionRequestMsg, opts ...grpc.CallOption) (*halproto.MirrorSessionResponseMsg, error) {
	return nil, nil
}

// MirrorSessionDelete mock
func (m *Mock) MirrorSessionDelete(ctx context.Context, in *halproto.MirrorSessionDeleteRequestMsg, opts ...grpc.CallOption) (*halproto.MirrorSessionDeleteResponseMsg, error) {
	return nil, nil
}

// MirrorSessionGet mock
func (m *Mock) MirrorSessionGet(ctx context.Context, in *halproto.MirrorSessionGetRequestMsg, opts ...grpc.CallOption) (*halproto.MirrorSessionGetResponseMsg, error) {
	return nil, nil
}
