package service

import (
	"sync"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/log"
)

const (
	outCount = 16
)

// RPCHandler handles all service gRPC calls.
type RPCHandler struct {
	sync.Mutex
	resolver types.ResolverService
	watchChs []chan *types.ServiceInstanceEvent
}

// NewRPCHandler returns a new service RPC handler.
func NewRPCHandler(resolver types.ResolverService) *RPCHandler {
	s := &RPCHandler{
		resolver: resolver,
		watchChs: make([]chan *types.ServiceInstanceEvent, 0),
	}
	resolver.Register(s)
	return s
}

// ListServices lists all services.
func (s *RPCHandler) ListServices(ctx context.Context, req *api.Empty) (*types.ServiceList, error) {
	return s.resolver.List(), nil
}

// GetService gets a service by name.
func (s *RPCHandler) GetService(ctx context.Context, req *api.ObjectMeta) (*types.Service, error) {
	return s.resolver.Get(req.Name), nil
}

// ListServiceInstances lists all service instances.
func (s *RPCHandler) ListServiceInstances(ctx context.Context, req *api.Empty) (*types.ServiceInstanceList, error) {
	return s.resolver.ListInstances(), nil
}

// GetServiceInstance gets a service instance by service and instance name.
func (s *RPCHandler) GetServiceInstance(ctx context.Context, req *types.ServiceInstanceReq) (*types.ServiceInstance, error) {
	return s.resolver.GetInstance(req.Service, req.Instance), nil
}

// WatchServiceInstances watches changes on service instances. Sends a
// ServiceInstanceEventList to enable batching in the future without
// breaking compatibility.
func (s *RPCHandler) WatchServiceInstances(req *api.Empty, server types.ServiceAPI_WatchServiceInstancesServer) error {
	ch := make(chan *types.ServiceInstanceEvent, outCount)
	s.Lock()
	s.watchChs = append(s.watchChs, ch)
	s.Unlock()
	defer func() {
		s.Lock()
		for ii := range s.watchChs {
			if s.watchChs[ii] == ch {
				s.watchChs = append(s.watchChs[:ii], s.watchChs[ii+1:]...)
				break
			}
		}
		s.Unlock()
	}()
	// Send existing instances first.
	sList := s.resolver.List()
	in := make([]*types.ServiceInstanceEvent, 0)
	for ii := range sList.Items {
		svc := sList.Items[ii]
		for jj := range svc.Instances {
			in = append(in, &types.ServiceInstanceEvent{
				Type:     types.ServiceInstanceEvent_Added,
				Instance: svc.Instances[jj],
			})
		}
	}
	if len(in) != 0 {
		out := &types.ServiceInstanceEventList{
			Items: in,
		}
		if err := server.Send(out); err != nil {
			log.Errorf("Error sending service instance event %v, %v", out, err)
			return err
		}
	}
	// Watch for new instances.
	for {
		select {
		case in := <-ch:
			out := &types.ServiceInstanceEventList{
				Items: []*types.ServiceInstanceEvent{in},
			}
			if err := server.Send(out); err != nil {
				log.Errorf("Error sending service instance event %v, %v", out, err)
				return err
			}
		case <-server.Context().Done():
			return nil
		}
	}
}

// OnNotifyServiceInstance handles service notification events and sends
// it to all watchers.
func (s *RPCHandler) OnNotifyServiceInstance(e types.ServiceInstanceEvent) error {
	for _, watchCh := range s.watchChs {
		watchCh <- &e
	}
	return nil
}
