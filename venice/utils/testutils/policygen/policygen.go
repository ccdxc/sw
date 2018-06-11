// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package policygen

import (
	"fmt"
	"math/rand"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
)

// CreateSmartNIC helper function to create smart NIC object with the given params.
func CreateSmartNIC(mac, phase, node string, condition *cluster.SmartNICCondition) *cluster.SmartNIC {
	creationTime, _ := types.TimestampProto(time.Now())

	return &cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind:       "SmartNIC",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            mac,
			Tenant:          globals.DefaultTenant,
			Namespace:       globals.DefaultNamespace,
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
		Spec: cluster.SmartNICSpec{
			Phase:    phase,
			HostName: node,
			MgmtIp:   "0.0.0.0",
		},
		Status: cluster.SmartNICStatus{
			Conditions: []*cluster.SmartNICCondition{
				condition,
			},
		},
	}
}
