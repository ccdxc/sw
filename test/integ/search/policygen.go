package search

import (
	"context"
	"fmt"
	"math/rand"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testutils"
)

var (
	infraTenant    = globals.DefaultTenant
	infraNamespace = "infra"

	// Tenants for testing
	Tenants = [...]string{"tesla", "audi"}
	// Namespaces for testing
	Namespaces = [...]string{globals.DefaultNamespace}

	healthy = cluster.SmartNICCondition{
		Type:   cluster.SmartNICCondition_HEALTHY.String(),
		Status: cluster.ConditionStatus_TRUE.String(),
	}
	unhealthy = cluster.SmartNICCondition{
		Type:   cluster.SmartNICCondition_HEALTHY.String(),
		Status: cluster.ConditionStatus_FALSE.String(),
	}
)

func createTenant(name string) *cluster.Tenant {

	creationTime, _ := types.TimestampProto(time.Now())
	tnt := cluster.Tenant{
		TypeMeta: api.TypeMeta{
			Kind:       "Tenant",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            name,
			UUID:            uuid.NewV4().String(),
			ResourceVersion: fmt.Sprintf("%d", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			Labels: map[string]string{
				"Location":   "us-west-zone1",
				"Department": "human resources",
				"Namespace":  infraNamespace,
			},
		},
	}

	return &tnt
}

func createNIC(mac, phase, host string, condition *cluster.SmartNICCondition) *cluster.SmartNIC {

	creationTime, _ := types.TimestampProto(time.Now())
	nic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind:       "SmartNIC",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            mac,
			UUID:            uuid.NewV4().String(),
			ResourceVersion: fmt.Sprintf("%d", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			Labels: map[string]string{
				"Location":  "us-west-zone3",
				"Namespace": infraNamespace,
			},
		},
		Spec: cluster.SmartNICSpec{
			Hostname: host,
			IPConfig: &cluster.IPConfig{
				IPAddress: "0.0.0.0/0",
			},
			MgmtMode:    cluster.SmartNICSpec_NETWORK.String(),
			NetworkMode: cluster.SmartNICSpec_OOB.String(),
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: phase,
			Conditions: []cluster.SmartNICCondition{
				*condition,
			},
		},
	}

	return &nic
}

func createNetwork(tenant, namespace, name, subnet, gw string) *network.Network {

	creationTime, _ := types.TimestampProto(time.Now())
	net := network.Network{
		TypeMeta: api.TypeMeta{
			Kind:       "Network",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            name,
			Tenant:          tenant,
			Namespace:       namespace,
			UUID:            uuid.NewV4().String(),
			ResourceVersion: fmt.Sprintf("%d", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			Labels: map[string]string{
				"Location":    "us-west-zone2",
				"NetworkType": "External",
				"Application": "MS-Exchange",
			},
		},
		Spec: network.NetworkSpec{
			IPv4Subnet:  subnet,
			IPv4Gateway: gw,
		},
		Status: network.NetworkStatus{},
	}

	return &net
}

func createSg(tenant, namespace, name string, selectors *labels.Selector) *security.SecurityGroup {
	// sg object
	creationTime, _ := types.TimestampProto(time.Now())
	sg := security.SecurityGroup{
		TypeMeta: api.TypeMeta{
			Kind:       "SecurityGroup",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:          tenant,
			Name:            name,
			Namespace:       namespace,
			UUID:            uuid.NewV4().String(),
			ResourceVersion: fmt.Sprintf("%d    ", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			Labels: map[string]string{
				"Location":    "us-west-zone2",
				"Application": "SAP-HANA",
			},
		},
		Spec: security.SecurityGroupSpec{
			WorkloadSelector: selectors,
		},
	}

	return &sg
}

// helper function to record events
func recordEvents(proxyURL, eventsDir string, eventCount int64, logger log.Logger) {
	// create recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
		Source:       &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "search_integ_test"},
		EvtTypes:     evtsapi.GetEventTypes(),
		EvtsProxyURL: proxyURL,
		BackupDir:    eventsDir}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder")
	}

	for i := int64(0); i < eventCount; i++ {
		evtsRecorder.Event(evtsapi.ServiceRunning, evtsapi.SeverityLevel_INFO, fmt.Sprintf("Service %s running", testutils.CreateAlphabetString(5)), nil)
	}
}

func createSGPolicy(tenant, namespace, name string, rules []security.SGRule) *security.SGPolicy {
	// SGPolicy object
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{
			Kind:       "SGPolicy",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Name:      name,
			Namespace: namespace,
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}
	return &sgp
}

// PolicyGenerator is a helper function that creates config/policy
// objects in API-server
func PolicyGenerator(ctx context.Context, apiClient apiclient.Services, objCount int64) {

	// Create Tenant objects
	for _, tenant := range Tenants {

		tnt := createTenant(tenant)
		log.Infof("Creating Tenant uuid: %s name: %s", tnt.ObjectMeta.UUID, tnt.Name)
		if _, err := apiClient.ClusterV1().Tenant().Create(ctx, tnt); err != nil {
			log.Errorf("Failed to create tenant object: %s err: %v", tenant, err)
		}
	}

	// Create SmartNIC, Network and Sgpolicy objects in API-server
	var i int64
	for i = 0; i < objCount; i++ {

		// Create SmartNIC object
		phase := cluster.SmartNICStatus_ADMITTED.String()
		condition := &healthy
		if i%2 == 0 {
			phase = cluster.SmartNICStatus_PENDING.String()
			condition = &unhealthy
		}
		mac := fmt.Sprintf("4444.4400.%02x%02x", i/256, i%256)
		node := fmt.Sprintf("esx-%05x", i)
		nic := createNIC(mac, phase, node, condition)
		log.Infof("Creating SmartNIC uuid: %s name: %s", nic.ObjectMeta.UUID, nic.Name)
		if _, err := apiClient.ClusterV1().SmartNIC().Create(ctx, nic); err != nil {
			log.Errorf("Failed to create smartNIC object: %s err: %v", nic.Name, err)
		}

		// Create Network object
		netObj := createNetwork(Tenants[i%2],
			Namespaces[0],
			fmt.Sprintf("net%02x", i),
			fmt.Sprintf("12.%x.%x.0/24", i/256, i%256),
			fmt.Sprintf("12.%x.%x.254", i/256, i%256))
		log.Infof("Creating Network uuid: %s name: %s", netObj.ObjectMeta.UUID, netObj.Name)
		if _, err := apiClient.NetworkV1().Network().Create(ctx, netObj); err != nil {
			log.Errorf("Failed to create tenant object: %s err: %v", netObj.Name, err)
		}

		// Create Security group object
		sg := createSg(Tenants[i%2],
			Namespaces[0],
			fmt.Sprintf("sg%02x", i),
			labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		log.Infof("\nCreating Security-Group uuid: %s name: %s", sg.ObjectMeta.UUID, sg.Name)
		if _, err := apiClient.SecurityV1().SecurityGroup().Create(ctx, sg); err != nil {
			log.Errorf("Failed to create Security-Group object: %s err: %v", sg.Name, err)
		}
	}

	for _, sgname := range []string{"dns-clients", "dns-servers", "test-servers", "web-servers", "app-servers"} {
		sg := createSg("default", "default", sgname,
			labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		log.Infof("\nCreating Security-Group uuid: %s name: %s", sg.ObjectMeta.UUID, sg.Name)
		if _, err := apiClient.SecurityV1().SecurityGroup().Create(ctx, sg); err != nil {
			log.Errorf("Failed to create Security-Group object: %s err: %v", sg.Name, err)
		}
	}
	// Create SGPolicy object-1
	rules1 := []security.SGRule{
		// 0
		{
			ProtoPorts: []security.ProtoPort{
				{Protocol: "tcp", Ports: "80"},
				{Protocol: "udp", Ports: "53"},
			},

			FromIPAddresses: []string{
				"172.0.0.1",
				"172.0.0.2",
				"10.0.0.1/30",
			},
			ToIPAddresses: []string{
				"229.204.171.210/16",
			},
			Action: security.SGRule_PERMIT.String(),
		},
		// 1
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "443"}},
			FromIPAddresses: []string{
				"37.232.218.135/22",
			},
			ToIPAddresses: []string{
				"37.232.218.136/30",
			},
			Action: security.SGRule_PERMIT.String(),
		},
		// 2
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "22"}},
			FromIPAddresses: []string{
				"any",
			},
			ToIPAddresses: []string{
				"any",
			},
			Action: security.SGRule_PERMIT.String(),
		},
		// 3
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "icmp"}},
			FromIPAddresses: []string{
				"10.1.1.1",
			},
			ToIPAddresses: []string{
				"20.1.1.1",
			},
			Action: security.SGRule_PERMIT.String(),
		},
		// 4
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "udp", Ports: "53"}},
			FromSecurityGroups: []string{
				"dns-clients",
			},
			ToSecurityGroups: []string{
				"dns-servers",
			},
			Action: security.SGRule_PERMIT.String(),
		},
		// 5
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "udp", Ports: "53"}},
			FromSecurityGroups: []string{
				"test-servers",
			},
			ToSecurityGroups: []string{
				"dns-servers",
			},
			Action: security.SGRule_DENY.String(),
		},
	}
	sgp1 := createSGPolicy(globals.DefaultTenant, globals.DefaultNamespace, "sgp-1", rules1)
	log.Infof("\nCreating SGP policy name: %s", sgp1.Name)
	if _, err := apiClient.SecurityV1().SGPolicy().Create(ctx, sgp1); err != nil {
		log.Errorf("Failed to create SGPolicy object: %s err: %v", sgp1.Name, err)
	}

	// Create SGPolicy object-2
	rules2 := []security.SGRule{
		// 0
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "1024"}},
			FromSecurityGroups: []string{
				"web-servers",
			},
			ToSecurityGroups: []string{
				"app-servers",
			},
			Action: security.SGRule_PERMIT.String(),
		},
		{
			ProtoPorts: []security.ProtoPort{{Protocol: "tcp", Ports: "80"}},
			FromIPAddresses: []string{
				"30.1.1.1-30.1.1.10",
			},
			ToIPAddresses: []string{
				"40.1.1.1-40.1.1.10",
			},
			Action: security.SGRule_PERMIT.String(),
		},
	}
	sgp2 := createSGPolicy(globals.DefaultTenant, globals.DefaultNamespace, "sgp-2", rules2)
	log.Infof("\nCreating SGP policy name: %s", sgp2.Name)
	if _, err := apiClient.SecurityV1().SGPolicy().Create(ctx, sgp2); err != nil {
		log.Errorf("Failed to create SGPolicy object: %s err: %v", sgp2.Name, err)
	}

	// Create SGPolicy object-3 with 70k rules
	proto := []string{"tcp", "udp"}
	actions := []string{security.SGRule_PERMIT.String(), security.SGRule_DENY.String()}
	rules3 := make([]security.SGRule, 70000)
	for i := 0; i < 70000; i++ {
		rules3[i] = security.SGRule{
			ProtoPorts:      []security.ProtoPort{{Protocol: fmt.Sprintf("%s", proto[i%2]), Ports: fmt.Sprintf("%d", (i+1)%65536)}},
			Action:          actions[i%2],
			FromIPAddresses: []string{fmt.Sprintf("10.%d.%d.%d/32", (i/(256*256))%256, (i/256)%256, i%256)},
			ToIPAddresses:   []string{fmt.Sprintf("20.%d.%d.%d/32", (i/(256*256))%256, (i/256)%256, i%256)},
		}
	}
	sgp3 := createSGPolicy(globals.DefaultTenant, globals.DefaultNamespace, "sgp-scale", rules3)
	if _, err := apiClient.SecurityV1().SGPolicy().Create(ctx, sgp3); err != nil {
		log.Errorf("Failed to create SGPolicy object: %s err: %v", sgp3.Name, err)
	}
}

// DefaultTenantPolicyGenerator generates objects in the default tenant
func DefaultTenantPolicyGenerator(ctx context.Context, apiClient apiclient.Services, objCount int64, offset int64) {
	// Create SmartNIC, Network and Sgpolicy objects in API-server
	var i int64
	for i = offset; i < offset+objCount; i++ {

		switch i % 3 {
		case 0:
			// Create SmartNIC object
			phase := cluster.SmartNICStatus_ADMITTED.String()
			condition := &healthy
			if i%2 == 0 {
				phase = cluster.SmartNICStatus_PENDING.String()
				condition = &unhealthy
			}
			mac := fmt.Sprintf("4444.4400.%02x%02x", i/256, i%256)
			node := fmt.Sprintf("esx-%05x", i)
			nic := createNIC(mac, phase, node, condition)
			log.Infof("Creating SmartNIC uuid: %s name: %s", nic.ObjectMeta.UUID, nic.Name)
			if _, err := apiClient.ClusterV1().SmartNIC().Create(ctx, nic); err != nil {
				log.Errorf("Failed to create smartNIC object: %s err: %v", nic.Name, err)
			}
		case 1:
			netObj := createNetwork(globals.DefaultTenant,
				Namespaces[0],
				fmt.Sprintf("net%02x", i),
				fmt.Sprintf("12.%x.%x.0/24", i/256, i%256),
				fmt.Sprintf("12.%x.%x.254", i/256, i%256))
			log.Infof("Creating Network uuid: %s name: %s", netObj.ObjectMeta.UUID, netObj.Name)
			if _, err := apiClient.NetworkV1().Network().Create(ctx, netObj); err != nil {
				log.Errorf("Failed to create network object: %s err: %v", netObj.Name, err)
			}
		case 2:
			// Create Security group object
			sg := createSg(globals.DefaultTenant,
				Namespaces[0],
				fmt.Sprintf("sg%02x", i),
				labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
			log.Infof("Creating Security-Group uuid: %s name: %s", sg.ObjectMeta.UUID, sg.Name)
			if _, err := apiClient.SecurityV1().SecurityGroup().Create(ctx, sg); err != nil {
				log.Errorf("Failed to create Security-Group object: %s err: %v", sg.Name, err)
			}
		}
	}
}

// DeleteDefaultTenantPolicyGenerator deletes objects generated by DefaultTenantPolicyGenerator
func DeleteDefaultTenantPolicyGenerator(ctx context.Context, apiClient apiclient.Services, objCount int64, offset int64) {
	// Create SmartNIC, Network and Sgpolicy objects in API-server
	var i int64
	for i = offset; i < offset+objCount; i++ {

		switch i % 3 {
		case 0:
			// Create SmartNIC object
			phase := cluster.SmartNICStatus_ADMITTED.String()
			condition := &healthy
			if i%2 == 0 {
				phase = cluster.SmartNICStatus_PENDING.String()
				condition = &unhealthy
			}
			mac := fmt.Sprintf("4444.4400.%02x%02x", i/256, i%256)
			node := fmt.Sprintf("esx-%05x", i)
			nic := createNIC(mac, phase, node, condition)
			log.Infof("Deleting SmartNIC uuid: %s name: %s", nic.ObjectMeta.UUID, nic.Name)
			if _, err := apiClient.ClusterV1().SmartNIC().Delete(ctx, nic.GetObjectMeta()); err != nil {
				log.Errorf("Failed to Delete smartNIC object: %s err: %v", nic.Name, err)
			}
		case 1:
			netObj := createNetwork(globals.DefaultTenant,
				Namespaces[0],
				fmt.Sprintf("net%02x", i),
				fmt.Sprintf("12.%x.%x.0/24", i/256, i%256),
				fmt.Sprintf("12.%x.%x.254", i/256, i%256))
			log.Infof("Deleting Network uuid: %s name: %s", netObj.ObjectMeta.UUID, netObj.Name)
			if _, err := apiClient.NetworkV1().Network().Delete(ctx, netObj.GetObjectMeta()); err != nil {
				log.Errorf("Failed to Delete network object: %s err: %v", netObj.Name, err)
			}
		case 2:
			// Create Security group object
			sg := createSg(globals.DefaultTenant,
				Namespaces[0],
				fmt.Sprintf("sg%02x", i),
				labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
			log.Infof("Deleting Security-Group uuid: %s name: %s", sg.ObjectMeta.UUID, sg.Name)
			if _, err := apiClient.SecurityV1().SecurityGroup().Delete(ctx, sg.GetObjectMeta()); err != nil {
				log.Errorf("Failed to Delete Security-Group object: %s err: %v", sg.Name, err)
			}
		}
	}
}
