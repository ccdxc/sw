// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/log"
)

// networkName returns network name for the external vlan
func (sm *Statemgr) networkName(extVlan uint32) string {
	return "Network-Vlan-" + fmt.Sprintf("%d", extVlan)
}

// OnWorkloadCreate handle workload creation
func (sm *Statemgr) OnWorkloadCreate(w *ctkit.Workload) error {
	// loop over each interface of the workload
	for ii := range w.Spec.Interfaces {
		// check if we have a network for this workload
		netName := sm.networkName(w.Spec.Interfaces[ii].ExternalVlan)
		_, err := sm.FindNetwork(w.Tenant, netName)
		if (err != nil) && (ErrIsObjectNotFound(err)) {
			err = sm.ctrler.Network().Create(&network.Network{
				TypeMeta: api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{
					Name:      netName,
					Namespace: w.Namespace,
					Tenant:    w.Tenant,
				},
				Spec: network.NetworkSpec{
					IPv4Subnet:  "",
					IPv4Gateway: "",
					VlanID:      w.Spec.Interfaces[ii].ExternalVlan,
				},
				Status: network.NetworkStatus{},
			})
			if err != nil {
				log.Errorf("Error creating network. Err: %v", err)
				return err
			}
		} else if err != nil {
			return err
		}

		// check if we already have the endpoint for this workload
		epName := w.Name + "-" + w.Spec.Interfaces[ii].MACAddress
		_, err = sm.FindEndpoint(w.Tenant, epName)
		if (err != nil) && (ErrIsObjectNotFound(err)) {
			// find the host for the workload
			host, err := sm.FindHost("", w.Spec.HostName)
			if err != nil {
				log.Errorf("Error finding the host %s for endpoint %v. Err: %v", w.Spec.HostName, epName, err)
				return err
			}

			// find the smart nic by mac addr
			nodeUUID := ""
			for jj := range host.Host.Spec.SmartNICs {
				snicMac := host.Host.Spec.SmartNICs[jj].MACAddress
				snic, err := sm.FindSmartNICByMacAddr(snicMac)
				if err != nil {
					snic, err = sm.FindSmartNIC("", host.Host.Spec.SmartNICs[jj].Name)
					if err != nil {
						log.Errorf("Error finding smart nic for mac add %v", snicMac)
						return err
					}
				}
				if snic.SmartNIC.Status.PrimaryMAC != "" {
					nodeUUID = snic.SmartNIC.Status.PrimaryMAC
				} else {
					nodeUUID = snic.SmartNIC.Name
				}
			}
			// create the endpoint for the interface
			epInfo := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      epName,
					Tenant:    w.Tenant,
					Namespace: w.Namespace,
				},
				Spec: workload.EndpointSpec{},
				Status: workload.EndpointStatus{
					Network:            netName,
					NodeUUID:           nodeUUID,
					WorkloadName:       w.Name,
					WorkloadAttributes: w.Labels,
					MacAddress:         w.Spec.Interfaces[ii].MACAddress,
					HomingHostAddr:     "", // TODO: get host address
					HomingHostName:     w.Spec.HostName,
					MicroSegmentVlan:   w.Spec.Interfaces[ii].MicroSegVlan,
				},
			}

			err = sm.ctrler.Endpoint().Create(&epInfo)
			if err != nil {
				log.Errorf("Error creating endpoint. Err: %v", err)
				return err
			}
		} else if err != nil {
			return err
		}
	}

	return nil
}

// OnWorkloadUpdate handles workload update event
func (sm *Statemgr) OnWorkloadUpdate(w *ctkit.Workload) error {
	return nil
}

// OnWorkloadDelete handles workload deletion
func (sm *Statemgr) OnWorkloadDelete(w *ctkit.Workload) error {
	// loop over each interface of the workload
	for ii := range w.Spec.Interfaces {
		// find the network for the interface
		netName := sm.networkName(w.Spec.Interfaces[ii].ExternalVlan)
		nw, err := sm.FindNetwork(w.Tenant, netName)
		if err != nil {
			log.Errorf("Error finding the network %v. Err: %v", netName, err)
			return err
		}

		// check if we created an endpoint for this workload interface
		epName := w.Name + "-" + w.Spec.Interfaces[ii].MACAddress
		_, ok := nw.FindEndpoint(epName)
		if !ok {
			log.Errorf("Could not find endpoint %v", epName)
			return fmt.Errorf("Endpoint not found")
		}

		// delete the endpoint
		err = sm.ctrler.Endpoint().Delete(&workload.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Name:      epName,
				Tenant:    w.Tenant,
				Namespace: w.Namespace,
			},
		})
		if err != nil {
			log.Errorf("Error deleting the endpoint. Err: %v", err)
			return err
		}
	}

	return nil
}
