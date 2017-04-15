# RPCKit example

Simple rpckit example

### Usage:

1. Generate protobuf definitions

```
pushd proto
$GOPATH/src/github.com/pensando/sw/utils/rpckit/codegen.sh ./example.proto
popd
```

2. Compile the example:

```
go install
```

3. Run server in a window:

```
$GOPATH/bin/example -server -url :9000
```

4. Run client in another window:

```
$GOPATH/bin/example -client -url localhost:9000
```

### Mock testing

We can use https://github.com/golang/mock tool for mocking grpc client/server interfaces.


1. Generate mock grpc interface from protobuf definitions

```
pushd proto
mockgen -source=example.pb.go -destination=example_mock.go -package=proto
popd
```

2. Use `mockgen` generated code in unit tests as described in https://godoc.org/github.com/golang/mock/gomock

Example
```
// test example rpc client using go mock package
func TestExampleRpcClient(t *testing.T) {
    mockCtrl := gomock.NewController(t)
    defer mockCtrl.Finish()

    ctx := context.Background()
    req := ExampleReq{"example request"}

    // create a mock grpc client
    mockClient := NewMockExampleClient(mockCtrl)

    // Setup expected request and response to return
    mockClient.EXPECT().ExampleRPC(ctx, &req).Return(&ExampleResp{"example resp"}, nil)

    // make an rpc call and verify we get the results we expected..
    resp, err := mockClient.ExampleRPC(ctx, &req)
    Assert(t, (err == nil), "Error calling rpc client", err)
    Assert(t, (resp.RespMsg == "example resp"), "Error response from rpc client", resp)
}
```
