package memdb

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/runtime"
)

// event types
const (
	CreateEvent memdb.EventType = memdb.CreateEvent
	UpdateEvent memdb.EventType = memdb.UpdateEvent
	DeleteEvent memdb.EventType = memdb.DeleteEvent
)

// MemDb in-memory database to store policy objects and alerts
type MemDb struct {
	*memdb.Memdb
}

// FilterFn represents various filters that can be applied before returning the list response
type FilterFn func(obj runtime.Object) bool

// WithTenantFilter returns a fn() which returns true if the object matches the given tenant
func WithTenantFilter(tenant string) FilterFn {
	return func(obj runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		return objm.GetObjectMeta().GetTenant() == tenant
	}
}

// WithResourceFilter returns a fn() which returns true if the alert policy object matches the given resource
// * applicable only for alert policy object *
func WithResourceFilter(resource string) FilterFn {
	return func(obj runtime.Object) bool {
		switch obj.(type) {
		case *monitoring.AlertPolicy:
			return obj.(*monitoring.AlertPolicy).Spec.Resource == resource
		}
		return false
	}
}

// WithEnabledFilter returns a fn() which returns true if the alert policy object matches the given enabled flag
// * applicable only for alert policy object *
func WithEnabledFilter(enabled bool) FilterFn {
	return func(obj runtime.Object) bool {
		switch obj.(type) {
		case *monitoring.AlertPolicy:
			return obj.(*monitoring.AlertPolicy).Spec.Enable == enabled
		}
		return false
	}
}

// WithAlertStateFilter returns a fn() which returns true if the alert object matches the given state
// * applicable only for alert object *
func WithAlertStateFilter(state string) FilterFn {
	return func(obj runtime.Object) bool {
		switch obj.(type) {
		case *monitoring.Alert:
			return obj.(*monitoring.Alert).Spec.GetState() == state
		}
		return false
	}
}

// WithAlertPolicyIDFilter returns a fn() which returns true if the alert object matches the given policy ID
// * applicable only for alert object *
func WithAlertPolicyIDFilter(policyID string) FilterFn {
	return func(obj runtime.Object) bool {
		switch obj.(type) {
		case *monitoring.Alert:
			return obj.(*monitoring.Alert).Status.Reason.GetPolicyID() == policyID
		}
		return false
	}
}

// WithObjectRefFilter returns a fn() which returns true if the alert object matches the given object ref
// * applicable only for alert object *
func WithObjectRefFilter(objRef *api.ObjectRef) FilterFn {
	return func(obj runtime.Object) bool {
		switch obj.(type) {
		case *monitoring.Alert:
			alert := obj.(*monitoring.Alert)
			return reflect.DeepEqual(alert.Status.GetObjectRef(), objRef)
		}
		return false
	}
}

// GetAlertPolicies returns the list of alert policies that matches all the given filters
func (m *MemDb) GetAlertPolicies(filters ...FilterFn) []*monitoring.AlertPolicy {
	var alertPolicies []*monitoring.AlertPolicy
	for _, policy := range m.ListObjects("AlertPolicy") {
		ap := *(policy.(*monitoring.AlertPolicy))

		matched := 0
		for _, filter := range filters { // run through filters
			if !filter(&ap) {
				break
			}
			matched++
		}

		if matched == len(filters) {
			var reqs []*fields.Requirement
			for _, t := range ap.Spec.GetRequirements() {
				r := *t
				reqs = append(reqs, &r)
			}
			ap.Spec.Requirements = reqs
			alertPolicies = append(alertPolicies, &ap)
		}
	}

	return alertPolicies
}

// GetAlerts returns the list of alerts that matches all the given filters
func (m *MemDb) GetAlerts(filters ...FilterFn) []*monitoring.Alert {
	var alerts []*monitoring.Alert
	for _, alert := range m.ListObjects("Alert") {
		a := alert.(*monitoring.Alert)

		matched := 0
		for _, filter := range filters { // run through filters
			if !filter(a) {
				break
			}
			matched++
		}

		if matched == len(filters) {
			alerts = append(alerts, a)
		}
	}

	return alerts
}

// GetAlertDestination returns the alert destionation matching the given name
func (m *MemDb) GetAlertDestination(name string) *monitoring.AlertDestination {
	for _, alertD := range m.ListObjects("AlertDestination") {
		ad := alertD.(*monitoring.AlertDestination)
		if ad.GetName() == name {
			return ad
		}
	}

	return nil
}

// WatchAlertDestinations returns the watcher to watch for alert destionation events
func (m *MemDb) WatchAlertDestinations() chan memdb.Event {
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	m.WatchObjects("AlertDestination", watchChan)
	return watchChan
}

// StopWatchAlertDestinations stops the alert destionation watcher
func (m *MemDb) StopWatchAlertDestinations(watchChan chan memdb.Event) {
	m.StopWatchObjects("AlertDestination", watchChan)
}

// NewMemDb creates a new mem DB
func NewMemDb() *MemDb {
	amemDb := &MemDb{
		memdb.NewMemdb(),
	}

	return amemDb
}
