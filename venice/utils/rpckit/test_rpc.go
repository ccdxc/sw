// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpckit

import (
	"context"
	"errors"

	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/log"
)

// TestRPCHandler is a handler for the test RPC defined in test.proto
type TestRPCHandler struct {
	ReqMsg  string
	RespMsg string
}

// TestRPC is the function that handles the TestRPC method calls
func (tst *TestRPCHandler) TestRPC(ctx context.Context, req *TestReq) (*TestResp, error) {
	log.Infof("Test rpc handler got request: %+v", req)

	// save the request
	tst.ReqMsg = req.ReqMsg

	// response message
	testResp := &TestResp{
		RespMsg: tst.RespMsg,
	}

	return testResp, nil
}

// TestRPCWithContext is the function that handles the TestRPCCtx method calls
func (tst *TestRPCHandler) TestRPCWithContext(ctx context.Context, req *TestReq) (*TestResp, error) {
	log.Infof("TestRPCCtx handler got request: %+v", req)

	// save the request
	tst.ReqMsg = req.ReqMsg

	// response message
	testResp := &TestResp{
		RespMsg:       tst.RespMsg,
		CallerID:      ctxutils.GetPeerID(ctx),
		CallerAddress: ctxutils.GetPeerAddress(ctx),
	}

	return testResp, nil
}

// TestRPCErr is the function that handles the TestRPCErr method calls
func (tst *TestRPCHandler) TestRPCErr(ctx context.Context, req *TestReq) (*TestResp, error) {
	return nil, errors.New("Test RPC Error response")
}

// NewTestRPCHandler returns a new TestRPCHandler with supplied parameters
func NewTestRPCHandler(reqMsg, respMsg string) *TestRPCHandler {
	return &TestRPCHandler{
		ReqMsg:  reqMsg,
		RespMsg: respMsg,
	}
}
