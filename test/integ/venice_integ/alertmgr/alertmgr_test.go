package alertmgr

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventattrs"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

func TestAlertmgrStates(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// start spyglass (backend service for events)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, nil, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start spyglass finder, err: %v", err)
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()
	ti.updateResolver(globals.Spyglass, fdrAddr)

	// API gateway
	apiGw, _, err := testutils.StartAPIGateway(":0", false,
		map[string]string{}, []string{"telemetry_query", "objstore", "tokenauth"}, []string{}, ti.mockResolver, ti.logger)
	AssertOk(t, err, "failed to start API gateway, err: %v", err)
	defer apiGw.Stop()

	// setup authn and get authz token
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(ti.apiServerAddr, true, nil, nil, userCreds, ti.logger)
	AssertOk(t, err, "failed to setup authN service, err: %v", err)
	defer testutils.CleanupAuth(ti.apiServerAddr, true, false, userCreds, ti.logger)
	//authzHeader, err := testutils.GetAuthorizationHeader(apiGwAddr, userCreds)
	AssertOk(t, err, "failed to get authZ header, err: %v", err)

	// make sure there are no outstanding alerts
	alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
		&api.ListWatchOptions{
			ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
	AssertOk(t, err, "failed to list alerts{ap1-*}, err: %v", err)
	Assert(t, len(alerts) == 0, "expected 0 outstanding alerts, got %d alerts", len(alerts))

	// add object based alert policy
	req := []*fields.Requirement{&fields.Requirement{Key: "status.version-mismatch", Operator: "equals", Values: []string{"true"}}}
	alertPolicy := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap1-%s", uuid.NewV4().String()), "DistributedServiceCard", eventattrs.Severity_INFO, "DSC mac check", req, []string{})
	alertPolicy, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy)
	AssertOk(t, err, "failed to add alert policy{ap1-*}, err: %v", err)

	time.Sleep(1 * time.Second)

	// add matching DSC object
	dsc := policygen.CreateSmartNIC("00ae.cd00.1142", "admitted", "naples-1", &cluster.DSCCondition{Type: "healthy", Status: "true", LastTransitionTime: ""})
	dsc.Status.VersionMismatch = true
	dsc, err = ti.apiClient.ClusterV1().DistributedServiceCard().Create(context.Background(), dsc)
	AssertOk(t, err, "failed to add dsc{ap1-*}, err: %v", err)

	// make sure alert got generated
	AssertEventually(t, func() (bool, interface{}) {
		alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
			&api.ListWatchOptions{})
		//ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant}})
		if err != nil {
			return false, err
		}

		// there should be just one, open alert
		if len(alerts) == 1 {
			alert := alerts[0]
			if alert.Spec.State == monitoring.AlertState_OPEN.String() && alert.Status.Resolved == nil {
				return true, nil
			}
		}

		return false, fmt.Sprintf("expected: 1 alert, obtained: %v", len(alerts))
	}, "did not receive the expected alert", string("5ms"), string("100ms"))

	// update DSC object such that it no longer matches the alert policy
	dsc.Status.VersionMismatch = false
	dsc, err = ti.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), dsc)
	AssertOk(t, err, "failed to update dsc{ap1-*}, err: %v", err)

	// Wait for debounce interval.
	time.Sleep(3 * time.Second)

	// make sure alert is resolved
	AssertEventually(t, func() (bool, interface{}) {
		alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
			&api.ListWatchOptions{})
		//ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant}})
		if err != nil {
			return false, err
		}

		// there should be just one, resolved alert
		if len(alerts) == 1 {
			alert := alerts[0]
			if alert.Spec.State == monitoring.AlertState_RESOLVED.String() && alert.Status.Resolved != nil {
				return true, nil
			}
		}

		return false, fmt.Sprintf("alert not resolved, number of alerts %v", len(alerts))
	}, "alert not in correct state", string("5ms"), string("100ms"))

	// update DSC object such that it matches the alert policy again
	dsc.Status.VersionMismatch = true
	dsc, err = ti.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), dsc)
	AssertOk(t, err, "failed to update dsc{ap1-*}, err: %v", err)

	// delete dsc before exiting
	defer func() {
		dsc.Spec.Admit = false
		dsc.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_PENDING.String()
		dsc, err := ti.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), dsc)
		AssertOk(t, err, "failed to update dsc{ap1-*}, err: %v", err)
		ti.apiClient.ClusterV1().DistributedServiceCard().Delete(context.Background(), dsc.GetObjectMeta())
	}()

	// make sure alert is reopened
	AssertEventually(t, func() (bool, interface{}) {
		alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
			&api.ListWatchOptions{})
		//ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant}})
		if err != nil {

			return false, err
		}

		// there should be just one, open alert
		if len(alerts) == 1 {
			alert := alerts[0]
			if alert.Spec.State == monitoring.AlertState_OPEN.String() && alert.Status.Resolved == nil {
				return true, nil
			}
		}

		return false, fmt.Sprintf("expected alert in open state, number of alerts: %v", len(alerts))
	}, "did not receive the expected alert", string("5ms"), string("100ms"))

	// delete alert policy
	ti.apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy.GetObjectMeta())

	// make sure alert is deleted
	AssertEventually(t, func() (bool, interface{}) {
		alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
			&api.ListWatchOptions{})
		//ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant}})
		if err != nil {
			return false, err
		}

		// there should be zero alerts
		if len(alerts) == 0 {
			return true, nil
		}

		return false, fmt.Sprintf("expected 0 alerts, received %v alerts", len(alerts))
	}, "received alert(s) when none expected", string("5ms"), string("100ms"))
}

func TestAlertmgrObjDelete(t *testing.T) {
	ti := tInfo{}
	AssertOk(t, ti.setup(t), "failed to setup test")
	defer ti.teardown()

	// start spyglass (backend service for events)
	fdrTemp, fdrAddr, err := testutils.StartSpyglass("finder", "", ti.mockResolver, nil, ti.logger, ti.esClient)
	AssertOk(t, err, "failed to start spyglass finder, err: %v", err)
	fdr := fdrTemp.(finder.Interface)
	defer fdr.Stop()
	ti.updateResolver(globals.Spyglass, fdrAddr)

	// API gateway
	apiGw, _, err := testutils.StartAPIGateway(":0", false,
		map[string]string{}, []string{"telemetry_query", "objstore", "tokenauth"}, []string{}, ti.mockResolver, ti.logger)
	AssertOk(t, err, "failed to start API gateway, err: %v", err)
	defer apiGw.Stop()

	// setup authn and get authz token
	userCreds := &auth.PasswordCredential{Username: testutils.TestLocalUser, Password: testutils.TestLocalPassword, Tenant: testutils.TestTenant}
	err = testutils.SetupAuth(ti.apiServerAddr, true, nil, nil, userCreds, ti.logger)
	AssertOk(t, err, "failed to setup authN service, err: %v", err)
	defer testutils.CleanupAuth(ti.apiServerAddr, true, false, userCreds, ti.logger)
	//authzHeader, err := testutils.GetAuthorizationHeader(apiGwAddr, userCreds)
	AssertOk(t, err, "failed to get authZ header, err: %v", err)

	// add object based alert policy
	req := []*fields.Requirement{&fields.Requirement{Key: "status.version-mismatch", Operator: "equals", Values: []string{"true"}}}
	alertPolicy := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ap1-%s", uuid.NewV4().String()), "DistributedServiceCard", eventattrs.Severity_INFO, "DSC mac check", req, []string{})
	alertPolicy, err = ti.apiClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy)
	AssertOk(t, err, "failed to add alert policy{ap1-*}, err: %v", err)

	defer ti.apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy.GetObjectMeta())

	time.Sleep(1 * time.Second)

	// add matching DSC object
	dsc := policygen.CreateSmartNIC("00ae.cd00.1142", "admitted", "naples-1", &cluster.DSCCondition{Type: "healthy", Status: "true", LastTransitionTime: ""})
	dsc.Status.VersionMismatch = true
	dsc, err = ti.apiClient.ClusterV1().DistributedServiceCard().Create(context.Background(), dsc)
	AssertOk(t, err, "failed to add dsc{ap1-*}, err: %v", err)

	// make sure alert got generated
	AssertEventually(t, func() (bool, interface{}) {
		alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
			&api.ListWatchOptions{})
		//ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant}})
		if err != nil {
			return false, err
		}

		// there should be just one, open alert
		if len(alerts) == 1 {
			alert := alerts[0]
			if alert.Spec.State == monitoring.AlertState_OPEN.String() && alert.Status.Resolved == nil {
				return true, nil
			}
		}

		return false, fmt.Sprintf("expected: 1 alert, obtained: %v", len(alerts))
	}, "did not receive the expected alert", string("5ms"), string("100ms"))

	// delete DSC object
	dsc.Spec.Admit = false
	dsc.Status.AdmissionPhase = cluster.DistributedServiceCardStatus_PENDING.String()
	dsc, err = ti.apiClient.ClusterV1().DistributedServiceCard().Update(context.Background(), dsc)
	AssertOk(t, err, "failed to delete dsc{ap1-*}, err: %v", err)
	ti.apiClient.ClusterV1().DistributedServiceCard().Delete(context.Background(), dsc.GetObjectMeta())

	// make sure alert is deleted
	AssertEventually(t, func() (bool, interface{}) {
		alerts, err := ti.apiClient.MonitoringV1().Alert().List(context.Background(),
			&api.ListWatchOptions{})
		//ObjectMeta:    api.ObjectMeta{Tenant: globals.DefaultTenant}})
		if err != nil {
			return false, err
		}

		// there should be zero alerts
		if len(alerts) == 0 {
			return true, nil
		}

		return false, fmt.Sprintf("expected 0 alerts, received %v alerts", len(alerts))
	}, "received alert(s) when none expected", string("5ms"), string("100ms"))
}
