// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package policygen

import (
	"fmt"
	"math/rand"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
)

// CreateAlertDestinationObj helper function to create alert destionation object with the given params.
func CreateAlertDestinationObj(tenant, namespace, name string, syslogServers []*monitoring.SyslogExport) *monitoring.AlertDestination {
	creationTime, _ := types.TimestampProto(time.Now())

	alertDest := &monitoring.AlertDestination{
		TypeMeta: api.TypeMeta{Kind: "AlertDestination"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			UUID:      name,
			Tenant:    tenant,
			Namespace: namespace,
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
		},
		Spec: monitoring.AlertDestinationSpec{
			SyslogServers: syslogServers,
		},
	}

	return alertDest
}

// CreateAlertObj helper function to create alert obj
func CreateAlertObj(tenant, namespace, name, state, message string, alertPolicy *monitoring.AlertPolicy,
	evt *evtsapi.Event, matchedRequirements []*monitoring.MatchedRequirement) *monitoring.Alert {
	creationTime, _ := types.TimestampProto(time.Now())

	alert := &monitoring.Alert{
		TypeMeta: api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			UUID:      name,
			Tenant:    tenant,
			Namespace: namespace,
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
		},
		Spec: monitoring.AlertSpec{
			State: state,
		},
		Status: monitoring.AlertStatus{
			Message: message,
		},
	}

	if matchedRequirements != nil {
		alert.Status.Reason = monitoring.AlertReason{
			MatchedRequirements: matchedRequirements,
		}
	}

	if alertPolicy != nil {
		alert.Status.Severity = alertPolicy.Spec.GetSeverity()
		alert.Status.Reason.PolicyID = alertPolicy.GetUUID()
	}

	if evt != nil {
		alert.Status.Source = &monitoring.AlertSource{
			Component: evt.GetSource().GetComponent(),
			NodeName:  evt.GetSource().GetNodeName(),
		}
		alert.Status.EventURI = evt.GetSelfLink()
		alert.Status.ObjectRef = evt.GetObjectRef()
	}

	return alert
}

// CreateAlertPolicyObj helper function to create alert policy object with the given params.
func CreateAlertPolicyObj(tenant, namespace, name, resource string, severity evtsapi.SeverityLevel, message string,
	requirements []*fields.Requirement, destinations []string) *monitoring.AlertPolicy {
	creationTime, _ := types.TimestampProto(time.Now())

	return &monitoring.AlertPolicy{
		TypeMeta: api.TypeMeta{
			Kind:       "AlertPolicy",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            name,
			Tenant:          tenant,
			Namespace:       namespace,
			ResourceVersion: fmt.Sprintf("%d", rand.Intn(10000)),
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
		},
		Spec: monitoring.AlertPolicySpec{
			Resource:     resource,
			Severity:     evtsapi.SeverityLevel_name[int32(severity)],
			Message:      message,
			Requirements: requirements,
			Enable:       true,
			Destinations: destinations,
			// TODO: add other parameters(persisstence duration, clear duration, destionations, etc.) when required
		},
	}
}

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
			Hostname: node,
			IPConfig: &cluster.IPConfig{
				IPAddress: "0.0.0.0/0",
			},
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: phase,
			Conditions: []cluster.SmartNICCondition{
				*condition,
			},
		},
	}
}
