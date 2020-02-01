package elastic

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	auditapi "github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/audit"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type synchAuditor struct {
	elasticClient     elastic.ESClient
	elasticServer     string
	rslver            resolver.Interface
	logger            log.Logger
	elasticWaitIntvl  time.Duration
	maxElasticRetries int
}

// Option fills the optional params for elastic-based synchronous auditor
type Option func(*synchAuditor)

// WithElasticClient passes a custom client for Elastic
func WithElasticClient(esClient elastic.ESClient) Option {
	return func(a *synchAuditor) {
		a.elasticClient = esClient
	}
}

// NewSynchAuditor creates an auditor to save audit events synchronously to Elastic Server
func NewSynchAuditor(elasticServer string, rslver resolver.Interface, logger log.Logger, opts ...Option) audit.Auditor {
	auditor := &synchAuditor{
		elasticServer:     elasticServer,
		rslver:            rslver,
		logger:            logger,
		elasticWaitIntvl:  time.Second,
		maxElasticRetries: 200,
	}
	for _, opt := range opts {
		if opt != nil {
			opt(auditor)
		}
	}
	return auditor
}

func (a *synchAuditor) ProcessEvents(events ...*auditapi.AuditEvent) error {
	ctx, cancel := context.WithDeadline(context.Background(), time.Now().Add(30*time.Second))
	defer cancel()
	_, err := utils.ExecuteWithContext(ctx, func(nctx context.Context) (interface{}, error) {
		// index single audit event; it is costly to perform bulk operation for a single event (doc)
		if len(events) == 1 {
			event := events[0]
			if err := a.elasticClient.Index(ctx,
				elastic.GetIndex(globals.AuditLogs, event.GetTenant()),
				elastic.GetDocType(globals.AuditLogs), event.GetUUID(), event); err != nil {
				a.logger.Errorf("error logging audit event to elastic, err: %v", err)
				return false, err
			}
		} else {
			// index multiple audit events; construct bulk audit events request
			requests := make([]*elastic.BulkRequest, len(events))
			for i, evt := range events {
				requests[i] = &elastic.BulkRequest{
					RequestType: "index",
					IndexType:   elastic.GetDocType(globals.AuditLogs),
					ID:          evt.GetUUID(),
					Obj:         evt,
					Index:       elastic.GetIndex(globals.AuditLogs, evt.GetTenant()),
				}
			}
			if bulkResp, err := a.elasticClient.Bulk(ctx, requests); err != nil {
				a.logger.Errorf("error logging bulk audit events to elastic, err: %v", err)
				return false, err
			} else if len(bulkResp.Failed()) > 0 {
				a.logger.Errorf("bulk audit events logging failed on elastic")
				return false, err
			}
		}
		return true, nil
	})
	return err
}

func (a *synchAuditor) Run(stopCh <-chan struct{}) error {
	if a.elasticClient == nil {
		// Initialize elastic client
		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return elastic.NewAuthenticatedClient(a.elasticServer, a.rslver, a.logger)
		}, a.elasticWaitIntvl, a.maxElasticRetries)
		if err != nil {
			a.logger.ErrorLog("method", "Run", "msg", "failed to create elastic client", "err", err)
			return err
		}
		a.logger.DebugLog("method", "Run", "msg", "created elastic client")
		a.elasticClient = result.(elastic.ESClient)
	}
	return a.createAuditLogsElasticTemplate()
}

func (a *synchAuditor) Shutdown() {
	a.elasticClient.Close()
}

// createAuditLogsElasticTemplate helper function to create index template for audit logs.
func (a *synchAuditor) createAuditLogsElasticTemplate() error {
	docType := elastic.GetDocType(globals.AuditLogs)
	mapping, err := mapper.ElasticMapper(auditapi.AuditEvent{
		EventAttributes: auditapi.EventAttributes{
			// Need to make sure pointer fields are valid to
			// generate right mappings using reflect
			Resource: &api.ObjectRef{},
			User:     &api.ObjectRef{},
		},
	},
		docType,
		mapper.WithShardCount(3),
		mapper.WithReplicaCount(2),
		mapper.WithMaxInnerResults(globals.SpyglassMaxResults),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", docType)),
		mapper.WithCharFilter())
	if err != nil {
		a.logger.Errorf("failed to get elastic mapping for audit log object (%v), err: %v", auditapi.AuditEvent{}, err)
		return err
	}

	// JSON string mapping
	strMapping, err := mapping.JSONString()
	if err != nil {
		a.logger.Errorf("failed to convert elastic mapping (%v) to JSON string", mapping)
		return err
	}
	a.logger.Debugf("audit log elastic mapping: %v", strMapping)
	// create audit logs template
	if err := a.elasticClient.CreateIndexTemplate(context.Background(), elastic.GetTemplateName(globals.AuditLogs), strMapping); err != nil {
		a.logger.Errorf("failed to create audit logs index template, err: %v", err)
		return err
	}

	return nil
}
