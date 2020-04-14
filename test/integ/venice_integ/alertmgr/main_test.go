package alertmgr

import (
	"context"
	"crypto/x509"
	"fmt"
	"sync"
	"testing"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/alertmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	testURL = "localhost:0"

	// create mock events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Component:     "events_integ_test",
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, log.GetNewLogger(log.GetDefaultConfig("alertmgr_integ_test")))
)

// tInfo represents test info.
type tInfo struct {
	logger            log.Logger
	mockResolver      *mockresolver.ResolverClient // resolver
	alertmgr          alertmgr.Interface           // events manager to write events to elastic
	esClient          elastic.ESClient             // elastic client to verify the results
	elasticsearchAddr string                       // elastic address
	elasticsearchName string                       // name of the elasticsearch server name; used to stop the server
	elasticsearchDir  string                       // name of the directory where Elastic credentials and logs are stored
	recorders         *recorders
	apiServer         apiserver.Server    // venice API server
	apiServerAddr     string              // API server address
	signer            certs.CSRSigner     // function to sign CSRs for TLS
	trustRoots        []*x509.Certificate // trust roots to verify TLS certs
	apiClient         apiclient.Services
	testName          string
}

// list of recorders belonging to the test
type recorders struct {
	sync.Mutex
	list []events.Recorder
}

func (t *tInfo) setup(tst *testing.T) error {
	var err error
	logConfig := log.GetDefaultConfig("alertmgr_test")
	logConfig.Format = log.JSONFmt
	logConfig.Filter = log.AllowInfoFilter

	t.logger = log.GetNewLogger(logConfig).WithContext("t_name", tst.Name())
	t.logger.Infof("Starting test %s", tst.Name())
	t.mockResolver = mockresolver.New()
	t.testName = tst.Name()

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	// start certificate server
	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	t.signer, _, t.trustRoots, err = testutils.GetCAKit()
	if err != nil {
		t.logger.Errorf("Error getting CA artifacts: %v", err)
		return err
	}

	t.recorders = &recorders{}

	// start elasticsearch
	if err = t.startElasticsearch(); err != nil {
		t.logger.Errorf("failed to start elasticsearch, err: %v", err)
		return err
	}

	// create elasticsearch client
	if err = t.createElasticClient(); err != nil {
		t.logger.Errorf("failed to create elasticsearch client, err: %v", err)
		return err
	}

	// start API server
	if err = t.startAPIServer(tst.Name()); err != nil {
		t.logger.Errorf("failed to start API server, err: %v", err)
		return err
	}
	t.logger.Errorf("started API server, err: %v", err)

	// start alertmgr
	amgr, err := alertmgr.New(t.logger, t.mockResolver)
	if err != nil {
		t.logger.Errorf("failed to start alert manager, err: %v", err)
		return err
	}
	t.alertmgr = amgr

	// Run alertsmgr
	go amgr.Run()

	return nil
}

// teardown stops all the services that were started during setup
func (t *tInfo) teardown() {
	t.alertmgr.Stop()
	t.recorders.close()

	if t.apiClient != nil {
		t.apiClient.ClusterV1().Version().Delete(context.Background(), &api.ObjectMeta{Name: t.testName})
		t.apiClient.Close()
		t.apiClient = nil
	}

	if t.esClient != nil {
		t.esClient.Close()
	}

	testutils.StopElasticsearch(t.elasticsearchName, t.elasticsearchDir)

	if t.apiServer != nil {
		t.apiServer.Stop()
		t.apiServer = nil
	}

	// stop certificate server
	testutils.CleanupIntegTLSProvider()

	if t.mockResolver != nil {
		t.mockResolver.Stop()
		t.mockResolver = nil
	}

	t.logger.Infof("completed test")
}

func (t *tInfo) startAPIServer(clusterName string) error {
	var err error
	t.apiServer, t.apiServerAddr, err = serviceutils.StartAPIServer(testURL, clusterName, t.logger)
	if err != nil {
		return err
	}
	t.updateResolver(globals.APIServer, t.apiServerAddr)

	if t.apiClient != nil { // close existing client if any
		t.apiClient.Close()
	}

	t.apiClient, err = apiclient.NewGrpcAPIClient("events_test", t.apiServerAddr, t.logger)
	if err != nil {
		return err
	}

	return nil
}

func (t *tInfo) stopAPIServer() {
	t.apiServer.Stop()
	t.removeResolverEntry(globals.APIServer, t.apiServerAddr)

	if t.apiClient != nil {
		t.apiClient.Close()
		t.apiClient = nil
	}
}

// updateResolver helper function to update mock resolver with the given service and URL
func (t *tInfo) updateResolver(serviceName, url string) {
	t.mockResolver.AddServiceInstance(&types.ServiceInstance{
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

// createElasticClient helper function to create elastic client
func (t *tInfo) createElasticClient() error {
	var err error
	t.esClient, err = testutils.CreateElasticClient(t.elasticsearchAddr, t.mockResolver, t.logger, t.signer, t.trustRoots)
	return err
}

// startElasticsearch helper function to start elasticsearch
func (t *tInfo) startElasticsearch() error {
	var err error
	t.elasticsearchName = uuid.NewV4().String()
	t.elasticsearchAddr, t.elasticsearchDir, err = testutils.StartElasticsearch(t.elasticsearchName, t.elasticsearchDir, t.signer, t.trustRoots)
	if err != nil {
		return fmt.Errorf("failed to start elasticsearch, err: %v", err)
	}

	// add mock elastic service to mock resolver
	t.updateResolver(globals.ElasticSearch, t.elasticsearchAddr)
	return nil
}

// removeResolverEntry helper function to remove entry from mock resolver
func (t *tInfo) removeResolverEntry(serviceName, url string) {
	t.mockResolver.DeleteServiceInstance(&types.ServiceInstance{
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

// close all the recorders
func (r *recorders) close() {
	r.Lock()
	defer r.Unlock()

	for _, re := range r.list {
		re.Close()
	}
}
