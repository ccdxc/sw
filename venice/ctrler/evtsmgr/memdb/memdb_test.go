package memdb

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

// TestMemDb tests CRUD on mem DB
func TestMemDb(t *testing.T) {
	mDb := NewMemDb()

	// add some objects
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "infra", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_CRITICAL, "convert CRITICAL events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_WARNING, "convert WARNING events to alerts", []*fields.Requirement{}, []string{}))

	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert1", nil, nil, nil))
	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert2", nil, nil, nil))

	mDb.AddObject(policygen.CreateSmartNIC("00-14-22-01-23-45", cluster.SmartNICSpec_ADMITTED.String(), "esx-1",
		&cluster.SmartNICCondition{Type: cluster.SmartNICCondition_HEALTHY.String(), Status: cluster.ConditionStatus_FALSE.String()}))
	mDb.AddObject(policygen.CreateSmartNIC("00-14-22-01-23-34", cluster.SmartNICSpec_ADMITTED.String(), "esx-2",
		&cluster.SmartNICCondition{Type: cluster.SmartNICCondition_HEALTHY.String(), Status: cluster.ConditionStatus_FALSE.String()}))

	objs := mDb.ListObjects("AlertPolicy")
	Assert(t, len(objs) == 4, "invalid number of alert policies, expected: %v, got: %v", 4, len(objs))
	objs = mDb.ListObjects("Alert")
	Assert(t, len(objs) == 2, "invalid number of alerts objects, expected: %v, got: %v", 2, len(objs))
	objs = mDb.ListObjects("SmartNIC")
	Assert(t, len(objs) == 2, "invalid number of Smart NIC objects, expected: %v, got: %v", 2, len(objs))

	// test update
	ap := policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_WARNING, "convert WARNING events to alerts", []*fields.Requirement{}, []string{})
	AssertOk(t, mDb.AddObject(ap), "failed to add object to mem DB")
	ap.Spec.Enable = false
	AssertOk(t, mDb.UpdateObject(ap), "failed to update object to mem DB")
	objs = mDb.ListObjects("AlertPolicy")
	Assert(t, len(objs) == 5, "invalid number of alert policies, expected: %v, got: %v", 5, len(objs))

	// test delete
	AssertOk(t, mDb.DeleteObject(ap), "failed to delete object from mem DB")
	objs = mDb.ListObjects("AlertPolicy")
	Assert(t, len(objs) == 4, "invalid number of alert policies, expected: %v, got: %v", 5, len(objs))
}

// TestGetAlertPolicies tests GetAlertPolicies(...) with various filters
func TestGetAlertPolicies(t *testing.T) {
	mDb := NewMemDb()

	// add some objects
	mDb.AddObject(policygen.CreateAlertPolicyObj("infra", "infra", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Node", evtsapi.SeverityLevel_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_CRITICAL, "convert CRITICAL events to alerts", []*fields.Requirement{}, []string{}))
	mDb.AddObject(policygen.CreateAlertPolicyObj(globals.DefaultTenant, "system", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_WARNING, "convert WARNING events to alerts", []*fields.Requirement{}, []string{}))

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
		// this should have no effect the below filters are applicable only for alert object now
		{
			filters:             []FilterFn{WithObjectRefFilter(nil)},
			expNumAlertPolicies: 0,
		},
		{
			filters:             []FilterFn{WithAlertStateFilter("n/a")},
			expNumAlertPolicies: 0,
		},
		{
			filters:             []FilterFn{WithAlertPolicyIDFilter("n/a")},
			expNumAlertPolicies: 0,
		},
	}

	for _, test := range tests {
		objs := mDb.GetAlertPolicies(test.filters...)
		Assert(t, test.expNumAlertPolicies == len(objs), "invalid number of alert policies, expected: %v, got: %v", test.expNumAlertPolicies, len(objs))
	}

	// test with enable = false
	pol := policygen.CreateAlertPolicyObj("infra", "infra", CreateAlphabetString(5), "Event", evtsapi.SeverityLevel_INFO, "convert INFO events to alerts", []*fields.Requirement{}, []string{})
	err := mDb.AddObject(pol)
	AssertOk(t, err, "failed to add object to mem DB, err: %v", err)
	pol.Spec.Enable = false
	err = mDb.UpdateObject(pol)
	AssertOk(t, err, "failed to update object to mem DB, err: %v", err)

	objs := mDb.GetAlertPolicies(WithEnabledFilter(false))
	Assert(t, len(objs) == 1, "invalid number of alert policies, expected: %v, got: %v", 1, len(objs))
}

// TestGetAlerts tests GetAlerts(...) with various filters
func TestGetAlerts(t *testing.T) {
	mDb := NewMemDb()

	// add some objects
	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert1", nil, nil, nil))
	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert1",
		&monitoring.AlertPolicy{
			ObjectMeta: api.ObjectMeta{
				UUID: "policy1",
			},
		},
		&evtsapi.Event{
			EventAttributes: evtsapi.EventAttributes{
				ObjectRef: &api.ObjectRef{
					Kind: "Node",
					Name: "node1",
				},
			},
		}, nil))
	mDb.AddObject(policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert1", nil,
		&evtsapi.Event{
			EventAttributes: evtsapi.EventAttributes{
				ObjectRef: &api.ObjectRef{
					Kind: "Node",
					Name: "node1",
				},
			},
		}, nil))
	mDb.AddObject(policygen.CreateAlertObj("infra", globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert1", nil, nil, nil))
	tests := []struct {
		filters      []FilterFn
		expNumAlerts int
	}{
		{
			filters:      []FilterFn{},
			expNumAlerts: 4,
		},
		{
			filters:      []FilterFn{WithObjectRefFilter(nil)},
			expNumAlerts: 2,
		},
		{
			filters:      []FilterFn{WithObjectRefFilter(&api.ObjectRef{})},
			expNumAlerts: 0,
		},
		{
			filters:      []FilterFn{WithObjectRefFilter(&api.ObjectRef{Kind: "Node", Name: "node1"})},
			expNumAlerts: 2,
		},
		{
			filters:      []FilterFn{WithObjectRefFilter(&api.ObjectRef{Kind: "Node", Name: "node1"}), WithAlertPolicyIDFilter("policy1")},
			expNumAlerts: 1,
		},
		{
			filters:      []FilterFn{WithTenantFilter("infra"), WithAlertPolicyIDFilter("")},
			expNumAlerts: 1,
		},
		{
			filters:      []FilterFn{WithTenantFilter("infra"), WithAlertPolicyIDFilter(""), WithAlertStateFilter("INFO")},
			expNumAlerts: 1,
		},
		{
			filters:      []FilterFn{WithObjectRefFilter(&api.ObjectRef{Kind: "Node", Name: "node1"}), WithAlertPolicyIDFilter("invalid")},
			expNumAlerts: 0,
		},
		{
			filters:      []FilterFn{WithTenantFilter("invalid")},
			expNumAlerts: 0,
		},
		{
			filters:      []FilterFn{WithAlertPolicyIDFilter("invalid")},
			expNumAlerts: 0,
		},
		// this should have no effect the below filters are applicable only for alert policy object now
		{
			filters:      []FilterFn{WithEnabledFilter(false)},
			expNumAlerts: 0,
		},
		{
			filters:      []FilterFn{WithResourceFilter("n/a")},
			expNumAlerts: 0,
		},
	}

	for i := range tests {
		objs := mDb.GetAlerts(tests[i].filters...)
		Assert(t, tests[i].expNumAlerts == len(objs), "invalid number of alerts for tc#%v, expected: %v, got: %v", i, tests[i].expNumAlerts, len(objs))
	}

	a := policygen.CreateAlertObj("infra", globals.DefaultNamespace, CreateAlphabetString(5), evtsapi.SeverityLevel_name[int32(evtsapi.SeverityLevel_INFO)], "test-alert1", nil, nil, nil)
	AssertOk(t, mDb.AddObject(a), "failed to add object to mem DB")
	objs := mDb.GetAlerts(WithAlertStateFilter("RESOLVED"))
	Assert(t, len(objs) == 0, "invalid number of alerts, expected: %v, got: %v", 0, len(objs))

	a.Spec.State = "RESOLVED"
	AssertOk(t, mDb.UpdateObject(a), "failed to update object to mem DB")
	objs = mDb.GetAlerts(WithAlertStateFilter("RESOLVED"))
	Assert(t, len(objs) == 1, "invalid number of alerts, expected: %v, got: %v", 1, len(objs))
}
