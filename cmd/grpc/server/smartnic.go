package server

import (
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/cmd/grpc"
	context "golang.org/x/net/context"
)

// SmartNICServer handles SmartNIC gRPC service.
type SmartNICServer struct {
}

// NewSmartNICServer returns a SmartNIC RPC server object
func NewSmartNICServer() *SmartNICServer {
	return &SmartNICServer{}
}

// RegisterNIC handler skeleton
func (s *SmartNICServer) RegisterNIC(ctx context.Context, req *grpc.RegisterNICRequest) (*grpc.RegisterNICResponse, error) {

	return &grpc.RegisterNICResponse{Status: cmd.SmartNICPhase_NIC_ADMITTED}, nil
}

// UpdateNIC handler skeleton
func (s *SmartNICServer) UpdateNIC(ctx context.Context, req *grpc.UpdateNICRequest) (*grpc.UpdateNICResponse, error) {

	return &grpc.UpdateNICResponse{}, nil
}
