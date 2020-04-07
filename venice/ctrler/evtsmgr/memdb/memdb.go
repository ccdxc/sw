package memdb

import (
	"fmt"
	"strings"
	"sync"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/runtime"
)

// event types
const (
	CreateEvent = memdb.CreateEvent
	UpdateEvent = memdb.UpdateEvent
	DeleteEvent = memdb.DeleteEvent
)

// MemDb in-memory database to store policy objects and alerts
type MemDb struct {
	sync.RWMutex
	*memdb.Memdb
	alertsByPolicy map[string]map[monitoring.AlertState]*alertsGroup
}

// local cache alerts group
type alertsGroup struct {
	grpByEventURI                 map[string]string
	grpByEventMessageAndObjectRef map[string]string
}

// AnyOutstandingAlertsByURI returns true if there an outstanding alert by the given tenant, policy and URI,
// otherwise false.
func (m *MemDb) AnyOutstandingAlertsByURI(tenant, policyID, URI string) (string, bool) {
	m.RLock()
	defer m.RUnlock()

	key := fmt.Sprintf("%s.%s", tenant, policyID)

	if alertsByPolicy, found := m.alertsByPolicy[key]; found {
		if grp, found := alertsByPolicy[monitoring.AlertState_OPEN]; found {
			if aName, ok := grp.grpByEventURI[URI]; ok {
				return aName, true
			}
		}

		if grp, found := alertsByPolicy[monitoring.AlertState_ACKNOWLEDGED]; found {
			if aName, ok := grp.grpByEventURI[URI]; ok {
				return aName, true
			}
		}
	}
	return "", false
}

// AnyOutstandingAlertsByMessageAndRef returns true if there an outstanding alert by the given tenant, policy,
// message and object ref, otherwise false.
func (m *MemDb) AnyOutstandingAlertsByMessageAndRef(tenant, policyID, message string,
	objectRef *api.ObjectRef) (string, bool) {
	if objectRef == nil {
		return "", false
	}

	m.RLock()
	defer m.RUnlock()

	key := fmt.Sprintf("%s.%s", tenant, policyID)
	evtMessageObjRefKey := fmt.Sprintf("%s.%s", message, objectRef.String())

	if alertsByPolicy, found := m.alertsByPolicy[key]; found {
		if grp, found := alertsByPolicy[monitoring.AlertState_OPEN]; found {
			if aName, ok := grp.grpByEventMessageAndObjectRef[evtMessageObjRefKey]; ok {
				return aName, true
			}
		}

		if grp, found := alertsByPolicy[monitoring.AlertState_ACKNOWLEDGED]; found {
			if aName, ok := grp.grpByEventMessageAndObjectRef[evtMessageObjRefKey]; ok {
				return aName, true
			}
		}
	}
	return "", false
}

// AddOrUpdateAlertToGrps adds the given alert to cache groups.
func (m *MemDb) AddOrUpdateAlertToGrps(alert *monitoring.Alert) {
	m.Lock()
	defer m.Unlock()

	alertState := monitoring.AlertState(monitoring.AlertState_vvalue[alert.Spec.State])
	key := fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)

	var evtMessageObjRefKey string
	if alert.Status.ObjectRef != nil {
		message := alert.Status.Message
		if strings.Contains(message, globals.DiskHighThresholdMessage) {
			message = globals.DiskHighThresholdMessage
		}

		evtMessageObjRefKey = fmt.Sprintf("%s.%s", message, alert.Status.ObjectRef.String())
	}

	alertsByPolicy, found := m.alertsByPolicy[key]
	if !found {
		m.alertsByPolicy[key] = make(map[monitoring.AlertState]*alertsGroup)
	}

	if alertState != monitoring.AlertState_RESOLVED {
		// find if the state already exists
		aGrp, found := alertsByPolicy[alertState]
		if !found {
			aGrp = &alertsGroup{grpByEventURI: make(map[string]string), grpByEventMessageAndObjectRef: make(map[string]string)}
			m.alertsByPolicy[key][alertState] = aGrp
		}
		// group by events URI
		aGrp.grpByEventURI[alert.Status.EventURI] = alert.Name

		// group by event message and object ref
		if alert.Status.ObjectRef != nil {
			aGrp.grpByEventMessageAndObjectRef[evtMessageObjRefKey] = alert.Name
		}
	}

	// remove it from other groups if it exists
	for state, aGrp := range alertsByPolicy {
		if alertState != state {
			// remove it from the other alert state groups
			delete(aGrp.grpByEventURI, alert.Status.EventURI)

			if alert.Status.ObjectRef != nil {
				delete(aGrp.grpByEventMessageAndObjectRef, evtMessageObjRefKey)
			}

			if len(aGrp.grpByEventURI) == 0 && len(aGrp.grpByEventMessageAndObjectRef) == 0 {
				delete(m.alertsByPolicy[key], state)
			}
		}
	}

	// delete the policy from cache if there are no entries for it
	if len(m.alertsByPolicy[key]) == 0 {
		delete(m.alertsByPolicy, key)
		return
	}
}

// DeleteAlertFromGrps deletes the alert from the cached groups.
func (m *MemDb) DeleteAlertFromGrps(alert *monitoring.Alert) {
	m.Lock()
	defer m.Unlock()

	key := fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)
	alertState := monitoring.AlertState(monitoring.AlertState_vvalue[alert.Spec.State])
	if alertsByPolicy, found := m.alertsByPolicy[key]; found {
		grp, found := alertsByPolicy[alertState]
		if found {
			delete(grp.grpByEventURI, alert.Status.EventURI)

			if alert.Status.ObjectRef != nil {
				message := alert.Status.Message
				if strings.Contains(message, globals.DiskHighThresholdMessage) {
					message = globals.DiskHighThresholdMessage
				}

				evtMessageObjRefKey := fmt.Sprintf("%s.%s", message, alert.Status.ObjectRef.String())
				delete(grp.grpByEventMessageAndObjectRef, evtMessageObjRefKey)
			}

			if len(grp.grpByEventURI) == 0 && len(grp.grpByEventMessageAndObjectRef) == 0 {
				delete(m.alertsByPolicy, key)
			}
		}
	}
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

// GetAlertPolicies returns the list of alert policies that matches all the given filters
func (m *MemDb) GetAlertPolicies(filters ...FilterFn) []*monitoring.AlertPolicy {
	var alertPolicies []*monitoring.AlertPolicy
	for _, policy := range m.ListObjects("AlertPolicy", nil) {
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

// GetAlertDestination returns the alert destination matching the given name
func (m *MemDb) GetAlertDestination(name string) *monitoring.AlertDestination {
	for _, alertD := range m.ListObjects("AlertDestination", nil) {
		ad := alertD.(*monitoring.AlertDestination)
		if ad.GetName() == name {
			return ad
		}
	}

	return nil
}

// WatchAlertDestinations returns the watcher to watch for alert destination events
func (m *MemDb) WatchAlertDestinations() *memdb.Watcher {
	watcher := memdb.Watcher{Name: "alert-dest"}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	m.WatchObjects("AlertDestination", &watcher)
	return &watcher
}

// StopWatchAlertDestinations stops the alert destination watcher
func (m *MemDb) StopWatchAlertDestinations(watcher *memdb.Watcher) {
	m.StopWatchObjects("AlertDestination", watcher)
}

// WatchVersion returns the watcher to watch for version events
func (m *MemDb) WatchVersion() *memdb.Watcher {
	watcher := memdb.Watcher{Name: "version"}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	m.WatchObjects("Version", &watcher)
	return &watcher
}

// StopWatchVersion stops the version watcher
func (m *MemDb) StopWatchVersion(watcher *memdb.Watcher) {
	m.StopWatchObjects("Version", watcher)
}

// WatchStatsAlertPolicy returns the watcher to watch for stats alert policy events
func (m *MemDb) WatchStatsAlertPolicy() *memdb.Watcher {
	watcher := memdb.Watcher{Name: "stats-alert-policy"}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	m.WatchObjects("StatsAlertPolicy", &watcher)
	return &watcher
}

// StopWatchStatsAlertPolicy stops the stats alert policy watcher
func (m *MemDb) StopWatchStatsAlertPolicy(watcher *memdb.Watcher) {
	m.StopWatchObjects("StatsAlertPolicy", watcher)
}

// NewMemDb creates a new mem DB
func NewMemDb() *MemDb {
	amemDb := &MemDb{
		Memdb:          memdb.NewMemdb(),
		alertsByPolicy: map[string]map[monitoring.AlertState]*alertsGroup{},
	}

	return amemDb
}
