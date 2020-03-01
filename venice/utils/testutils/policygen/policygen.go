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
	"github.com/pensando/sw/events/generated/eventattrs"
)

// CreateEventPolicyObj helper function to create event policy object with the given params.
func CreateEventPolicyObj(tenant, namespace, name, format string, targets []*monitoring.ExportConfig,
	syslogConfig *monitoring.SyslogExportConfig) *monitoring.EventPolicy {
	creationTime, _ := types.TimestampProto(time.Now())

	eventPolicyObj := &monitoring.EventPolicy{
		TypeMeta: api.TypeMeta{Kind: "EventPolicy"},
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
		Spec: monitoring.EventPolicySpec{
			Format:       format,
			Targets:      targets,
			SyslogConfig: syslogConfig,
		},
	}

	return eventPolicyObj
}

// CreateEventObj helper function to create event object with the given params.
func CreateEventObj(tenant, namespace, name, eType, severity, message string) *evtsapi.Event {
	creationTime, _ := types.TimestampProto(time.Now())

	eventObj := &evtsapi.Event{
		TypeMeta: api.TypeMeta{Kind: "Event"},
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
		EventAttributes: evtsapi.EventAttributes{
			Severity: severity,
			Type:     eType,
			Message:  message,
		},
	}
	return eventObj
}

// CreateAlertDestinationObj helper function to create alert destination object with the given params.
func CreateAlertDestinationObj(tenant, namespace, name string, syslogExport *monitoring.SyslogExport) *monitoring.AlertDestination {
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
			SyslogExport: syslogExport,
		},
	}

	return alertDest
}

// CreateAlertObj helper function to create alert obj
func CreateAlertObj(tenant, namespace, name string, state monitoring.AlertState, message string, alertPolicy *monitoring.AlertPolicy,
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
			State: state.String(),
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
func CreateAlertPolicyObj(tenant, namespace, name, resource string, severity eventattrs.Severity, message string,
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
			Severity:     severity.String(),
			Message:      message,
			Requirements: requirements,
			Enable:       true,
			Destinations: destinations,
			// TODO: add other parameters(persistence duration, clear duration, destinations, etc.) when required
		},
	}
}

// CreateSmartNIC helper function to create smart NIC object with the given params.
func CreateSmartNIC(mac, phase, node string, condition *cluster.DSCCondition) *cluster.DistributedServiceCard {
	creationTime, _ := types.TimestampProto(time.Now())

	return &cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{
			Kind:       "DistributedServiceCard",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:            mac,
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
		Spec: cluster.DistributedServiceCardSpec{
			ID: node,
			IPConfig: &cluster.IPConfig{
				IPAddress: "0.0.0.0/0",
			},
			MgmtMode:    cluster.DistributedServiceCardSpec_NETWORK.String(),
			NetworkMode: cluster.DistributedServiceCardSpec_OOB.String(),
			DSCProfile:  "default",
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: phase,
			PrimaryMAC:     mac,
			Conditions: []cluster.DSCCondition{
				*condition,
			},
		},
	}
}
