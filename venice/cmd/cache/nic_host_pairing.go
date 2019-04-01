// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cache

import (
	"fmt"
	"reflect"

	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

func isNICHostPair(nic *cluster.SmartNIC, host *cluster.Host) bool {
	// TODO -- do range check for MAC address
	// Right now host can only have 1 SmartNICID. If in the future it can have more, we should
	// report conflicts between SmartNICIDs belonging to the same host.
	for _, nicID := range host.Spec.SmartNICs {
		if nic != nil && nic.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String() &&
			((nicID.Name != "" && nicID.Name == nic.Spec.Hostname) || (nicID.MACAddress != "" && nicID.MACAddress == nic.Status.PrimaryMAC)) {
			return true
		}
	}
	return false
}

// UpdateHostPairingStatus is called when a NIC is created/updated/deleted to recompute pairing with existing Host objects
// newNIC is the notification object and is always present, oldNIC is a snapshot of the object before the update and
// is non-nil only if event type == update
func (sm *Statemgr) UpdateHostPairingStatus(et kvstore.WatchEventType, newNIC, oldNIC *cluster.SmartNIC) ([]*cluster.SmartNIC, []*cluster.Host, error) {
	var hostUpdates []*cluster.Host
	var nicUpdates []*cluster.SmartNIC

	hosts, err := sm.ListHosts()
	if err != nil {
		return nil, nil, fmt.Errorf("Error getting list of Host objects")
	}

	handleCreate := func(nic *cluster.SmartNIC) {
		// Go through all hosts, check SmartNIC IDs, add nic to status if there's a match
		// If there's more than 1 match, report conflict
		for _, host := range hosts {
			host.Lock()
			if isNICHostPair(nic, host.Host) {
				if nic.Status.Host == "" {
					host.Status.AdmittedSmartNICs = utils.AppendStringIfNotPresent(nic.Name, host.Status.AdmittedSmartNICs)
					hostUpdates = append(hostUpdates, host.Host)
					nic.Status.Host = host.Name
					nicUpdates = append(nicUpdates, nic)
					log.Infof("NIC %s(%s) paired with host %s", nic.Name, nic.Spec.Hostname, host.Name)
				} else if nic.Status.Host != host.Name {
					errMsg := fmt.Sprintf("NIC %s(%s) matches Spec IDs of host %s but is already associated with host %s", nic.Name, nic.Spec.Hostname, host.Name, nic.Status.Host)
					recorder.Event(cluster.HostSmartNICSpecConflict, evtsapi.SeverityLevel_WARNING, errMsg, nil)
					log.Errorf(errMsg)
				}
			}
			host.Unlock()
		}
	}

	handleDelete := func(nic *cluster.SmartNIC) {
		// go through all hosts, check SmartNIC IDs, remove reference if present
		for _, host := range hosts {
			host.Lock()
			for ii, hostNIC := range host.Status.AdmittedSmartNICs {
				if hostNIC == nic.Name {
					host.Status.AdmittedSmartNICs = append(host.Status.AdmittedSmartNICs[:ii], host.Status.AdmittedSmartNICs[ii+1:]...)
					hostUpdates = append(hostUpdates, host.Host)
					log.Infof("Removed pairing between NIC %s(%s) and host %s", nic.Name, nic.Spec.Hostname, host.Name)
					break
				}
			}
			host.Unlock()
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
		if newNIC.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String() {
			if oldNIC != nil && oldNIC.Spec.Hostname != newNIC.Spec.Hostname {
				handleDelete(oldNIC)
				newNIC.Status.Host = ""
				nicUpdates = append(nicUpdates, newNIC)
			}
			if newNIC.Status.Host == "" {
				handleCreate(newNIC)
			}
		}
	}

	return nicUpdates, hostUpdates, nil
}

// UpdateNICPairingStatus is called when a Host is created/updated/deleted to recompute pairing with existing NIC objects
// newHost is the notification object and is always present, oldHost is a snapshot of the object before the update and
// is non-nil only if event type == update
func (sm *Statemgr) UpdateNICPairingStatus(et kvstore.WatchEventType, newHost, oldHost *cluster.Host) ([]*cluster.SmartNIC, []*cluster.Host, error) {
	var nicUpdates []*cluster.SmartNIC
	var hostUpdates []*cluster.Host

	nics, err := sm.ListSmartNICs()
	if err != nil {
		return nil, nil, fmt.Errorf("Error getting list of Host objects")
	}

	handleCreate := func(host *cluster.Host) {
		// Go through all NICs, see if any of them matches one of the host SmartNIC IDs.
		// If we find a NIC that matches but is already paired with another Host, report conflict
		for _, nic := range nics {
			nic.Lock()
			if isNICHostPair(nic.SmartNIC, host) {
				if nic.Status.Host == "" {
					host.Status.AdmittedSmartNICs = utils.AppendStringIfNotPresent(nic.Name, host.Status.AdmittedSmartNICs)
					hostUpdates = append(hostUpdates, host)
					nic.Status.Host = host.Name
					nicUpdates = append(nicUpdates, nic.SmartNIC)
					log.Infof("NIC %s(%s) paired with host %s", nic.Name, nic.Spec.Hostname, host.Name)
				} else {
					errMsg := fmt.Sprintf("NIC %s(%s) matches Spec IDs of host %s but is already associated with host %s", nic.Name, nic.Spec.Hostname, host.Name, nic.Status.Host)
					log.Errorf(errMsg)
					recorder.Event(cluster.HostSmartNICSpecConflict, evtsapi.SeverityLevel_WARNING, errMsg, nil)
				}
			}
			nic.Unlock()
		}
	}

	handleDelete := func(host *cluster.Host) {
		// go through all NICs paired with this host and mark them as free
		for _, nicName := range host.Status.AdmittedSmartNICs {
			nic, err := sm.FindSmartNIC("", nicName)
			if err != nil {
				log.Errorf("Error getting SmartNIC %s. Err: %v", nicName, err)
				continue
			}
			nic.Lock()
			nic.Status.Host = ""
			nicUpdates = append(nicUpdates, nic.SmartNIC)
			log.Infof("Removed pairing between NIC %s(%s) and host %s", nic.Name, nic.Spec.Hostname, host.Name)
			nic.Unlock()
		}
	}

	switch et {
	case kvstore.Created:
		handleCreate(newHost)

	case kvstore.Deleted:
		handleDelete(newHost)

	case kvstore.Updated:
		if oldHost != nil && !reflect.DeepEqual(newHost.Spec.SmartNICs, oldHost.Spec.SmartNICs) {
			handleDelete(oldHost)
			newHost.Status.AdmittedSmartNICs = []string{}
			hostUpdates = append(hostUpdates, newHost)
			handleCreate(newHost)
		}
	}

	return nicUpdates, hostUpdates, nil
}
