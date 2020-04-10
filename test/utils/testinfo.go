package utils

import (
	"context"
	"crypto/x509"
	"fmt"
	"strings"

	"github.com/jeromer/syslogparser"

	"github.com/pensando/sw/venice/utils/syslog"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
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
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/objstore/memclient"
	"github.com/pensando/sw/venice/utils/resolver"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
	"github.com/pensando/sw/venice/utils/trace"
)

// TestInfo represents test info.
type TestInfo struct {
	Name              string
	Logger            log.Logger
	Rslvr             *mockresolver.ResolverClient // resolver
	ESClient          elastic.ESClient             // elastic client to verify the results
	ElasticSearchAddr string                       // elastic address
	ElasticSearchName string                       // Elastic Search server name; used to stop the server
	ElasticSearchDir  string                       // name of the directory where Elastic credentials and logs are stored
	signer            certs.CSRSigner              // function to sign CSRs for TLS
	trustRoots        []*x509.Certificate          // trust roots to verify TLS certs
	APIServer         apiserver.Server
	APIServerAddr     string
	APIGw             apigw.APIGateway
	APIGwAddr         string
	pcache            pcache.Interface
	Fdr               finder.Interface
	fdrAddr           string
	Restcl            apiclient.Services
	Apicl             apiclient.Services
	Objstorecl        objstore.Client
	SyslogBSDInfo     *SyslogInfo
	SyslogRFC5424Info *SyslogInfo
}

// SyslogInfo represents info about syslog server
type SyslogInfo struct {
	Addr   string
	IP     string
	Port   string
	Ch     chan syslogparser.LogParts
	Cancel context.CancelFunc
}

// SetupElastic helper function starts elasticsearch and creates elastic client
func (t *TestInfo) SetupElastic() error {
	t.Rslvr = mockresolver.New()

	config := log.GetDefaultConfig(t.Name)
	config.Filter = log.AllowAllFilter
	t.Logger = log.GetNewLogger(config)

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	var err error
	err = SetupIntegTLSProvider()
	if err != nil {
		t.Logger.Fatalf("error setting up TLS provider: %v", err)
	}

	t.signer, _, t.trustRoots, err = GetCAKit()
	if err != nil {
		t.Logger.Errorf("Error getting CA artifacts: %v", err)
		return err
	}
	// start elasticsearch
	if err := t.StartElasticSearch(); err != nil {
		t.Logger.Errorf("failed to start Elastic Search, err: %v", err)
		return err
	}

	t.ESClient, err = CreateElasticClient(t.ElasticSearchAddr, t.Rslvr, t.Logger, t.signer, t.trustRoots)
	if err != nil {
		t.Logger.Errorf("failed to create Elastic Search client, err: %v", err)
		return err
	}
	if !IsElasticClusterHealthy(t.ESClient) {
		return fmt.Errorf("elasticsearch cluster not healthy")
	}
	return err
}

// StartSpyglass helper function starts spyglass
func (t *TestInfo) StartSpyglass() error {
	// Create new policy cache for spyglass
	t.pcache = pcache.NewCache(t.Logger)
	// start spyglass finder
	archiveService := archmock.GetMockArchiveService()
	if t.APIServerAddr != "" {
		t.Objstorecl = memclient.NewMemObjstore()
		expter, err := archexp.NewObjstoreExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), globals.DefaultTenant, t.Rslvr, t.Logger, archexp.WithObjectstoreClient(t.Objstorecl))
		if err != nil {
			return err
		}
		createJobCb := func(req *monitoring.ArchiveRequest, exporter archive.Exporter, rslvr resolver.Interface, logger log.Logger) archive.Job {
			return finder.NewArchiveJob(req, exporter, t.ESClient, rslvr, logger)
		}
		archiveService = archsvc.GetService("finder", t.APIServerAddr, t.Rslvr, t.Logger, createJobCb, archsvc.WithExporter(monitoring.ArchiveRequestSpec_AuditEvent.String(), expter))
	}
	fdr, fdrAddr, err := StartSpyglassWithArchiveService("finder", t.APIServerAddr, t.Rslvr, t.pcache, t.Logger, t.ESClient, archiveService)
	if err != nil {
		return err
	}
	t.Fdr = fdr.(finder.Interface)
	t.fdrAddr = fdrAddr
	t.UpdateResolver(globals.Spyglass, t.fdrAddr)
	return nil
}

// StartAPIServer helper function starts API server
func (t *TestInfo) StartAPIServer() error {
	// start API server
	trace.Init("ApiServer")
	var err error
	t.APIServer, t.APIServerAddr, err = serviceutils.StartAPIServer(":0", t.Name, t.Logger)
	if err != nil {
		return err
	}
	t.UpdateResolver(globals.APIServer, t.APIServerAddr)
	// grpc client
	apicl, err := apiclient.NewGrpcAPIClient(t.Name, t.APIServerAddr, t.Logger)
	if err != nil {
		t.Logger.Errorf("cannot create grpc client, Err: %v", err)
		return err
	}
	t.Apicl = apicl
	return nil
}

// StartAPIGateway helper functions starts API gateway
func (t *TestInfo) StartAPIGateway() error {
	// start API gateway
	elasticAuditor := elasticauditor.NewSynchAuditor(t.ElasticSearchAddr, t.Rslvr, t.Logger, elasticauditor.WithElasticClient(t.ESClient))
	if err := elasticAuditor.Run(); err != nil {
		return err
	}
	syslogAuditor := syslogauditor.NewSynchAuditor(t.Rslvr, t.Logger, syslogauditor.WithPolicyGetter(audit.GetPolicyGetter(t.Name, t.APIServerAddr, t.Rslvr, t.Logger)))
	if err := syslogAuditor.Run(); err != nil {
		return err
	}
	var err error
	t.APIGw, t.APIGwAddr, err = StartAPIGatewayWithAuditor(":0", false,
		map[string]string{},
		[]string{"telemetry_query", "objstore", "tokenauth"},
		[]string{}, t.Rslvr, t.Logger, auditmgr.WithAuditors(elasticAuditor, syslogAuditor))
	if err != nil {
		return err
	}
	// REST Client
	restcl, err := apiclient.NewRestAPIClient(t.APIGwAddr)
	if err != nil {
		t.Logger.Errorf("cannot create REST client, Err: %v", err)
		return err
	}
	t.Restcl = restcl
	return err
}

// StartElasticSearch helper function to start elasticsearch
func (t *TestInfo) StartElasticSearch() error {
	var err error

	log.Infof("starting Elastic Search")

	t.ElasticSearchName = uuid.NewV4().String()
	t.ElasticSearchAddr, t.ElasticSearchDir, err = StartElasticsearch(t.ElasticSearchName, t.ElasticSearchDir, t.signer, t.trustRoots)
	if err != nil {
		return fmt.Errorf("failed to start elasticsearch, err: %v", err)
	}
	// add elastic service to mock resolver
	t.UpdateResolver(globals.ElasticSearch, t.ElasticSearchAddr)
	return nil
}

// UpdateResolver helper function to update mock resolver with the given service and URL
func (t *TestInfo) UpdateResolver(serviceName, url string) {
	t.Rslvr.AddServiceInstance(&types.ServiceInstance{
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

// TeardownElastic shuts down elastic server
func (t *TestInfo) TeardownElastic() {
	if t.ESClient != nil {
		t.ESClient.Close()
	}
	StopElasticsearch(t.ElasticSearchName, t.ElasticSearchDir)
	CleanupIntegTLSProvider()
}

// StartSyslogServers helper function starts syslog servers
func (t *TestInfo) StartSyslogServers() error {
	ctx, cancel := context.WithCancel(context.TODO())
	addr, ch, err := syslog.Server(ctx, ":0", monitoring.MonitoringExportFormat_SYSLOG_BSD.String(), "udp")
	if err != nil {
		t.Logger.Errorf("failed to start BSD syslog server: %v", err)
		cancel()
		return err
	}
	s := strings.Split(addr, ":")
	t.SyslogBSDInfo = &SyslogInfo{
		Addr:   addr,
		IP:     "127.0.0.1",
		Port:   s[len(s)-1],
		Cancel: cancel,
		Ch:     ch,
	}
	t.Logger.Infof("bsd syslog server started at addr: %v", addr)
	addr, ch, err = syslog.Server(ctx, ":0", monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String(), "udp")
	if err != nil {
		t.Logger.Errorf("failed to start RFC5424 syslog server: %v", err)
		cancel()
		return err
	}
	s = strings.Split(addr, ":")
	t.SyslogRFC5424Info = &SyslogInfo{
		Addr:   addr,
		IP:     "127.0.0.1",
		Port:   s[len(s)-1],
		Cancel: cancel,
		Ch:     ch,
	}
	t.Logger.Infof("rfc5424 syslog server started at addr: %v", addr)
	return nil
}

// StopSyslogServers helper function shuts down syslog servers
func (t *TestInfo) StopSyslogServers() {
	if t.SyslogBSDInfo != nil {
		t.SyslogBSDInfo.Cancel()
	}
	if t.SyslogRFC5424Info != nil {
		t.SyslogRFC5424Info.Cancel()
	}
}
