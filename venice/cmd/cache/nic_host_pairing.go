// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cache

import (
	"fmt"
	"reflect"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

func isNICHostPair(nic *cluster.DistributedServiceCard, host *cluster.Host) bool {
	// TODO -- do range check for MAC address
	// Right now host can only have 1 SmartNICID. If in the future it can have more, we should
	// report conflicts between SmartNICIDs belonging to the same host.
	for _, nicID := range host.Spec.DSCs {
		if nic != nil && nic.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() &&
			((nicID.ID != "" && nicID.ID == nic.Spec.ID) || (nicID.MACAddress != "" && nicID.MACAddress == nic.Status.PrimaryMAC)) {
			return true
		}
	}
	return false
}

// UpdateHostPairingStatus is called when a NIC is created/updated/deleted to recompute pairing with existing Host objects.
// newNIC is the updated object and is always present, oldNIC is a snapshot of the object before the update and
// is non-nil only if event type == update
// Caller is responsible for acquiring the lock on newNIC before invocation and releasing it afterwards.
// Function updates local cache and sends notifications for Host and SmartNIC objects to ApiServer.
func (sm *Statemgr) UpdateHostPairingStatus(et kvstore.WatchEventType, newNIC, oldNIC *cluster.DistributedServiceCard) error {
	hosts, err := sm.ListHosts()
	if err != nil {
		return fmt.Errorf("Error getting list of Host objects")
	}

	handleCreate := func(nic *cluster.DistributedServiceCard) {
		// Go through all hosts, check SmartNIC IDs, add nic to status if there's a match
		// If there's more than 1 match, report conflict
		for _, hostState := range hosts {
			hostState.Lock()
			host := hostState.Host
			if isNICHostPair(nic, host) {
				if nic.Status.Host == "" {
					host.Status.AdmittedDSCs = utils.AppendStringIfNotPresent(nic.Name, host.Status.AdmittedDSCs)
					sm.UpdateHost(host, true)
					nic.Status.Host = host.Name
					sm.UpdateSmartNIC(nic, true, false)
					log.Infof("NIC %s(%s) paired with host %s", nic.Spec.ID, nic.Name, host.Name)
				} else if nic.Status.Host != host.Name {
					errMsg := fmt.Sprintf("DSC %s(%s) matches Spec IDs of host %s but is already associated with host"+
						" %s", nic.Spec.ID, nic.Name, host.Name, nic.Status.Host)
					recorder.Event(eventtypes.HOST_DSC_SPEC_CONFLICT, errMsg, nil)
					log.Errorf(errMsg)
				}
			}
			hostState.Unlock()
		}
	}

	handleDelete := func(nic *cluster.DistributedServiceCard) {
		// go through all hosts, check SmartNIC IDs, remove reference if present
		for _, hostState := range hosts {
			hostState.Lock()
			host := hostState.Host
			for ii, hostNIC := range host.Status.AdmittedDSCs {
				if hostNIC == nic.Name {
					host.Status.AdmittedDSCs = append(host.Status.AdmittedDSCs[:ii], host.Status.AdmittedDSCs[ii+1:]...)
					sm.UpdateHost(host, true)
					log.Infof("Removed pairing between NIC %s(%s) and host %s", nic.Name, nic.Spec.ID, host.Name)
					break
				}
			}
			hostState.Unlock()
		}
	}

	switch et {
	case kvstore.Created:
		handleCreate(newNIC)

	case kvstore.Deleted:
		handleDelete(newNIC)

	case kvstore.Updated:
		// We don't care about updates for non-admitted NICs.
		// The de-admission case is handled explicitly in venice/cmd/services/master.go
		if newNIC.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String() {
			if oldNIC != nil && oldNIC.Spec.ID != newNIC.Spec.ID {
				handleDelete(oldNIC)
				newNIC.Status.Host = ""
				sm.UpdateSmartNIC(newNIC, true, false)
			}
			if newNIC.Status.Host == "" {
				handleCreate(newNIC)
			}
		}
	}

	return nil
}

// UpdateNICPairingStatus is called when a Host is created/updated/deleted to recompute pairing with existing NIC objects.
// newHost is the updated object and is always present, oldHost is a snapshot of the object before the update and
// is non-nil only if event type == update
// Caller is responsible for acquiring the lock on newHost before invocation and releasing it afterwards.
// Function updates local cache and sends notifications for Host and SmartNIC objects to ApiServer.
func (sm *Statemgr) UpdateNICPairingStatus(et kvstore.WatchEventType, newHost, oldHost *cluster.Host) error {
	nics, err := sm.ListSmartNICs()
	if err != nil {
		return fmt.Errorf("Error getting list of Host objects")
	}

	handleCreate := func(host *cluster.Host) {
		// Go through all NICs, see if any of them matches one of the host SmartNIC IDs.
		// If we find a NIC that matches but is already paired with another Host, report conflict
		for _, nic := range nics {
			nic.Lock()
			if isNICHostPair(nic.DistributedServiceCard, host) {
				if nic.Status.Host == "" {
					host.Status.AdmittedDSCs = utils.AppendStringIfNotPresent(nic.Name, host.Status.AdmittedDSCs)
					sm.UpdateHost(host, true)
					nic.Status.Host = host.Name
					sm.UpdateSmartNIC(nic.DistributedServiceCard, true, false)
					log.Infof("NIC %s(%s) paired with host %s", nic.Name, nic.Spec.ID, host.Name)
				} else if nic.Status.Host != host.Name {
					errMsg := fmt.Sprintf("DSC %s(%s) matches Spec IDs of host %s but is already associated with host"+
						" %s", nic.Name, nic.Spec.ID, host.Name, nic.Status.Host)
					recorder.Event(eventtypes.HOST_DSC_SPEC_CONFLICT, errMsg, nil)
					log.Errorf(errMsg)
				}
			}
			nic.Unlock()
		}
	}

	handleDelete := func(host *cluster.Host) {
		// go through all NICs paired with this host and mark them as free
		for _, nicName := range host.Status.AdmittedDSCs {
			nic, err := sm.FindSmartNIC(nicName)
			if err != nil {
				log.Errorf("Error getting SmartNIC %s. Err: %v", nicName, err)
				continue
			}
			nic.Lock()
			nic.Status.Host = ""
			log.Infof("Removed pairing between NIC %s(%s) and host %s", nic.Name, nic.Spec.ID, host.Name)
			// See if they can be paired with another host
			allHosts, err := sm.ListHosts()
			if err == nil {
				for _, otherHost := range allHosts {
					if otherHost.Name == host.Name {
						continue
					}
					otherHost.Lock()
					if isNICHostPair(nic.DistributedServiceCard, otherHost.Host) {
						nic.Status.Host = otherHost.Name
						otherHost.Status.AdmittedDSCs = utils.AppendStringIfNotPresent(nic.Name, otherHost.Status.AdmittedDSCs)
						sm.UpdateHost(otherHost.Host, true)
						otherHost.Unlock()
						break
					}
					otherHost.Unlock()
				}
			} else {
				log.Errorf("Error getting list of Host objects: %v", err)
			}
			sm.UpdateSmartNIC(nic.DistributedServiceCard, true, false)
			nic.Unlock()
		}
	}

	switch et {
	case kvstore.Created:
		handleCreate(newHost)

	case kvstore.Deleted:
		handleDelete(newHost)

	case kvstore.Updated:
		if oldHost != nil && !reflect.DeepEqual(newHost.Spec.DSCs, oldHost.Spec.DSCs) {
			handleDelete(oldHost)
			newHost.Status.AdmittedDSCs = []string{}
			handleCreate(newHost)
			sm.UpdateHost(newHost, true)
		}
	}

	return nil
}
