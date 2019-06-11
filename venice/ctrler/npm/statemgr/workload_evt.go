// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
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
		time.Sleep(time.Second)
		host, err = ws.stateMgr.ctrler.Host().Find(&api.ObjectMeta{Name: w.Spec.HostName})
		if err != nil {
			log.Errorf("Error finding the host %s for workload %v. Err: %v", w.Spec.HostName, w.Name, err)
			return err
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

	// trigger creation of new endpoints
	return ws.createEndpoints()
}

// reconcileWorkload checks if the endpoints are create for the workload and tries to create them
func (sm *Statemgr) reconcileWorkload(w *ctkit.Workload, hst *HostState, snic *SmartNICState) error {
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
			if err != nil {
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
	ws, err := sm.FindWorkload(w.Tenant, w.Name)
	if err != nil {
		return err
	}

	return ws.deleteEndpoints()
}

// createEndpoints tries to create all endpoints for a workload
func (ws *WorkloadState) createEndpoints() error {
	// find the host for the workload
	host, err := ws.stateMgr.FindHost("", ws.Workload.Spec.HostName)
	if err != nil {
		log.Errorf("Error finding the host %s for workload %v. Err: %v", ws.Workload.Spec.HostName, ws.Workload.Name, err)
		return err
	}

	// loop over each interface of the workload
	for ii := range ws.Workload.Spec.Interfaces {
		// check if we have a network for this workload
		netName := ws.stateMgr.networkName(ws.Workload.Spec.Interfaces[ii].ExternalVlan)
		_, err = ws.stateMgr.FindNetwork(ws.Workload.Tenant, netName)
		if (err != nil) && (ErrIsObjectNotFound(err)) {
			err = ws.stateMgr.ctrler.Network().Create(&network.Network{
				TypeMeta: api.TypeMeta{Kind: "Network"},
				ObjectMeta: api.ObjectMeta{
					Name:      netName,
					Namespace: ws.Workload.Namespace,
					Tenant:    ws.Workload.Tenant,
				},
				Spec: network.NetworkSpec{
					IPv4Subnet:  "",
					IPv4Gateway: "",
					VlanID:      ws.Workload.Spec.Interfaces[ii].ExternalVlan,
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
		name, _ := strconv.ParseMacAddr(ws.Workload.Spec.Interfaces[ii].MACAddress)
		epName := ws.Workload.Name + "-" + name
		nodeUUID := ""
		// find the smart nic by name or mac addr
		for jj := range host.Host.Spec.SmartNICs {
			if host.Host.Spec.SmartNICs[jj].ID != "" {
				snic, err := ws.stateMgr.FindSmartNICByHname(host.Host.Spec.SmartNICs[jj].ID)
				if err != nil {
					log.Warnf("Error finding smart nic for name %v", host.Host.Spec.SmartNICs[jj].ID)
					return nil
				}
				nodeUUID = snic.SmartNIC.Name
			} else if host.Host.Spec.SmartNICs[jj].MACAddress != "" {
				snicMac := host.Host.Spec.SmartNICs[jj].MACAddress
				snic, err := ws.stateMgr.FindSmartNICByMacAddr(snicMac)
				if err != nil {
					log.Warnf("Error finding smart nic for mac add %v", snicMac)
					return nil
				}
				nodeUUID = snic.SmartNIC.Name
			}
		}

		/* FIXME: comment out this code till CMD publishes associated NICs in host
		// check if the host has associated smart nic
		if len(host.Host.Status.AdmittedSmartNICs) == 0 {
			log.Errorf("Host %v does not have a smart nic", w.Spec.HostName)
			return fmt.Errorf("Host does not have associated smartnic")
		}

		for _, snicName := range host.Host.Status.AdmittedSmartNICs {
			snic, err := sm.FindSmartNIC(host.Host.Tenant, snicName)
			if err != nil {
				log.Errorf("Error finding smart nic object for %v", snicName)
				return err
			}
			nodeUUID = snic.SmartNIC.Name
		}
		*/

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
				MacAddress:         ws.Workload.Spec.Interfaces[ii].MACAddress,
				HomingHostAddr:     "", // TODO: get host address
				HomingHostName:     ws.Workload.Spec.HostName,
				MicroSegmentVlan:   ws.Workload.Spec.Interfaces[ii].MicroSegVlan,
			},
		}
		if len(ws.Workload.Spec.Interfaces[ii].IpAddresses) > 0 {
			epInfo.Status.IPv4Address = ws.Workload.Spec.Interfaces[ii].IpAddresses[0]
		}

		// see if we need to delete old endpoint
		oldEP, err := ws.stateMgr.FindEndpoint(ws.Workload.Tenant, epName)
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
