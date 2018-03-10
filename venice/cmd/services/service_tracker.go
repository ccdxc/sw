package services

import (
	"sync"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// ServiceTracker tracks location of interested services
type ServiceTracker struct {
	sync.Mutex

	resolver       types.ResolverService
	nodeService    types.NodeService
	resolverClient resolver.Interface

	// used on master node to track the current advertised leader.
	//  At this point the list is kube apiserver only
	leaderAddr string

	// addrs is used to collect all possible service instances from resolver
	addrs map[string]map[string]struct{}
}

// NewServiceTracker returns an instance of ServiceTracker.
func NewServiceTracker(resolver types.ResolverService) types.ServiceTracker {
	s := &ServiceTracker{
		resolver: resolver,
		addrs:    make(map[string]map[string]struct{}),
	}

	s.addrs[globals.KubeAPIServer] = make(map[string]struct{})
	s.addrs[globals.ElasticSearch] = make(map[string]struct{})
	return s
}

// Run the service tracker
func (m *ServiceTracker) Run(resolverClient interface{}, nodeService types.NodeService) {
	m.nodeService = nodeService
	m.resolverClient = resolverClient.(resolver.Interface)
	m.resolverClient.Register(m)

	srvInstanceList := m.resolverClient.Lookup(globals.KubeAPIServer)
	if srvInstanceList != nil {
		for _, i := range srvInstanceList.Items {
			m.OnNotifyResolver(protos.ServiceInstanceEvent{Type: protos.ServiceInstanceEvent_Added, Instance: i})
		}
	}

	srvInstanceList = m.resolverClient.Lookup(globals.ElasticSearch)
	if srvInstanceList != nil {
		for _, i := range srvInstanceList.Items {
			m.OnNotifyResolver(protos.ServiceInstanceEvent{Type: protos.ServiceInstanceEvent_Added, Instance: i})
		}
	}

}

// Stop the service tracker
func (m *ServiceTracker) Stop() {
	m.resolverClient.Deregister(m)
	m.nodeService = nil
	m.resolverClient = nil
}

// OnNotifyLeaderEvent is called on Quorum node when leadership changes
func (m *ServiceTracker) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	if m.leaderAddr != e.Leader {
		m.resolver.DeleteServiceInstance(&protos.ServiceInstance{
			TypeMeta: api.TypeMeta{
				Kind: "ServiceInstance",
			},
			ObjectMeta: api.ObjectMeta{
				Name: m.leaderAddr,
			},
			Service: globals.KubeAPIServer,
			Node:    m.leaderAddr,
		})
	}
	m.leaderAddr = e.Leader
	m.resolver.AddServiceInstance(&protos.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: m.leaderAddr,
		},
		Service: globals.KubeAPIServer,
		Node:    m.leaderAddr,
	})
	return nil
}

// OnNotifyResolver is called when resolverClient finds an updated info
func (m *ServiceTracker) OnNotifyResolver(e protos.ServiceInstanceEvent) error {
	m.Lock()
	defer m.Unlock()

	if e.Instance.Node == "" {
		return nil
	}

	addrs, ok := m.addrs[e.Instance.Service]
	if !ok {
		return nil // we are not interested this service
	}

	switch e.Type {
	case protos.ServiceInstanceEvent_Added:
		addrs[e.Instance.Node] = struct{}{}
	case protos.ServiceInstanceEvent_Deleted:
		delete(addrs, e.Instance.Node)
	}
	var list []string
	for a := range addrs {
		list = append(list, a)
	}
	m.setAPIAddress(e.Instance.Service, list)
	return nil
}

func (m *ServiceTracker) setAPIAddress(service string, addrs []string) {
	if addrs == nil || len(addrs) == 0 {
		return
	}
	switch service {
	case globals.KubeAPIServer:
		if err := m.nodeService.KubeletConfig(addrs[0]); err != nil {
			log.Errorf("Failed to update kubelet config, err: %v", err)
		}
		if err := m.nodeService.ElasticMgmtConfig(); err != nil {
			log.Errorf("Failed to update elastic-mgmt config, err: %v", err)
		}
	case globals.ElasticSearch:
		if err := m.nodeService.FileBeatConfig(addrs); err != nil {
			log.Errorf("Failed to update filebeat config, err: %v", err)
		}
		if err := m.nodeService.ElasticDiscoveryConfig(addrs); err != nil {
			log.Errorf("Failed to update elastic-discovery config, err: %v", err)
		}
	}
}
