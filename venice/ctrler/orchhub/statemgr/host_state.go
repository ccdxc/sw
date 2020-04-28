package statemgr

import (
	"context"
	"fmt"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/runtime"
)

// HostState is a wrapper for host object
type HostState struct {
	sync.Mutex
	Host     *ctkit.Host
	stateMgr *Statemgr
}

//GetHostWatchOptions gets options
func (sm *Statemgr) GetHostWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
}

// OnHostCreate creates a host based on watch event
func (sm *Statemgr) OnHostCreate(nh *ctkit.Host) error {
	hostState, err := NewHostState(nh, sm)
	if err != nil {
		return err
	}

	isHostOrchhubManaged := false
	orchNameValue := ""

	if hostState.Host.Labels != nil {
		orchNameValue, isHostOrchhubManaged = hostState.Host.Labels[utils.OrchNameKey]
	}

	if isHostOrchhubManaged {
		var snic *DistributedServiceCardState
		for jj := range hostState.Host.Spec.DSCs {
			snic = sm.FindDSC(hostState.Host.Spec.DSCs[jj].MACAddress, hostState.Host.Spec.DSCs[jj].ID)
			if snic != nil {
				break
			}
		}

		if snic == nil || snic.DistributedServiceCard.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
			sm.logger.Infof("DSC for host [%v] is not admitted", hostState.Host.Name)
			recorder.Event(eventtypes.ORCH_DSC_NOT_ADMITTED,
				fmt.Sprintf("DSC for host [%v] is not admitted", hostState.Host.Name),
				nil)
			return nil
		}

		// Add Orchhub label to the DSC object
		if snic.DistributedServiceCard.Labels == nil {
			snic.DistributedServiceCard.Labels = make(map[string]string)
		}

		snic.DistributedServiceCard.Labels[utils.OrchNameKey] = orchNameValue
		err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
		if err != nil {
			sm.logger.Errorf("Failed to update orchhub label for DSC [%v]. Err : %v", snic.DistributedServiceCard.Name, err)
		}

		if !snic.isOrchestratorCompatible() {
			sm.AddIncompatibleDSCToOrch(snic.DistributedServiceCard.Name, orchNameValue)
			recorder.Event(eventtypes.ORCH_DSC_MODE_INCOMPATIBLE,
				fmt.Sprintf("DSC [%v] mode is incompatible for orchestration feature", snic.DistributedServiceCard.Name),
				nil)
		}
	}

	return nil
}

// OnHostUpdate handles update event
func (sm *Statemgr) OnHostUpdate(nh *ctkit.Host, newHost *cluster.Host) error {
	hostState, err := HostStateFromObj(nh)
	if err != nil {
		return err
	}
	hostState.Host.Host = *newHost

	var snic *DistributedServiceCardState
	for jj := range hostState.Host.Spec.DSCs {
		snic = sm.FindDSC(hostState.Host.Spec.DSCs[jj].MACAddress, hostState.Host.Spec.DSCs[jj].ID)
		if snic != nil {
			break
		}
	}

	if snic != nil {
		ok := false
		isHostOrchhubManaged := false
		orchNameValue := ""

		if hostState.Host.Labels != nil {
			orchNameValue, isHostOrchhubManaged = hostState.Host.Labels[utils.OrchNameKey]
		}

		if snic.DistributedServiceCard.Labels != nil {
			_, ok = snic.DistributedServiceCard.Labels[utils.OrchNameKey]
		}

		if !ok && isHostOrchhubManaged {
			// Add Orchhub label to the DSC object
			if snic.DistributedServiceCard.Labels == nil {
				snic.DistributedServiceCard.Labels = make(map[string]string)
			}

			snic.DistributedServiceCard.Labels[utils.OrchNameKey] = orchNameValue
			err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
			if err != nil {
				sm.logger.Errorf("Failed to update orchhub label for DSC [%v]. Err : %v", snic.DistributedServiceCard.Name, err)
			}
		} else if ok && !isHostOrchhubManaged && snic.DistributedServiceCard.Labels != nil {
			// Remove Label from DSC object
			delete(snic.DistributedServiceCard.Labels, utils.OrchNameKey)
			err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
			if err != nil {
				sm.logger.Errorf("Failed to remove orhchub label from DSC object [%v]. Err : %v", snic.DistributedServiceCard.Name, err)
			}
		}
	}

	return nil
}

// OnHostDelete deletes a host
func (sm *Statemgr) OnHostDelete(nh *ctkit.Host) error {
	hostState, err := HostStateFromObj(nh)
	if err != nil {
		return err
	}

	if hostState.Host.Labels != nil {
		_, isHostOrchhubManaged := hostState.Host.Labels[utils.OrchNameKey]
		if isHostOrchhubManaged {
			var snic *DistributedServiceCardState
			for jj := range hostState.Host.Spec.DSCs {
				snic = sm.FindDSC(hostState.Host.Spec.DSCs[jj].MACAddress, hostState.Host.Spec.DSCs[jj].ID)
				if snic != nil {
					break
				}
			}

			if snic != nil && snic.DistributedServiceCard.ObjectMeta.Labels != nil {
				delete(snic.DistributedServiceCard.Labels, utils.OrchNameKey)
				err := snic.stateMgr.ctrler.DistributedServiceCard().Update(&snic.DistributedServiceCard.DistributedServiceCard)
				if err != nil {
					sm.logger.Errorf("Failed to update DSC. Err : %v", err)
				}
			}
		}
	}

	return nil
}

// OnHostReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnHostReconnect() {
	sm.logger.Infof("Sending host reconnect event")
	sm.ctkitReconnectCh <- string(cluster.KindHost)
	return
}

// NewHostState create new network state
func NewHostState(host *ctkit.Host, stateMgr *Statemgr) (*HostState, error) {
	w := &HostState{
		Host:     host,
		stateMgr: stateMgr,
	}
	host.HandlerCtx = w

	return w, nil
}

// HostStateFromObj converts from memdb object to host state
func HostStateFromObj(obj runtime.Object) (*HostState, error) {
	switch obj.(type) {
	case *ctkit.Host:
		nobj := obj.(*ctkit.Host)
		switch nobj.HandlerCtx.(type) {
		case *HostState:
			nts := nobj.HandlerCtx.(*HostState)
			return nts, nil
		default:
			return nil, fmt.Errorf("Wrong type")
		}
	default:
		return nil, fmt.Errorf("Wrong type")
	}
}

// ListHostByNamespace list all hosts which have the namespace label
func (sm *Statemgr) ListHostByNamespace(orchName, namespace string) ([]*ctkit.Host, error) {
	opts := api.ListWatchOptions{}
	opts.LabelSelector = fmt.Sprintf("%v=%v,%v=%v", utils.OrchNameKey, orchName, utils.NamespaceKey, namespace)
	return sm.ctrler.Host().List(context.Background(), &opts)
}

// DeleteHostByNamespace deletes hosts by namespace
func (sm *Statemgr) DeleteHostByNamespace(orchName, namespace string) error {
	hosts, err := sm.ListHostByNamespace(orchName, namespace)
	if err != nil {
		return err
	}

	for _, host := range hosts {
		hostState, err := HostStateFromObj(host)
		if err != nil {
			sm.logger.Errorf("Failed to get host. Err : %v", err)
		}

		hostState.stateMgr.ctrler.Host().Delete(&host.Host)
	}

	return nil
}
