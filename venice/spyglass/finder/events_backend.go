package finder

import (
	"context"
	"encoding/json"
	"fmt"

	es "github.com/olivere/elastic"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/auth"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpc "github.com/pensando/sw/venice/utils/authz/grpc"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/log"
)

// This file implements EventsV1Server; spyglass will use these handlers to serve /events requests.

// GetEvent retuns the event identified by given UUID
func (fdr *Finder) GetEvent(ctx context.Context, r *evtsapi.GetEventRequest) (*evtsapi.Event, error) {
	// construct query
	query := es.NewMatchPhraseQuery("meta.uuid", r.GetUUID())

	// execute query
	result, err := fdr.elasticClient.Search(ctx,
		"*.events.*", // search only in event indices
		"",           // skip the index type
		query,        // query to be executed
		nil,          // no aggregation
		0,            // from
		1,            // to; there should be only one request matching the query
		"",           // sorting is not required
		false)        // sorting order doesn't matter as there will be only one event
	if err != nil {
		fdr.logger.Errorf("failed to query elasticsearch, err: %+v", err)
		return nil, status.Error(codes.Internal, "could not get the event")
	}

	// parse the result
	if result.TotalHits() == 0 {
		return nil, status.Errorf(codes.NotFound, "event not found")
	}

	res := evtsapi.Event{}
	if err := json.Unmarshal(*result.Hits.Hits[0].Source, &res); err != nil {
		fdr.logger.Errorf("failed to unmarshal elasticsearch result, err: %+v", err)
		return nil, status.Errorf(codes.Internal, "could not get the event")
	}
	// check if user is authorized to view the event
	if ctxutils.GetPeerID(ctx) == globals.APIGw {
		userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(ctx)
		if !ok {
			fdr.logger.Errorf("no user in grpc metadata")
			return nil, status.Errorf(codes.Internal, "no user in context")
		}
		user := &auth.User{ObjectMeta: *userMeta}
		// check if user is authorized to view the event
		authorizer, err := authzgrpc.NewAuthorizer(ctx)
		if err != nil {
			fdr.logger.Errorf("error creating grpc authorizer for GetEvent request: %v", err)
			return nil, status.Error(codes.Internal, err.Error())
		}
		resource := authz.NewResource(res.Tenant, "", auth.Permission_Event.String(), res.Namespace, res.Name)
		ok, _ = authorizer.IsAuthorized(user, authz.NewOperation(resource, auth.Permission_Read.String()))
		if !ok {
			return nil, status.Error(codes.PermissionDenied, fmt.Sprintf("unauthorized to view event (%s)", r.GetUUID()))
		}
	}

	return &res, nil
}

// GetEvents return the list of events matching the given listOptions
func (fdr *Finder) GetEvents(ctx context.Context, r *api.ListWatchOptions) (*evtsapi.EventList, error) {
	if fdr.elasticClient == nil {
		return nil, status.Error(codes.Internal, "could not get the events")
	}

	// construct the query
	query := es.NewBoolQuery()

	// maxinum number to events to fetch for the given request; default = 10
	var maxResults int32 = 1000
	if r.GetMaxResults() > 0 {
		maxResults = r.GetMaxResults()
	}

	fdr.logger.Infof("GetEvents request: {%+v}", *r)

	// parse field selector
	fSelector := r.GetFieldSelector()
	if !utils.IsEmpty(fSelector) {
		fldSelectors, err := fields.ParseWithValidation("events.Event", fSelector) // syntax check + validation of fields
		if err != nil {
			log.Errorf("failed to parse the field selector, %v, err: %+v", fSelector, err)
			return nil, status.Error(codes.Internal, "could not get the events")
		}

		// ParseWithValidation replaces keys with actual field names e.g. spec -> Spec
		// but in elastic the objects are stored with JSON keys. In order to use JSON keys
		// in the query, 'Parse' is used which returns JSON keys.
		fldSelectors, err = fields.Parse(fSelector)
		if err != nil {
			log.Errorf("failed to parse the field selector, %v, err: %+v", fSelector, err)
			return nil, status.Error(codes.Internal, "could not get the events")
		}

		for _, req := range fldSelectors.GetRequirements() {
			key := req.GetKey()
			values := req.GetValues()

			switch fields.Operator(fields.Operator_value[req.GetOperator()]) {
			case fields.Operator_equals:
				query = query.Must(es.NewMatchPhraseQuery(key, values[0]))

			case fields.Operator_notEquals:
				query = query.MustNot(es.NewMatchPhraseQuery(key, values[0]))

			case fields.Operator_in: // should match atleast one of the value
				query = query.MinimumNumberShouldMatch(1)
				for _, val := range values {
					query = query.Should(es.NewMatchPhraseQuery(key, val))
				}

			case fields.Operator_notIn: // must not match any of the values
				for _, val := range values {
					query = query.MustNot(es.NewMatchPhraseQuery(key, val))
				}

			case fields.Operator_lt:
				query = query.Must(es.NewRangeQuery(key).Lt(values[0]))

			case fields.Operator_lte:
				query = query.Must(es.NewRangeQuery(key).Lte(values[0]))

			case fields.Operator_gt:
				query = query.Must(es.NewRangeQuery(key).Gt(values[0]))

			case fields.Operator_gte:
				query = query.Must(es.NewRangeQuery(key).Gte(values[0]))
			}
		}
	}

	// TODO: handle label selector; currently, events do not carry any labels.
	// Once we finalize on what goes in the label, implement this

	// constrain event search by tenant and namespace
	query = query.Must(es.NewMatchPhraseQuery("meta.tenant", r.Tenant)).Must(es.NewMatchPhraseQuery("meta.namespace", r.Namespace))

	// execute query
	result, err := fdr.elasticClient.Search(ctx,
		"*.events.*",    // search only in event indices
		"",              // skip the index type
		query,           // query to be executeds
		nil,             // no aggregation
		r.GetFrom(),     // from
		maxResults,      // max count of events to fetch from elastic
		"meta.mod-time", // sort by modified time
		false)           // sort in descending oder
	if err != nil {
		log.Errorf("failed to query elasticsearch, err: %+v", err)
		return nil, status.Error(codes.Internal, "could not get the events")
	}

	// parse the result
	evts := []*evtsapi.Event{}
	for _, res := range result.Hits.Hits {
		var evt evtsapi.Event
		if err := json.Unmarshal(*res.Source, &evt); err != nil {
			log.Debugf("failed to unmarshal elasticsearch result, err: %+v", err)
			continue
		}
		evts = append(evts, &evt)
	}
	fdr.logger.Infof("GetEvents response: {%+v}", evts)

	return &evtsapi.EventList{TypeMeta: api.TypeMeta{Kind: "Event"}, Items: evts}, nil
}
