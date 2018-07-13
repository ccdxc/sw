package cluster

import (
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("Elastic cluster test", func() {
	var esClient elastic.ESClient
	var expectedState string

	BeforeEach(func() {
		esAddr := fmt.Sprintf("%s:%s", ts.tu.FirstVeniceIP, globals.ElasticsearchRESTPort)
		logConfig := log.GetDefaultConfig("elastic_cluster_test")
		Eventually(func() error {
			var err error
			esClient, err = elastic.NewClient(esAddr, nil, log.GetNewLogger(logConfig))
			return err
		}, 30, 1).Should(BeNil(), "failed to initialize elastic client")

		// With RF factor 2 (excluding primary shard), we need atleast
		// 3 Venice nodes to have a "green" cluster. If the #nodes is less
		// than 3, then the cluster will be "yellow"
		if len(ts.tu.QuorumNodes) >= 3 {
			expectedState = "green"
		} else {
			expectedState = "yellow"
		}
	})

	It("Elastic cluster health should be green/yellow", func() {

		// Validate elastic cluster health
		Eventually(func() error {
			res, err := esClient.GetClusterHealth([]string{})
			if err != nil {
				return err
			}

			if res == nil {
				return fmt.Errorf("expected result to be valid; got: %v", res)
			}

			if res.Status != expectedState {
				return fmt.Errorf("expected health: %s got: %v", expectedState, res.Status)
			}
			By(fmt.Sprintf("ts:%s Elastic cluster status is: %s", time.Now().String(), res.Status))

			if res.NumberOfNodes != len(ts.tu.QuorumNodes) {
				return fmt.Errorf("expected node count: %d; got: %d", len(ts.tu.QuorumNodes), res.NumberOfNodes)
			}
			By(fmt.Sprintf("ts:%s Elastic node count is: %d", time.Now().String(), res.NumberOfNodes))

			if res.NumberOfDataNodes != len(ts.tu.QuorumNodes) {
				return fmt.Errorf("expected data node count: %d; got: %d", len(ts.tu.QuorumNodes), res.NumberOfDataNodes)
			}
			By(fmt.Sprintf("ts:%s Elastic data node count is: %d", time.Now().String(), res.NumberOfDataNodes))

			if len(ts.tu.QuorumNodes) >= 3 && res.ActiveShardsPercentAsNumber != 100.0 {
				return fmt.Errorf("expected active shard percentage: 100.0; got: %f", res.ActiveShardsPercentAsNumber)
			}
			By(fmt.Sprintf("ts:%s Elastic active shard percentage is: %f", time.Now().String(), res.ActiveShardsPercentAsNumber))

			return nil
		}, 60, 1).Should(BeNil(), "Elastic cluster health is not in expected state")

		// TODO : Add more tests for node health and indices health
	})

	AfterEach(func() {
		esClient.Close()
	})
})
