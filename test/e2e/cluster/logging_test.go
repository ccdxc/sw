package cluster

import (
	"context"
	"fmt"
	"time"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"

	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

type serviceInfo struct {
	daemonSet bool
}

var (
	indexName  = elastic.LogIndexPrefix
	indexType  = "doc"
	from       = int32(0)
	maxResults = int32(10)
	sortBy     = ""
)

var _ = Describe("Logging tests", func() {
	Context("Logs from all venice services should make it to elastic", func() {
		var (
			err      error
			esClient elastic.ESClient
			services map[string]serviceInfo
		)
		BeforeEach(func() {
			esAddr := fmt.Sprintf("%s:%s", ts.tu.FirstVeniceIP, globals.ElasticsearchRESTPort)
			logConfig := log.GetDefaultConfig("log-e2e-test")
			Eventually(func() error {
				esClient, err = elastic.NewClient(esAddr, nil, log.GetNewLogger(logConfig))
				return err
			}, 90, 1).Should(BeNil(), "failed to initialize elastic client")

			services = map[string]serviceInfo{
				globals.Cmd:       {true},
				globals.APIGw:     {true},
				globals.APIServer: {false},
				globals.VCHub:     {false},
				globals.EvtsMgr:   {true},
				globals.EvtsProxy: {true},
				globals.Collector: {false},
				globals.Spyglass:  {false},
				globals.Npm:       {false},
				globals.Tpm:       {false},
				globals.Tsm:       {false},
			}
		})

		It("Logs should be exported to elastic from services running on all venice nodes", func() {

			for service, info := range services {
				Eventually(func() error {
					str := fmt.Sprintf("%s is running", service)
					if info.daemonSet == true {
						// validate log from each node - for Daemon set
						for n := 1; n <= len(ts.tu.QuorumNodes); n++ {

							// verify for each venice node
							nodeName := fmt.Sprintf("node%d", n)
							query := es.NewBoolQuery().Must(es.NewMatchPhraseQuery("msg", str)).Must(es.NewTermQuery("beat.hostname", nodeName))
							result, err := esClient.Search(context.Background(),
								indexName,
								indexType,
								query,
								nil,
								from,
								maxResults,
								sortBy)
							if err != nil {
								return err
							}
							if result.TotalHits() == 0 {
								err = fmt.Errorf("No logs found for service %s on node %s", service, nodeName)
								return err
							}
							By(fmt.Sprintf("ts:%s Logs verified for Service: %s on Node: %s", time.Now().String(), service, nodeName))
						}
					} else {

						// Singleton service, use wildcard node suffix
						nodePattern := fmt.Sprintf("node.*")
						query := es.NewBoolQuery().Must(es.NewMatchPhraseQuery("msg", str)).Must(es.NewRegexpQuery("beat.hostname", nodePattern))
						result, err := esClient.Search(context.Background(),
							indexName,
							indexType,
							query,
							nil,
							from,
							maxResults,
							sortBy)
						if err != nil {
							return err
						}
						if result.TotalHits() == 0 {
							err = fmt.Errorf("No logs found for singleton service %s", service)
							return err
						}
						By(fmt.Sprintf("ts:%s Logs verified for Service: %s", time.Now().String(), service))
					}
					return nil
				}, 90, 1).Should(BeNil(), "failed to verify log export for {%s}", service)
			}
		})

		AfterEach(func() {
			esClient.Close()
		})
	})
})
