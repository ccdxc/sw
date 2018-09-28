package server

import (
	"fmt"
	"log"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	IOTAListenURL = "localhost:1337"
)

func StartIOTAService() {
	grpcServer, err := rpckit.NewRPCServer("iota-svc", IOTAListenURL)
	if err != nil {
		fmt.Printf("could not start grpc server for IOTA. Err: %v\n", err)
		log.Fatal("Could not start GRPC Server")
	}

	topoSvc := NewTopologyService()
	cfgSvc := NewConfigService()
	iota.RegisterTopologyApiServer(grpcServer.GrpcServer, topoSvc)
	iota.RegisterConfigMgmtApiServer(grpcServer.GrpcServer, cfgSvc)
	grpcServer.Start()
}
