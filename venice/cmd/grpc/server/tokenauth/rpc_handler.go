// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package tokenauth

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/tokenauth"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/log"
)

// RPCHandler handles all certificates gRPC calls.
type RPCHandler struct {
	TokenAuthService types.TokenAuthService
}

// GenerateNodeToken generates a node token for the supplied request
func (h *RPCHandler) GenerateNodeToken(ctx context.Context, req *tokenauth.NodeTokenRequest) (*tokenauth.NodeTokenResponse, error) {
	token, err := h.TokenAuthService.GenerateNodeToken(req.Audience, req.ValidityStart, req.ValidityEnd)
	if err != nil {
		return nil, fmt.Errorf("Error generating token: %v", err)
	}
	log.Infof("Generated Node Auth token, request: %v", req)
	return &tokenauth.NodeTokenResponse{
		Token: token,
	}, nil
}

// AutoWatchSvcTokenAuthV1 is an unimplemented method of the TokenAuthV1Server interface
func (h *RPCHandler) AutoWatchSvcTokenAuthV1(*api.AggWatchOptions, tokenauth.TokenAuthV1_AutoWatchSvcTokenAuthV1Server) error {
	return fmt.Errorf("AutoWatchSvcTokenAuth not implemented")
}

// NewRPCHandler returns a new handler for the RPC interface
func NewRPCHandler(svc types.TokenAuthService) *RPCHandler {
	return &RPCHandler{
		TokenAuthService: svc,
	}
}
