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
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	infraTenant    = globals.DefaultTenant
	infraNamespace = "infra"

	// Tenants for testing
	Tenants = [...]string{"tesla", "audi"}
	// Namespaces for testing
	Namespaces = [...]string{"staging", "production"}

	healthy = cmd.SmartNICCondition{
		Type:   cmd.SmartNICCondition_HEALTHY.String(),
		Status: cmd.ConditionStatus_TRUE.String(),
	}
	unhealthy = cmd.SmartNICCondition{
		Type:   cmd.SmartNICCondition_HEALTHY.String(),
		Status: cmd.ConditionStatus_FALSE.String(),
	}
)

func createTenant(name string) *network.Tenant {

	creationTime, _ := types.TimestampProto(time.Now())
	tnt := network.Tenant{
		TypeMeta: api.TypeMeta{
			Kind:       "Tenant",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            name,
			Tenant:          infraTenant,
			Namespace:       infraNamespace,
			UUID:            uuid.NewV4().String(),
			ResourceVersion: fmt.Sprintf("%d", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			Labels: map[string]string{
				"Location": "us-west-zone1",
			},
		},
	}

	return &tnt
}

func createNIC(mac, phase, node string, condition *cmd.SmartNICCondition) *cmd.SmartNIC {

	creationTime, _ := types.TimestampProto(time.Now())
	nic := cmd.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind:       "SmartNIC",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            mac,
			Tenant:          infraTenant,
			Namespace:       infraNamespace,
			UUID:            uuid.NewV4().String(),
			ResourceVersion: fmt.Sprintf("%d", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			Labels: map[string]string{
				"Location": "us-west-zone3",
			},
		},
		Spec: cmd.SmartNICSpec{
			Phase:    phase,
			NodeName: node,
			MgmtIp:   "0.0.0.0",
		},
		Status: cmd.SmartNICStatus{
			Conditions: []*cmd.SmartNICCondition{
				condition,
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

func createSg(tenant, namespace, name string, selectors *labels.Selector) *network.SecurityGroup {
	// sg object
	creationTime, _ := types.TimestampProto(time.Now())
	sg := network.SecurityGroup{
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
		Spec: network.SecurityGroupSpec{
			WorkloadSelector: selectors,
		},
	}

	return &sg
}

// PolicyGenerator is a helper function that creates config/policy
// objects in API-server
func PolicyGenerator(ctx context.Context, apiClient apiclient.Services, objCount int64) {

	// Create Tenant objects
	for _, tenant := range Tenants {

		tnt := createTenant(tenant)
		log.Infof("Creating Tenant uuid: %s name: %s", tnt.ObjectMeta.UUID, tnt.Name)
		if _, err := apiClient.TenantV1().Tenant().Create(ctx, tnt); err != nil {
			log.Errorf("Failed to create tenant object: %s err: %v", tenant, err)
		}
	}

	// Create SmartNIC, Network and Sgpolicy objects in API-server
	var i int64
	for i = 0; i < objCount; i++ {

		// Create SmartNIC object
		phase := cmd.SmartNICSpec_ADMITTED.String()
		condition := &healthy
		if i%2 == 0 {
			phase = cmd.SmartNICSpec_PENDING.String()
			condition = &unhealthy
		}
		mac := fmt.Sprintf("44.44.44.00.%02x.%02x", i/256, i%256)
		node := fmt.Sprintf("esx-%05x", i)
		nic := createNIC(mac, phase, node, condition)
		log.Infof("Creating SmartNIC uuid: %s name: %s", nic.ObjectMeta.UUID, nic.Name)
		if _, err := apiClient.CmdV1().SmartNIC().Create(ctx, nic); err != nil {
			log.Errorf("Failed to create smartNIC object: %s err: %v", nic.Name, err)
		}

		// Create Network object
		netObj := createNetwork(Tenants[i%2],
			Namespaces[i%2],
			fmt.Sprintf("net%02x", i),
			fmt.Sprintf("10.%x.%x.0/24", i/256, i%256),
			fmt.Sprintf("10.%x.%x.254", i/256, i%256))
		log.Infof("Creating Network uuid: %s name: %s", netObj.ObjectMeta.UUID, netObj.Name)
		if _, err := apiClient.NetworkV1().Network().Create(ctx, netObj); err != nil {
			log.Errorf("Failed to create tenant object: %s err: %v", netObj.Name, err)
		}

		// Create Security group object
		sg := createSg(Tenants[i%2],
			Namespaces[i%2],
			fmt.Sprintf("sg%02x", i),
			labels.SelectorFromSet(labels.Set{"env": "production", "app": "procurement"}))
		log.Infof("\nCreating Security-Group uuid: %s name: %s", sg.ObjectMeta.UUID, sg.Name)
		if _, err := apiClient.SecurityGroupV1().SecurityGroup().Create(ctx, sg); err != nil {
			log.Errorf("Failed to create Security-Group object: %s err: %v", sg.Name, err)
		}
	}
}
