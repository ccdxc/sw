// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"bytes"
	"context"
	"fmt"
	"math/rand"
	"os"
	"strconv"
	"strings"

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
	proto  string
	ports  []int
}

// WorkloadPairCollection is collection of workload pairs
type WorkloadPairCollection struct {
	err   error
	pairs []*WorkloadPair
}

// createWorkload creates a workload
func (sm *SysModel) createWorkload(w *workload.Workload, wtype iota.WorkloadType, wimage string, host *Host) (*Workload, error) {
	/*err := sm.tb.CreateWorkload(w)
	if err != nil {
		return nil, err
	}*/

	convertMac := func(s string) string {
		mac := strings.Replace(s, ".", "", -1)
		var buffer bytes.Buffer
		var l1 = len(mac) - 1
		for i, rune := range mac {
			buffer.WriteRune(rune)
			if i%2 == 1 && i != l1 {
				buffer.WriteRune(':')
			}
		}
		return buffer.String()
	}

	// create iota workload object
	iotaWorkload := iota.Workload{
		WorkloadType:    wtype,
		WorkloadName:    w.GetName(),
		NodeName:        host.iotaNode.Name,
		WorkloadImage:   wimage,
		EncapVlan:       w.Spec.Interfaces[0].MicroSegVlan,
		IpPrefix:        w.Spec.Interfaces[0].IpAddresses[0] + "/24", //Assuming it is /24 for now
		MacAddress:      convertMac(w.Spec.Interfaces[0].MACAddress),
		Interface:       "lif100", // ugly hack here: iota agent creates interfaces like lif100. matching that behavior
		ParentInterface: "lif100", // ugly hack here: iota agent creates interfaces like lif100. matching that behavior
		InterfaceType:   iota.InterfaceType_INTERFACE_TYPE_VSS,
		PinnedPort:      1, // another hack: always pinning to first uplink
		UplinkVlan:      w.Spec.Interfaces[0].ExternalVlan,
	}

	wr := Workload{
		iotaWorkload:   &iotaWorkload,
		veniceWorkload: w,
		host:           host,
		//	subnet:         subnet,
		sm: sm,
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
	sm.DeleteWorkload(wr.veniceWorkload)
	//Ignore the error as we don't care

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

// GetSingleWorkloadPair gets a single pair collection based on index
func (wlpc *WorkloadPairCollection) GetSingleWorkloadPair(i int) *WorkloadPairCollection {
	collection := WorkloadPairCollection{}
	collection.pairs = append(collection.pairs, wlpc.pairs[i])
	return &collection
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

// Any returns subset of workloads
func (wc *WorkloadCollection) Any(num int) *WorkloadCollection {
	if wc.err != nil {
		return wc
	}

	newWc := WorkloadCollection{workloads: []*Workload{}}
	tmpArry := make([]*Workload, len(wc.workloads))
	copy(tmpArry, wc.workloads)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		w := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newWc.workloads = append(newWc.workloads, w)
	}

	return &newWc
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
	for _, wrk := range wc.workloads {
		for _, gwrk := range appResp.Workloads {
			if gwrk.WorkloadName == wrk.iotaWorkload.WorkloadName {
				wrk.iotaWorkload.MgmtIp = gwrk.MgmtIp
				wrk.iotaWorkload.Interface = gwrk.GetInterface()
			}
		}
	}

	// copy over some binaries after workloads are brought up
	fuzBinPath := os.Getenv("GOPATH") + "/src/github.com/pensando/sw/iota/bin/fuz"
	for _, wrk := range wc.workloads {
		if err := sm.tb.CopyToWorkload(wrk.iotaWorkload.NodeName, wrk.iotaWorkload.WorkloadName, []string{fuzBinPath}, "."); err != nil {
			log.Errorf("error copying fuz binary to workload: %s", err)
		}
	}

	return nil
}

//Teardown bring down workload
func (wc *WorkloadCollection) Teardown() error {

	// Teardown the workloads
	for _, wrk := range wc.workloads {
		err := wrk.sm.deleteWorkload(wrk)
		if err != nil {
			log.Infof("Delete workload failed : %v", err.Error())
			return err
		}
	}

	log.Info("Delete workload successful")
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
func (wpc *WorkloadPairCollection) policyHelper(policyCollection *NetworkSecurityPolicyCollection, action, proto string) *WorkloadPairCollection {
	if wpc.err != nil {
		return wpc
	}
	newCollection := WorkloadPairCollection{}
	type ipPair struct {
		sip   string
		dip   string
		proto string
		ports string
	}
	actionCache := make(map[string][]ipPair)

	ipPairPresent := func(pair ipPair) bool {
		for _, pairs := range actionCache {
			for _, ippair := range pairs {
				if ippair.dip == pair.dip && ippair.sip == pair.sip && ippair.proto == pair.proto {
					return true
				}
			}
		}
		return false
	}
	for _, pol := range policyCollection.policies {
		for _, rule := range pol.venicePolicy.Spec.Rules {
			for _, sip := range rule.FromIPAddresses {
				for _, dip := range rule.ToIPAddresses {
					for _, proto := range rule.ProtoPorts {
						pair := ipPair{sip: sip, dip: dip, proto: proto.Protocol, ports: proto.Ports}
						if _, ok := actionCache[rule.Action]; !ok {
							actionCache[rule.Action] = []ipPair{}
						}
						//if this IP pair was already added, then don't pick it as precedence is based on order
						if !ipPairPresent(pair) {
							//Copy ports
							actionCache[rule.Action] = append(actionCache[rule.Action], pair)
						}
					}
				}
			}
		}
	}

	getPortsFromRange := func(ports string) []int {
		if ports == "any" {
			//Random port if any
			return []int{5555}
		}
		values := strings.Split(ports, "-")
		start, _ := strconv.Atoi(values[0])
		if len(values) == 1 {
			return []int{start}
		}
		end, _ := strconv.Atoi(values[1])
		portValues := []int{}
		for i := start; i <= end; i++ {
			portValues = append(portValues, i)
		}
		return portValues
	}
	for _, pair := range wpc.pairs {
		cache, ok := actionCache[action]
		if ok {
			for _, ippair := range cache {
				if ((ippair.dip == "any") || ippair.dip == strings.Split(pair.first.iotaWorkload.GetIpPrefix(), "/")[0]) &&
					((ippair.sip == "any") || ippair.sip == strings.Split(pair.second.iotaWorkload.GetIpPrefix(), "/")[0]) &&
					ippair.proto == proto &&
					pair.first.iotaWorkload.UplinkVlan == pair.second.iotaWorkload.UplinkVlan {
					pair.ports = getPortsFromRange(ippair.ports)
					pair.proto = ippair.proto
					newCollection.pairs = append(newCollection.pairs, pair)
				}
			}
		}
	}

	return &newCollection
}

// Permit get allowed workloads with proto
func (wpc *WorkloadPairCollection) Permit(policyCollection *NetworkSecurityPolicyCollection, proto string) *WorkloadPairCollection {
	return wpc.policyHelper(policyCollection, "PERMIT", proto)
}

// Deny get Denied workloads with proto
func (wpc *WorkloadPairCollection) Deny(policyCollection *NetworkSecurityPolicyCollection, proto string) *WorkloadPairCollection {
	return wpc.policyHelper(policyCollection, "DENY", proto)
}

// Reject get rejected workloads with proto
func (wpc *WorkloadPairCollection) Reject(policyCollection *NetworkSecurityPolicyCollection, proto string) *WorkloadPairCollection {
	return wpc.policyHelper(policyCollection, "REJECT", proto)
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

// ExcludeWorkloads excludes some workloads from collection
func (wpc *WorkloadPairCollection) ExcludeWorkloads(wc *WorkloadCollection) *WorkloadPairCollection {
	if wpc.err != nil {
		return wpc
	}
	newCollection := WorkloadPairCollection{}

	for _, pair := range wpc.pairs {
		for _, w := range wc.workloads {
			if pair.first.iotaWorkload.WorkloadName != w.iotaWorkload.WorkloadName &&
				pair.second.iotaWorkload.WorkloadName != w.iotaWorkload.WorkloadName {
				newCollection.pairs = append(newCollection.pairs, pair)
			}
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

// ListIPAddr lists work load ip address
func (wpc *WorkloadPairCollection) ListIPAddr() [][]string {
	workloadNames := [][]string{}

	for _, pair := range wpc.pairs {
		workloadNames = append(workloadNames, []string{strings.Split(pair.first.iotaWorkload.GetIpPrefix(), "/")[0], strings.Split(pair.second.iotaWorkload.GetIpPrefix(), "/")[0]})
	}

	return workloadNames
}

// HasError returns true if collection has an error
func (wpc *WorkloadPairCollection) HasError() bool {
	return (wpc.err != nil)
}

// Error returns the collection error
func (wpc *WorkloadPairCollection) Error() error {
	return wpc.err
}
