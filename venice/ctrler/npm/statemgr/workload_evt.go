// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/log"
)

// WorkloadReactor is the event reactor for workload events
type WorkloadReactor struct {
	stateMgr *Statemgr // state manager
}

// WorkloadHandler workload event handler interface
type WorkloadHandler interface {
	CreateWorkload(w workload.Workload) error
	DeleteWorkload(w workload.Workload) error
}

// networkName returns network name for the external vlan
func (wr *WorkloadReactor) networkName(extVlan uint32) string {
	return "Vlan-" + fmt.Sprintf("%d", extVlan)
}

// CreateWorkload handle workload creation
func (wr *WorkloadReactor) CreateWorkload(w workload.Workload) error {
	// loop over each interface of the workload
	for ii := range w.Spec.Interfaces {
		// check if we have a network for this workload
		netName := wr.networkName(w.Spec.Interfaces[ii].ExternalVlan)
		nw, err := wr.stateMgr.FindNetwork(w.Tenant, netName)
		if (err != nil) && (ErrIsObjectNotFound(err)) {
			err = wr.stateMgr.CreateNetwork(&network.Network{
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
			nw, _ = wr.stateMgr.FindNetwork(w.Tenant, netName)

		} else if err != nil {
			return err
		}

		// check if we already have the endpoint for this workload
		epName := w.Name + "-" + w.Spec.Interfaces[ii].MACAddress
		_, err = wr.stateMgr.FindEndpoint(w.Tenant, epName)
		if (err != nil) && (ErrIsObjectNotFound(err)) {
			// find the host for the workload
			host, err := wr.stateMgr.FindHost("", w.Spec.HostName)
			if err != nil {
				log.Errorf("Error finding the host %s for endpoint %v. Err: %v", w.Spec.HostName, epName, err)
				return err
			}

			// find the smart nic by mac addr
			nodeUUID := ""
			for jj := range host.Spec.SmartNICs {
				snicMac := host.Spec.SmartNICs[jj].MACAddress
				snic, err := wr.stateMgr.FindSmartNICByMacAddr(snicMac)
				if err != nil {
					snic, err = wr.stateMgr.FindSmartNIC("", host.Spec.SmartNICs[jj].Name)
					if err != nil {
						log.Errorf("Error finding smart nic for mac add %v", snicMac)
						return err
					}
				}
				if snic.Status.PrimaryMAC != "" {
					nodeUUID = snic.Status.PrimaryMAC
				} else {
					nodeUUID = snic.Name
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
					EndpointUUID:       epName,
					NodeUUID:           nodeUUID,
					WorkloadName:       w.Name,
					WorkloadUUID:       w.Name,
					WorkloadAttributes: w.Labels,
					MacAddress:         w.Spec.Interfaces[ii].MACAddress,
					HomingHostAddr:     "", // TODO: get host address
					HomingHostName:     w.Spec.HostName,
					MicroSegmentVlan:   w.Spec.Interfaces[ii].MicroSegVlan,
				},
			}

			_, err = nw.CreateEndpoint(&epInfo)
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

// DeleteWorkload handles workload deletion
func (wr *WorkloadReactor) DeleteWorkload(w workload.Workload) error {
	// loop over each interface of the workload
	for ii := range w.Spec.Interfaces {
		// find the network for the interface
		netName := wr.networkName(w.Spec.Interfaces[ii].ExternalVlan)
		nw, err := wr.stateMgr.FindNetwork(w.Tenant, netName)
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
		_, err = nw.DeleteEndpoint(&api.ObjectMeta{
			Name:      epName,
			Tenant:    w.Tenant,
			Namespace: w.Namespace,
		})
		if err != nil {
			log.Errorf("Error deleting the endpoint. Err: %v", err)
			return err
		}
	}

	return nil
}

// NewWorkloadReactor creates new workload event reactor
func NewWorkloadReactor(stateMgr *Statemgr) (*WorkloadReactor, error) {
	wr := WorkloadReactor{
		stateMgr: stateMgr,
	}
	return &wr, nil
}
