package statemgr

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// MockInstanceManager is the mock instance manager state
type MockInstanceManager struct {
	watchCtx          context.Context
	watchCancel       context.CancelFunc
	instanceManagerCh chan *kvstore.WatchEvent
}

// NewMockInstanceManger Creates mock instance manager
func NewMockInstanceManger(instanceManagerCh chan *kvstore.WatchEvent) *MockInstanceManager {
	watchCtx, watchCancel := context.WithCancel(context.Background())

	return &MockInstanceManager{
		instanceManagerCh: instanceManagerCh,
		watchCtx:          watchCtx,
		watchCancel:       watchCancel,
	}
}

// NewMockStateManager returns a new mock state manager
func NewMockStateManager() (*Statemgr, *MockInstanceManager, error) {
	instanceMgrCh := make(chan *kvstore.WatchEvent, 64)

	stateMgr, err := NewStatemgr(globals.APIServer, nil, log.GetNewLogger(log.GetDefaultConfig("orhhub-test")), instanceMgrCh)
	if err != nil {
		return nil, nil, err
	}

	stateMgr.SetAPIClient(nil)

	im := NewMockInstanceManger(instanceMgrCh)
	return stateMgr, im, nil
}

// CreateNetwork utility function to create a network
func CreateNetwork(stateMgr *Statemgr, tenant, net, subnet, gw string, vlanid uint32, labels map[string]string, orch *orchestration.Orchestrator) (*network.Network, error) {
	// network params
	np := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: "default",
			Tenant:    tenant,
			Labels:    labels,
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
			VlanID:      vlanid,
			Orchestrators: []*network.OrchestratorInfo{
				{
					Name:      orch.ObjectMeta.Name,
					Namespace: orch.ObjectMeta.Namespace,
				},
			},
		},
		Status: network.NetworkStatus{},
	}

	// create a network
	err := stateMgr.ctrler.Network().Create(&np)
	return &np, err
}

// GetOrchestratorConfig get orchestrator config
func GetOrchestratorConfig(name, user, pass string) *orchestration.Orchestrator {
	return &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: "default",
			// Don't set Tenant as object is not scoped inside Tenant in proto file.
		},
		TypeMeta: api.TypeMeta{
			Kind: "Orchestrator",
		},
		Spec: orchestration.OrchestratorSpec{
			Type: "vcenter",
			URI:  name,
			Credentials: &monitoring.ExternalCred{
				AuthType: "username-password",
				UserName: user,
				Password: pass,
			},
		},
	}
}
