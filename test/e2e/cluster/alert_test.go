package cluster

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

var _ = Describe("alert test", func() {
	var (
		err              error
		serviceStoppedOn string
	)
	It("ServiceStopped events to INFO alerts", func() {
		// create alert policy to convert `ServiceStopped` events to INFO alerts
		alertPolicy1 := policygen.CreateAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, "eventstoalerts", "Event", evtsapi.SeverityLevel_INFO, "convert `ServiceStopped` events to INFO alerts", []*fields.Requirement{
			&fields.Requirement{Key: "type", Operator: "In", Values: []string{evtsapi.ServiceStopped}},
		})

		// upload alert policy
		alertPolicy1, err = ts.tu.APIClient.MonitoringV1().AlertPolicy().Create(context.Background(), alertPolicy1)
		Expect(err).Should(BeNil())

		// stop pen-ntp daemon service
		Eventually(func() bool {
			out := ts.tu.LocalCommandOutput("kubectl get pods  -l name=pen-ntp -o json")
			var kubeOut struct {
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
			json.Unmarshal([]byte(out), &kubeOut)
			for _, i := range kubeOut.Items {
				if i.Status.Phase == "Running" {
					log.Infof("deleting pod %s", i.Metadata.Name)
					serviceStoppedOn = i.Spec.NodeName
					ts.tu.LocalCommandOutput(fmt.Sprintf("kubectl delete pod %s", i.Metadata.Name))
					return true
				}
			}
			return false
		}, 95, 1).Should(BeTrue(), "pen-ntp should be running")

		// ensure the respective alert got generated
		Eventually(func() bool {
			alerts, err := ts.tu.APIClient.MonitoringV1().Alert().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
			if err != nil {
				return false
			}

			for _, alert := range alerts {
				if alert.Status.Reason.GetPolicyID() == alertPolicy1.GetUUID() &&
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
