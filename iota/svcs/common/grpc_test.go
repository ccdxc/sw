package common

import (
	"testing"

	"github.com/pensando/sw/venice/utils/netutils"
)

func TestGRPCServer(t *testing.T) {
	t.Parallel()

	// Get an available port to start the server
	var testListener netutils.TestListenAddr
	err := testListener.GetAvailablePort()
	if err != nil {
		t.Fatalf("Could not get an available port")
	}

	// Start Server
	s, err := CreateNewGRPCServer("test-svc", testListener.ListenURL.String())
	defer s.Stop()
	if err != nil {
		t.Fatalf("Could not start a GRPC Server on %v", testListener.ListenURL.String())
	}
}

func TestGRPCClient(t *testing.T) {
	t.Parallel()

	// Get an available port to start the server
	var testListener netutils.TestListenAddr
	err := testListener.GetAvailablePort()
	if err != nil {
		t.Fatalf("Could not get an available port")
	}

	// Start Server
	s, err := CreateNewGRPCServer("test-svc", testListener.ListenURL.String())
	defer s.Stop()
	if err != nil {
		t.Fatalf("Could not start a GRPC Server on %v", testListener.ListenURL.String())
	}

	// Connect to the client
	_, err = CreateNewGRPCClient("test-client", s.GetListenerURL())
	if err != nil {
		t.Fatalf("Could not start a GRPC Server on port 13337")
	}

}
