package cluster

import (
	"context"
	"fmt"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	es "gopkg.in/olivere/elastic.v5"

	cmdenv "github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("events test", func() {
	var (
		esClient   elastic.ESClient
		from       = int32(0)
		maxResults = int32(10)
		sortBy     = ""
	)

	BeforeEach(func() {
		esAddr := fmt.Sprintf("%s:%s", ts.tu.FirstVeniceIP, globals.ElasticsearchRESTPort)
		logConfig := log.GetDefaultConfig("e2e_events_test")
		Eventually(func() error {
			var err error
			esClient, err = elastic.NewClient(esAddr, nil, log.GetNewLogger(logConfig))
			return err
		}, 30, 1).Should(BeNil(), "failed to initialize elastic client")
	})

	It("CMD should start recording events once the cluster is up", func() {
		// check for CMD events
		Eventually(func() error {
			query := es.NewTermQuery("source.component.keyword", globals.Cmd)
			res, err := esClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				elastic.GetDocType(globals.Events),
				query, nil, from, maxResults, sortBy)

			if err != nil {
				return err
			}

			if res.TotalHits() == 0 {
				return fmt.Errorf("could not find any CMD event")
			}
			return nil
		}, 30, 1).Should(BeNil(), "could not find any CMD event in elasticsearch")

		// check for `LeaderElected` event
		Eventually(func() error {
			query := es.NewBoolQuery().Must(es.NewTermQuery("source.component.keyword", globals.Cmd),
				es.NewTermQuery("type.keyword", cmdenv.LeaderElected))
			res, err := esClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				elastic.GetDocType(globals.Events),
				query, nil, from, maxResults, sortBy)

			if err != nil {
				return err
			}

			if res.TotalHits() == 0 {
				return fmt.Errorf("could not find `LeaderElected` event")
			}
			return nil
		}, 30, 1).Should(BeNil(), "could not find `LeaderElected` event in elasticsearch")
	})

	AfterEach(func() {
		esClient.Close()
	})
})
