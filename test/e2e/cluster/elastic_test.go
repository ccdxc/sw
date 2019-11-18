package cluster

import (
	"context"
	"fmt"
	"strings"
	"time"

	es "github.com/olivere/elastic"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	"github.com/pensando/sw/api"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
)

var _ = Describe("elastic cluster test", func() {
	var (
		esClient                 elastic.ESClient
		mr                       = mockresolver.New()
		resolverServiceInstances = map[string]*types.ServiceInstance{}
		searchAllQuery           = es.NewRawStringQuery(`{"match_all":{}}`)
		ctx                      = context.Background()
	)

	BeforeEach(func() {
		// e2e starts only quorum venice nodes
		for _, name := range ts.tu.QuorumNodes {
			si := &types.ServiceInstance{
				TypeMeta: api.TypeMeta{
					Kind: "ServiceInstance",
				},
				ObjectMeta: api.ObjectMeta{
					Name: name,
				},
				Service: globals.ElasticSearch,
				URL:     fmt.Sprintf("%s:%s", ts.tu.NameToIPMap[name], globals.ElasticsearchRESTPort),
			}
			resolverServiceInstances[name] = si
			mr.AddServiceInstance(si)
		}

		logConfig := log.GetDefaultConfig("elastic_cluster_test")
		Eventually(func() error {
			var err error
			esClient, err = elastic.NewAuthenticatedClient("", mr, log.GetNewLogger(logConfig))
			return err
		}, 60, 1).Should(BeNil(), "failed to initialize elastic client")
	})

	It("Elastic cluster health should be green/yellow", func() {
		var expectedState string

		// With RF factor 2 (excluding primary shard), we need atleast
		// 3 Venice nodes to have a "green" cluster. If the #nodes is less
		// than 3, then the cluster will be "yellow"
		if len(ts.tu.QuorumNodes) >= 3 {
			expectedState = "green"
		} else {
			expectedState = "yellow"
		}

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

	It("Elasticsearch node(s) failure test", func() {
		minMasters := ts.tu.NumQuorumNodes/2 + 1

		// elastic search should be in healthy (GREEN, 90.0 shards) state
		checkElasticClusterHealth(esClient, searchAllQuery)

		// remove node one after other to a tolerable limit. GREEN/YELLOW
		// it should be able to serve user requests.
		for i := 0; i < ts.tu.NumQuorumNodes-minMasters; i++ {
			log.Infof("removing %s from elasticsearch cluster", ts.tu.QuorumNodes[i])
			ts.tu.LocalCommandOutput(fmt.Sprintf("docker pause %s", ts.tu.QuorumNodes[i]))
			mr.DeleteServiceInstance(resolverServiceInstances[ts.tu.QuorumNodes[i]]) // update mock resolver

			Eventually(func() error {
				res, err := esClient.GetClusterHealth([]string{})
				if err != nil {
					return err
				}

				if res.NumberOfNodes != ts.tu.NumQuorumNodes-i-1 {
					return fmt.Errorf("expected #nodes: %v, got: %v", ts.tu.NumQuorumNodes-i-1, res.NumberOfNodes)
				}
				if res.NumberOfNodes != res.NumberOfDataNodes {
					return fmt.Errorf("expected #data_nodes: %v, got: %v", res.NumberOfNodes, res.NumberOfDataNodes)
				}
				if res.Status != "yellow" {
					return fmt.Errorf("expected `yellow` status, got: %v", res.Status)
				}

				resp, err := esClient.Search(context.Background(), "*external*", "", searchAllQuery, nil, 0, 10, "", true)
				if err != nil || resp == nil {
					return fmt.Errorf("search query failed")
				}

				log.Infof("successfully removed %s and elasticsearch cluster is still intact", ts.tu.QuorumNodes[i])
				return nil
			}, 120, 1).Should(BeNil(), "elastic cluster is not in expected state")
		}

		// removing any nodes at this point will make elasticsearch not reachable; it will fail to meet minMasters criteria
		nodeName := ts.tu.QuorumNodes[ts.tu.NumQuorumNodes-minMasters]
		log.Infof("removing %s from elasticsearch cluster", nodeName) // remove the next node
		ts.tu.LocalCommandOutput(fmt.Sprintf("docker pause %s", nodeName))
		mr.DeleteServiceInstance(resolverServiceInstances[nodeName]) // update mock resolver
		Eventually(func() bool {
			_, err := esClient.GetClusterHealth([]string{})
			if err != nil && strings.Contains(err.Error(), "no Elasticsearch node available") {
				return true
			}
			return false
		}, 120, 1).Should(BeTrue(), "expected elasticsearch cluster to be down")

		// add the nodes back in the same order
		for i := 0; i < ts.tu.NumQuorumNodes-minMasters; i++ { // add all the nodes back
			log.Infof("adding %s back to elasticsearch cluster", ts.tu.QuorumNodes[i])
			ts.tu.LocalCommandOutput(fmt.Sprintf("docker unpause %s", ts.tu.QuorumNodes[i]))
			mr.AddServiceInstance(resolverServiceInstances[ts.tu.QuorumNodes[i]])
		}
		log.Infof("adding %s back to elasticsearch cluster", nodeName)
		ts.tu.LocalCommandOutput(fmt.Sprintf("docker unpause %s", nodeName))
		mr.AddServiceInstance(resolverServiceInstances[nodeName])

		// elastic search should be in healthy (GREEN, 90.0 shards) state after adding all the nodes back
		checkElasticClusterHealth(esClient, searchAllQuery)
	})

	// tests data loss during node restart/deletion. It checks shard counts during node restarts.
	It("Elasticsearch data loss test", func() {
		if ts.tu.NumQuorumNodes <= 1 {
			Skip("cannot run data loss test with single node")
		}
		type shardsInfo struct {
			Total      int
			Successful int
			Failed     int
		}

		// elastic search should be in healthy (GREEN, 90.0 shards) state
		checkElasticClusterHealth(esClient, searchAllQuery)

		eventsIndex := elastic.GetIndex(globals.Events, globals.DefaultTenant)
		si := &shardsInfo{}

		// get index stats; retrieve the shards count
		Eventually(func() error {
			stats, err := esClient.GetIndicesStats(ctx, []string{eventsIndex})
			if err != nil {
				return err
			}

			if stats.Shards.Total != stats.Shards.Successful { // wait until all the shards become ready
				return fmt.Errorf("expected successful shards: %v, got : %v", stats.Shards.Total, stats.Shards.Successful)
			}
			si.Total = stats.Shards.Total
			si.Successful = stats.Shards.Successful
			si.Failed = stats.Shards.Failed

			return nil
		}, 60, 1).Should(BeNil(), "failed to get index stats")

		log.Infof("%v index shard stats: %+v", eventsIndex, si)

		// delete a node and ensure some shards are failed
		nodeName, err := getShardNode(esClient, eventsIndex)
		Expect(err).Should(BeNil())
		log.Infof("removing %s from elasticsearch cluster", nodeName)
		ts.tu.LocalCommandOutput(fmt.Sprintf("docker pause %s", nodeName))
		mr.DeleteServiceInstance(resolverServiceInstances[nodeName])
		Eventually(func() error {
			stats, err := esClient.GetIndicesStats(ctx, []string{eventsIndex})
			if err != nil {
				return err
			}

			if si.Total != stats.Shards.Total {
				return fmt.Errorf("expected total shards: %v, got: %v", si.Total, stats.Shards.Total)
			}
			// successful shards should be less than total as the node is removed
			if si.Successful <= stats.Shards.Successful {
				return fmt.Errorf("expected successful shards: <%v, got: %v", si.Successful, stats.Shards.Successful)
			}
			log.Infof("%v index shard stats after removing the node: %+v", eventsIndex, stats.Shards)

			return nil
		}, 150, 1).Should(BeNil(), "failed to find index stats")

		// add the node back again and check the index stats
		log.Infof("adding %s back to elasticsearch cluster", nodeName)
		ts.tu.LocalCommandOutput(fmt.Sprintf("docker unpause %s", nodeName))
		mr.AddServiceInstance(resolverServiceInstances[nodeName])
		Eventually(func() error {
			stats, err := esClient.GetIndicesStats(ctx, []string{eventsIndex})
			if err != nil {
				return err
			}

			// all the failed shards should be back to normal; 0 - failed
			if si.Total != stats.Shards.Total {
				return fmt.Errorf("expected total shards: %v, got: %v", si.Total, stats.Shards.Total)
			}
			if si.Successful != stats.Shards.Successful {
				return fmt.Errorf("expected successful shards: %v, got: %v", si.Total, stats.Shards.Successful)
			}
			if si.Failed != 0 {
				return fmt.Errorf("expected 0 failed shards")
			}
			log.Infof("%v index shard stats after adding back the node: %+v", eventsIndex, stats.Shards)

			return nil
		}, 120, 1).Should(BeNil(), "failed to find index stats")

		// elastic search should be in healthy (GREEN, 90.0 shards) state
		checkElasticClusterHealth(esClient, searchAllQuery)
	})

	AfterEach(func() {
		esClient.Close()
	})
})

// returns the node belonging to given index's shard
func getShardNode(esClient elastic.ESClient, index string) (string, error) {
	ctx := context.Background()
	searchShards, err := esClient.GetSearchShards(ctx, []string{index})
	if err != nil {
		return "", err
	}

	for _, shards := range searchShards.Shards {
		for _, shard := range shards {
			nodesInfo, err := esClient.GetNodesInfo(ctx, []string{shard.Node})
			if err != nil {
				return "", err
			}
			for _, nodeInfo := range nodesInfo.Nodes {
				return nodeInfo.Host, nil
			}
		}
	}

	return "", fmt.Errorf("failed to get node")
}

func checkElasticClusterHealth(esClient elastic.ESClient, query es.Query) {
	Eventually(func() error {
		res, err := esClient.GetClusterHealth([]string{})
		if err != nil {
			return err
		}

		if res.NumberOfNodes != ts.tu.NumQuorumNodes {
			return fmt.Errorf("expected #nodes: %v, got: %v", ts.tu.NumQuorumNodes, res.NumberOfNodes)
		}
		if res.NumberOfNodes != res.NumberOfDataNodes {
			return fmt.Errorf("expected #data_nodes: %v, got: %v", res.NumberOfNodes, res.NumberOfDataNodes)
		}
		// TODO:
		// Actually, ActiveShardsPercentAsNumber should be 100.0. Since some shards become unassigned/unallocated during
		// the test while removing maximum number of nodes from the cluster. We are temporarily checking for 90% availability here.
		// unassigned/unallocated shards should be re-tried manually by submitting a command to elastic.
		if ts.tu.NumQuorumNodes >= 3 && res.ActiveShardsPercentAsNumber < 90.0 {
			cmd := fmt.Sprintf("wget -O- --no-check-certificate --private-key=/var/lib/pensando/pki/shared/elastic"+
				"-client-auth/key.pem  --certificate=/var/lib/pensando/pki/shared/elastic-client-auth/cert."+
				"pem https://%s:9200/_cat/shards", ts.tu.VeniceNodeIPs[0])
			shardsOut := ts.tu.CommandOutput(ts.tu.VeniceNodeIPs[0], cmd)

			return fmt.Errorf("expected >= `90.0` active shards, got: %v, shards: %s",
				res.ActiveShardsPercentAsNumber, shardsOut)
		}
		if ts.tu.NumQuorumNodes >= 3 && !(res.Status == "green" || res.Status == "yellow") {
			return fmt.Errorf("expected `green/yellow` status, got: %v", res.Status)
		}

		resp, err := esClient.Search(context.Background(), "*external*", "", query, nil, 0, 10, "", true)
		if err != nil || resp == nil {
			return fmt.Errorf("search query failed")
		}
		return nil
	}, 120, 1).Should(BeNil(), "elastic cluster is not in healthy state")
}
