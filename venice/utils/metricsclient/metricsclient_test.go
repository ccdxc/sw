package metricsclient

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"net/http/httptest"
	"reflect"
	"testing"

	metrics_query "github.com/pensando/sw/api/generated/metrics_query"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func decodeHTTPQuerySpec(r *http.Request) (*metrics_query.QuerySpec, error) {
	var req metrics_query.QuerySpec
	if e := json.NewDecoder(r.Body).Decode(&req); e != nil {
		return nil, e
	}
	return &req, nil
}

func TestQuery(t *testing.T) {
	sampleQueryResponse := &QueryResponse{
		Results: []*QueryResult{
			&QueryResult{
				StatementID: 2,
				Series: []*ResultSeries{
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
		querySpec     *metrics_query.QuerySpec
		queryResponse *QueryResponse
		headerCode    int
		result        bool
	}{
		{
			name: "Field Query [pass]",
			querySpec: &metrics_query.QuerySpec{
				Fields: []string{"Field1"},
			},
			queryResponse: sampleQueryResponse,
			headerCode:    200,
			result:        true,
		},
		{
			name: "Invalid Query [fail]",
			querySpec: &metrics_query.QuerySpec{
				Fields: []string{"Field1"},
			},
			queryResponse: sampleQueryResponse,
			headerCode:    400,
			result:        false,
		},
	}

	testCaseNumber := 0

	ts := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		querySpec, err := decodeHTTPQuerySpec(r)
		if err != nil {
			t.Errorf("Failed to decode request %s", err)
		}
		c := cases[testCaseNumber]
		Assert(t, reflect.DeepEqual(c.querySpec, querySpec),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", c.querySpec, querySpec, c.name))

		// Sending response back
		w.WriteHeader(c.headerCode)
		err = json.NewEncoder(w).Encode(*c.queryResponse)
		if err != nil {
			t.Errorf("Failed to encode response, %s", err)
		}
	}))
	defer ts.Close()

	client, err := NewMetricsClient(ts.URL)
	if err != nil {
		t.Fatalf("Failed to create metrics client")
	}
	ctx := context.TODO()

	for index, c := range cases {
		testCaseNumber = index
		t.Logf(" -> Test [ %s ]", c.name)
		resp, err := client.Query(ctx, c.querySpec)
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
