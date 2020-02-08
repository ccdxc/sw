// +build apulu

package fakehal

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"time"

	"google.golang.org/grpc"

	apuluproto "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// Hal implements fake hal svc layer
type Hal struct {
	ListenURL string
	Srv       *grpc.Server
}

// ########################## HAL methods used by DSCAgent

// Stop stops the Fake HAL
func (h Hal) Stop() {
	log.Infof("Stopping HAL at: %s", h.ListenURL)
	if h.Srv != nil {
		h.Srv.Stop()
	}
}

// InterfaceCreate stubbed out
func (h Hal) InterfaceCreate(ctx context.Context, req *apuluproto.InterfaceRequest) (*apuluproto.InterfaceResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceCreate Request:")
	fmt.Println(string(dat))

	return &apuluproto.InterfaceResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// InterfaceUpdate stubbed out
func (h Hal) InterfaceUpdate(ctx context.Context, req *apuluproto.InterfaceRequest) (*apuluproto.InterfaceResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceUpdate Request:")
	fmt.Println(string(dat))

	return &apuluproto.InterfaceResponse{
		ApiStatus: apuluproto.ApiStatus_API_STATUS_OK,
	}, nil
}

// InterfaceDelete stubbed out
func (h Hal) InterfaceDelete(ctx context.Context, req *apuluproto.InterfaceDeleteRequest) (*apuluproto.InterfaceDeleteResponse, error) {
	dat, _ := json.MarshalIndent(req, "", "  ")
	log.Info("Got InterfaceDelete Request:")
	fmt.Println(string(dat))

	return &apuluproto.InterfaceDeleteResponse{
		ApiStatus: []apuluproto.ApiStatus{
			apuluproto.ApiStatus_API_STATUS_OK,
		},
	}, nil
}

// NewFakeHalServer returns a fake hal server on a specified port
func NewFakeHalServer(listenURL string) *Hal {
	lis, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	f := Hal{
		ListenURL: listenURL,
		Srv:       grpc.NewServer(),
	}

	apuluproto.RegisterIfSvcServer(f.Srv, &f)
	go f.Srv.Serve(lis)

	time.Sleep(time.Second * 2)
	log.Infof("Fake HAL Started at: %s", f.ListenURL)
	return &f
}
