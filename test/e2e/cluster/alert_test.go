package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

var _ = Describe("alert test", func() {
	var (
		err              error
		serviceStoppedOn string
	)
	It("ServiceStopped events to INFO alerts", func() {
		validateCluster()

		// create alert policy to convert `ServiceStopped` events to INFO alerts
		alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "eventstoalerts", "Event", evtsapi.SeverityLevel_INFO,
			"convert `ServiceStopped` events to INFO alerts",
			[]*fields.Requirement{
				{Key: "type", Operator: "in", Values: []string{evtsapi.ServiceStopped}},
			}, []string{})

		// upload alert policy
		alertPolicy1, err = ts.tu.APIClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
		Expect(err).Should(BeNil())

		// stop pen-ntp daemon service
		podName := getRunningPod("pen-ntp")
		Expect(podName).ShouldNot(BeEmpty())
		ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl delete pod %s", podName))

		// ensure the respective alert got generated
		Eventually(func() bool {
			alerts, err := ts.tu.APIClient.MonitoringV1().Alert().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			if err != nil {
				return false
			}

			for _, alert := range alerts {
				if alert.Status.Reason.GetPolicyID() == alertPolicy1.GetName() &&
					strings.Contains(alert.Status.GetMessage(), fmt.Sprintf("pen-ntp stopped on %s", serviceStoppedOn)) {
					return true
				}
			}
			return false
		}, 30, 1).Should(BeTrue(), "could not find the expected alert")

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
	})
})

// returns the name of the service instance that is running on a READY state node
func getRunningPod(serviceName string) string {
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

	for _, pod := range kubeGetPodsOut.Items {
		if pod.Status.Phase == "Running" {
			// check if the node is in READY state
			getNodesOut := ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl get nodes  -l kubernetes.io/hostname=%s -o json", pod.Spec.NodeName))
			json.Unmarshal([]byte(getNodesOut), &kubeGetNodesOut)
			if len(kubeGetNodesOut.Items) == 0 {
				By(fmt.Sprintf("ts=%s no matching node found: {%s}", time.Now().String(), pod.Spec.NodeName))
				return ""
			}

			for _, cond := range kubeGetNodesOut.Items[0].Status.Conditions {
				if cond.Type == "Ready" && cond.Status == "True" {
					By(fmt.Sprintf("ts=%s selected pod {%s} running on {%s}", time.Now().String(), pod.Metadata.Name, pod.Spec.NodeName))
					return pod.Metadata.Name
				}
			}
		}
	}

	By(fmt.Sprintf("ts=%s no pod instance found for service {%s}", time.Now().String(), serviceName))
	return ""
}
