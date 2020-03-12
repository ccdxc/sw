package elastic

import (
	"fmt"
	"testing"
	"time"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	esmock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	elasticSrv *esmock.ElasticServer
	rslvr      *mockresolver.ResolverClient
	logger     log.Logger
)

func setup() {
	config := log.GetDefaultConfig("audit")
	config.Filter = log.AllowAllFilter
	//config.Format = log.JSONFmt
	logger = log.GetNewLogger(config)

	elasticSrv = esmock.NewElasticServer(logger.WithContext("submodule", "elasticsearch-mock-server"))
	elasticSrv.Start()

	rslvr = mockresolver.New()
	// add mock elastic service to mock resolver
	rslvr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     elasticSrv.GetElasticURL(),
	})
}

func shutdown() {
	elasticSrv.Stop()
	rslvr.Stop()
}

func TestProcessEvents(t *testing.T) {
	setup()
	defer shutdown()
	tests := []struct {
		name   string
		events []*audit.AuditEvent
		err    bool
	}{
		{
			name: "single audit event",
			events: []*audit.AuditEvent{
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent1", UUID: uuid.NewV4().String(), Tenant: "default"},
					EventAttributes: audit.EventAttributes{
						Level:       audit.Level_RequestResponse.String(),
						Stage:       audit.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Network", Tenant: "default", Namespace: "default", Name: "network1"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     audit.Outcome_Failure.String(),
						GatewayNode: "node1",
						GatewayIP:   "192.168.75.136",
						Data:        make(map[string]string),
					},
				},
			},
			err: false,
		},
		{
			name: "multiple audit events",
			events: []*audit.AuditEvent{
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent2", UUID: uuid.NewV4().String(), Tenant: "default"},
					EventAttributes: audit.EventAttributes{
						Level:       audit.Level_RequestResponse.String(),
						Stage:       audit.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     audit.Outcome_Success.String(),
						GatewayNode: "node2",
						GatewayIP:   "192.168.75.139",
						Data:        make(map[string]string),
					},
				},
				{
					TypeMeta:   api.TypeMeta{Kind: "AuditEvent"},
					ObjectMeta: api.ObjectMeta{Name: "auditevent3", UUID: uuid.NewV4().String(), Tenant: "default"},
					EventAttributes: audit.EventAttributes{
						Level:       audit.Level_RequestResponse.String(),
						Stage:       audit.Stage_RequestProcessing.String(),
						User:        &api.ObjectRef{Kind: "User", Namespace: "default", Tenant: "default", Name: "admin"},
						Resource:    &api.ObjectRef{Kind: "Role", Tenant: "default", Namespace: "default", Name: "networkadmin"},
						ClientIPs:   []string{"192.168.75.133"},
						Action:      auth.Permission_Create.String(),
						Outcome:     audit.Outcome_Success.String(),
						GatewayNode: "node2",
						GatewayIP:   "192.168.75.139",
						Data:        make(map[string]string),
					},
				},
			},
			err: false,
		},
	}
	ec, err := elastic.NewClient("", rslvr, logger)
	AssertOk(t, err, "failed to create elastic client")
	auditor := &synchAuditor{
		rslver:            rslvr,
		logger:            logger,
		elasticWaitIntvl:  100 * time.Millisecond,
		maxElasticRetries: 3,
	}
	auditor.elasticClient = ec
	err = auditor.Run()
	AssertOk(t, err, "error starting elastic auditor")
	defer auditor.Shutdown()

	for _, test := range tests {
		err = auditor.ProcessEvents(test.events...)
		Assert(t, test.err == (err != nil), fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
	}
	// shutdown elastic server and test ProcessEvents
	shutdown()
	for _, test := range tests {
		if len(test.events) != 0 {
			err = auditor.ProcessEvents(test.events...)
		}
		Assert(t, err != nil, "elastic auditor should fail to process events if elastic server is down")
	}
}

func TestRunWithUnreachableElasticServer(t *testing.T) {
	setup()
	defer shutdown()
	auditor := &synchAuditor{
		elasticServer:     elasticSrv.GetElasticURL(),
		rslver:            rslvr,
		logger:            logger,
		elasticWaitIntvl:  10 * time.Millisecond,
		maxElasticRetries: 1,
	}
	err := auditor.Run()
	Assert(t, err != nil, "elastic auditor should fail because TLS credentials are not set")
	ec, err := elastic.NewClient(elasticSrv.GetElasticURL(), rslvr, logger)
	AssertOk(t, err, "failed to create elastic client")
	auditor.elasticClient = ec
	shutdown()
	err = auditor.Run()
	Assert(t, err != nil, "elastic auditor should fail to run if elastic server is down")
}

func TestNewSynchAuditor(t *testing.T) {
	setup()
	defer shutdown()
	ec, err := elastic.NewClient("", rslvr, logger)
	AssertOk(t, err, "failed to create elastic client")
	auditor := NewSynchAuditor(elasticSrv.GetElasticURL(), rslvr, logger, WithElasticClient(ec))
	Assert(t, auditor != nil, "unable to create auditor")
	time.Sleep(20 * time.Millisecond)
}
