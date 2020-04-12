// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package base

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/events/generated/eventtypes"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// SwitchPorts returns list of all switch ports
func (sm *SysModel) SwitchPorts() *objects.SwitchPortCollection {
	var swPc objects.SwitchPortCollection
	for _, port := range sm.SwitchPortsList {
		swPc.Ports = append(swPc.Ports, port)
	}

	return &swPc
}

// Hosts returns list of all hosts in the system
func (sm *SysModel) Hosts() *objects.HostCollection {

	hc := objects.NewHostCollection(sm.ObjClient(), sm.Tb)
	for _, hst := range sm.NaplesHosts {
		hc.Hosts = append(hc.Hosts, hst)
	}

	for _, hst := range sm.FakeHosts {
		hc.FakeHosts = append(hc.FakeHosts, hst)
	}

	for _, hst := range sm.ThirdPartyHosts {
		hc.Hosts = append(hc.Hosts, hst)
	}

	return hc
}

// HostWorkloads returns list of all hosts in the system along with their workloads
func (sm *SysModel) HostWorkloads() []*objects.HostWorkloadCollection {
	var hc []*objects.HostWorkloadCollection
	for _, hst := range sm.NaplesHosts {
		h := objects.NewHostWorkloadCollection(hst, sm.ObjClient(), sm.Tb)
		for _, w := range sm.WorkloadsObjs {
			if hst.Name() == w.NodeName() {
				h.AddWorkload(w)
			}
		}
		hc = append(hc, h)
	}

	for _, hst := range sm.ThirdPartyHosts {
		h := objects.NewHostWorkloadCollection(hst, sm.ObjClient(), sm.Tb)
		for _, w := range sm.WorkloadsObjs {
			if hst.Name() == w.NodeName() {
				h.AddWorkload(w)
			}
		}
		hc = append(hc, h)
	}

	return hc
}

// ForEachFakeNaples calls an iterator for each naples in the model
func (sm *SysModel) ForEachFakeNaples(fn objects.NaplesIteratorFn) error {
	fakeNaples := []*objects.Naples{}
	for _, node := range sm.FakeNaples {
		fakeNaples = append(fakeNaples, node)
	}
	err := fn(&objects.NaplesCollection{FakeNodes: fakeNaples})
	if err != nil {
		return err
	}
	return nil
}

// ForEachHost calls a function for each host
func (sm *SysModel) ForEachHost(fn objects.HostIteratorFn) error {
	for _, hst := range sm.NaplesHosts {
		err := fn(&objects.HostCollection{Hosts: []*objects.Host{hst}})
		if err != nil {
			return err
		}
	}

	return nil
}

// findWorkload creates a workload
func (sm *SysModel) findWorkload(name string) *objects.Workload {

	if wrk, ok := sm.WorkloadsObjs[name]; ok {
		return wrk
	}

	return nil
}

//BringUpNewWorkloads brings up new workload on the host
func (sm *SysModel) BringUpNewWorkloads(hc *objects.HostCollection, snc *objects.NetworkCollection, count int) *objects.WorkloadCollection {

	wc := &objects.WorkloadCollection{}
	newWloads := []*workload.Workload{}
	hosts := []*objects.Host{}
	for _, host := range hc.Hosts {
		hostWorkloads := 0
		wloads, err := sm.ListWorkloadsOnHost(host.VeniceHost)
		if err != nil {
			err := fmt.Errorf("Error finding Workloads on host")
			log.Errorf("%v", err.Error())
			wc.SetError(err)
			return wc
		}
		log.Infof("GOt workloads %v", len(wloads))
	hostL:
		for _, nw := range snc.Subnets() {
			for _, wload := range wloads {
				vlan := wload.GetSpec().Interfaces[0].GetExternalVlan()
				if wrk := sm.findWorkload(wload.Name); wrk != nil {
					//Workload already added.
					continue
				}
				if nw.VeniceNetwork.Spec.VlanID == vlan {
					log.Infof("Workload being added to sm  on host %+v %+v %+v", host.VeniceHost.Name, wload.Name, vlan)
					newWloads = append(newWloads, wload)
					hosts = append(hosts, host)
					hostWorkloads++
					if hostWorkloads == count {
						//Found enough Workloads to create on this host
						break hostL
					}
				}
			}
		}
	}

	if len(newWloads) != count*len(hc.Hosts) {
		err := fmt.Errorf("Not enough Workloads (%v) to bring up on host expected %v", len(wc.Workloads), count*len(hc.Hosts))
		log.Errorf("%v", err.Error())
		wc.SetError(err)
		return wc
	}
	//Now add the workload info to IOTA
	subnets := snc.Subnets()
	for i, wload := range newWloads {
		for _, subnet := range subnets {
			if subnet.VeniceNetwork.Spec.VlanID == wload.Spec.Interfaces[0].ExternalVlan {
				sm.WorkloadsObjs[wload.Name] = objects.NewWorkload(hosts[i], wload, sm.Tb.Topo.WorkloadType,
					sm.Tb.Topo.WorkloadImage, sm.Tb.GetSwitch(), subnet.Name)
				wc.Workloads = append(wc.Workloads, sm.WorkloadsObjs[wload.Name])
			}
		}
	}

	// bringup the Workloads
	err := wc.Bringup(sm.Tb)
	if err != nil {
		wc.SetError(err)
		return wc
	}

	return wc
}

// deleteWorkload deletes a workload
func (sm *SysModel) deleteWorkload(wr *objects.Workload) error {
	// FIXME: free mac addr for the workload
	// FIXME: free useg vlan
	// FIXME: free ip address for the workload

	// delete venice workload
	sm.CfgModel.DeleteWorkloads([]*workload.Workload{wr.VeniceWorkload})
	//Ignore the error as we don't care

	wrkLd := &iota.WorkloadMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		WorkloadOp:  iota.Op_DELETE,
		Workloads:   []*iota.Workload{wr.GetIotaWorkload()},
	}

	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	appResp, err := topoClient.DeleteWorkloads(context.Background(), wrkLd)

	if err != nil || appResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to instantiate Apps. %v/%v", err, appResp.ApiResponse.ApiStatus)
		return fmt.Errorf("Error deleting IOTA workload. Resp: %+v, err: %v", appResp.ApiResponse, err)
	}

	delete(sm.WorkloadsObjs, wr.VeniceWorkload.Name)

	return nil
}

func (sm *SysModel) TeardownWorkloads(wc *objects.WorkloadCollection) error {
	// Teardown the workloads
	for _, wrk := range wc.Workloads {
		err := sm.deleteWorkload(wrk)
		if err != nil {
			log.Infof("Delete workload failed : %v", err.Error())
			return err
		}
	}

	log.Info("Delete workload successful")
	return nil
}

// ForEachNaples calls an iterator for each naples in the model
func (sm *SysModel) ForEachNaples(fn objects.NaplesIteratorFn) error {
	for _, node := range sm.NaplesNodes {
		err := fn(&objects.NaplesCollection{Nodes: []*objects.Naples{node}})
		if err != nil {
			return err
		}
	}

	return nil
}

// Naples returns all naples in the model
func (sm *SysModel) Naples() *objects.NaplesCollection {
	var naples []*objects.Naples
	var fakesNaples []*objects.Naples
	for _, np := range sm.NaplesNodes {
		naples = append(naples, np)
	}

	for _, np := range sm.FakeNaples {
		fakesNaples = append(fakesNaples, np)
	}
	return &objects.NaplesCollection{
		CollectionCommon: objects.CollectionCommon{Client: sm.ObjClient(), Testbed: sm.Tb},
		Nodes:            naples, FakeNodes: fakesNaples}
}

// ThirdParties return
func (sm *SysModel) ThirdParties() *objects.ThirdPartyCollection {
	var nodes []*objects.ThirdPartyNode
	for _, np := range sm.ThirdPartyNodes {
		nodes = append(nodes, np)
	}

	return &objects.ThirdPartyCollection{
		CollectionCommon: objects.CollectionCommon{Client: sm.ObjClient(), Testbed: sm.Tb},
		Nodes:            nodes}
}

func (sm *SysModel) createVeniceNode(node *testbed.TestNode) error {

	sm.VeniceNodeMap[node.NodeName] = objects.NewVeniceNode(node)

	return nil
}

// ForEachVeniceNode runs an iterator function on each venice node
func (sm *SysModel) ForEachVeniceNode(fn objects.VeniceNodeIteratorFn) error {
	for _, node := range sm.VeniceNodeMap {
		err := fn(&objects.VeniceNodeCollection{Nodes: []*objects.VeniceNode{node}})
		if err != nil {
			return err
		}
	}

	return nil
}

// VeniceNodes returns a collection of venice nodes
func (sm *SysModel) VeniceNodes() *objects.VeniceNodeCollection {
	vnc := objects.NewVeniceNodeCollection(sm.ObjClient(), sm.Tb)
	for _, node := range sm.VeniceNodeMap {
		vnc.Nodes = append(vnc.Nodes, node)
	}

	return vnc
}

//CaptureGRETCPDump tcpdump untill ctx is done
func (sm *SysModel) CaptureGRETCPDump(ctx context.Context, vnc *objects.VeniceNodeCollection) (string, error) {

	trig := sm.Tb.NewTrigger()

	trig.AddBackgroundCommand("tcpdump -x -nni eth0 ip proto gre",
		vnc.Nodes[0].Name()+"_venice", vnc.Nodes[0].Name())

	resp, err := trig.Run()
	if err != nil {
		return "", fmt.Errorf("Error running command %v", err.Error())
	}

	<-ctx.Done()
	stopResp, err := trig.StopCommands(resp)
	if err != nil {
		return "", fmt.Errorf("Error stopping command %v", err.Error())
	}

	return stopResp[0].GetStdout(), nil
}

//GetVeniceNodeWithService  Get nodes running service
func (sm *SysModel) GetVeniceNodeWithService(vnc *objects.VeniceNodeCollection, service string) (*objects.VeniceNodeCollection, error) {
	srvVnc := objects.NewVeniceNodeCollection(sm.ObjClient(), sm.Tb)

	leader := sm.VeniceNodes().Leader()

	//There is any error
	if leader.Error() != nil {
		return nil, leader.Error()
	}

	trig := sm.Tb.NewTrigger()

	entity := leader.Nodes[0].Name() + "_venice"

	cmd := fmt.Sprintf(`/pensando/iota/bin/kubectl get pods -a --server=https://%s:6443  --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | select(.metadata.labels.name == `+fmt.Sprintf("%q", service)+
		` ) | .status.hostIP'`, leader.Nodes[0].IP())
	trig.AddCommand(cmd, entity, leader.Nodes[0].Name())

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		err = fmt.Errorf("Failed to run command to get service node")
		return nil, err
	}

	if triggerResp[0].ExitCode != 0 {
		err = fmt.Errorf("Failed to run command to get service node : %v",
			triggerResp[0].Stderr)
		return nil, err
	}

	ret := triggerResp[0].Stdout
	hostIP := strings.Split(ret, "\n")

	for _, vn := range sm.VeniceNodeMap {
		for _, ip := range hostIP {
			if vn.IP() == ip {
				srvVnc.Nodes = append(srvVnc.Nodes, vn)
			}
		}
	}

	if len(srvVnc.Nodes) == 0 {
		log.Errorf("Did not find node running %v", service)
		err = fmt.Errorf("Did not find node running %v", service)
		return nil, err
	}
	return srvVnc, nil
}

// ListRealHosts gets all real hosts from venice cluster
func (sm *SysModel) ListRealHosts() ([]*objects.Host, error) {

	hosts := []*objects.Host{}
	for _, h := range sm.NaplesHosts {
		if testbed.IsNaplesHW(h.PersonalityType()) || testbed.IsThirdParty(h.PersonalityType()) {
			hosts = append(hosts, h)
		}
	}

	return hosts, nil
}

// ListFakeHosts gets all real hosts from venice cluster
func (sm *SysModel) ListFakeHosts() ([]*objects.Host, error) {

	hosts := []*objects.Host{}
	for _, h := range sm.FakeHosts {
		hosts = append(hosts, h)
	}

	return hosts, nil
}

// ListWorkloadsOnHost gets Workloads on host
func (sm *SysModel) ListWorkloadsOnHost(h *cluster.Host) (objs []*workload.Workload, err error) {
	objs, err = sm.ListWorkload()
	if err != nil {
		return nil, err
	}

	i := 0
	for _, obj := range objs {
		if obj.Spec.HostName == h.ObjectMeta.Name {
			objs[i] = obj
			i++
		}
	}

	return objs[:i], nil
}

// ServiceStoppedEvents returns list of service stopped events.
func (sm *SysModel) ServiceStoppedEvents(since time.Time, npc *objects.NaplesCollection) *objects.EventsCollection {
	var naplesNames []string
	for _, naples := range npc.Nodes {
		naplesNames = append(naplesNames, naples.Name())
	}

	fieldSelector := fmt.Sprintf("type=%s,meta.mod-time>=%v,object-ref.kind=DistributedServiceCard,object-ref.name in (%v)",
		eventtypes.SERVICE_STOPPED, since.Format(time.RFC3339Nano), fmt.Sprintf("%s", strings.Join(naplesNames, ",")))

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		ec := objects.EventsCollection{}
		ec.SetError(err)
		return &ec
	}

	for _, ev := range eventsList.Items {
		log.Errorf("Service stopped event %v %v", ev.Name, ev.Message)
	}
	return &objects.EventsCollection{List: eventsList}
}
