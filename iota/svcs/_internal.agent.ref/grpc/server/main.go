package server

import (
	"fmt"
	"log"
	"net"
	"os"

	"google.golang.org/grpc"

	agent "github.com/pensando/sw/iota/svcs/agent"
	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
)

var grpcServer *grpc.Server

//QemuInstance qemu instance
var QemuInstance *Qemu

//AppServerInstance appserver instance
var AppServerInstance *AppServer

//NaplesSimInstance naples sim instance
var NaplesSimInstance *NaplesSim

//VeniceInstance naples sim instance
var VeniceInstance *Venice

//StartServer Start Grpc Service
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
	pb.RegisterAppAgentServer(grpcServer, AppServerInstance)
	AppServerInstance.log("Registered App server")
	pb.RegisterNaplesSimServer(grpcServer, NaplesSimInstance)
	AppServerInstance.log("Registered Naples sim server")
	pb.RegisterQemuServer(grpcServer, QemuInstance)
	AppServerInstance.log("Registered Qemu server")
	pb.RegisterVeniceServer(grpcServer, VeniceInstance)
	AppServerInstance.log("Registered Venice server")

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

func init() {
	QemuInstance = NewQemu()
	AppServerInstance = NewAppServer()
	NaplesSimInstance = NewNaplesSim()
	VeniceInstance = NewVenice()
}
