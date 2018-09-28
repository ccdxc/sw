package server

import (
	"fmt"
	"log"
	"net"

	"google.golang.org/grpc"

	iota "github.com/pensando/sw/iota/protos/gogen"
	//"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	IOTAListenURL = "localhost:8888"
)

func StartIOTAService() {
	// create a listener on TCP port 7777
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", 8888))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	// create a gRPC server object
	// create a gRPC server object
	var grpcServer *grpc.Server

	grpcServer = grpc.NewServer()

	topoSvc := NewTopologyService()
	cfgSvc := NewConfigService()
	iota.RegisterTopologyApiServer(grpcServer, topoSvc)
	iota.RegisterConfigMgmtApiServer(grpcServer, cfgSvc)
	log.Print("Starting Grpc server started on port :", IOTAListenURL)
	if err := grpcServer.Serve(lis); err == grpc.ErrServerStopped {
		// grpc doesn't close our listener for us, so we have to do it ourselves.
		log.Print("Stopped Grpc server started on port :", 8888)
		lis.Close()
	}

}
