package telemetryclient

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"reflect"
	"testing"

	telemetry_query "github.com/pensando/sw/api/generated/telemetry_query"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func decodeHTTPMetricsQueryList(r *http.Request) (*telemetry_query.MetricsQueryList, error) {
	var req telemetry_query.MetricsQueryList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return &req, nil
}

func decodeHTTPFwlogsQueryList(r *http.Request) (*telemetry_query.FwlogsQueryList, error) {
	var req telemetry_query.FwlogsQueryList
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return &req, nil
}

func TestMetrics(t *testing.T) {
	sampleQueryResponse := &MetricsQueryResponse{
		Results: []*MetricsQueryResult{
			&MetricsQueryResult{
				StatementID: 2,
				Series: []*MetricsResultSeries{
					{
						Values: [][]interface{}{
							[]interface{}{"timestamp", 2.0},
						},
					},
				},
			},
		},
	}

	cases := []struct {
		name          string
		queryList     *telemetry_query.MetricsQueryList
		queryResponse *MetricsQueryResponse
		headerCode    int
		result        bool
	}{
		{
			name: "Field Query [pass]",
			queryList: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						Fields: []string{"Field1"},
					},
				},
			},
			queryResponse: sampleQueryResponse,
			headerCode:    200,
			result:        true,
		},
		{
			name: "Invalid Query [fail]",
			queryList: &telemetry_query.MetricsQueryList{
				Queries: []*telemetry_query.MetricsQuerySpec{
					&telemetry_query.MetricsQuerySpec{
						Fields: []string{"Field1"},
					},
				},
			},
			queryResponse: sampleQueryResponse,
			headerCode:    400,
			result:        false,
		},
	}

	testCaseNumber := 0

	ts := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		queryList, err := decodeHTTPMetricsQueryList(r)
		if err != nil {
			t.Errorf("Failed to decode request %s", err)
		}
		c := cases[testCaseNumber]
		Assert(t, reflect.DeepEqual(c.queryList, queryList),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", c.queryList, queryList, c.name))

		// Sending response back
		w.WriteHeader(c.headerCode)
		err = json.NewEncoder(w).Encode(*c.queryResponse)
		if err != nil {
			t.Errorf("Failed to encode response, %s", err)
		}
	}))
	defer ts.Close()

	client, err := NewTelemetryClient(ts.URL)
	if err != nil {
		t.Fatalf("Failed to create metrics client")
	}
	ctx := context.TODO()

	for index, c := range cases {
		testCaseNumber = index
		t.Logf(" -> Test [ %s ]", c.name)
		resp, err := client.Metrics(ctx, c.queryList)
		if err != nil && c.result {
			t.Errorf("   ** [%s] Query should have passed, [%s] test failed", c.name, err)
		}
		if err == nil && !c.result {
			t.Errorf("   ** [%s] Query should have failed but passed", c.name)
		}
		if err == nil {
			Assert(t, reflect.DeepEqual(
				c.queryResponse, resp),
				fmt.Sprintf("expected returned object %v, got %v, [%s] test failed", c.queryResponse, resp, c.name))
		}
	}
}

func TestFwlogs(t *testing.T) {
	sampleQueryResponse := &telemetry_query.FwlogsQueryResponse{
		Results: []*telemetry_query.FwlogsQueryResult{
			&telemetry_query.FwlogsQueryResult{
				StatementID: 2,
				Logs: []*telemetry_query.Fwlog{
					&telemetry_query.Fwlog{Src: "10.1.1.1"},
				},
			},
		},
	}

	cases := []struct {
		name          string
		queryList     *telemetry_query.FwlogsQueryList
		queryResponse *telemetry_query.FwlogsQueryResponse
		headerCode    int
		result        bool
	}{
		{
			name: "Field Query [pass]",
			queryList: &telemetry_query.FwlogsQueryList{
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
					},
				},
			},
			queryResponse: sampleQueryResponse,
			headerCode:    200,
			result:        true,
		},
		{
			name: "Invalid Query [fail]",
			queryList: &telemetry_query.FwlogsQueryList{
				Queries: []*telemetry_query.FwlogsQuerySpec{
					&telemetry_query.FwlogsQuerySpec{
						SourceIPs: []string{"10.1.1.1"},
					},
				},
			},
			queryResponse: sampleQueryResponse,
			headerCode:    400,
			result:        false,
		},
	}

	testCaseNumber := 0

	ts := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		queryList, err := decodeHTTPFwlogsQueryList(r)
		if err != nil {
			t.Errorf("Failed to decode request %s", err)
		}
		c := cases[testCaseNumber]
		Assert(t, reflect.DeepEqual(c.queryList, queryList),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", c.queryList, queryList, c.name))

		// Sending response back
		w.WriteHeader(c.headerCode)
		err = json.NewEncoder(w).Encode(*c.queryResponse)
		if err != nil {
			t.Errorf("Failed to encode response, %s", err)
		}
	}))
	defer ts.Close()

	client, err := NewTelemetryClient(ts.URL)
	if err != nil {
		t.Fatalf("Failed to create metrics client")
	}
	ctx := context.TODO()

	for index, c := range cases {
		testCaseNumber = index
		t.Logf(" -> Test [ %s ]", c.name)
		resp, err := client.Fwlogs(ctx, c.queryList)
		if err != nil && c.result {
			t.Errorf("   ** [%s] Query should have passed, [%s] test failed", c.name, err)
		}
		if err == nil && !c.result {
			t.Errorf("   ** [%s] Query should have failed but passed", c.name)
		}
		if err == nil {
			Assert(t, reflect.DeepEqual(
				c.queryResponse, resp),
				fmt.Sprintf("expected returned object %v, got %v, [%s] test failed", c.queryResponse, resp, c.name))
		}
	}
}
