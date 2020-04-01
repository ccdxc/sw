package fwlog

import (
	"context"
	"crypto/x509"
	"fmt"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/fwlog"
	"github.com/pensando/sw/api/generated/monitoring"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	pcache "github.com/pensando/sw/venice/spyglass/cache"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/utils/archive"
	archexp "github.com/pensando/sw/venice/utils/archive/exporter"
	archmock "github.com/pensando/sw/venice/utils/archive/mock"
	archsvc "github.com/pensando/sw/venice/utils/archive/service"
	"github.com/pensando/sw/venice/utils/audit"
	elasticauditor "github.com/pensando/sw/venice/utils/audit/elastic"
	auditmgr "github.com/pensando/sw/venice/utils/audit/manager"
	syslogauditor "github.com/pensando/sw/venice/utils/audit/syslog"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/objstore/memclient"
	"github.com/pensando/sw/venice/utils/resolver"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/trace"

	_ "github.com/pensando/sw/api/generated/audit"
	. "github.com/pensando/sw/test/utils"
)

const (
	testUser     = "test"
	testPassword = TestLocalPassword
	testTenant   = "testtenant"
)

var (
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("fwlog_integ_test",
		log.GetNewLogger(log.GetDefaultConfig("fwlog_integ_test"))))
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
	objstorecl        objstore.Client
}

// setupElastic helper function starts elasticsearch and creates elastic client
func (t *tInfo) setupElastic() error {
	t.rslvr = mockresolver.New()

	config := log.GetDefaultConfig("AuditIntegTest")
	config.Filter = log.AllowAllFilter
	t.logger = log.GetNewLogger(config)

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

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
	return t.createFwLogsElasticIndex()
}

func (t *tInfo) startSpyglass() error {
	// Create new policy cache for spyglass
	t.pcache = pcache.NewCache(t.logger)
	// start spyglass finder
	archiveService := archmock.GetMockArchiveService()
	if t.apiServerAddr != "" {
		t.objstorecl = memclient.NewMemObjstore()
		expter, err := archexp.NewObjstoreExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), globals.DefaultTenant, t.rslvr, t.logger, archexp.WithObjectstoreClient(t.objstorecl))
		if err != nil {
			return err
		}
		createJobCb := func(req *monitoring.ArchiveRequest, exporter archive.Exporter, rslvr resolver.Interface, logger log.Logger) archive.Job {
			return finder.NewArchiveJob(req, exporter, t.esClient, rslvr, logger)
		}
		archiveService = archsvc.GetService("finder", t.apiServerAddr, t.rslvr, t.logger, createJobCb, archsvc.WithExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), expter))
	}
	fdr, fdrAddr, err := testutils.StartSpyglassWithArchiveService("finder", t.apiServerAddr, t.rslvr, t.pcache, t.logger, t.esClient, archiveService)
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
	elasticAuditor := elasticauditor.NewSynchAuditor(t.elasticSearchAddr, t.rslvr, t.logger, elasticauditor.WithElasticClient(t.esClient))
	if err := elasticAuditor.Run(); err != nil {
		return err
	}
	syslogAuditor := syslogauditor.NewSynchAuditor(t.rslvr, t.logger, syslogauditor.WithPolicyGetter(audit.GetPolicyGetter("AuditIntegTest", t.apiServerAddr, t.rslvr, t.logger)))
	if err := syslogAuditor.Run(); err != nil {
		return err
	}
	var err error
	t.apiGw, t.apiGwAddr, err = testutils.StartAPIGatewayWithAuditor(":0", false,
		map[string]string{},
		[]string{"telemetry_query", "objstore", "tokenauth"},
		[]string{}, t.rslvr, t.logger, auditmgr.WithAuditors(elasticAuditor, syslogAuditor))
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
	t.elasticSearchAddr, t.elasticSearchDir, err = testutils.StartElasticsearch(t.elasticSearchName, t.elasticSearchDir, t.signer, t.trustRoots)
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

// createFwLogsElasticIndex helper function to create index template for fw logs.
func (t *tInfo) createFwLogsElasticIndex() error {
	docType := elastic.GetDocType(globals.FwLogs)
	mapping, err := mapper.ElasticMapper(fwlog.FwLog{},
		docType,
		mapper.WithShardCount(3),
		mapper.WithReplicaCount(1),
		mapper.WithMaxInnerResults(globals.SpyglassMaxResults),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", docType)),
		mapper.WithCharFilter())
	if err != nil {
		return err
	}
	// JSON string mapping
	strMapping, err := mapping.JSONString()
	if err != nil {
		return err
	}
	t.logger.Infof("fw log elastic mapping: %v", strMapping)
	// create fwlog index
	if err := t.esClient.CreateIndex(context.Background(), elastic.GetIndex(globals.FwLogs, globals.DefaultTenant), strMapping); err != nil {
		t.logger.Errorf("failed to create fw logs index template, err: %v", err)
		return err
	}
	return nil
}
