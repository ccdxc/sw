package utils

import (
	"fmt"
	"math"
	"os"
	"strings"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	pegasusGRPCDefaultBaseURL = globals.Localhost
)

// CreateNewGRPCClient creates a grpc connection to HAL
func CreateNewGRPCClient(port string) (*grpc.ClientConn, error) {
	pdsPort := os.Getenv("PDS_MS_GRPC_PORT")
	if pdsPort == "" {
		pdsPort = port
	}
	srvURL := pegasusGRPCDefaultBaseURL + ":" + pdsPort
	var grpcOpts []grpc.DialOption
	grpcOpts = append(grpcOpts, grpc.WithMaxMsgSize(math.MaxInt32-1))
	grpcOpts = append(grpcOpts, grpc.WithInsecure())
	rpcClient, err := grpc.Dial(srvURL, grpcOpts...)
	if err != nil {
		log.Errorf("Creating gRPC Client failed. Server URL: %s", srvURL)
		return nil, err
	}

	return rpcClient, err
}

// PrintHeader prints a CLI header given the format and fields names
func PrintHeader(format string, fields string) {
	var hdrs []interface{}
	strs := strings.Split(fields, ",")
	for _, s := range strs {
		hdrs = append(hdrs, s)
	}
	headerStr := fmt.Sprintf(format, hdrs...)
	fmt.Println(strings.Repeat("-", len(headerStr)))
	fmt.Printf("%s\n", headerStr)
	fmt.Println(strings.Repeat("-", len(headerStr)))
}
