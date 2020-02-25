package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

var _ = Describe("alert test", func() {
	var err error
	It("ServiceStopped events to INFO alerts", func() {

		// create alert policy to convert `ServiceStopped` events to INFO alerts
		alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "eventstoalerts", "Event", eventattrs.Severity_INFO,
			"convert `ServiceStopped` events to INFO alerts",
			[]*fields.Requirement{
				{Key: "type", Operator: "in", Values: []string{eventtypes.EventType_name[int32(eventtypes.SERVICE_STOPPED)]}},
			}, []string{})

		// upload alert policy
		alertPolicy1, err = ts.tu.APIClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
		Expect(err).Should(BeNil())
		defer ts.tu.APIClient.MonitoringV1().AlertPolicy().Delete(context.Background(), alertPolicy1.GetObjectMeta())

		// stop pen-citadel daemon service
		podName, serviceStoppedOn := getRunningPod("pen-citadel")
		if utils.IsEmpty(podName) {
			Skip("no running pod found for service {pen-citadel} on a READY node")
		}
		ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl delete pod %s", podName))

		// wait for the pod to be running again and delete it (this should increase total-hits on the existing alert)
		Eventually(func() bool {
			out := ts.tu.LocalCommandOutput(fmt.Sprintf(
				"kubectl get pods  --field-selector=status.phase=Running,spec.nodeName=%s | grep pen-citadel | awk '{print $1}'", serviceStoppedOn))
			if utils.IsEmpty(out) {
				return false
			}
			ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl delete pod %s", strings.TrimSpace(out)))
			return true
		}, 60, 1).Should(BeTrue(), "could not find the running pod")

		// ensure the respective alert got generated
		var alertObjMeta api.ObjectMeta
		Eventually(func() bool {
			alerts, err := ts.tu.APIClient.MonitoringV1().Alert().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			if err != nil {
				return false
			}

			for _, alert := range alerts {
				if alert.Status.Reason.GetPolicyID() == fmt.Sprintf("%s/%s", alertPolicy1.GetName(), alertPolicy1.GetUUID()) &&
					strings.Contains(alert.Status.GetMessage(), fmt.Sprintf("pen-citadel stopped on %s", serviceStoppedOn)) {
					alertObjMeta = alert.ObjectMeta
					return true
				}
			}
			return false
		}, 90, 1).Should(BeTrue(), "could not find the expected alert")

		// ensure alert policy status is updated
		Eventually(func() error {
			ap, err := ts.tu.APIClient.MonitoringV1().AlertPolicy().Get(context.Background(), &api.ObjectMeta{Tenant: globals.DefaultTenant, Name: alertPolicy1.GetObjectMeta().GetName()})
			if err != nil {
				return err
			}

			if ap.Status.GetTotalHits() < 1 {
				return fmt.Errorf("total hits expected: >=%v, obtained: %v", 1, ap.Status.GetTotalHits())
			}
			if ap.Status.GetOpenAlerts() < 1 {
				return fmt.Errorf("open alerts expected: >=%v, obtained: %v", 1, ap.Status.GetOpenAlerts())
			}
			return nil
		}, 90, 1).Should(BeNil(), "could not find the expected alert policy status")

		// resolve the alert
		Eventually(func() error {
			alert, err := ts.tu.APIClient.MonitoringV1().Alert().Get(context.Background(), &alertObjMeta)
			if err != nil {
				return err
			}

			// use REST client to resolve an alert
			alert.Spec.State = monitoring.AlertState_RESOLVED.String()
			if _, err = ts.restSvc.MonitoringV1().Alert().Update(ts.loggedInCtx, alert); err != nil {
				return err
			}

			return nil
		}, 90, 1).Should(BeNil(), "could not resolve the alert")

		// invoke GC on demand and ensure the alert gets deleted
		Eventually(func() bool {
			out := ts.tu.CommandOutput(ts.tu.VeniceNodeIPs[0], fmt.Sprintf("curl -sS http://127.0.0.1:%s/gcalerts", globals.EvtsMgrRESTPort))
			Expect(strings.TrimSpace(out) == "{}").Should(BeTrue())

			if _, err := ts.tu.APIClient.MonitoringV1().Alert().Get(context.Background(), &alertObjMeta); err != nil {
				if errStatus, _ := status.FromError(err); errStatus.Code() == codes.NotFound {
					return true
				}
			}
			return false
		}, 90, 1).Should(BeTrue(), "alert did not get deleted")
	})
})

// returns the name of the service instance that is running on a READY state node
func getRunningPod(serviceName string) (string, string) {
	var kubeGetPodsOut struct {
		Items []struct {
			Metadata struct {
				Name string
			}
			Spec struct {
				NodeName string
			}
			Status struct {
				Phase string
			}
		}
	}

	var kubeGetNodesOut struct {
		Items []struct {
			Metadata struct {
				Name string
			}
			Status struct {
				Conditions []struct {
					Type   string
					Status string
				}
			}
		}
	}

	getPodsOut := ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get pods  -l name=%s -o json", serviceName))
	json.Unmarshal([]byte(getPodsOut), &kubeGetPodsOut)
	if len(kubeGetPodsOut.Items) == 0 {
		By(fmt.Sprintf("ts=%s no pods found for service {%s}", time.Now().String(), serviceName))
	}

	for _, pod := range kubeGetPodsOut.Items {
		if pod.Status.Phase == "Running" {
			// check if the node is in READY state
			getNodesOut := ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get nodes  -l kubernetes.io/hostname=%s -o json", pod.Spec.NodeName))
			json.Unmarshal([]byte(getNodesOut), &kubeGetNodesOut)
			if len(kubeGetNodesOut.Items) == 0 {
				By(fmt.Sprintf("ts=%s no matching node found: {%s}", time.Now().String(), pod.Spec.NodeName))
				return "", ""
			}

			for _, cond := range kubeGetNodesOut.Items[0].Status.Conditions {
				if cond.Type == "Ready" && cond.Status == "True" {
					By(fmt.Sprintf("ts=%s selected pod {%s} running on {%s}", time.Now().String(), pod.Metadata.Name, pod.Spec.NodeName))
					return pod.Metadata.Name, pod.Spec.NodeName
				}
			}
			By(fmt.Sprintf("ts=%s found a pod instance for service {%s} on the node {%s} but it is not READY", time.Now().String(), pod.Metadata.Name, pod.Spec.NodeName))
		}
	}

	By(fmt.Sprintf("ts=%s no pod instance found for service {%s} on a READY state node", time.Now().String(), serviceName))
	return "", ""
}
