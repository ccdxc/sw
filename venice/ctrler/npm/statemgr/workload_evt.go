// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/strconv"
)

// WorkloadState is a wrapper for host object
type WorkloadState struct {
	Workload  *ctkit.Workload `json:"-"` // workload object
	stateMgr  *Statemgr       // pointer to state manager
	endpoints sync.Map        // list of endpoints
}

// WorkloadStateFromObj conerts from ctkit object to workload state
func WorkloadStateFromObj(obj runtime.Object) (*WorkloadState, error) {
	switch obj.(type) {
	case *ctkit.Workload:
		wobj := obj.(*ctkit.Workload)
		switch wobj.HandlerCtx.(type) {
		case *WorkloadState:
			nsobj := wobj.HandlerCtx.(*WorkloadState)
			return nsobj, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

//GetWorkloadWatchOptions gets options
func (sm *Statemgr) GetWorkloadWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
}

// NewWorkloadState creates new workload state object
func NewWorkloadState(wrk *ctkit.Workload, stateMgr *Statemgr) (*WorkloadState, error) {
	w := &WorkloadState{
		Workload: wrk,
		stateMgr: stateMgr,
	}
	wrk.HandlerCtx = w

	return w, nil
}

// networkName returns network name for the external vlan
func (sm *Statemgr) networkName(extVlan uint32) string {
	return "Network-Vlan-" + fmt.Sprintf("%d", extVlan)
}

// OnWorkloadCreate handle workload creation
func (sm *Statemgr) OnWorkloadCreate(w *ctkit.Workload) error {
	log.Infof("Creating workload: %+v", w)

	// create new workload object
	ws, err := NewWorkloadState(w, sm)
	if err != nil {
		log.Errorf("Error creating workload %+v. Err: %v", w, err)
		return err
	}

	// find the host for the workload
	host, err := ws.stateMgr.ctrler.Host().Find(&api.ObjectMeta{Name: w.Spec.HostName})
	if err != nil {
		// retry again if we cant find the host. In cases where host and workloads are created back to back,
		// there might be slight delay before host is available
		time.Sleep(20 * time.Millisecond)
		host, err = ws.stateMgr.ctrler.Host().Find(&api.ObjectMeta{Name: w.Spec.HostName})
		if err != nil {
			log.Errorf("Error finding the host %s for workload %v. Err: %v", w.Spec.HostName, w.Name, err)
			return kvstore.NewKeyNotFoundError(w.Spec.HostName, 0)
		}
	}

	// lock the host to make sure only one workload is operating on the host
	host.Lock()
	defer host.Unlock()

	hsts, err := HostStateFromObj(host)
	if err != nil {
		log.Errorf("Error finding the host state %s for workload %v. Err: %v", w.Spec.HostName, w.Name, err)
		return err
	}

	hsts.addWorkload(w)

	return ws.createEndpoints()
}

// OnWorkloadUpdate handles workload update event
func (sm *Statemgr) OnWorkloadUpdate(w *ctkit.Workload, nwrk *workload.Workload) error {
	// see if anything changed
	_, ok := ref.ObjDiff(w.Spec, nwrk.Spec)
	if (nwrk.GenerationID == w.GenerationID) && !ok {
		w.ObjectMeta = nwrk.ObjectMeta
		return nil
	}
	w.ObjectMeta = nwrk.ObjectMeta

	log.Infof("Updating workload: %+v", nwrk)

	recreate := false

	// check if host parameter has changed
	if nwrk.Spec.HostName != w.Spec.HostName {
		recreate = true
	}

	// check interface params changed
	if len(nwrk.Spec.Interfaces) != len(w.Spec.Interfaces) {
		// number of interfaces changed, delete old ones
		recreate = true
	} else {
		for idx, intf := range nwrk.Spec.Interfaces {
			// check what changed
			if w.Spec.Interfaces[idx].ExternalVlan != intf.ExternalVlan {
				// external VLAN changed delete old endpoints
				recreate = true
			}
			if w.Spec.Interfaces[idx].MicroSegVlan != intf.MicroSegVlan {
				// useg vlan changed, delete old endpoint
				recreate = true
			}
			if w.Spec.Interfaces[idx].MACAddress != intf.MACAddress {
				// mac address changed, delete old endpoints
				recreate = true
			}
		}
	}

	// if we dont need to recreate endpoints, we are done
	if !recreate {
		return nil
	}

	ws, err := sm.FindWorkload(w.Tenant, w.Name)
	if err != nil {
		return err
	}

	// delete old endpoints
	err = ws.deleteEndpoints()
	if err != nil {
		log.Errorf("Error deleting old endpoint. Err: %v", err)
	}

	// update the spec
	w.Spec = nwrk.Spec
	ws.Workload.Spec = nwrk.Spec

	// trigger creation of new endpoints
	return ws.createEndpoints()
}

// reconcileWorkload checks if the endpoints are create for the workload and tries to create them
func (sm *Statemgr) reconcileWorkload(w *ctkit.Workload, hst *HostState, snic *DistributedServiceCardState) error {
	// find workload
	ws, err := sm.FindWorkload(w.Tenant, w.Name)
	if err != nil {
		return err
	}
	if snic == nil {
		// delete all endpoint for this workload since we dont have an associated smartnic
		err := ws.deleteEndpoints()
		if err != nil {
			log.Errorf("Error deleting all endpoints on workload %v: %v", w.Name, err)
		}
	} else {
		// make sure we have endpoint for all workload interfaces
		for ii := range w.Spec.Interfaces {
			// check if we already have the endpoint for this workload
			name, _ := strconv.ParseMacAddr(w.Spec.Interfaces[ii].MACAddress)
			epName := w.Name + "-" + name
			_, err := sm.FindEndpoint(w.Tenant, epName)
			pending, _ := sm.EndpointIsPending(w.Tenant, epName)
			if err != nil && !pending {
				err = ws.createEndpoints()
				if err != nil {
					log.Errorf("Error creating endpoints for workload. Err: %v", err)
				}
			}
		}
	}

	return nil
}

// OnWorkloadDelete handles workload deletion
func (sm *Statemgr) OnWorkloadDelete(w *ctkit.Workload) error {
	log.Infof("Deleting workload: %+v", w)

	// find the host for the workload
	host, err := sm.FindHost("", w.Spec.HostName)
	if err != nil {
		log.Errorf("Error finding the host %s for workload %v. Err: %v", w.Spec.HostName, w.Name, err)
	} else {
		host.removeWorkload(w)
	}

	ws, err := WorkloadStateFromObj(w)
	if err != nil {
		log.Errorf("Error finding workload state %s for workload %v. Err: %v", w.Spec.HostName, w.Name, err)
		return err
	}

	return ws.deleteEndpoints()
}

// createNetwork creates a network for workload's external vlan
func (ws *WorkloadState) createNetwork(netName string, extVlan uint32) error {
	// acquire a lock per network
	lk, ok := ws.stateMgr.networkLocks[netName]
	if !ok {
		lk = &sync.Mutex{}
		ws.stateMgr.networkLocks[netName] = lk
	}
	lk.Lock()
	defer lk.Unlock()

	nwt := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      netName,
			Namespace: ws.Workload.Namespace,
			Tenant:    ws.Workload.Tenant,
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  "",
			IPv4Gateway: "",
			VlanID:      extVlan,
		},
		Status: network.NetworkStatus{},
	}

	// create it in apiserver
	return ws.stateMgr.ctrler.Network().CreateEvent(&nwt)
}

// createEndpoints tries to create all endpoints for a workload
func (ws *WorkloadState) createEndpoints() error {
	var ns *NetworkState
	// find the host for the workload
	host, err := ws.stateMgr.FindHost("", ws.Workload.Spec.HostName)
	if err != nil {
		log.Errorf("Error finding the host %s for workload %v. Err: %v", ws.Workload.Spec.HostName, ws.Workload.Name, err)
		return kvstore.NewKeyNotFoundError(ws.Workload.Spec.HostName, 0)
	}

	// loop over each interface of the workload
	ws.stateMgr.Lock()
	defer ws.stateMgr.Unlock()
	for ii := range ws.Workload.Spec.Interfaces {
		// check if we have a network for this workload
		netName := ws.stateMgr.networkName(ws.Workload.Spec.Interfaces[ii].ExternalVlan)
		ns, err = ws.stateMgr.FindNetwork(ws.Workload.Tenant, netName)
		if err != nil {
			err = ws.createNetwork(netName, ws.Workload.Spec.Interfaces[ii].ExternalVlan)
			if err != nil {
				log.Errorf("Error creating network. Err: %v", err)
				return err
			}
		}

		// check if we already have the endpoint for this workload
		name, _ := strconv.ParseMacAddr(ws.Workload.Spec.Interfaces[ii].MACAddress)
		epName := ws.Workload.Name + "-" + name
		nodeUUID := ""
		// find the smart nic by name or mac addr
		for jj := range host.Host.Spec.DSCs {
			if host.Host.Spec.DSCs[jj].ID != "" {
				snic, err := ws.stateMgr.FindDistributedServiceCardByHname(host.Host.Spec.DSCs[jj].ID)
				if err != nil {
					log.Warnf("Error finding smart nic for name %v", host.Host.Spec.DSCs[jj].ID)
					return nil
				}
				nodeUUID = snic.DistributedServiceCard.Name
			} else if host.Host.Spec.DSCs[jj].MACAddress != "" {
				snicMac := host.Host.Spec.DSCs[jj].MACAddress
				snic, err := ws.stateMgr.FindDistributedServiceCardByMacAddr(snicMac)
				if err != nil {
					log.Warnf("Error finding smart nic for mac add %v", snicMac)
					return nil
				}
				nodeUUID = snic.DistributedServiceCard.Name
			}
		}

		/* FIXME: comment out this code till CMD publishes associated NICs in host
		// check if the host has associated smart nic
		if len(host.Host.Status.AdmittedDSCs) == 0 {
			log.Errorf("Host %v does not have a smart nic", w.Spec.HostName)
			return fmt.Errorf("Host does not have associated smartnic")
		}

		for _, snicName := range host.Host.Status.AdmittedDSCs {
			snic, err := sm.FindDistributedServiceCard(host.Host.Tenant, snicName)
			if err != nil {
				log.Errorf("Error finding smart nic object for %v", snicName)
				return err
			}
			nodeUUID = snic.DistributedServiceCard.Name
		}
		*/

		// check if an endpoint with this mac address already exists in this network
		epMac := ws.Workload.Spec.Interfaces[ii].MACAddress
		if ns != nil {
			mep, err := ns.FindEndpointByMacAddr(epMac)
			if err == nil && mep.Endpoint.Name != epName {
				// we found a duplicate mac address
				log.Errorf("Error creating endpoint %s. Macaddress %s already exists in ep %s", epName, epMac, mep.Endpoint.Name)
				ws.Workload.Status.PropagationStatus.Status = "Propagation Failed. Duplicate MAC address"

				// write the status back
				return ws.Workload.Write()
			}
		}

		// create the endpoint for the interface
		epInfo := workload.Endpoint{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Name:      epName,
				Tenant:    ws.Workload.Tenant,
				Namespace: ws.Workload.Namespace,
			},
			Spec: workload.EndpointSpec{},
			Status: workload.EndpointStatus{
				Network:            netName,
				NodeUUID:           nodeUUID,
				WorkloadName:       ws.Workload.Name,
				WorkloadAttributes: ws.Workload.Labels,
				MacAddress:         epMac,
				HomingHostAddr:     "", // TODO: get host address
				HomingHostName:     ws.Workload.Spec.HostName,
				MicroSegmentVlan:   ws.Workload.Spec.Interfaces[ii].MicroSegVlan,
			},
		}
		if len(ws.Workload.Spec.Interfaces[ii].IpAddresses) > 0 {
			epInfo.Status.IPv4Address = ws.Workload.Spec.Interfaces[ii].IpAddresses[0]
		}

		// see if we need to delete old endpoint
		/*oldEP, err := ws.stateMgr.FindEndpoint(ws.Workload.Tenant, epName)
		if err == nil {
			_, isSpecDifferent := ref.ObjDiff(oldEP.Endpoint.Spec, epInfo.Spec)
			_, isStatusDifferent := ref.ObjDiff(oldEP.Endpoint.Status, epInfo.Status)

			if isSpecDifferent || isStatusDifferent {
				// delete the old endpoint
				ws.stateMgr.ctrler.Endpoint().Delete(&oldEP.Endpoint.Endpoint)

				// create new endpoint
				err = ws.stateMgr.ctrler.Endpoint().Create(&epInfo)
				if err != nil {
					log.Errorf("Error creating endpoint. Err: %v", err)
				}
			} else {
				log.Infof("Existing endpoint for workload is same as new one %+v ", epInfo)
			}
		} else {
			// create new endpoint
			err = ws.stateMgr.ctrler.Endpoint().Create(&epInfo)
			if err != nil {
				log.Errorf("Error creating endpoint. Err: %v", err)
			}
		}*/
		// create new endpoint
		err = ws.stateMgr.ctrler.Endpoint().Create(&epInfo)
		if err != nil {
			log.Errorf("Error creating endpoint. Err: %v", err)
		}
	}

	return nil
}

// deleteEndpoints deletes all endpoints for a workload
func (ws *WorkloadState) deleteEndpoints() error {
	// loop over each interface of the workload
	for ii := range ws.Workload.Spec.Interfaces {
		// find the network for the interface
		netName := ws.stateMgr.networkName(ws.Workload.Spec.Interfaces[ii].ExternalVlan)
		nw, err := ws.stateMgr.FindNetwork(ws.Workload.Tenant, netName)
		if err != nil {
			log.Errorf("Error finding the network %v. Err: %v", netName, err)
		} else {

			// check if we created an endpoint for this workload interface
			name, _ := strconv.ParseMacAddr(ws.Workload.Spec.Interfaces[ii].MACAddress)
			epName := ws.Workload.Name + "-" + name
			_, ok := nw.FindEndpoint(epName)
			if !ok {
				log.Errorf("Could not find endpoint %v", epName)
			} else {

				epInfo := workload.Endpoint{
					TypeMeta: api.TypeMeta{Kind: "Endpoint"},
					ObjectMeta: api.ObjectMeta{
						Name:      epName,
						Tenant:    ws.Workload.Tenant,
						Namespace: ws.Workload.Namespace,
					},
				}

				// delete the endpoint in api server
				err = ws.stateMgr.ctrler.Endpoint().Delete(&epInfo)
				if err != nil {
					log.Errorf("Error deleting the endpoint. Err: %v", err)
				}
			}
		}
	}

	return nil
}

// FindWorkload finds a workload
func (sm *Statemgr) FindWorkload(tenant, name string) (*WorkloadState, error) {
	// find the object
	obj, err := sm.FindObject("Workload", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return WorkloadStateFromObj(obj)
}

//RemoveStaleEndpoints remove stale endpoints
func (sm *Statemgr) RemoveStaleEndpoints() error {

	endpoints, err := sm.ctrler.Endpoint().List(context.Background(), &api.ListWatchOptions{})
	if err != nil {
		log.Errorf("Failed to get endpoints. Err : %v", err)
		return err
	}

	workloads, err := sm.ctrler.Workload().List(context.Background(), &api.ListWatchOptions{})
	workloadCacheEmpty := false
	if err != nil {
		log.Errorf("Failed to get workloads. Err : %v", err)
		workloadCacheEmpty = true
	}

	workloadMacPresent := func(wName, mac string) bool {
		for _, workload := range workloads {
			if workload.Name == wName {
				for ii := range workload.Workload.Spec.Interfaces {
					wmac, _ := strconv.ParseMacAddr(workload.Workload.Spec.Interfaces[ii].MACAddress)
					if wmac == mac {
						return true
					}
				}
			}
		}
		return false
	}

	for _, ep := range endpoints {
		splitString := strings.Split(ep.Name, "-")
		if len(splitString) < 2 {
			continue
		}
		workloadName := strings.Join(splitString[0:len(splitString)-1], "-")
		macAddress := splitString[len(splitString)-1]
		if workloadCacheEmpty || !workloadMacPresent(workloadName, macAddress) {
			// delete the endpoint in api server
			epInfo := workload.Endpoint{
				TypeMeta: api.TypeMeta{Kind: "Endpoint"},
				ObjectMeta: api.ObjectMeta{
					Name:      ep.Name,
					Tenant:    ep.Tenant,
					Namespace: ep.Namespace,
				},
			}
			err := sm.ctrler.Endpoint().Delete(&epInfo)
			if err != nil {
				log.Errorf("Error deleting the endpoint. Err: %v", err)
			}
		}
	}

	return nil
}
