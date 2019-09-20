// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cluster

import (
	"context"
	"fmt"
	"strings"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	api "github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/apiclient"
)

var _ = Describe("negative tests for unsupported cluster operations", func() {
	Context("Unsupported Cluster ops test", func() {
		var (
			obj       api.ObjectMeta
			cl        *cmd.Cluster
			clusterIf cmd.ClusterV1ClusterInterface
			err       error
		)
		BeforeEach(func() {
			validateCluster()

			apiGwAddr := ts.tu.ClusterVIP + ":" + globals.APIGwRESTPort
			apiClient, err := apiclient.NewRestAPIClient(apiGwAddr)
			Expect(err).ShouldNot(HaveOccurred())
			clusterIf = apiClient.ClusterV1().Cluster()
			obj = api.ObjectMeta{Name: "testCluster"}
			cl, err = clusterIf.Get(ts.tu.MustGetLoggedInContext(context.Background()), &obj)
			Expect(err).ShouldNot(HaveOccurred())

		})

		It("Modify immutable fields in Cluster object", func() {
			// modifying immutable fields should throw an error
			By(fmt.Sprintf("CL IS: %+v\n", cl))

			Eventually(func() bool {
				mcl := *cl
				mcl.Spec.QuorumNodes = mcl.Spec.QuorumNodes[1:]
				_, err = clusterIf.Update(ts.loggedInCtx, &mcl)
				if err != nil && strings.Contains(err.Error(), "QuorumNodes") {
					return true
				}
				By(fmt.Sprintf("Unexpected error: %+v", err))
				return false
			}, 30, 1).Should(BeTrue(), "ApiServer did not return expected error")

			Eventually(func() bool {
				mcl := *cl
				mcl.Spec.VirtualIP = "0.0.0.0"
				_, err = clusterIf.Update(ts.loggedInCtx, &mcl)
				if err != nil && strings.Contains(err.Error(), "VirtualIP") {
					return true
				}
				By(fmt.Sprintf("Unexpected error: %+v", err))
				return false
			}, 30, 1).Should(BeTrue(), "ApiServer did not return expected error")

			// writing back unmodified immutable fields should succeed
			Eventually(func() error {
				_, err = clusterIf.Update(ts.loggedInCtx, cl)
				return err
			}, 30, 1).Should(BeNil(), "Valid update call failed unexpectedly ")
		})

		AfterEach(func() {
			validateCluster()
		})
	})
})
