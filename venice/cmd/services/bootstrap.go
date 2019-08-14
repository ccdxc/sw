// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package services

import (
	"fmt"
	"net"
	"net/http"

	"github.com/pensando/sw/venice/cmd/rest"
	"github.com/pensando/sw/venice/utils/log"
)

// ClusterBootstrapService provides an API to trigger cluster formation.
// Only available on nodes that are not part of a cluster yet.
// Once the cluster is created, the endpoint is shut down and all ops come
// through API Gateway over gRPC.
type ClusterBootstrapService struct {
	addr     string
	listener net.Listener
}

// Start starts the service endpoint
func (s *ClusterBootstrapService) Start() error {
	handler := rest.NewClusterRESTHandler()
	listener, err := net.Listen("tcp", s.addr)
	if err != nil {
		return fmt.Errorf("Error starting REST endpoint: %v", err)
	}
	go http.Serve(listener, handler)
	s.listener = listener

	log.Infof("Started Cluster Bootstrap Service at %s", listener.Addr())
	return nil
}

// Stop stops the service endpoint
func (s *ClusterBootstrapService) Stop() {
	s.listener.Close()
	s.listener = nil
	log.Infof("Stopped Cluster Bootstrap Service")
}

// NewClusterBootstrapService returns a new instance of the service
func NewClusterBootstrapService(addr string) *ClusterBootstrapService {
	return &ClusterBootstrapService{
		addr: addr,
	}
}
