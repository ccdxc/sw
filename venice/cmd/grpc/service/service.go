package service

import (
	"fmt"
	"sync"
	"time"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	outCount       = 32
	defSendTimeout = 6 * time.Second
)

// RPCHandler handles all service gRPC calls.
type RPCHandler struct {
	sync.Mutex
	resolver types.ResolverService
	watchChs []chan *protos.ServiceInstanceEvent
}

// NewRPCHandler returns a new service RPC handler.
func NewRPCHandler(resolver types.ResolverService) *RPCHandler {
	s := &RPCHandler{
		resolver: resolver,
		watchChs: make([]chan *protos.ServiceInstanceEvent, 0),
	}
	resolver.Register(s)
	return s
}

// ListServices lists all services.
func (s *RPCHandler) ListServices(ctx context.Context, req *api.Empty) (*protos.ServiceList, error) {
	return s.resolver.List(), nil
}

// GetService gets a service by name.
func (s *RPCHandler) GetService(ctx context.Context, req *api.ObjectMeta) (*protos.Service, error) {
	return s.resolver.Get(req.Name), nil
}

// ListServiceInstances lists all service instances.
func (s *RPCHandler) ListServiceInstances(ctx context.Context, req *api.Empty) (*protos.ServiceInstanceList, error) {
	return s.resolver.ListInstances(), nil
}

// GetServiceInstance gets a service instance by service and instance name.
func (s *RPCHandler) GetServiceInstance(ctx context.Context, req *protos.ServiceInstanceReq) (*protos.ServiceInstance, error) {
	return s.resolver.GetInstance(req.Service, req.Instance), nil
}

// WatchServiceInstances watches changes on service instances. Sends a
// ServiceInstanceEventList to enable batching in the future without
// breaking compatibility.
func (s *RPCHandler) WatchServiceInstances(req *api.Empty, server protos.ServiceAPI_WatchServiceInstancesServer) error {
	ch := make(chan *protos.ServiceInstanceEvent, outCount)
	defer close(ch)
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
	in := make([]*protos.ServiceInstanceEvent, 0)
	for ii := range sList.Items {
		svc := sList.Items[ii]
		for jj := range svc.Instances {
			in = append(in, &protos.ServiceInstanceEvent{
				Type:     protos.ServiceInstanceEvent_Added,
				Instance: svc.Instances[jj],
			})
		}
	}
	if len(in) != 0 {
		out := &protos.ServiceInstanceEventList{
			Items: in,
		}
		if err := server.Send(out); err != nil {
			log.Errorf("Error sending service instance event %v, %v", out, err)
			return err
		}
	}
	// Watch for new instances.
	callCh := make(chan error, 1)
	var mu sync.Mutex
	for {
		select {
		case in := <-ch:
			out := &protos.ServiceInstanceEventList{
				Items: []*protos.ServiceInstanceEvent{in},
			}
			// The goroutine makes the Send() an action with a timeout. Use of the mutex is needed
			// to avoid sending on a closed channel.
			go func() {
				err1 := server.Send(out)
				mu.Lock()
				select {
				case callCh <- err1:
				default:
				}
				mu.Unlock()
			}()
			select {
			case err := <-callCh:
				if err != nil {
					log.Errorf("Error sending service instance event %v, %v", out, err)
					return err
				}
			case <-time.After(defSendTimeout):
				log.Errorf("Timeout sending service instance event %v", out)
				// do not leak the Go routine and the channel. Close the channel so the go routine can exit.
				mu.Lock()
				close(callCh)
				callCh = nil
				mu.Unlock()
				return fmt.Errorf("timeout sending event")
			}

		case <-server.Context().Done():
			return nil
		}
	}
}

// OnNotifyServiceInstance handles service notification events and sends
// it to all watchers.
func (s *RPCHandler) OnNotifyServiceInstance(e protos.ServiceInstanceEvent) error {
	for _, watchCh := range s.watchChs {
		watchCh <- &e
	}
	return nil
}
