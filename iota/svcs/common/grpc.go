package common

import (
	"fmt"
	"net"
	"time"

	"google.golang.org/grpc"

	log "github.com/pensando/sw/venice/utils/log"
)

// GRPCServer captures the GRPC Server for a service
type GRPCServer struct {
	SvcName string
	Srv     *grpc.Server
	lis     net.Listener
}

// GRPCClient captures the GRPC client connection for a service
type GRPCClient struct {
	SvcName string
	Client  *grpc.ClientConn
}

// CreateNewGRPCServer creates a new GRPC Server
func CreateNewGRPCServer(svcName, URL string) (*GRPCServer, error) {
	log.Infof("Creating a new GRPC Server for service: %v | Listening on %v", svcName, URL)
	s := GRPCServer{
		SvcName: svcName,
		Srv:     grpc.NewServer(),
	}

	if s.Srv == nil {
		log.Errorf("Error creating GRPC Server for %v", s.SvcName)
		return nil, fmt.Errorf("error creating GRPC Server for %v", s.SvcName)
	}

	// Start a listener
	lis, err := net.Listen("tcp", URL)
	if err != nil {
		log.Errorf("failed to listen to %s: Err %v", URL, err)
		return nil, err
	}
	s.lis = lis

	return &s, nil
}

// CreateNewGRPCClient creates a new GRPC Client
func CreateNewGRPCClient(svcName, URL string) (*GRPCClient, error) {
	log.Infof("Creating a new GRPC Client for service, %v. Dialing %v", svcName, URL)
	conn, err := grpc.Dial(URL, grpc.WithInsecure(), grpc.WithTimeout(3*time.Second))
	if err != nil {
		log.Errorf("Error creating GRPC Client for %v. Err: %v", URL, err)
		return nil, fmt.Errorf("error creating GRPC Client for %v. Err: %v", URL, err)
	}

	c := GRPCClient{
		SvcName: svcName,
		Client:  conn,
	}
	return &c, nil
}

// Stop gracefully stops the GRPC Server
func (s *GRPCServer) Stop() {
	log.Infof("Stopping GRPC Server for %v", s.SvcName)
	s.Srv.GracefulStop()
}

// GetListenerURL returns the Server Listener URL
func (s *GRPCServer) GetListenerURL() (URL string) {
	URL = s.lis.Addr().String()
	return
}

// Start starts a GRPC Server
func (s *GRPCServer) Start() {
	s.Srv.Serve(s.lis)
}
