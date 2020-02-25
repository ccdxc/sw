package memdb

import (
	"fmt"
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

// TestMemDb tests CRUD on mem DB
func TestMemDb(t *testing.T) {
	mDb := NewMemDb()

	// add some objects
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "infra", CreateAlphabetString(5), "Event", eventattrs.Severity_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", eventattrs.Severity_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", eventattrs.Severity_CRITICAL, "convert CRITICAL events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", eventattrs.Severity_WARN, "convert WARNING events to alerts", []*fields.Requirement{}, []string{}))

	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), monitoring.AlertState_OPEN, "test-alert1", nil, nil, nil))
	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), monitoring.AlertState_OPEN, "test-alert2", nil, nil, nil))

	mDb.AddObject(policygen.CreateSmartNIC("0014.2201.2345", cluster.DistributedServiceCardStatus_ADMITTED.String(), "esx-1",
		&cluster.DSCCondition{Type: cluster.DSCCondition_HEALTHY.String(), Status: cluster.ConditionStatus_FALSE.String()}))
	mDb.AddObject(policygen.CreateSmartNIC("0014.2201.2334", cluster.DistributedServiceCardStatus_ADMITTED.String(), "esx-2",
		&cluster.DSCCondition{Type: cluster.DSCCondition_HEALTHY.String(), Status: cluster.ConditionStatus_FALSE.String()}))

	objs := mDb.ListObjects("AlertPolicy", nil)
	Assert(t, len(objs) == 4, "invalid number of alert policies, expected: %v, got: %v", 4, len(objs))
	objs = mDb.ListObjects("Alert", nil)
	Assert(t, len(objs) == 2, "invalid number of alerts objects, expected: %v, got: %v", 2, len(objs))
	objs = mDb.ListObjects("DistributedServiceCard", nil)
	Assert(t, len(objs) == 2, "invalid number of Smart NIC objects, expected: %v, got: %v", 2, len(objs))

	// test update
	ap := policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", eventattrs.Severity_WARN, "convert WARNING events to alerts", []*fields.Requirement{}, []string{})
	AssertOk(t, mDb.AddObject(ap), "failed to add object to mem DB")
	ap.Spec.Enable = false
	AssertOk(t, mDb.UpdateObject(ap), "failed to update object to mem DB")
	objs = mDb.ListObjects("AlertPolicy", nil)
	Assert(t, len(objs) == 5, "invalid number of alert policies, expected: %v, got: %v", 5, len(objs))

	// test delete
	AssertOk(t, mDb.DeleteObject(ap), "failed to delete object from mem DB")
	objs = mDb.ListObjects("AlertPolicy", nil)
	Assert(t, len(objs) == 4, "invalid number of alert policies, expected: %v, got: %v", 5, len(objs))

	// test alert destination
	Assert(t, mDb.GetAlertDestination("dest-1") == nil, "expected nil but got alert destination")
	mDb.AddObject(policygen.CreateAlertDestinationObj(globals.DefaultTenant, "system", "dest-1",
		&monitoring.SyslogExport{
			Format: monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(),
		}))
	objs = mDb.ListObjects("AlertDestination", nil)
	Assert(t, len(objs) == 1, "invalid number of alert destinations, expected: %v, got: %v", 1, len(objs))
	Assert(t, mDb.GetAlertDestination("dest-1") != nil, "failed to get alert destination")

	_, found := mDb.AnyOutstandingAlertsByURI(globals.DefaultTenant, "p1", "dummy")
	Assert(t, !found, "expected no outstanding alerts but found")
	_, found = mDb.AnyOutstandingAlertsByMessageAndRef(globals.DefaultTenant, "p1", "dummy message", nil)
	Assert(t, !found, "expected no outstanding alerts but found")
	_, found = mDb.AnyOutstandingAlertsByMessageAndRef(globals.DefaultTenant, "p1", "dummy message", &api.ObjectRef{})
	Assert(t, !found, "expected no outstanding alerts but found")

	alert := policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5),
		monitoring.AlertState_OPEN, "test-alert1",
		&monitoring.AlertPolicy{ObjectMeta: api.ObjectMeta{Name: CreateAlphabetString(5)}},
		&evtsapi.Event{ObjectMeta: api.ObjectMeta{SelfLink: fmt.Sprintf("/events/v1/events/%s",
			uuid.NewV4().String())}, EventAttributes: evtsapi.EventAttributes{ObjectRef: &api.ObjectRef{Name: "test-node", Kind: "Node"}}},
		nil)

	// add new alert to the cache
	mDb.AddOrUpdateAlertToGrps(alert)
	_, found = mDb.AnyOutstandingAlertsByURI(globals.DefaultTenant, alert.Status.Reason.PolicyID, alert.Status.EventURI)
	Assert(t, found, "no outstanding alert found, but expected one")
	_, found = mDb.AnyOutstandingAlertsByMessageAndRef(globals.DefaultTenant, alert.Status.Reason.PolicyID, alert.Status.Message, alert.Status.ObjectRef)
	Assert(t, found, "no outstanding alert found, but expected one")
	Assert(t, len(mDb.alertsByPolicy) == 1, "expected 1 alert policy")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)]) == 1,
		"expected 1 state in cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN] != nil,
		"expected open state in cache")

	// update existing alert to the cache
	alert.Spec.State = monitoring.AlertState_ACKNOWLEDGED.String()
	mDb.AddOrUpdateAlertToGrps(alert)
	_, found = mDb.AnyOutstandingAlertsByURI(globals.DefaultTenant, alert.Status.Reason.PolicyID, alert.Status.EventURI)
	Assert(t, found, "no outstanding alert found, but expected one")
	_, found = mDb.AnyOutstandingAlertsByMessageAndRef(globals.DefaultTenant, alert.Status.Reason.PolicyID, alert.Status.Message, alert.Status.ObjectRef)
	Assert(t, found, "no outstanding alert found, but expected one")
	Assert(t, len(mDb.alertsByPolicy) == 1, "expected 1 alert policy")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)]) == 1,
		"expected 1 state in cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_ACKNOWLEDGED] != nil,
		"expected acknowledged state in cache")

	// resolve the alert and update cache
	alert.Spec.State = monitoring.AlertState_RESOLVED.String()
	mDb.AddOrUpdateAlertToGrps(alert)
	Assert(t, len(mDb.alertsByPolicy) == 0, "expected 0 alert policy")

	// delete the alert from cache
	mDb.DeleteAlertFromGrps(alert)
	_, found = mDb.AnyOutstandingAlertsByURI(globals.DefaultTenant, alert.Status.Reason.PolicyID, alert.Status.EventURI)
	Assert(t, !found, "no outstanding alert expected, but found one")
	_, found = mDb.AnyOutstandingAlertsByMessageAndRef(globals.DefaultTenant, alert.Status.Reason.PolicyID, alert.Status.Message, alert.Status.ObjectRef)
	Assert(t, !found, "no outstanding alert expected, but found one")
	Assert(t, len(mDb.alertsByPolicy) == 0, "expected 0 alert policy")

	// add the same alert again and delete
	alert.Spec.State = monitoring.AlertState_ACKNOWLEDGED.String()
	mDb.AddOrUpdateAlertToGrps(alert)
	Assert(t, len(mDb.alertsByPolicy) == 1, "expected 1 alert policy")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)]) == 1,
		"expected 1 state in cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_ACKNOWLEDGED] != nil,
		"expected acknowledged state in cache")
	mDb.DeleteAlertFromGrps(alert)
	Assert(t, len(mDb.alertsByPolicy) == 0, "expected 0 alert policy")

	// add 2 alerts
	alert.Spec.State = monitoring.AlertState_OPEN.String()
	mDb.AddOrUpdateAlertToGrps(alert)
	alert2 := policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5),
		monitoring.AlertState_OPEN, "test-alert2",
		&monitoring.AlertPolicy{ObjectMeta: api.ObjectMeta{Name: CreateAlphabetString(5)}},
		&evtsapi.Event{ObjectMeta: api.ObjectMeta{SelfLink: fmt.Sprintf("/events/v1/events/%s",
			uuid.NewV4().String())}, EventAttributes: evtsapi.EventAttributes{ObjectRef: &api.ObjectRef{Name: "test-node", Kind: "Node"}}},
		nil)
	mDb.AddOrUpdateAlertToGrps(alert2)
	Assert(t, len(mDb.alertsByPolicy) == 1, "expected 1 alert policy")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)]) == 1,
		"expected 1 state in cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN] != nil,
		"expected open state in cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.
		AlertState_ACKNOWLEDGED] == nil, "did not expect acknowledged state in the cache")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN].grpByEventMessageAndObjectRef) == 2,
		"expected 2 entries in the cache")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN].grpByEventURI) == 2,
		"expected 2 entries in the cache")

	// delete one of the alert and ensure the cache is updated
	mDb.DeleteAlertFromGrps(alert)
	Assert(t, len(mDb.alertsByPolicy) == 1, "expected 1 alert policy")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN] != nil,
		"expected open state in cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.
		AlertState_ACKNOWLEDGED] == nil, "did not expect acknowledged state in the cache")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN].grpByEventMessageAndObjectRef) == 1,
		"expected 1 entries in the cache")
	Assert(t, len(mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN].grpByEventURI) == 1,
		"expected 1 entries in the cache")

	// delete
	mDb.DeleteAlertFromGrps(alert2)
	Assert(t, len(mDb.alertsByPolicy) == 0, "expected 1 alert policy")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.AlertState_OPEN] == nil,
		"did not expect open state in the cache")
	Assert(t, mDb.alertsByPolicy[fmt.Sprintf("%s.%s", alert.Tenant, alert.Status.Reason.PolicyID)][monitoring.
		AlertState_ACKNOWLEDGED] == nil, "did not expect acknowledged state in the cache")
}

// TestGetAlertPolicies tests GetAlertPolicies(...) with various filters
func TestGetAlertPolicies(t *testing.T) {
	mDb := NewMemDb()

	// add some objects
	mDb.AddObject(policygen.CreateAlertPolicyObj("infra", "infra", CreateAlphabetString(5), "Event", eventattrs.Severity_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Node", eventattrs.Severity_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", eventattrs.Severity_CRITICAL, "convert CRITICAL events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", eventattrs.Severity_WARN, "convert WARNING events to alerts", []*fields.Requirement{}, []string{}))

	tests := []struct {
		filters             []FilterFn
		expNumAlertPolicies int
	}{
		{
			filters:             []FilterFn{},
			expNumAlertPolicies: 4,
		},
		{
			filters:             []FilterFn{WithTenantFilter("infra")},
			expNumAlertPolicies: 1,
		},
		{
			filters:             []FilterFn{WithTenantFilter(globals.DefaultTenant)},
			expNumAlertPolicies: 3,
		},
		{
			filters:             []FilterFn{WithTenantFilter(globals.DefaultTenant), WithResourceFilter("Node"), WithEnabledFilter(true)},
			expNumAlertPolicies: 1,
		},
		{
			filters:             []FilterFn{WithTenantFilter("invalid")},
			expNumAlertPolicies: 0,
		},
		{
			filters:             []FilterFn{WithEnabledFilter(false)},
			expNumAlertPolicies: 0,
		},
	}

	for _, test := range tests {
		objs := mDb.GetAlertPolicies(test.filters...)
		Assert(t, test.expNumAlertPolicies == len(objs), "invalid number of alert policies, expected: %v, got: %v", test.expNumAlertPolicies, len(objs))
	}

	// test with enable = false
	pol := policygen.CreateAlertPolicyObj("infra", "infra", CreateAlphabetString(5), "Event", eventattrs.Severity_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{})
	err := mDb.AddObject(pol)
	AssertOk(t, err, "failed to add object to mem DB, err: %v", err)
	pol.Spec.Enable = false
	err = mDb.UpdateObject(pol)
	AssertOk(t, err, "failed to update object to mem DB, err: %v", err)

	objs := mDb.GetAlertPolicies(WithEnabledFilter(false))
	Assert(t, len(objs) == 1, "invalid number of alert policies, expected: %v, got: %v", 1, len(objs))
}
