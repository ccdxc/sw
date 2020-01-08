package finder

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	es "github.com/olivere/elastic"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	apiutils "github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpc "github.com/pensando/sw/venice/utils/authz/grpc"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
)

type auditHandler struct {
	fdr *Finder
}

// GetAuditEvent returns the audit event identified by given UUID
func (a *auditHandler) GetEvent(ctx context.Context, r *audit.AuditEventRequest) (*audit.AuditEvent, error) {
	// construct query
	query := es.NewMatchPhraseQuery("meta.uuid", r.GetUUID())

	// execute query
	result, err := a.fdr.elasticClient.Search(ctx,
		"*.auditlogs.*", // search only in auditlogs indices
		"",              // skip the index type
		query,           // query to be executed
		nil,             // no aggregation
		0,               // from
		1,               // to; there should be only one request matching the query
		"",              // sorting is not required
		false)           // sorting order doesn't matter as there will be only one event
	if err != nil {
		a.fdr.logger.Errorf("failed to query ElasticSearch, err: %+v", err)
		return nil, status.Error(codes.Internal, "could not get the audit event")
	}

	// parse the result
	if result.TotalHits() == 0 {
		return nil, status.Errorf(codes.NotFound, "audit event not found")
	}

	res := audit.AuditEvent{}
	if err := json.Unmarshal(*result.Hits.Hits[0].Source, &res); err != nil {
		a.fdr.logger.Errorf("failed to unmarshal audit log from ElasticSearch result, err: %+v", err)
		return nil, status.Errorf(codes.Internal, "could not get the audit event")
	}
	// check if user is authorized to view the event
	if apiutils.IsAuditsReaderCtx(ctx) {
		userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(ctx)
		if !ok {
			a.fdr.logger.Errorf("no user in grpc metadata")
			return nil, status.Errorf(codes.Internal, "no user in context")
		}
		user := &auth.User{ObjectMeta: *userMeta}
		// check if user is authorized to view the event
		authorizer, err := authzgrpc.NewAuthorizer(ctx)
		if err != nil {
			a.fdr.logger.Errorf("error creating grpc authorizer for GetEvent request: %v", err)
			return nil, status.Error(codes.Internal, err.Error())
		}
		resource := authz.NewResource(res.Tenant, "", auth.Permission_AuditEvent.String(), res.Namespace, res.Name)
		ok, _ = authorizer.IsAuthorized(user, authz.NewOperation(resource, auth.Permission_Read.String()))
		if !ok {
			return nil, status.Error(codes.PermissionDenied, fmt.Sprintf("unauthorized to view event (%s)", r.GetUUID()))
		}
	}
	return &res, nil
}

// AutoWatchSvcAuditV1 is not implemented
func (a *auditHandler) AutoWatchSvcAuditV1(*api.ListWatchOptions, audit.AuditV1_AutoWatchSvcAuditV1Server) error {
	return errors.New("not implemented")
}

func newAuditHandler(fdr *Finder) audit.AuditV1Server {
	return &auditHandler{fdr: fdr}
}
