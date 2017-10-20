package main

import (
	"flag"
	"fmt"
	"math/rand"
	"os"
	"runtime"
	"runtime/pprof"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
)

// Usage:
// 0. Compile the example: go install
// 1. Run server in a window: $GOPATH/bin/example -server -url :9000
// 2. Run client in another window: $GOPATH/bin/example -client -url localhost:9000

// ExampleRPCHandler is the grpc handler
type ExampleRPCHandler struct {
	srvMsg string // dummy variable
}

// ExampleRPC is example rpc call handler
func (es *ExampleRPCHandler) ExampleRPC(ctx context.Context, req *ExampleReq) (*ExampleResp, error) {
	log.Infof("Example server got request: %+v", req)
	exampleResp := &ExampleResp{
		RespMsg: req.ReqMsg,
	}

	return exampleResp, nil
}

// runServer runs the server
func runServer(url string, stopChannel chan bool, tlsProvider rpckit.TLSProvider) {
	// create an rpc handler object
	exampleHandler := &ExampleRPCHandler{
		srvMsg: "Example Server",
	}

	// create an RPC server
	rpcServer, err := rpckit.NewRPCServer("benchmarkServer", url, rpckit.WithTLSProvider(tlsProvider))
	if err != nil {
		log.Fatalf("Error creating rpc server. Err; %v", err)
	}

	// register the RPC handler
	RegisterExampleServer(rpcServer.GrpcServer, exampleHandler)
	defer func() { rpcServer.Stop() }()

	// wait forever
	<-stopChannel
}

func randString(n int) string {
	var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	b := make([]rune, n)
	for i := range b {
		b[i] = letters[rand.Intn(len(letters))]
	}
	return string(b)
}

// runClient runs the client
func runClient(url string, length, count, concurrency int, tlsProvider rpckit.TLSProvider) {
	endChannel := make(chan bool, concurrency)

	// record starting time
	startTime := time.Now()

	// create a request message of required length
	req := ExampleReq{
		ReqMsg: randString(length),
	}

	// run concurrent goroutines
	for c := 0; c < concurrency; c++ {
		go func(endChannel chan bool) {
			rpcClient, err := rpckit.NewRPCClient("grpc.local", url, rpckit.WithTLSProvider(tlsProvider))
			if err != nil {
				log.Errorf("Error connecting to server. Err: %v", err)
				endChannel <- true
				return
			}

			exampleClient := NewExampleClient(rpcClient.ClientConn)

			// make calls `count` times
			for i := 0; i < count; i++ {
				// make a call
				resp, err := exampleClient.ExampleRPC(context.Background(), &req)
				if err != nil {
					log.Errorf("Got RPC error: %v", err)
				} else {
					log.Infof("Got RPC response: %+v", resp)
				}

			}

			log.Infof("RPC stats: %+v", rpcClient.GetRPCStats())

			// close client connection and stop the server
			rpcClient.Close()

			endChannel <- true
		}(endChannel)
	}

	// wait for all goroutines to end
	for c := 0; c < concurrency; c++ {
		<-endChannel
	}

	// record end time and report the difference
	endTime := time.Now()
	totDuration := endTime.Sub(startTime)

	fmt.Printf("%d Requests with %d concurrency took %s\n", (count * concurrency), concurrency, totDuration.String())
	fmt.Printf("%f requests per second\n", (float64(count*concurrency) / totDuration.Seconds()))
}

// main function
func main() {
	var clientFlag, serverFlag bool
	var url string
	var certFile, keyFile, caFile string
	var count int
	var cpus int
	var length int
	var concurrency int
	var cpuprofile string

	// parse all flags
	flag.BoolVar(&clientFlag, "client", false, "Run in client mode")
	flag.BoolVar(&serverFlag, "server", false, "Run in server mode")
	flag.StringVar(&url, "url", "localhost:9100", "URL for server or client")
	flag.StringVar(&certFile, "cert", "", "Certificate file")
	flag.StringVar(&keyFile, "key", "", "Key file")
	flag.StringVar(&caFile, "ca", "", "Root CA file")
	flag.IntVar(&cpus, "cpu", 1, "Number of CPUs to use")
	flag.IntVar(&length, "len", 100, "Size of RPC request message")
	flag.IntVar(&count, "count", 1, "Number of times to make rpc call")
	flag.IntVar(&concurrency, "concurrency", 1, "Number of concurrent connections")
	flag.StringVar(&cpuprofile, "cpuprofile", "", "write cpu profile to file")

	flag.Parse()

	// set the number of CPUs to use
	runtime.GOMAXPROCS(cpus)

	// make sure necessary flags are specified
	if !serverFlag && !clientFlag {
		fmt.Printf("Need to run as either -client or -server\n  See example --help for more details\n")
		os.Exit(1)
	} else if serverFlag && clientFlag {
		fmt.Printf("Can not specify both -client and -server flags\n")
		os.Exit(1)
	}

	// profile the cpu if required
	if cpuprofile != "" {
		f, err := os.Create(cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}

	var tlsProvider rpckit.TLSProvider
	var err error
	if certFile != "" {
		tlsProvider, err = tlsproviders.NewFileBasedProvider(certFile, keyFile, caFile)
		if err != nil {
			log.Fatal(err)
		}
		log.Info("Initialized TLS provider")
	} else {
		log.Info("No TLS provider configured")
	}

	// suppress info logs while running benchmark
	log.SetFilter(log.AllowWarnFilter)

	// run server or client
	if serverFlag {
		runServer(url, make(chan bool), tlsProvider)
	} else {
		runClient(url, length, count, concurrency, tlsProvider)
	}
}
