package audit

import (
	"crypto/x509"
	"fmt"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
)

// tInfo represents test info.
type tInfo struct {
	logger            log.Logger
	rslvr             *mockresolver.ResolverClient // resolver
	esClient          elastic.ESClient             // elastic client to verify the results
	elasticSearchAddr string                       // elastic address
	elasticSearchName string                       // Elastic Search server name; used to stop the server
	elasticSearchDir  string                       // name of the directory where Elastic credentials and logs are stored
	signer            certs.CSRSigner              // function to sign CSRs for TLS
	trustRoots        []*x509.Certificate          // trust roots to verify TLS certs
}

// setup helper function create evtsmgr, evtsproxy, etc. services
func (t *tInfo) setup() error {
	t.rslvr = mockresolver.New()

	config := log.GetDefaultConfig("AuditIntegTest")
	config.Filter = log.AllowAllFilter
	t.logger = log.GetNewLogger(config)

	var err error
	t.signer, _, t.trustRoots, err = testutils.GetCAKit()
	if err != nil {
		t.logger.Errorf("Error getting CA artifacts: %v", err)
		return err
	}
	// start elasticsearch
	if err := t.startElasticSearch(); err != nil {
		log.Errorf("failed to start Elastic Search, err: %v", err)
		return err
	}

	t.esClient, err = testutils.CreateElasticClient(t.elasticSearchAddr, t.rslvr, t.logger, t.signer, t.trustRoots)
	if err != nil {
		log.Errorf("failed to create Elastic Search client, err: %v", err)
		return err
	}
	return nil
}

// startElasticsearch helper function to start elasticsearch
func (t *tInfo) startElasticSearch() error {
	var err error

	log.Infof("starting Elastic Search")

	t.elasticSearchName = uuid.NewV4().String()
	t.elasticSearchAddr, t.elasticSearchDir, err = testutils.StartElasticsearch(t.elasticSearchName, t.signer, t.trustRoots)
	if err != nil {
		return fmt.Errorf("failed to start elasticsearch, err: %v", err)
	}
	// add mock elastic service to mock resolver
	t.updateResolver(globals.ElasticSearch, t.elasticSearchAddr)
	return nil
}

// updateResolver helper function to update mock resolver with the given service and URL
func (t *tInfo) updateResolver(serviceName, url string) {
	t.rslvr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     url,
	})
}

func (t *tInfo) teardown() {
	if t.esClient != nil {
		t.esClient.Close()
	}
	testutils.StopElasticsearch(t.elasticSearchName, t.elasticSearchDir)
}
