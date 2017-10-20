package main

import (
	"testing"

	"golang.org/x/net/context"

	"github.com/golang/mock/gomock"

	exproto "github.com/pensando/sw/venice/utils/rpckit/example/proto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// test example rpc client using go mock package
func TestExampleRpcClient(t *testing.T) {
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()

	ctx := context.Background()
	req := exproto.ExampleReq{ReqMsg: "example request"}

	// create a mock grpc client
	mockClient := exproto.NewMockExampleClient(mockCtrl)

	// Setup expected request and response to return
	mockClient.EXPECT().ExampleRPC(ctx, &req).Return(&exproto.ExampleResp{RespMsg: "example resp"}, nil)

	// make an rpc call and verify we get the results we expected..
	resp, err := mockClient.ExampleRPC(ctx, &req)
	Assert(t, (err == nil), "Error calling rpc client", err)
	Assert(t, (resp.RespMsg == "example resp"), "Error response from rpc client", resp)
}

// test example rpc server using go mock package
func TestExampleRpcServer(t *testing.T) {
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()

	ctx := context.Background()
	req := exproto.ExampleReq{ReqMsg: "example request"}

	mockServer := exproto.NewMockExampleServer(mockCtrl)
	mockServer.EXPECT().ExampleRPC(ctx, &req).Return(&exproto.ExampleResp{RespMsg: "example resp"}, nil)
	resp, err := mockServer.ExampleRPC(ctx, &req)
	Assert(t, (err == nil), "Error calling rpc server", err)
	Assert(t, (resp.RespMsg == "example resp"), "Error response from rpc server", resp)
}
