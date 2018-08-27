package server

import (
	"fmt"
	"log"
	"net"
	"os"

	"google.golang.org/grpc"

	agent "github.com/pensando/sw/test/utils/infra/agent"
	pb "github.com/pensando/sw/test/utils/infra/agent/grpc/api/pb"
)

var grpcServer *grpc.Server

//StartServer Start Grpc Server
func StartServer(port int) {
	_ = os.Mkdir(agent.LogDir, 644)
	// create a listener on TCP port 7777
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
	// create a gRPC server object
	grpcServer = grpc.NewServer()
	// attach the App service to the server
	pb.RegisterAppAgentServer(grpcServer, NewAppServer())
	pb.RegisterNaplesSimServer(grpcServer, NewNaplesSim())
	// start the server
	log.Print("Starting Grpc server started on port :", port)
	if err := grpcServer.Serve(lis); err == grpc.ErrServerStopped {
		// grpc doesn't close our listener for us, so we have to do it ourselves.
		log.Print("Stopped Grpc server started on port :", port)
		lis.Close()
	}
}

//StopServer stops grpc server
func StopServer() {
	log.Print("Stopping grpc server")
	grpcServer.Stop()
}
