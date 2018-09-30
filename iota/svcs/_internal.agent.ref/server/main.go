package main

import (
	"context"
	"flag"
	"log"

	"google.golang.org/grpc"

	pb "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/globals"
)

var grpcServer *grpc.Server

type iotaStubAgent struct {
}

// AddNode brings up the node with the personality
func (*iotaStubAgent) AddNode(context.Context, *pb.IotaNode) (*pb.IotaNode, error) {
	return nil, nil
}

// DeleteNode, remove the personaltiy set
func (*iotaStubAgent) DeleteNode(context.Context, *pb.Node) (*pb.Node, error) {

	return nil, nil
}

// AddWorkloads brings up a workload type on a given node
func (*iotaStubAgent) AddWorkload(context.Context, *pb.Workload) (*pb.Workload, error) {

	return nil, nil
}

// DeleteWorkloads deletes a given workload
func (*iotaStubAgent) DeleteWorkload(context.Context, *pb.Workload) (*pb.Workload, error) {
	return nil, nil

}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (*iotaStubAgent) Trigger(context.Context, *pb.TriggerMsg) (*pb.TriggerMsg, error) {
	return nil, nil

}

// CheckClusterHealth returns the cluster health
func (*iotaStubAgent) CheckHealth(context.Context, *pb.NodeHealth) (*pb.NodeHealth, error) {
	return nil, nil

}

//AgentInstance instance
//var iotaAgent *iotaStubAgent

//StartServer Start Grpc Server
func StartServer(port int, iotaAgent pb.IotaAgentApiServer) {
	////_ = os.Mkdir(agent.LogDir, 644)
	//// create a listener on TCP port 7777
	//lis, err := net.Listen("tcp", fmt.Sprintf(":%d", port))
	//if err != nil {
	//	log.Fatalf("failed to listen: %v", err)
	//}
	//// create a gRPC server object
	//grpcServer = grpc.NewServer()
	// attach the App service to the server
	pb.RegisterIotaAgentApiServer(grpcServer, iotaAgent)

	//// start the server
	//log.Print("Starting Grpc server started on port :", port)
	//if err := grpcServer.Serve(lis); err == grpc.ErrServerStopped {
	//	// grpc doesn't close our listener for us, so we have to do it ourselves.
	//	log.Print("Stopped Grpc server started on port :", port)
	//	lis.Close()
	//}
}

//StopServer stops grpc server
func StopServer() {
	log.Print("Stopping grpc server")
	grpcServer.Stop()
}

func newiotaStubAgent() *iotaStubAgent {
	return &iotaStubAgent{}
}

func main() {
	var iotaAgent pb.IotaAgentApiServer

	if *args.stubmode {
		iotaAgent = newiotaStubAgent()
	} else {
		iotaAgent = newiotaAgent()
	}
	StartServer(globals.IotaAgentPort, iotaAgent)
}

type cmdArgs struct {
	stubmode *bool
}

var args = cmdArgs{}

func init() {
	args.stubmode = flag.Bool("stubmode", true, "Stub mode")
}
