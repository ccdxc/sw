package metricsclient

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"

	"github.com/pensando/sw/api"
	metrics_query "github.com/pensando/sw/api/generated/metrics_query"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// QueryResponse is the response send out
// The protobuf generated type for the values is
// []*api.InterfaceSlice which does not match
// the actual response type of [][]interface{}.
// We manually create the correct type here.
type QueryResponse struct {
	//
	Results []*QueryResult
}

// QueryResult contains tsdb series from citadel query
type QueryResult struct {
	//
	StatementID int32
	//
	Series []*ResultSeries
}

// ResultSeries contains the values of a series
type ResultSeries struct {
	// Name of the series
	Name string
	// Tags are the TSDB tags in the query response
	Tags map[string]string
	// columns list all available fields in tsdb
	Columns []string
	// values contain field values received frpm tsdb, it is in the form of [][]interface{}
	Values [][]interface{}
}

// MetricsClient is the API Client for metric queries
type MetricsClient struct {
	logger   log.Logger
	client   *http.Client
	instance string
}

// NewMetricsClient returns a new metrics client
func NewMetricsClient(instance string) (*MetricsClient, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "http://" + instance
	}
	return &MetricsClient{
		instance: instance,
		client:   http.DefaultClient,
	}, nil
}

// Query makes a metrics query and returns the Query Response
func (r *MetricsClient) Query(ctx context.Context, qs *metrics_query.QuerySpec) (*QueryResponse, error) {
	path := "/metrics/v1/query"
	req, err := r.getHTTPRequest(ctx, qs, "POST", path)
	if err != nil {
		return nil, err
	}
	httpresp, err := r.client.Do(req.WithContext(ctx))

	if err != nil {
		return nil, fmt.Errorf("request failed (%s)", err)
	}
	defer httpresp.Body.Close()

	ret, err := decodeHTTPQueryResponse(ctx, httpresp)

	if err != nil {
		return nil, err
	}
	return ret, nil
}

func (r *MetricsClient) getHTTPRequest(ctx context.Context, qs interface{}, method, path string) (*http.Request, error) {

	req, err := netutils.CreateHTTPRequest(r.instance, qs, method, path)
	if err != nil {
		return nil, err
	}
	val, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		req.Header.Add("Authorization", val)
	}
	return req, nil
}

func decodeHTTPQueryResponse(_ context.Context, r *http.Response) (*QueryResponse, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp QueryResponse
	err := json.NewDecoder(r.Body).Decode(&resp)
	return &resp, err
}

func errorDecoder(r *http.Response) error {
	var w api.Status
	if err := json.NewDecoder(r.Body).Decode(&w); err != nil {
		return fmt.Errorf("Status:(%v) Reason:(%s)", r.StatusCode, r.Status)
	}
	return fmt.Errorf("Status:(%v) Result:(%v) Reason(%v)", r.StatusCode, w.Result, w.Message)
}
