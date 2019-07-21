// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/telemetryclient"
)

// TelemetryClient returns an array of telemetry clients for each venice node
func (tb *TestBed) TelemetryClient() ([]*telemetryclient.TelemetryClient, error) {
	// if we are already connected, just return the client
	if tb.telemetryClient != nil {
		return tb.telemetryClient, nil
	}

	var telemecls []*telemetryclient.TelemetryClient
	for _, url := range tb.GetVeniceURL() {
		// connect to Venice
		tmc, err := telemetryclient.NewTelemetryClient(url)
		if err != nil {
			log.Errorf("Error connecting to Venice %v. Err: %v", url, err)
			return nil, err
		}

		telemecls = append(telemecls, tmc)
	}
	tb.telemetryClient = telemecls

	return telemecls, nil
}

// QueryFwlog queries firewall log
func (tb *TestBed) QueryFwlog(protocol, fwaction, timestr string, port uint32) (*telemetry_query.FwlogsQueryResponse, error) {
	// validate parameters
	_, ok := telemetry_query.FwlogActions_value[fwaction]
	if !ok {
		log.Errorf("Invalid firewall action %s", fwaction)
		return nil, fmt.Errorf("Invalid fwaction")
	}
	stime := &api.Timestamp{}
	stime.Parse(timestr)

	// build the query
	query := telemetry_query.FwlogsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.FwlogsQuerySpec{
			{
				Protocols: []string{protocol},
				DestPorts: []uint32{port},
				Actions:   []string{fwaction},
				StartTime: stime,
			},
		},
	}

	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}

	// telemetry client
	tmcs, err := tb.TelemetryClient()
	if err != nil {
		return nil, err
	}

	var result *telemetry_query.FwlogsQueryResponse
	for _, tmc := range tmcs {
		result, err = tmc.Fwlogs(ctx, &query)
		if err == nil {
			break
		}
	}

	return result, err
}

// QueryMetrics queries venice metrics
func (tb *TestBed) QueryMetrics(kind, name, timestr string, count int32) (*telemetryclient.MetricsQueryResponse, error) {
	ctx, err := tb.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}

	// telemetry client
	tmcs, err := tb.TelemetryClient()
	if err != nil {
		return nil, err
	}

	stime := &api.Timestamp{}
	stime.Parse(timestr)

	// build the query
	query := &telemetry_query.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: kind,
				},
				Selector: &fields.Selector{
					Requirements: []*fields.Requirement{
						{
							Key:    "name",
							Values: []string{name},
						},
					},
				},
				// Name: name,
				StartTime:    stime,
				GroupbyField: "reporterID",
				SortOrder:    "descending",
				Pagination: &telemetry_query.PaginationSpec{
					Count: count,
				},
			},
		},
	}

	log.Debugf("Sending metrics query: %+v", query.Queries[0])

	var result *telemetryclient.MetricsQueryResponse
	for _, tmc := range tmcs {
		result, err = tmc.Metrics(ctx, query)
		if err == nil {
			break
		}
	}

	return result, err
}
