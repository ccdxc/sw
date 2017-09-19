# rpckit

rpckit is a library with convenient wrappers for using gRPC for inter process communication.

Please see http://www.grpc.io/ for more information on gRPC.
gRPC uses [protobuf](https://github.com/google/protobuf) as the IDL definition format.
[Here](http://www.grpc.io/docs/tutorials/basic/go.html) is a good tutorial on how to use gRPC in golang.

## Getting started

### Step 1: Define a `.proto` file for the gRPC service.

Here is an example:

```
// Example RPC definitions

syntax = "proto3";

package main;

// Example RPC definitions
service Example {
	// example rpc call
	rpc ExampleRPC (ExampleReq) returns (ExampleResp) {};
}

// RPC request
message ExampleReq {
	string reqMsg = 1;
}

// RPC response
message ExampleResp {
	string respMsg = 1;
}
```

Once you create the `example.proto` file, you can compile it by running

```
$GOPATH/src/github.com/pensando/sw/utils/rpckit/codegen.sh ./example.proto
```

This will generate golang protobuf bindings `example.pb.go` file in current folder.


### Step 2: Define the RPC backend object that handles the RPC calls

```
// ExampleRpcHandler is the grpc handler
type ExampleRpcHandler struct {
    srvMsg string
}

// Example rpc call handler
func (es *ExampleRpcHandler) ExampleRPC(ctx context.Context, req *ExampleReq) (*ExampleResp, error) {
    log.Infof("Example server got request: %+v", req)
    exampleResp := &ExampleResp{
        RespMsg: es.srvMsg,
    }

    return exampleResp, nil
}
```

### Step 3: Create an RPC server

```
    // create an RPC server
    rpcServer := rpckit.NewRpcServer("exampleServer", ":9000")
```

### Step 4: Register the backend object with RPC server

```
    // create an rpc handler object
    exampleHandler := &ExampleRpcHandler{
        srvMsg: "Example Server Response",
    }

    // register the object with RPC server
    RegisterExampleServer(rpcServer.GrpcServer, exampleHandler)

```

### Step 4: Create an RPC client

```
    // create an RPC client
    rpcClient, err := rpckit.NewRpcClient("exampleClient", "localhost:9000")
    if err != nil {
        log.Errorf("Error connecting to server. Err: %v", err)
        return
    }
```

### Step 5: Make RPC call

```
    // make a call
    exampleClient := NewExampleClient(rpcClient.ClientConn)
    resp, err := exampleClient.ExampleRPC(context.Background(), &ExampleReq{"example request"})
    if err != nil {
        log.Errorf("Got RPC error: %v", err)
        return
    }

    // print the response
    log.Infof("Got RPC response: %+v", resp)
```

## Example
See [example](./example) directory for an example

## Compiling protobuf definitions

### Install protobuf compiler

```
git clone https://github.com/google/protobuf.git
cd protobuf
./autogen.sh && ./configure && make && make check
sudo make install && sudo ldconfig
```

### Get go-grpc packages

```
go get google.golang.org/grpc
go get -a github.com/golang/protobuf/protoc-gen-go
```

## Using TLS for mutual authentication

gRPC supports TLS transport and certificate based mutual authentication. Please see [here](http://www.grpc.io/docs/guides/auth.html) and [here](https://github.com/grpc/grpc-go/blob/master/Documentation/grpc-auth-support.md) for more information on how to use TLS with gRPC.

```
              +---------+
              | Root CA |
              +---------+
               /       \
         Sign /         \ Sign
             V           V
+----------------+ TLS  +----------------+
| Client Keypair |<---->| Server Keypair |
+----------------+      +----------------+


```

In summary, both Server and Client trust a common Root CA. Both Server and Client get their public key certificates signed by the Root CA. Now they can authenticate each other. gRPC servers are setup to request client certificates and vice versa. So, in TLS environment, clients and servers need valid certificates and trusted Root CA certificate to connect to each other.

There is a utility script `genkey.sh` which can be used to create Root CA, server and client certificates. Just run it with `genkey.sh <prefix>` and it'll generate all three certificates and private keys. See [example](./example) directory for an example of how to use TLS certificates.

TLS is an add-on functionality for rpckit. To use TLS, library user must first instantiate a TLS provider and pass it to NewRPCServer or NewRPCClient using WithTLSProvider option.

TLS providers are located in directory rpckit/tlsproviders. The simplest one is FileBasedProvider which simply reads keys and certificates from files and generates TLS options for clients and server. All TLS providers must implement the interface defined in rpckit/interfaces.go

This code snippet, for example, enables TLS for an RPC Server

```
tlsProvider, err := tlsproviders.NewFileBasedProvider(certFile, keyFile, caFile)
//  check err here
tlsRPCServer, err := NewRPCServer("grpc.local", tlsURL, WithTLSProvider(tlsProvider))
```

## RPC tracing using zipkin

rpckit has [opentracing](http://opentracing.io/) based middleware that sends RPC call traces to zipkin backend. Currently this is being sent to a hardcoded URL `http://node1:9411/api/v1/spans`. Going forward, this will be a configurable URL. Traces are sent in background after RPC call completes(though call timestamps are collected inline). To view trace output run openzipkin container on vagrant VM `node1`:

```
sudo docker run -d -p 9410-9411:9410-9411 openzipkin/zipkin:1.12.0
```

to view the trace point your browser to `http://192.168.30.11:9411/`. Then you can select the service names from pull down menu on the left and click `Find Traces` button. You should see all the traces for your service.
