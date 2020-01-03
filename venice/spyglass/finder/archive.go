package finder

import (
	"context"
	"errors"
	"fmt"
	"strings"
	"time"

	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type archiveJob struct {
	ctx        context.Context
	cancel     context.CancelFunc
	logger     log.Logger
	rslvr      resolver.Interface
	esClient   elastic.ESClient
	archiveReq *monitoring.ArchiveRequest
	exporter   archive.Exporter
}

func (j *archiveJob) ID() string {
	return j.archiveReq.UUID
}

func (j *archiveJob) Cancel() {
	if j.cancel != nil {
		j.cancel()
	}
	// TODO: should we wait before calling close on elastic client?
	j.close()
}

// Run runs the archive job and returns any error updating the archive request status
func (j *archiveJob) Run(ctx context.Context) error {
	defer j.close()
	nctx, cancel := context.WithCancel(ctx)
	j.ctx = nctx
	j.cancel = cancel
	j.logger.DebugLog("method", "Run", "msg", fmt.Sprintf("processing archive request [%#v]", *j.archiveReq))
	if err := validateArchiveRequest(j.archiveReq); err != nil {
		j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		j.archiveReq.Status.Reason = err.Error()
		j.logger.ErrorLog("method", "Run", "msg", fmt.Sprintf("invalid archive request: %v", j.archiveReq), "error", err)
		return err
	}
	query, err := archiveQuery(j.archiveReq)
	if err != nil {
		j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		j.archiveReq.Status.Reason = err.Error()
		j.logger.ErrorLog("method", "Run", "msg", "unable to create archive query", "error", err)
		return err
	}
	var index string
	switch j.archiveReq.Spec.Type {
	case monitoring.ArchiveRequestSpec_Event.String():
		index = "*.events.*"
	case monitoring.ArchiveRequestSpec_AuditEvent.String():
		index = "*.auditlogs.*"
	}
	scroller, err := j.esClient.Scroll(j.ctx, index, "", query, 8000)
	if err != nil {
		j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		j.archiveReq.Status.Reason = err.Error()
		j.logger.ErrorLog("method", "Run", "msg", "scroll failed", "error", err)
		return err
	}
	uri, _, err := j.exporter.Export(j.ctx, scroller, make(map[string]string)) // TODO: put file name in map
	defer func() {
		if err != nil {
			err = j.exporter.Delete(j.archiveReq)
			if err != nil {
				j.logger.ErrorLog("method", "Run", "msg", "failed to delete archive upon export failure", "error", err)
				// TODO: throw an event
			}
		}
	}()
	switch err {
	case context.Canceled:
		j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Canceled.String()
		j.archiveReq.Status.Reason = err.Error()
		j.logger.ErrorLog("method", "Run", "msg", "archive request cancelled", "error", err)
	case context.DeadlineExceeded:
		j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_TimeOut.String()
		j.archiveReq.Status.Reason = err.Error()
		j.logger.ErrorLog("method", "Run", "msg", "archive request timed out", "error", err)
	case nil:
		j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Completed.String()
		j.archiveReq.Status.URI = uri
	default:
		if strings.Contains(err.Error(), "context canceled") {
			j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Canceled.String()
		} else {
			j.archiveReq.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		}
		j.archiveReq.Status.Reason = err.Error()
		j.logger.ErrorLog("method", "Run", "msg", fmt.Sprintf("export failed for archive request [%#v]", *j.archiveReq), "error", err)
	}
	return err
}

func (j *archiveJob) GetArchiveRequest() *monitoring.ArchiveRequest {
	return j.archiveReq
}

func (j *archiveJob) close() {
	if j.esClient != nil {
		j.esClient.Close()
	}
}

// NewArchiveJob creates a job to archive audit events or events
func NewArchiveJob(archiveReq *monitoring.ArchiveRequest, exporter archive.Exporter, esClient elastic.ESClient, rslvr resolver.Interface, logger log.Logger) archive.Job {
	return &archiveJob{
		logger:     logger,
		rslvr:      rslvr,
		esClient:   esClient,
		archiveReq: archiveReq,
		exporter:   exporter,
	}
}

// CreateJobCb is a callback from archive service to create archive job
func CreateJobCb(req *monitoring.ArchiveRequest, exporter archive.Exporter, rslvr resolver.Interface, logger log.Logger) archive.Job {
	var elasticClient elastic.ESClient
	result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		return elastic.NewAuthenticatedClient("", rslvr, logger)
	}, elasticWaitIntvl, maxElasticRetries)
	if err != nil {
		logger.ErrorLog("method", "CreateJobCb", "msg", "failed to create elastic client", "error", err)
		req.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		req.Status.Reason = err.Error()
		return NewArchiveJob(req, exporter, elasticClient, rslvr, logger)
	}
	elasticClient = result.(elastic.ESClient)
	logger.DebugLog("method", "CreateJobCb", "msg", "created Elastic client")

	if exporter == nil {
		req.Status.Status = monitoring.ArchiveRequestStatus_Failed.String()
		req.Status.Reason = "no exporter specified to archive"
		return NewArchiveJob(req, exporter, elasticClient, rslvr, logger)

	}
	return NewArchiveJob(req, exporter, elasticClient, rslvr, logger)
}

func validateArchiveRequest(req *monitoring.ArchiveRequest) error {
	if req.Status.Status != monitoring.ArchiveRequestStatus_Running.String() {
		return errors.New(req.Status.Reason)
	}
	switch req.Spec.Type {
	case monitoring.ArchiveRequestSpec_Event.String(), monitoring.ArchiveRequestSpec_AuditEvent.String():
	default:
		return fmt.Errorf("unsupported archive request type %s", req.Spec.Type)
	}
	return nil
}

func archiveQuery(req *monitoring.ArchiveRequest) (es.Query, error) {
	searchReq := &search.SearchRequest{}
	searchReq.Query = &search.SearchQuery{}
	searchReq.Query.Fields = req.Spec.Query.Fields
	switch req.Spec.Type {
	case monitoring.ArchiveRequestSpec_Event.String():
		searchReq.Query.Kinds = []string{auth.Permission_Event.String()}
	case monitoring.ArchiveRequestSpec_AuditEvent.String():
		searchReq.Query.Kinds = []string{auth.Permission_AuditEvent.String()}
	}
	searchReq.Query.Labels = req.Spec.Query.Labels
	// set time window
	if req.Spec.Query.StartTime != nil {
		startTime, err := req.Spec.Query.StartTime.Time()
		if err == nil {
			fieldReq := &fields.Requirement{
				Key:      "meta.mod-time",
				Operator: "gte",
				Values:   []string{startTime.Format(time.RFC3339Nano)},
			}
			searchReq.Query.Fields.Requirements = append(searchReq.Query.Fields.Requirements, fieldReq)
		}
	}
	if req.Spec.Query.EndTime != nil {
		endTime, err := req.Spec.Query.EndTime.Time()
		if err == nil {
			fieldReq := &fields.Requirement{
				Key:      "meta.mod-time",
				Operator: "lte",
				Values:   []string{endTime.Format(time.RFC3339Nano)},
			}
			searchReq.Query.Fields.Requirements = append(searchReq.Query.Fields.Requirements, fieldReq)
		}
	}
	searchReq.Query.Texts = req.Spec.Query.Texts
	query, err := QueryBuilder(searchReq)
	if err != nil {
		return nil, err
	}
	return query, nil
}
