package objects

import (
	"context"
	"fmt"
	"math/rand"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/telemetry_query"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/telemetryclient"
)

// VeniceNode represents a venice node
type VeniceNode struct {
	ClusterNode *cluster.Node
	iotaNode    *iota.Node
	testNode    *testbed.TestNode
}

//Name return name
func (v *VeniceNode) Name() string {
	return v.iotaNode.Name
}

//IP return IP
func (v *VeniceNode) IP() string {
	if v.testNode.SecondaryIP != "" {
		return v.testNode.SecondaryIP
	}
	return v.iotaNode.IpAddress
}

//GetTestNode return name
func (v *VeniceNode) GetTestNode() *testbed.TestNode {
	return v.testNode
}

// VeniceNodeCollection is collection of venice nodes
type VeniceNodeCollection struct {
	CollectionCommon
	Nodes        []*VeniceNode
	TelemetryCli []*telemetryclient.TelemetryClient
}

// VeniceContainer represents a venice container
type VeniceContainer struct {
	Name        string
	ContainerID string
	Node        *VeniceNode
}

// VeniceContainerCollection is collection venice container a service
type VeniceContainerCollection struct {
	CollectionCommon
	Containers []*VeniceContainer
}

// NewVeniceNodeCollection create a new VeniceNodeCollection
func NewVeniceNodeCollection(client objClient.ObjClient, testbed *testbed.TestBed) *VeniceNodeCollection {
	return &VeniceNodeCollection{
		CollectionCommon: CollectionCommon{Client: client,
			Testbed: testbed},
	}
}

//NewVeniceNode returns venice node
func NewVeniceNode(node *testbed.TestNode) *VeniceNode {
	return &VeniceNode{
		testNode: node,
		iotaNode: node.GetIotaNode(),
	}
}

// GenVeniceIPs get venice IPs
func (vnc *VeniceNodeCollection) GenVeniceIPs() []string {

	ipAddrs := []string{}
	for _, node := range vnc.Nodes {
		ipAddrs = append(ipAddrs, node.iotaNode.GetIpAddress())
	}

	return ipAddrs
}

func (vnc *VeniceNodeCollection) Error() error {
	return vnc.err
}

// Leader returns the leader node
func (vnc *VeniceNodeCollection) Leader() *VeniceNodeCollection {
	if vnc.HasError() {
		return vnc
	}

	cl, err := vnc.Client.GetCluster()
	if err != nil {
		vnc.err = err
		return vnc
	}
	nvnc := &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	// get the cluster from venice
	for _, node := range vnc.Nodes {
		if cl.Status.Leader == node.iotaNode.Name {
			nvnc.Nodes = []*VeniceNode{node}
			return nvnc
		} else if cl.Status.Leader == node.iotaNode.IpAddress || cl.Status.Leader == node.testNode.SecondaryIP {
			nvnc.Nodes = []*VeniceNode{node}
			return nvnc
		}
	}

	vnc = &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	vnc.SetError(fmt.Errorf("Could not find a leader node"))

	return vnc
}

// NonLeaders returns all nodes except leaders
func (vnc *VeniceNodeCollection) NonLeaders() *VeniceNodeCollection {
	if vnc.HasError() {
		return vnc
	}

	cl, err := vnc.Client.GetCluster()
	if err != nil {
		vnc.err = err
		return vnc
	}

	nonLeaders := VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	for _, node := range vnc.Nodes {
		if cl.Status.Leader == node.iotaNode.Name || cl.Status.Leader == node.iotaNode.IpAddress {
			continue
		}
		nonLeaders.Nodes = append(nonLeaders.Nodes, node)
	}

	if len(nonLeaders.Nodes) == 0 {
		nonLeaders.err = fmt.Errorf("Could not find a leader node")
	}

	return &nonLeaders
}

// Any returns the requested number of venice from collection in random
func (vnc *VeniceNodeCollection) Any(num int) *VeniceNodeCollection {
	if vnc.HasError() || len(vnc.Nodes) <= num {
		return vnc
	}

	newVnc := &VeniceNodeCollection{Nodes: []*VeniceNode{}, CollectionCommon: vnc.CollectionCommon}
	tmpArry := make([]*VeniceNode, len(vnc.Nodes))
	copy(tmpArry, vnc.Nodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newVnc.Nodes = append(newVnc.Nodes, sn)
	}

	return newVnc
}

// SelectByPercentage returns a collection with the specified venice nodes based on percentage.
func (vnc *VeniceNodeCollection) SelectByPercentage(percent int) (*VeniceNodeCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if vnc.err != nil {
		return nil, fmt.Errorf("venice collection error (%s)", vnc.err)
	}

	ret := &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	for i, node := range vnc.Nodes {
		ret.Nodes = append(ret.Nodes, node)
		if (i + 1) >= len(vnc.Nodes)*percent/100 {
			break
		}
	}

	if len(ret.Nodes) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
	return ret, nil
}

type selectParams struct {
	names []string
}

func parseSelectorString(str string) (selectParams, error) {
	ret := selectParams{}
	// Only handling Spaces. not all space characters.
	str = strings.Replace(str, " ", "", -1)
	parts := strings.SplitN(str, "=", 2)
	if len(parts) != 2 {
		return ret, fmt.Errorf("failed to parse selector string")
	}
	if parts[0] != "name" {
		return ret, fmt.Errorf("only name selector supported")
	}
	ret.names = strings.Split(parts[1], ",")
	return ret, nil
}

// Select returns a collection with the specified venice nodes, error if any of the specified nodes is not found
func (vnc *VeniceNodeCollection) Select(sel string) (*VeniceNodeCollection, error) {
	if vnc.HasError() {
		return nil, fmt.Errorf("node collection error (%s)", vnc.err)
	}
	ret := &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	params, err := parseSelectorString(sel)
	if err != nil {
		return ret, fmt.Errorf("could not parse selector")
	}
	var notFound []string
nodeLoop:
	for _, name := range params.names {
		for _, node := range vnc.Nodes {
			if node.iotaNode.Name == name {
				ret.Nodes = append(ret.Nodes, node)
				continue nodeLoop
			}
		}
		notFound = append(notFound, name)
	}
	if len(notFound) != 0 {
		return nil, fmt.Errorf("%v not found", notFound)
	}
	return ret, nil
}

// CaptureGRETCPDump capture the gre tcp dump
func (vnc *VeniceNodeCollection) CaptureGRETCPDump(ctx context.Context) (string, error) {

	trig := vnc.Testbed.NewTrigger()

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

// GetGRETCPDumpCount get gre tcp dump count
func (vnc *VeniceNodeCollection) GetGRETCPDumpCount(ctx context.Context) (int, error) {

	trig := vnc.Testbed.NewTrigger()

	trig.AddBackgroundCommand("tcpdump -x -nni eth0 ip proto gre -w test.pcap",
		vnc.Nodes[0].Name()+"_venice", vnc.Nodes[0].Name())

	resp, err := trig.Run()
	if err != nil {
		return 0, fmt.Errorf("Error running command %v", err.Error())
	}

	<-ctx.Done()
	_, err = trig.StopCommands(resp)
	if err != nil {
		return 0, fmt.Errorf("Error stopping command %v", err.Error())
	}

	trig = vnc.Testbed.NewTrigger()

	trig.AddCommand("tcpdump -r test.pcap  | wc -l",
		vnc.Nodes[0].Name()+"_venice", vnc.Nodes[0].Name())

	resp, err = trig.Run()
	if err != nil {
		return 0, fmt.Errorf("Error running command %v", err.Error())
	}

	return strconv.Atoi(strings.TrimSuffix(resp[0].GetStdout(), "\n"))
}

//GetVeniceNodeWithService  Get nodes running service
func (vnc *VeniceNodeCollection) GetVeniceNodeWithService(service string) (*VeniceNodeCollection, error) {
	if vnc.err != nil {
		return nil, vnc.err
	}
	srvVnc := VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}

	leader := vnc.Leader()

	//There is any error
	if leader.err != nil {
		return nil, leader.err
	}

	trig := vnc.Testbed.NewTrigger()

	entity := leader.Nodes[0].iotaNode.Name + "_venice"

	cmd := fmt.Sprintf(`/pensando/iota/bin/kubectl get pods -a --server=https://%s:6443 --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | select(.metadata.labels.name == `+fmt.Sprintf("%q", service)+
		` ) | .status.hostIP'`, leader.Nodes[0].IP())
	trig.AddCommand(cmd, entity, leader.Nodes[0].iotaNode.Name)

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		srvVnc.err = fmt.Errorf("Failed to run command to get service node")
		return nil, srvVnc.err
	}

	if triggerResp[0].ExitCode != 0 {
		srvVnc.err = fmt.Errorf("Failed to run command to get service node : %v",
			triggerResp[0].Stderr)
		return nil, srvVnc.err
	}

	ret := triggerResp[0].Stdout
	hostIP := strings.Split(ret, "\n")

	for _, vn := range vnc.Nodes {
		for _, ip := range hostIP {
			if ip == vn.IP() {
				srvVnc.Nodes = append(srvVnc.Nodes, vn)
			}
		}
	}

	if len(srvVnc.Nodes) == 0 {
		log.Errorf("Did not find node running %v", service)
		srvVnc.err = fmt.Errorf("Did not find node running %v", service)
		return nil, srvVnc.err
	}
	return &srvVnc, nil
}

// ForEachVeniceNode runs an iterator function on each venice node collection
func (vnc *VeniceNodeCollection) ForEachVeniceNode(fn VeniceNodeIteratorFn) error {
	for _, node := range vnc.Nodes {
		err := fn(&VeniceNodeCollection{Nodes: []*VeniceNode{node}, CollectionCommon: vnc.CollectionCommon})
		if err != nil {
			return err
		}
	}

	return nil
}

//RunCommand runs command on the venice node
func (vnc *VeniceNodeCollection) RunCommand(node *VeniceNode, cmd string) (string, string, int32, error) {

	//Derivce the container ID
	trig := vnc.Testbed.NewTrigger()

	entity := node.iotaNode.Name + "_venice"

	trig.AddCommand(cmd, entity, node.iotaNode.Name)

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		return "", "", -1, err
	}

	return triggerResp[0].Stdout, triggerResp[0].Stderr, triggerResp[0].ExitCode, nil
}

//GetVeniceContainersWithService  Get nodes running service
func (vnc *VeniceNodeCollection) GetVeniceContainersWithService(service string, sideCar bool) (*VeniceContainerCollection, error) {
	if vnc.err != nil {
		return nil, vnc.err
	}

	vContCollection := VeniceContainerCollection{CollectionCommon: vnc.CollectionCommon}
	veniceNodes := []*VeniceNode{}

	leader := vnc.Leader()

	//There is any error
	if leader.err != nil {
		return nil, leader.err
	}

	trig := vnc.Testbed.NewTrigger()

	entity := leader.Nodes[0].iotaNode.Name + "_venice"

	cmd := fmt.Sprintf(`/pensando/iota/bin/kubectl  get pods -a  --server=https://%s:6443 --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | select(.metadata.labels.name == `+fmt.Sprintf("%q", service)+
		` ) | .status.hostIP'`, leader.Nodes[0].IP())
	trig.AddCommand(cmd, entity, leader.Nodes[0].iotaNode.Name)

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		vContCollection.err = fmt.Errorf("Failed to run command to get service node")
		return nil, vContCollection.err
	}

	if triggerResp[0].ExitCode != 0 {
		vContCollection.err = fmt.Errorf("Failed to run command to get service node : %v",
			triggerResp[0].Stderr)
		return nil, vContCollection.err
	}

	ret := triggerResp[0].Stdout
	hostIP := strings.Split(ret, "\n")

	for _, vn := range vnc.Nodes {
		for _, ip := range hostIP {
			if ip == "" {
				continue
			}
			if vn.iotaNode.IpAddress == ip || vn.testNode.SecondaryIP == ip {
				veniceNodes = append(veniceNodes, vn)
			}
		}
	}

	if len(veniceNodes) == 0 {
		log.Errorf("Did not find node running %v", service)
		vContCollection.err = fmt.Errorf("Did not find node running %v", service)
		return nil, vContCollection.err
	}

	item := "first"
	if sideCar {
		item = "last"
	}

	for _, vn := range veniceNodes {
		vContainer := &VeniceContainer{Node: vn, Name: service}

		//Derivce the container ID
		trig := vnc.Testbed.NewTrigger()

		entity := leader.Nodes[0].iotaNode.Name + "_venice"

		ip := vn.iotaNode.IpAddress
		if vn.testNode.SecondaryIP != "" {
			ip = vn.testNode.SecondaryIP
		}
		cmd := fmt.Sprintf(`/pensando/iota/bin/kubectl get pods -a --server=https://%s:6443  --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | select(.metadata.labels.name == `+fmt.Sprintf("%q", service)+
			` ) | select(.status.hostIP == `+fmt.Sprintf("%q", ip)+` ) | .status.containerStatuses | `+item+` | .containerID ' | cut -d "/" -f 3`, leader.Nodes[0].IP())
		trig.AddCommand(cmd, entity, leader.Nodes[0].iotaNode.Name)

		// trigger commands
		triggerResp, err := trig.Run()
		if err != nil {
			log.Errorf("Failed to run command to get service node Err: %v", err)
			vContCollection.err = fmt.Errorf("Failed to run command to get service node")
			return nil, vContCollection.err
		}

		ret := triggerResp[0].Stdout
		vContainer.ContainerID = strings.Split(ret, "\n")[0]
		vContCollection.Containers = append(vContCollection.Containers, vContainer)
	}
	return &vContCollection, nil
}

// InitTelemetryClient init telemetry clients for each venice node
func (vnc *VeniceNodeCollection) InitTelemetryClient() error {
	// if we are already connected, just return the client

	var telemecls []*telemetryclient.TelemetryClient
	for _, url := range vnc.Client.Urls() {
		// connect to Venice
		tmc, err := telemetryclient.NewTelemetryClient(url)
		if err != nil {
			log.Errorf("Error connecting to Venice %v. Err: %v", url, err)
			return err
		}

		telemecls = append(telemecls, tmc)
	}
	vnc.TelemetryCli = telemecls

	return nil
}

// QueryMetricsByReporter query metrics
func (vnc *VeniceNodeCollection) QueryMetricsByReporter(kind, reporter, timestr string) (*telemetryclient.MetricsQueryResponse, error) {
	err := vnc.InitTelemetryClient()
	if err != nil {
		return nil, err
	}
	stime := &api.Timestamp{}
	if err := stime.Parse(timestr); err != nil {
		return nil, fmt.Errorf("invalid time %v", timestr)
	}

	// build the query
	query := &telemetry_query.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: kind,
				},
				Selector: &fields.Selector{

					Requirements: []*fields.Requirement{
						{
							Key:      "reporterID",
							Operator: fields.Operator_equals.String(),
							Values:   []string{reporter},
						},
					},
				},
				StartTime: stime,
				SortOrder: "descending",
				Pagination: &telemetry_query.PaginationSpec{
					Count: 1,
				},
			},
		},
	}

	var result *telemetryclient.MetricsQueryResponse
	for _, tmc := range vnc.TelemetryCli {
		result, err = tmc.Metrics(vnc.Client.Context(), query)
		if err == nil {
			return result, nil
		}
	}

	log.Errorf("got error %v metrics fields query: %+v", err, query.Queries[0])

	return result, err
}

// QueryFwlog queries firewall log
func (vnc *VeniceNodeCollection) QueryFwlog(protocol, fwaction, timestr string, port uint32) (*telemetry_query.FwlogsQueryResponse, error) {
	// validate parameters
	_, ok := telemetry_query.FwlogActions_value[fwaction]
	if !ok {
		log.Errorf("Invalid firewall action %s", fwaction)
		return nil, fmt.Errorf("Invalid fwaction")
	}
	stime := &api.Timestamp{}
	stime.Parse(timestr)

	// build the query
	query := telemetry_query.FwlogsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.FwlogsQuerySpec{
			{
				Protocols: []string{protocol},
				DestPorts: []uint32{port},
				Actions:   []string{fwaction},
				StartTime: stime,
			},
		},
	}

	var err error
	var result *telemetry_query.FwlogsQueryResponse
	for _, tmc := range vnc.TelemetryCli {
		result, err = tmc.Fwlogs(vnc.Client.Context(), &query)
		if err == nil {
			break
		}
	}

	return result, err
}

// QueryMetricsSelector query metrics selector
func (vnc *VeniceNodeCollection) QueryMetricsSelector(kind, timestr string, sel fields.Selector) (*telemetryclient.MetricsQueryResponse, error) {
	stime := &api.Timestamp{}
	if err := stime.Parse(timestr); err != nil {
		log.Errorf("failed to parse time %v", timestr)
		return nil, err
	}

	// build the query
	query := &telemetry_query.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: kind,
				},
				Selector:     &sel,
				StartTime:    stime,
				GroupbyField: "reporterID",
				SortOrder:    "descending",
				Pagination: &telemetry_query.PaginationSpec{
					Count: 1,
				},
			},
		},
	}

	log.Debugf("Sending metrics query: %+v", query.Queries[0])

	var err error
	var result *telemetryclient.MetricsQueryResponse
	for _, tmc := range vnc.TelemetryCli {
		result, err = tmc.Metrics(vnc.Client.Context(), query)
		if err == nil {
			break
		}
	}

	return result, err
}

// QueryMetrics queries venice metrics
func (vnc *VeniceNodeCollection) QueryMetrics(kind, name, timestr string, count int32) (*telemetryclient.MetricsQueryResponse, error) {
	stime := &api.Timestamp{}
	if err := stime.Parse(timestr); err != nil {
		log.Errorf("failed to parse time %v", timestr)
		return nil, err
	}

	// build the query
	query := &telemetry_query.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: kind,
				},
				Selector: &fields.Selector{
					Requirements: []*fields.Requirement{
						{
							Key:    "name",
							Values: []string{name},
						},
					},
				},
				// Name: name,
				StartTime:    stime,
				GroupbyField: "reporterID",
				SortOrder:    "descending",
				Pagination: &telemetry_query.PaginationSpec{
					Count: count,
				},
			},
		},
	}

	log.Debugf("Sending metrics query: %+v", query.Queries[0])

	var err error
	var result *telemetryclient.MetricsQueryResponse
	for _, tmc := range vnc.TelemetryCli {
		result, err = tmc.Metrics(vnc.Client.Context(), query)
		if err == nil {
			break
		}
	}

	return result, err
}

// QueryMetricsFields query metrics fields
func (vnc *VeniceNodeCollection) QueryMetricsFields(kind, timestr string) (*telemetryclient.MetricsQueryResponse, error) {
	stime := &api.Timestamp{}
	if err := stime.Parse(timestr); err != nil {
		log.Errorf("failed to parse time %v", timestr)
		return nil, err
	}

	// build the query
	query := &telemetry_query.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: kind,
				},
				StartTime:    stime,
				GroupbyField: "reporterID",
				Pagination: &telemetry_query.PaginationSpec{
					Count: 1,
				},
			},
		},
	}

	log.Debugf("Sending metrics fields query: %+v", query.Queries[0])

	var err error
	var result *telemetryclient.MetricsQueryResponse
	for _, tmc := range vnc.TelemetryCli {
		result, err = tmc.Metrics(vnc.Client.Context(), query)
		if err == nil {
			break
		}
	}

	return result, err
}

// QueryDropMetricsForWorkloadPairs query drop metrics for workload pairs
func (vnc *VeniceNodeCollection) QueryDropMetricsForWorkloadPairs(wpc *WorkloadPairCollection, timestr string) error {
	for _, pair := range wpc.Pairs {
		dstIPAddr := pair.Second.GetIP()
		srcIPAddr := pair.First.GetIP()
		sel := fields.Selector{
			Requirements: []*fields.Requirement{
				{
					Key:    "source",
					Values: []string{srcIPAddr},
				},
				{
					Key:    "destination",
					Values: []string{dstIPAddr},
				},
			},
		}
		res, err := vnc.QueryMetricsSelector("IPv4FlowDropMetrics", timestr, sel)
		if err != nil {
			return err
		}
		log.Infof("Done with query selection")
		testFields := map[string]string{
			"DropPackets": "1",
			"DropReason":  "1",
		}

		for _, rslt := range res.Results {
			log.Infof("Results %d", len(rslt.Series))
			for _, series := range rslt.Series {
				// find the column
				log.Infof("series")
				cIndex := map[string]int{}
				for i, c := range series.Columns {
					if _, ok := testFields[c]; ok {
						cIndex[c] = i

					}
				}
				for _, t := range series.Values {
					for k, v := range cIndex {
						temp := fmt.Sprintf("%d", int(t[v].(float64)))
						if temp != testFields[k] {
							return fmt.Errorf("received %v : %v expected: %v", k, temp, testFields[k])
						}
					}
				}
			}
		}
	}
	return nil
}

//RunCommand runs command int the container
func (vnc *VeniceContainerCollection) RunCommand(cont *VeniceContainer, cmd string) (string, string, int32, error) {

	//Derivce the container ID
	trig := vnc.Testbed.NewTrigger()

	entity := cont.Node.iotaNode.Name + "_venice"

	fullCommand := "docker exec  " + cont.ContainerID + " " + cmd

	trig.AddCommand(fullCommand, entity, cont.Node.iotaNode.Name)

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		return "", "", -1, err
	}

	return triggerResp[0].Stdout, triggerResp[0].Stderr, triggerResp[0].ExitCode, nil
}
