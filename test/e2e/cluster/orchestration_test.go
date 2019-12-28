// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"fmt"
	"os/exec"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/orchestration"
)

const (
	vcIP     = "192.168.30.100:8989"
	username = "user"
	password = "pass"
	govc     = "docker exec vc /bin/bash -c 'export GOVC_URL=https://user:pass@192.168.30.100:8989/sdk && export GOVC_SIM_PID=1 && export GOVC_INSECURE=1 && /go/bin/govc "
)

func runGovcCmd(cmdStr string) {
	cmd := fmt.Sprintf("%s %s'", govc, cmdStr)
	c := exec.Command("bash", "-c", cmd)
	_, e := c.Output()
	if e != nil {
		return
	}
}

func clearExistingObjects() {
	// TODO ; use govmomi APIs for this
}

func createDC() {
	// TODO ; use govmomi APIs for this
}

func addHost() {
	// TODO ; use govmomi APIs for this
}

func deleteHost() {
	// TODO ; use govmomi APIs for this
}

func addVM() {
	// TODO ; use govmomi APIs for this
}

func deleteVM() {
	// TODO ; use govmomi APIs for this
}

func createOrchestrator(name, ip, user, password string) *orchestration.Orchestrator {
	return &orchestration.Orchestrator{
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		TypeMeta: api.TypeMeta{
			Kind: "Orchestrator",
		},
		Spec: orchestration.OrchestratorSpec{
			Type: "vcenter",
			URI:  ip,
			Credentials: &monitoring.ExternalCred{
				AuthType: "username-password",
				UserName: user,
				Password: password,
			},
		},
		Status: orchestration.OrchestratorStatus{
			Status: "unknown",
		},
	}
}

// run the tests
var _ = Describe("orchestration object tests", func() {

	Context("Orchestration tests", func() {
		BeforeEach(func() {
			clearExistingObjects()
			createDC()
			addHost()
			deleteHost()
			addVM()
			deleteVM()
		})

		// run tests
		It("Orchestrator pause-unpause", func() {

			ts.tu.LocalCommandOutput(fmt.Sprintf("docker pause vc"))
			orch := createOrchestrator("vcenter-pause-unpause", vcIP, username, password)
			_, err := ts.tu.APIClient.OrchestratorV1().Orchestrator().Create(ts.loggedInCtx, orch)
			Expect(err).Should(BeNil())
			ts.tu.LocalCommandOutput(fmt.Sprintf("docker unpause vc"))
			// VCHub retry interval is 3 seconds
			time.Sleep(5 * time.Second)
			Eventually(func() error {
				opts := api.ListWatchOptions{}
				objList, err := ts.tu.APIClient.OrchestratorV1().Orchestrator().List(ts.loggedInCtx, &opts)
				if err != nil {
					return err
				}

				if len(objList) == 0 {
					return fmt.Errorf("Orchestrator list is empty")
				}

				found := false
				for _, obj := range objList {
					if obj.Name == "vcenter-pause-unpause" {
						found = true
					}
				}

				if !found {
					return fmt.Errorf("Orchestrator object was not found")
				}

				return nil
			}, 10, 1).Should(BeNil())

		})

		It("Orchestration operations should succeed", func() {
			Eventually(func() error {
				orch := createOrchestrator("vcenter", vcIP, username, password)
				_, err := ts.tu.APIClient.OrchestratorV1().Orchestrator().Create(ts.loggedInCtx, orch)
				return err
			}, 10, 1).Should(BeNil())

			time.Sleep(5 * time.Second)
			opts := api.ListWatchOptions{LabelSelector: "vcenter.orch-name=vcenter"}

			_, err := ts.tu.APIClient.ClusterV1().Host().List(ts.loggedInCtx, &opts)
			Expect(err).Should(BeNil())
		})

		// cleanup
		AfterEach(func() {
			clearExistingObjects()
		})
	})
})
