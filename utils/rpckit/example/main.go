package main

import (
	"flag"
	"fmt"
	"os"

	log "github.com/Sirupsen/logrus"
	"golang.org/x/net/context"

	"github.com/pensando/sw/utils/rpckit"
)

// Usage:
// 0. Compile the example: go install
// 1. Run server in a window: $GOPATH/bin/example -server -url :9000
// 2. Run client in another window: $GOPATH/bin/example -client -url localhost:9000

// ExampleRPCHandler is the grpc handler
type ExampleRPCHandler struct {
	srvMsg string
}

// ExampleRPC is example rpc call handler
func (es *ExampleRPCHandler) ExampleRPC(ctx context.Context, req *ExampleReq) (*ExampleResp, error) {
	log.Infof("Example server got request: %+v", req)
	exampleResp := &ExampleResp{
		RespMsg: es.srvMsg,
	}

	return exampleResp, nil
}

// runServer runs the server
func runServer(url, certFile, keyFile, caFile string, stopChannel chan bool) {
	// create an rpc handler object
	exampleHandler := &ExampleRPCHandler{
		srvMsg: "Example Server Response",
	}

	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer(url, certFile, keyFile, caFile)
	if err != nil {
		log.Fatalf("Error creating rpc server. Err; %v", err)
	}

	// register the RPC handler
	RegisterExampleServer(rpcServer.GrpcServer, exampleHandler)
	defer func() { rpcServer.Stop() }()

	// wait forever
	<-stopChannel
}

// runClient runs the client
func runClient(url, certFile, keyFile, caFile string, count int) {
	// create an RPC client
	rpcClient, err := rpckit.NewRPCClient(url, certFile, keyFile, caFile)
	if err != nil {
		log.Errorf("Error connecting to server. Err: %v", err)
		return
	}

	exampleClient := NewExampleClient(rpcClient.ClientConn)

	// make calls `count` times
	for i := 0; i < count; i++ {
		// make a call
		resp, err := exampleClient.ExampleRPC(context.Background(), &ExampleReq{"example request"})
		if err != nil {
			log.Errorf("Got RPC error: %v", err)
			return
		}

		log.Infof("Got RPC response: %+v", resp)
	}

	log.Infof("RPC stats: %+v", rpcClient.GetRPCStats())

	// close client connection and stop the server
	rpcClient.Close()
}

// main function
func main() {
	var clientFlag, serverFlag bool
	var url string
	var certFile, keyFile, caFile string
	var count int

	// parse all flags
	flag.BoolVar(&clientFlag, "client", false, "Run in client mode")
	flag.BoolVar(&serverFlag, "server", false, "Run in server mode")
	flag.StringVar(&url, "url", "localhost:9100", "URL for server or client")
	flag.StringVar(&certFile, "cert", "", "Certificate file")
	flag.StringVar(&keyFile, "key", "", "Key file")
	flag.StringVar(&caFile, "ca", "", "Root CA file")
	flag.IntVar(&count, "count", 1, "Number of times to make rpc call")

	flag.Parse()

	// make sure necessary flags are specified
	if !serverFlag && !clientFlag {
		fmt.Printf("Need to run as either -client or -server\n  See example --help for more details\n")
		os.Exit(1)
	} else if serverFlag && clientFlag {
		fmt.Printf("Can not specify both -client and -server flags\n")
		os.Exit(1)
	} else if serverFlag {
		runServer(url, certFile, keyFile, caFile, make(chan bool))
	} else {
		runClient(url, certFile, keyFile, caFile, count)
	}
}
