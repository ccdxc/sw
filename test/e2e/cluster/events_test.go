package cluster

import (
	"context"
	"fmt"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	es "gopkg.in/olivere/elastic.v5"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

var _ = Describe("events test", func() {
	var (
		esClient    elastic.ESClient
		from        = int32(0)
		maxResults  = int32(10)
		sortByField = ""
		sortAsc     = true
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
				query, nil, from, maxResults, sortByField, sortAsc)

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
				es.NewTermQuery("type.keyword", cmd.LeaderElected))
			res, err := esClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				elastic.GetDocType(globals.Events),
				query, nil, from, maxResults, sortByField, sortAsc)

			if err != nil {
				return err
			}

			if res.TotalHits() == 0 {
				return fmt.Errorf("could not find `LeaderElected` event")
			}
			return nil
		}, 30, 1).Should(BeNil(), "could not find `LeaderElected` event in elasticsearch")
	})

	It("NMD should start recording events once NAPLES node is added to the cluster", func() {
		if ts.tu.NumNaplesHosts == 0 {
			Skip("No NAPLES node to be added to the cluster. Skipping NMD events test")
		}

		// check for `NICAdmitted` event
		Eventually(func() error {
			query := es.NewBoolQuery().Must(es.NewTermQuery("source.component.keyword", globals.Nmd),
				es.NewTermQuery("type.keyword", cmd.NICAdmitted))
			res, err := esClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				elastic.GetDocType(globals.Events),
				query, nil, from, maxResults, sortByField, sortAsc)

			if err != nil {
				return err
			}

			if ts.tu.NumNaplesHosts != int(res.TotalHits()) {
				return fmt.Errorf("could not find `NICAdmitted` events")
			}
			return nil
		}, 60, 1).Should(BeNil(), "could not find `NICAdmitted` events in elasticsearch")

		// check for `NICUpdated` event; there are relatively large number of updates.
		// so, check for deduped events i.e. count>1
		Eventually(func() error {
			query := es.NewBoolQuery().Must(es.NewTermQuery("source.component.keyword", globals.Nmd),
				es.NewTermQuery("type.keyword", cmd.NICUpdated),
				es.NewRangeQuery("count").Gt(1))
			res, err := esClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				elastic.GetDocType(globals.Events),
				query, nil, from, maxResults, sortByField, sortAsc)

			if err != nil {
				return err
			}

			if res.TotalHits() == 0 {
				return fmt.Errorf("could not find `NICUpdated` event")
			}
			return nil
		}, 120, 1).Should(BeNil(), "could not find deduped `NICUpdated` event in elasticsearch")
	})

	AfterEach(func() {
		esClient.Close()
	})
})
