// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"
	"math/rand"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// Workload represents a VM/container/Baremetal workload (endpoints are associated with the workload)
type Workload struct {
	iotaWorkload       *iota.Workload
	veniceWorkload     *workload.Workload
	sm                 *SysModel // pointer back to the model
	host               *Host
	subnet             *Network
	isFTPServerRunning bool // is FTP server running already on this workload
}

// WorkloadCollection is the collection of workloads
type WorkloadCollection struct {
	err       error
	workloads []*Workload
}

// WorkloadPair is a pair of workloads
type WorkloadPair struct {
	first  *Workload
	second *Workload
}

// WorkloadPairCollection is collection of workload pairs
type WorkloadPairCollection struct {
	err   error
	pairs []*WorkloadPair
}

// createWorkload creates a workload
func (sm *SysModel) createWorkload(wtype iota.WorkloadType, wimage, name string, host *Host, subnet *Network) (*Workload, error) {
	// allocate mac addr for the workload
	mac, err := sm.allocMacAddress()
	if err != nil {
		return nil, err
	}

	// allocate useg vlan
	usegVlan, err := host.allocUsegVlan()
	if err != nil {
		return nil, err
	}

	// venice workload
	w := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      name,
		},
		Spec: workload.WorkloadSpec{
			HostName: host.veniceHost.Name,
			Interfaces: []workload.WorkloadIntfSpec{
				workload.WorkloadIntfSpec{
					ExternalVlan: subnet.vlan,
					MicroSegVlan: usegVlan,
					MACAddress:   mac,
				},
			},
		},
	}
	err = sm.tb.CreateWorkload(&w)
	if err != nil {
		return nil, err
	}

	ipAddr, err := subnet.allocateIPAddr()
	if err != nil {
		return nil, err
	}
	iotaWorkload := iota.Workload{
		WorkloadType:    wtype,
		WorkloadName:    name,
		NodeName:        host.veniceHost.Name,
		WorkloadImage:   wimage,
		EncapVlan:       usegVlan,
		IpPrefix:        ipAddr,
		MacAddress:      mac,
		Interface:       "lif100", // ugly hack here: iota agent creates interfaces like lif100. matching that behavior
		ParentInterface: "lif100", // ugly hack here: iota agent creates interfaces like lif100. matching that behavior
		InterfaceType:   iota.InterfaceType_INTERFACE_TYPE_VSS,
		PinnedPort:      1, // another hack: always pinning to first uplink
		UplinkVlan:      subnet.vlan,
	}

	wr := Workload{
		iotaWorkload:   &iotaWorkload,
		veniceWorkload: &w,
		host:           host,
		subnet:         subnet,
		sm:             sm,
	}

	sm.workloads[w.Name] = &wr

	return &wr, nil
}

// deleteWorkload deletes a workload
func (sm *SysModel) deleteWorkload(wr *Workload) error {
	// FIXME: free mac addr for the workload
	// FIXME: free useg vlan
	// FIXME: free ip address for the workload

	// delete venice workload
	err := sm.tb.DeleteWorkload(wr.veniceWorkload)
	if err != nil {
		return err
	}

	wrkLd := &iota.WorkloadMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		WorkloadOp:  iota.Op_DELETE,
		Workloads:   []*iota.Workload{wr.iotaWorkload},
	}

	topoClient := iota.NewTopologyApiClient(sm.tb.iotaClient.Client)
	appResp, err := topoClient.DeleteWorkloads(context.Background(), wrkLd)

	if err != nil || appResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to instantiate Apps. %v/%v", err, appResp.ApiResponse.ApiStatus)
		return fmt.Errorf("Error deleting IOTA workload. Resp: %+v, err: %v", appResp.ApiResponse, err)
	}

	delete(sm.workloads, wr.veniceWorkload.Name)

	return nil
}

// Workloads returns all workloads in the model
func (sm *SysModel) Workloads() *WorkloadCollection {

	wpc := WorkloadCollection{}

	for _, wf := range sm.workloads {
		wpc.workloads = append(wpc.workloads, wf)
	}

	return &wpc
}

// WorkloadPairs creates full mesh of workload pairs
func (sm *SysModel) WorkloadPairs() *WorkloadPairCollection {
	collection := WorkloadPairCollection{}
	var wrklds []*Workload
	for _, w := range sm.workloads {
		wrklds = append(wrklds, w)
	}
	for i, wf := range wrklds {
		for j := i + 1; j < len(wrklds); j++ {
			ws := wrklds[j]
			pair := WorkloadPair{
				first:  wf,
				second: ws,
			}
			collection.pairs = append(collection.pairs, &pair)
		}
	}

	return &collection
}

// HasError returns true if the collection has error
func (wc *WorkloadCollection) HasError() bool {
	return (wc.err != nil)
}

// Error returns the error in collection
func (wc *WorkloadCollection) Error() error {
	return wc.err
}

// MeshPairs returns full-mesh pair of workloads
func (wc *WorkloadCollection) MeshPairs() *WorkloadPairCollection {
	if wc.HasError() {
		return &WorkloadPairCollection{err: wc.Error()}
	}

	collection := WorkloadPairCollection{}

	for i, wf := range wc.workloads {
		for j, ws := range wc.workloads {
			if i != j {
				pair := WorkloadPair{
					first:  wf,
					second: ws,
				}
				collection.pairs = append(collection.pairs, &pair)
			}
		}
	}

	return &collection
}

// Bringup brings up all workloads in the collection
func (wc *WorkloadCollection) Bringup() error {
	var workloads []*iota.Workload
	var sm *SysModel

	// check for errors
	if wc.HasError() {
		return wc.Error()
	}

	// if there are no workloads, nothing to do
	if len(wc.workloads) == 0 {
		return nil
	}

	// build workload list
	for _, wrk := range wc.workloads {
		workloads = append(workloads, wrk.iotaWorkload)
		sm = wrk.sm
	}

	// send workload add message
	wrkLd := &iota.WorkloadMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		WorkloadOp:  iota.Op_ADD,
		Workloads:   workloads,
	}
	topoClient := iota.NewTopologyApiClient(sm.tb.iotaClient.Client)
	appResp, err := topoClient.AddWorkloads(context.Background(), wrkLd)
	log.Debugf("Got add workload resp: %+v, err: %v", appResp, err)
	if err != nil {
		log.Errorf("Failed to instantiate Apps. Err: %v", err)
		return fmt.Errorf("Error creating IOTA workload. err: %v", err)
	} else if appResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to instantiate Apps. resp: %+v.", appResp.ApiResponse)
		return fmt.Errorf("Error creating IOTA workload. Resp: %+v", appResp.ApiResponse)
	}

	return nil
}

// Delete deletes each workload in the collection
func (wc *WorkloadCollection) Delete() error {
	if wc.HasError() {
		return wc.Error()
	}

	for _, wr := range wc.workloads {
		err := wr.sm.deleteWorkload(wr)
		if err != nil {
			return err
		}
	}

	return nil
}

// WithinNetwork filters workload pairs to only withon subnet
func (wpc *WorkloadPairCollection) WithinNetwork() *WorkloadPairCollection {
	if wpc.err != nil {
		return wpc
	}
	newCollection := WorkloadPairCollection{}

	for _, pair := range wpc.pairs {
		if pair.first.iotaWorkload.UplinkVlan == pair.second.iotaWorkload.UplinkVlan {
			newCollection.pairs = append(newCollection.pairs, pair)
		}
	}

	return &newCollection
}

// Any returna one pair from the collection in random
func (wpc *WorkloadPairCollection) Any(num int) *WorkloadPairCollection {
	if wpc.err != nil || len(wpc.pairs) <= num {
		return wpc
	}
	newWpc := WorkloadPairCollection{pairs: []*WorkloadPair{}}
	tmpArry := make([]*WorkloadPair, len(wpc.pairs))
	copy(tmpArry, wpc.pairs)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		wpair := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newWpc.pairs = append(newWpc.pairs, wpair)
	}

	return &newWpc
}

// ReversePairs reverses the pairs by swapping first and second entries
func (wpc *WorkloadPairCollection) ReversePairs() *WorkloadPairCollection {
	if wpc.err != nil || len(wpc.pairs) < 1 {
		return wpc
	}
	newWpc := WorkloadPairCollection{pairs: []*WorkloadPair{}}
	for _, pair := range wpc.pairs {
		newPair := WorkloadPair{
			first:  pair.second,
			second: pair.first,
		}
		newWpc.pairs = append(newWpc.pairs, &newPair)
	}

	return &newWpc
}

// HasError returns true if collection has an error
func (wpc *WorkloadPairCollection) HasError() bool {
	return (wpc.err != nil)
}

// Error returns the collection error
func (wpc *WorkloadPairCollection) Error() error {
	return wpc.err
}
