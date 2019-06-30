package services

import (
	"fmt"
	"reflect"
	"sort"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// ServiceTracker tracks location of interested services
type ServiceTracker struct {
	sync.Mutex

	resolver       types.ResolverService
	nodeService    types.NodeService
	resolverClient resolver.Interface
	isQuorumNode   bool

	// used on master node to track the current advertised leader.
	//  At this point the list is kube apiserver only
	leaderAddr string

	// before the service got enabled with Run(), leader election already happened and informed us.
	//	However we could not take action immediately. As soon as Run() is called, do the necessary action
	savedLeaderAddr string

	// addrs is used to collect all possible service instances from resolver
	addrs map[string]map[string]struct{}

	enabled bool
}

// NewServiceTracker returns an instance of ServiceTracker.
func NewServiceTracker(resolver types.ResolverService) types.ServiceTracker {
	s := &ServiceTracker{
		resolver: resolver,
		addrs:    make(map[string]map[string]struct{}),
	}
	if resolver != nil {
		s.isQuorumNode = true
	}

	s.addrs[globals.KubeAPIServer] = make(map[string]struct{})
	return s
}

// Run the service tracker
func (m *ServiceTracker) Run(resolverClient interface{}, nodeService types.NodeService) {

	m.Lock()
	m.enabled = true
	m.nodeService = nodeService
	m.resolverClient = resolverClient.(resolver.Interface)
	m.resolverClient.Register(m)
	if m.savedLeaderAddr != "" {
		m.leaderEventHandler(m.savedLeaderAddr)
	}
	m.Unlock()

	srvInstanceList := m.resolverClient.Lookup(globals.KubeAPIServer)
	if srvInstanceList != nil {
		for _, i := range srvInstanceList.Items {
			m.OnNotifyResolver(protos.ServiceInstanceEvent{Type: protos.ServiceInstanceEvent_Added, Instance: i})
		}
	}
}

// Stop the service tracker
func (m *ServiceTracker) Stop() {
	m.Lock()
	defer m.Unlock()
	m.enabled = false
	m.resolverClient.Deregister(m)
	m.nodeService = nil
	m.resolverClient = nil
}

func (m *ServiceTracker) leaderEventHandler(newLeader string) {
	log.Infof("updating leader from %s to %s in service tracker", m.leaderAddr, newLeader)

	if m.leaderAddr != newLeader && m.leaderAddr != "" {
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

		m.resolver.DeleteServiceInstance(&protos.ServiceInstance{
			TypeMeta: api.TypeMeta{
				Kind: "ServiceInstance",
			},
			ObjectMeta: api.ObjectMeta{
				Name: globals.CmdNICUpdatesSvc,
			},
			Service: globals.CmdNICUpdatesSvc,
			Node:    m.leaderAddr,
			URL:     fmt.Sprintf("%s:%s", m.leaderAddr, globals.CMDSmartNICUpdatesPort),
		})
	}

	m.leaderAddr = newLeader
	if newLeader == "" {
		return
	}

	_ = m.resolver.AddServiceInstance(&protos.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: m.leaderAddr,
		},
		Service: globals.KubeAPIServer,
		Node:    m.leaderAddr,
	})

	_ = m.resolver.AddServiceInstance(&protos.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.CmdNICUpdatesSvc,
		},
		Service: globals.CmdNICUpdatesSvc,
		Node:    m.leaderAddr,
		URL:     fmt.Sprintf("%s:%s", m.leaderAddr, globals.CMDSmartNICUpdatesPort),
	})

	// On a quorum node use the leadership event to redirect kubelet to the new kube-api-server
	// This has faster reaction time than listening to this event via resolver, in case the
	// resolver server is dead (the grpc balancer times out after a while adding unnecessary extra delay)
	// Also see VS-391 for a complicated error case
	if m.isQuorumNode {
		if err := m.nodeService.KubeletConfig(m.leaderAddr); err != nil {
			log.Errorf("Failed to update kubelet config, err: %v", err)
		}
	}
}

// OnNotifyLeaderEvent is called on Quorum node when leadership changes
func (m *ServiceTracker) OnNotifyLeaderEvent(e types.LeaderEvent) error {
	m.Lock()
	defer m.Unlock()
	if !m.enabled {
		m.savedLeaderAddr = e.Leader
		return nil
	}
	m.leaderEventHandler(e.Leader)

	return nil
}

// OnNotifyResolver is called when resolverClient finds an updated info
func (m *ServiceTracker) OnNotifyResolver(e protos.ServiceInstanceEvent) error {
	m.Lock()
	defer m.Unlock()

	if !m.enabled {
		return nil
	}

	if e.Instance.Node == "" {
		return nil
	}

	addrs, ok := m.addrs[e.Instance.Service]
	if !ok {
		return nil // we are not interested this service
	}

	var existingList []string
	var newList []string

	// get the existing list of addresses
	for addr := range addrs {
		existingList = append(existingList, addr)
	}

	switch e.Type {
	case protos.ServiceInstanceEvent_Added:
		addrs[e.Instance.Node] = struct{}{}
	case protos.ServiceInstanceEvent_Deleted:
		delete(addrs, e.Instance.Node)
	}

	// new list of addresses
	for addr := range addrs {
		newList = append(newList, addr)
	}

	sort.Strings(existingList)
	sort.Strings(newList)
	if !reflect.DeepEqual(existingList, newList) {
		m.setAPIAddress(e.Instance.Service, newList)
	}
	return nil
}

func (m *ServiceTracker) setAPIAddress(service string, addrs []string) {
	if addrs == nil || len(addrs) == 0 {
		return
	}
	switch service {
	case globals.KubeAPIServer:
		if !m.isQuorumNode {
			if err := m.nodeService.KubeletConfig(addrs[0]); err != nil {
				log.Errorf("Failed to update kubelet config, err: %v", err)
			}
		}
		if err := m.nodeService.ElasticMgmtConfig(); err != nil {
			log.Errorf("Failed to update elastic-mgmt config, err: %v", err)
		}
	}
}
