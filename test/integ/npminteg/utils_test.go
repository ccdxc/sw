// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/netagent"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/netagent/ctrlerif/restapi"
	"github.com/pensando/sw/nic/agent/netagent/datapath"
	state "github.com/pensando/sw/nic/agent/netagent/protos"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Dpagent is an agent instance
type Dpagent struct {
	nagent *netagent.Agent
}

// objKey returns endpoint key
func objKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// pollTimeout returns the poll timeout value based on number of agents
func (it *integTestSuite) pollTimeout() string {
	return fmt.Sprintf("%ds", 10+(it.numAgents))
}

// CreateAgent creates an instance of agent
func CreateAgent(kind datapath.Kind, srvURL string, resolver resolver.Interface) (*Dpagent, error) {
	// create new network agent
	nagent, err := netagent.NewAgent(kind.String(), "", srvURL, resolver, state.AgentMode_MANAGED)
	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	restServer, err := restapi.NewRestServer(nagent.NetworkAgent, nil, nil, "")
	nagent.RestServer = restServer

	// Create NPM Client.
	// TODO Remove this when nmd and delphi hub are integrated with venice_integ and npm_integ
	npmClient, err := ctrlerif.NewNpmClient(nagent.NetworkAgent, srvURL, resolver)
	if err != nil {
		log.Errorf("Error creating NPM client. Err: %v", err)
	}
	nagent.NpmClient = npmClient

	// create an agent instance
	ag := Dpagent{
		nagent: nagent,
	}

	return &ag, nil
}

func (ag *Dpagent) createEndpointReq(tenant, namespace, net, epname, host string) (*netproto.Endpoint, error) {
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Name:      epname,
			Namespace: namespace,
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   epname,
			WorkloadName:   epname,
			WorkloadUUID:   epname,
			NetworkName:    net,
			HomingHostAddr: host,
			HomingHostName: host,
		},
	}

	// make a create request
	ep, _, err := ag.nagent.NetworkAgent.EndpointCreateReq(&epinfo)

	return ep, err
}

func (ag *Dpagent) deleteEndpointReq(tenant, net, epname, host string) (*netproto.Endpoint, error) {
	epinfo := netproto.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenant,
			Name:   epname,
		},
		Spec: netproto.EndpointSpec{
			EndpointUUID:   epname,
			WorkloadName:   epname,
			WorkloadUUID:   epname,
			NetworkName:    net,
			HomingHostAddr: host,
			HomingHostName: host,
		},
	}

	// make a create request
	err := ag.nagent.NetworkAgent.EndpointDeleteReq(&epinfo)
	return &epinfo, err
}

// CreateNetwork injects a create network event on the watcher
func (it *integTestSuite) CreateNetwork(tenant, namespace, net, subnet, gw string) error {
	// build network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:      net,
			Namespace: namespace,
			Tenant:    tenant,
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	_, err := it.apisrvClient.NetworkV1().Network().Create(context.Background(), &nw)

	return err
}

// DeleteNetwork injects a delete network event to the watcher
func (it *integTestSuite) DeleteNetwork(tenant, net string) error {
	// create a dummy network object
	nw := network.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Name:   net,
			Tenant: tenant,
		},
	}

	_, err := it.apisrvClient.NetworkV1().Network().Delete(context.Background(), &nw.ObjectMeta)

	return err
}

// CreateEndpoint injects an endpoint create event
func (it *integTestSuite) CreateEndpoint(tenant, namespace, net, epName, vmName, macAddr, hostName, hostAddr string, attr map[string]string, usegVlan uint32) error {
	epInfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      epName,
			Tenant:    tenant,
			Namespace: namespace,
		},
		Spec: workload.EndpointSpec{},
		Status: workload.EndpointStatus{
			Network:            net,
			WorkloadName:       vmName,
			WorkloadAttributes: attr,
			MacAddress:         macAddr,
			HomingHostAddr:     hostAddr,
			HomingHostName:     hostName,
			MicroSegmentVlan:   usegVlan,
		},
	}

	_, err := it.apisrvClient.WorkloadV1().Endpoint().Create(context.Background(), &epInfo)

	return err
}

// DeleteEndpoint injects an endpoint delete event
func (it *integTestSuite) DeleteEndpoint(tenant, namespace, epName string) error {
	epInfo := workload.Endpoint{
		TypeMeta: api.TypeMeta{Kind: "Endpoint"},
		ObjectMeta: api.ObjectMeta{
			Name:      epName,
			Tenant:    tenant,
			Namespace: namespace,
		},
	}

	_, err := it.apisrvClient.WorkloadV1().Endpoint().Delete(context.Background(), &epInfo.ObjectMeta)

	return err
}

// CreateSecurityGroup injects a create sg event on the watcher
func (it *integTestSuite) CreateSecurityGroup(tenant, namespace, sgname string, selector *labels.Selector) error {
	// build sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      sgname,
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selector,
		},
	}

	_, err := it.apisrvClient.SecurityV1().SecurityGroup().Create(context.Background(), &sg)

	return err
}

// DeleteSecurityGroup injects a delete sg event to the watcher
func (it *integTestSuite) DeleteSecurityGroup(tenant, sgname string) error {
	// build a sg object
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{Kind: "SecurityGroup"},
		ObjectMeta: api.ObjectMeta{
			Name:   sgname,
			Tenant: tenant,
		},
	}

	_, err := it.apisrvClient.SecurityV1().SecurityGroup().Delete(context.Background(), &sg.ObjectMeta)

	return err
}

// CreateSgpolicy injects a create sg policy event on the watcher
func (it *integTestSuite) CreateSgpolicy(tenant, namespace, pname string, attachTenant bool, attachGroups []string, rules []security.SGRule) error {
	// build sg object
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      pname,
		},
		Spec: security.SGPolicySpec{
			AttachTenant: attachTenant,
			AttachGroups: attachGroups,
			Rules:        rules,
		},
	}

	_, err := it.apisrvClient.SecurityV1().SGPolicy().Create(context.Background(), &sgp)

	return err
}

// DeleteSgpolicy injects a delete sg policy event to the watcher
func (it *integTestSuite) DeleteSgpolicy(tenant, namespace, pname string) error {
	// build a sg object
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      pname,
			Namespace: namespace,
			Tenant:    tenant,
		},
	}

	_, err := it.apisrvClient.SecurityV1().SGPolicy().Delete(context.Background(), &sgp.ObjectMeta)

	return err
}

// CreateWorkload creates a workload
func (it *integTestSuite) CreateWorkload(tenant, namespace, name, host, macAddr string, usegVlan, extVlan uint32) error {
	// build workload object
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: namespace,
			Tenant:    tenant,
		},
		Spec: workload.WorkloadSpec{
			HostName: host,
			Interfaces: []workload.WorkloadIntfSpec{
				{
					MACAddress:   macAddr,
					MicroSegVlan: usegVlan,
					ExternalVlan: extVlan,
				},
			},
		},
	}

	_, err := it.apisrvClient.WorkloadV1().Workload().Create(context.Background(), &wr)

	return err
}

// DeleteWorkload deletes a workload
func (it *integTestSuite) DeleteWorkload(tenant, namespace, name string) error {
	// create a dummy workload object
	wr := workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "Workload"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			Namespace: namespace,
			Tenant:    tenant,
		},
	}

	_, err := it.apisrvClient.WorkloadV1().Workload().Delete(context.Background(), &wr.ObjectMeta)

	return err
}

// CreateHost creates a host and an associated smart nic
func (it *integTestSuite) CreateHost(name, macAddr string) error {
	// smartNic params
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name: name + "-NIC",
		},
		Spec: cluster.SmartNICSpec{
			MgmtMode:    "NETWORK",
			NetworkMode: "OOB",
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     macAddr,
		},
	}

	_, err := it.apisrvClient.ClusterV1().SmartNIC().Create(context.Background(), &snic)
	if err != nil {
		return err
	}

	// build host object
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.HostSpec{
			SmartNICs: []cluster.SmartNICID{
				{
					MACAddress: macAddr,
				},
			},
		},
		Status: cluster.HostStatus{},
	}

	_, err = it.apisrvClient.ClusterV1().Host().Create(context.Background(), &host)

	return err
}

// DeleteHost deletes a workload
func (it *integTestSuite) DeleteHost(name string) error {
	// create a dummy tenant object
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}

	_, err := it.apisrvClient.ClusterV1().Host().Delete(context.Background(), &host.ObjectMeta)
	if err != nil {
		return err
	}

	// smartNic object
	snic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{Kind: "SmartNIC"},
		ObjectMeta: api.ObjectMeta{
			Name:      name + "-NIC",
			Namespace: "",
			Tenant:    "default",
		},
	}

	_, err = it.apisrvClient.ClusterV1().SmartNIC().Delete(context.Background(), &snic.ObjectMeta)

	return err
}
