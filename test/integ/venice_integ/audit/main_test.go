package audit

import (
	"crypto/x509"
	"fmt"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	evtsapi "github.com/pensando/sw/api/generated/events"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	pcache "github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/utils"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/trace"
)

const (
	testUser     = "test"
	testPassword = "pensandoo0"
	testTenant   = "testtenant"
)

var (
	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "auth_integ_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, log.GetNewLogger(log.GetDefaultConfig("audit_integ_test")))
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
	apiServer         apiserver.Server
	apiServerAddr     string
	apiGw             apigw.APIGateway
	apiGwAddr         string
	pcache            pcache.Interface
	fdr               finder.Interface
	fdrAddr           string
	restcl            apiclient.Services
	apicl             apiclient.Services
}

// setupElastic helper function starts elasticsearch and creates elastic client
func (t *tInfo) setupElastic() error {
	t.rslvr = mockresolver.New()

	config := log.GetDefaultConfig("AuditIntegTest")
	config.Filter = log.AllowAllFilter
	t.logger = log.GetNewLogger(config)

	var err error
	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		t.logger.Fatalf("error setting up TLS provider: %v", err)
	}

	t.signer, _, t.trustRoots, err = testutils.GetCAKit()
	if err != nil {
		t.logger.Errorf("Error getting CA artifacts: %v", err)
		return err
	}
	// start elasticsearch
	if err := t.startElasticSearch(); err != nil {
		t.logger.Errorf("failed to start Elastic Search, err: %v", err)
		return err
	}

	t.esClient, err = testutils.CreateElasticClient(t.elasticSearchAddr, t.rslvr, t.logger, t.signer, t.trustRoots)
	if err != nil {
		t.logger.Errorf("failed to create Elastic Search client, err: %v", err)
		return err
	}
	if !testutils.IsElasticClusterHealthy(t.esClient) {
		return fmt.Errorf("elasticsearch cluster not healthy")
	}
	return nil
}

func (t *tInfo) startSpyglass() error {
	// Create new policy cache for spyglass
	t.pcache = pcache.NewCache(t.logger)
	// start spyglass finder
	fdr, fdrAddr, err := testutils.StartSpyglass("finder", "", t.rslvr, t.pcache, t.logger, t.esClient)
	if err != nil {
		return err
	}
	t.fdr = fdr.(finder.Interface)
	t.fdrAddr = fdrAddr
	t.updateResolver(globals.Spyglass, t.fdrAddr)
	return nil
}

func (t *tInfo) startAPIServer() error {
	// start API server
	trace.Init("ApiServer")
	var err error
	t.apiServer, t.apiServerAddr, err = serviceutils.StartAPIServer(":0", "AuditIntegTest", t.logger)
	if err != nil {
		return err
	}
	t.updateResolver(globals.APIServer, t.apiServerAddr)
	// grpc client
	apicl, err := apiclient.NewGrpcAPIClient("AuditIntegTest", t.apiServerAddr, t.logger)
	if err != nil {
		t.logger.Errorf("cannot create grpc client, Err: %v", err)
		return err
	}
	t.apicl = apicl
	return nil
}

func (t *tInfo) startAPIGateway() error {
	// start API gateway
	var err error
	t.apiGw, t.apiGwAddr, err = testutils.StartAPIGatewayWithAuditor(":0", false,
		map[string]string{globals.APIServer: t.apiServerAddr, globals.Spyglass: t.fdrAddr},
		[]string{"metrics_query", "objstore"},
		[]string{}, t.logger, auditmgr.WithAuditors(elasticauditor.NewSynchAuditor(t.elasticSearchAddr, t.rslvr, t.logger, elasticauditor.WithElasticClient(t.esClient))))
	if err != nil {
		return err
	}
	// REST Client
	restcl, err := apiclient.NewRestAPIClient(t.apiGwAddr)
	if err != nil {
		t.logger.Errorf("cannot create REST client, Err: %v", err)
		return err
	}
	t.restcl = restcl
	return err
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
	// add elastic service to mock resolver
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

func (t *tInfo) teardownElastic() {
	if t.esClient != nil {
		t.esClient.Close()
	}
	testutils.StopElasticsearch(t.elasticSearchName, t.elasticSearchDir)
	testutils.CleanupIntegTLSProvider()
}
