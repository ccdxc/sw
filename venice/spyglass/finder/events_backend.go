package finder

import (
	"context"
	"encoding/json"
	"strings"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils"
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
		log.Errorf("failed to query elasticsearch, err: %+v", err)
		return nil, status.Error(codes.Internal, "could not get the event")
	}

	// parse the result
	if result.TotalHits() == 0 {
		return nil, status.Errorf(codes.NotFound, "event not found")
	}

	res := evtsapi.Event{}
	if err := json.Unmarshal(*result.Hits.Hits[0].Source, &res); err != nil {
		log.Errorf("failed to unmarshal elasticsearch result, err: %+v", err)
		return nil, status.Errorf(codes.Internal, "could not get the event")
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

	// parse field selector;
	// field selector only supports "=", "!=", " in ", " notin " operators now
	fSelector := r.GetFieldSelector()
	if !utils.IsEmpty(fSelector) {
		// TODO: change it to parse with validation once the json inline issue is addressed
		fldSelectors, err := fields.Parse(fSelector)
		if err != nil {
			log.Errorf("failed to parse the field selector, %v, err: %+v", r.GetFieldSelector(), err)
			return nil, status.Error(codes.Internal, "could not get the events")
		}

		// TODO: validate requirements
		for _, req := range fldSelectors.GetRequirements() {
			switch req.GetOperator() {
			case fields.Operator_equals.String():
				query = query.Must(es.NewMatchPhraseQuery(req.GetKey(), strings.Join(req.GetValues(), "")))
			case fields.Operator_notEquals.String():
				query = query.MustNot(es.NewMatchPhraseQuery(req.GetKey(), strings.Join(req.GetValues(), "")))
			case fields.Operator_in.String(): // should match atleast one of the value
				query = query.MinimumNumberShouldMatch(1)
				for _, val := range req.GetValues() {
					query = query.Should(es.NewMatchPhraseQuery(req.GetKey(), val))
				}
			case fields.Operator_notIn.String(): // must not match any of the values
				for _, val := range req.GetValues() {
					query = query.MustNot(es.NewMatchPhraseQuery(req.GetKey(), val))
				}
			}
		}
	}

	// TODO: handle label selector; currently, events do not carry any labels.
	// Once we finalize on what goes in the label, implement this

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

	return &evtsapi.EventList{TypeMeta: api.TypeMeta{Kind: "Event"}, Items: evts}, nil
}
