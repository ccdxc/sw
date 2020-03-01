// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"
	"strconv"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/nic/agent/dscagent"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// Dpagent is an agent instance
type Dpagent struct {
	dscAgent *dscagent.DSCAgent
}

// objKey returns endpoint key
func objKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// pollTimeout returns the poll timeout value based on number of agents
func (it *integTestSuite) pollTimeout() string {
	return fmt.Sprintf("%ds", 10+(it.numAgents))
}

// endpointPollTimeout returns the endpoint poll timeout value based on number of agents
func (it *integTestSuite) endpointPollTimeout() string {
	return fmt.Sprintf("%ds", 3*(10+(it.numAgents)))
}

// CreateAgent creates an instance of agent
func CreateAgent(logger log.Logger, veniceURL, nodeUUID string) (*Dpagent, error) {
	// create new network agent
	var lis netutils.TestListenAddr
	if err := lis.GetAvailablePort(); err != nil {
		log.Errorf("Failed to find an available port. Err: %v", err)
		return nil, fmt.Errorf("failed to find an available port. Err: %v", err)
	}
	nagent, err := dscagent.NewDSCAgent(logger, globals.Npm, globals.Tpm, globals.Tsm, lis.ListenURL.String())
	if err != nil {
		log.Errorf("Error creating network agent. Err: %v", err)
		return nil, err
	}

	// Handle mode change

	o := types.DistributedServiceCardStatus{
		DSCName:     nodeUUID,
		DSCMode:     "network_managed_inband",
		MgmtIP:      "42.42.42.42/24",
		Controllers: []string{veniceURL},
	}
	err = nagent.ControllerAPI.HandleVeniceCoordinates(o)
	logger.Infof("RestURL: %v", lis.ListenURL.String())
	time.Sleep(time.Second * 5)

	if err != nil {
		log.Errorf("Failed to perform mode switch. Err: %v", err)
		return nil, fmt.Errorf("failed to perform mode switch. Err: %v", err)
	}
	// create an agent instance
	ag := Dpagent{
		dscAgent: nagent,
	}

	return &ag, nil
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
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
			VlanID:      42,
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
			NodeUUID:           "testHost-0",
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

// CreateApp injects a create sg app on the watcher
func (it *integTestSuite) CreateApp(tenant, namespace, name, port string) error {
	// build sg object
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      name,
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				security.ProtoPort{
					Ports:    port,
					Protocol: "tcp",
				},
			},
		},
	}

	_, err := it.apisrvClient.SecurityV1().App().Create(context.Background(), &app)

	return err
}

// DeleteApp delete app and send to the  watcher
func (it *integTestSuite) DeleteApp(tenant, appName string) error {
	// build a sg object
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:   appName,
			Tenant: tenant,
		},
	}

	_, err := it.apisrvClient.SecurityV1().App().Delete(context.Background(), &app.ObjectMeta)

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
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      pname,
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: attachTenant,
			AttachGroups: attachGroups,
			Rules:        rules,
		},
	}

	_, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Create(context.Background(), &sgp)

	return err
}

// CreateSgpolicy injects a create sg policy event on the watcher
func (it *integTestSuite) UpdateSgpolicy(tenant, namespace, pname string, attachTenant bool, attachGroups []string, rules []security.SGRule) error {
	// build sg object
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      pname,
		},
	}

	pol, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Get(context.Background(), &sgp.ObjectMeta)
	if err != nil {
		return err
	}

	// rule that uses the app
	sgp.Spec = pol.Spec
	for _, rule := range rules {
		sgp.Spec.Rules = append(pol.Spec.Rules, rule)
	}

	curr, _ := strconv.Atoi(pol.ObjectMeta.GenerationID)
	sgp.ObjectMeta.GenerationID = strconv.Itoa(curr + 1)

	_, err = it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Update(context.Background(), &sgp)

	fmt.Printf("SG Policy updated %v\n", sgp)
	return err
}

// DeleteSgpolicy injects a delete sg policy event to the watcher
func (it *integTestSuite) DeleteSgpolicy(tenant, namespace, pname string) error {
	// build a sg object
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      pname,
			Namespace: namespace,
			Tenant:    tenant,
		},
	}

	_, err := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Delete(context.Background(), &sgp.ObjectMeta)

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

// UpdateWorkload updates an existing workload
func (it *integTestSuite) UpdateWorkload(tenant, namespace, name, host, macAddr string, usegVlan, extVlan uint32) error {
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

	_, err := it.apisrvClient.WorkloadV1().Workload().Update(context.Background(), &wr)

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
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: macAddr,
		},
		Spec: cluster.DistributedServiceCardSpec{
			MgmtMode:    "NETWORK",
			NetworkMode: "OOB",
			DSCProfile:  "default",
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     macAddr,
			Conditions: []cluster.DSCCondition{
				{
					Type:   "HEALTHY",
					Status: cluster.ConditionStatus_TRUE.String(),
				},
			},
		},
	}

	_, err := it.apisrvClient.ClusterV1().DistributedServiceCard().Create(context.Background(), &snic)
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
			DSCs: []cluster.DistributedServiceCardID{
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
func (it *integTestSuite) DeleteHost(name, macAddr string) error {
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
	snic := cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{Kind: "DistributedServiceCard"},
		ObjectMeta: api.ObjectMeta{
			Name: macAddr,
		},
		Spec: cluster.DistributedServiceCardSpec{
			Admit:       false,
			MgmtMode:    cluster.DistributedServiceCardSpec_NETWORK.String(),
			NetworkMode: cluster.DistributedServiceCardSpec_OOB.String(),
			DSCProfile:  "default",
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: cluster.DistributedServiceCardStatus_PENDING.String(),
		},
	}

	// need to de-admit before deleting
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Update(context.Background(), &snic)
	if err != nil {
		log.Infof("Deleting snc %v..", err)
		return err
	}
	_, err = it.apisrvClient.ClusterV1().DistributedServiceCard().Delete(context.Background(), &snic.ObjectMeta)

	return err
}

// CreateTenant creates a Tenant
func (it *integTestSuite) CreateTenant(name string) error {
	tenant := &cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: string(cluster.KindTenant)},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
	}
	_, err := it.apisrvClient.ClusterV1().Tenant().Create(context.Background(), tenant)
	return err
}
