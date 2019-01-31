package telemetryclient

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"

	"github.com/pensando/sw/api"
	telemetry_query "github.com/pensando/sw/api/generated/telemetry_query"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// MetricsQueryResponse is the response send out
// The protobuf generated type for the values is
// []*api.InterfaceSlice which does not match
// the actual response type of [][]interface{}.
// We manually create the correct type here.
type MetricsQueryResponse struct {
	//
	Results []*MetricsQueryResult
}

// MetricsQueryResult contains tsdb series from citadel query
type MetricsQueryResult struct {
	//
	StatementID int32
	//
	Series []*MetricsResultSeries
}

// MetricsResultSeries contains the values of a series
type MetricsResultSeries struct {
	// Name of the series
	Name string
	// Tags are the TSDB tags in the query response
	Tags map[string]string
	// columns list all available fields in tsdb
	Columns []string
	// values contain field values received frpm tsdb, it is in the form of [][]interface{}
	Values [][]interface{}
}

// TelemetryClient is the API Client for telemetry queries
type TelemetryClient struct {
	logger   log.Logger
	client   *http.Client
	instance string
}

// NewTelemetryClient returns a new telemetry client
func NewTelemetryClient(instance string) (*TelemetryClient, error) {
	if !strings.HasPrefix(instance, "http") {
		instance = "https://" + instance
	}
	return &TelemetryClient{
		instance: instance,
		client: &http.Client{
			Transport: &http.Transport{
				TLSClientConfig: &tls.Config{
					InsecureSkipVerify: true,
				},
			},
		},
	}, nil
}

// Metrics makes a metrics query and returns the Query Response
func (r *TelemetryClient) Metrics(ctx context.Context, ql *telemetry_query.MetricsQueryList) (*MetricsQueryResponse, error) {
	path := "/telemetry/v1/metrics"
	req, err := r.getHTTPRequest(ctx, ql, "POST", path)
	if err != nil {
		return nil, err
	}
	httpresp, err := r.client.Do(req.WithContext(ctx))

	if err != nil {
		return nil, fmt.Errorf("request failed (%s)", err)
	}
	defer httpresp.Body.Close()

	ret, err := decodeHTTPMetricsQueryResponse(ctx, httpresp)

	if err != nil {
		return nil, err
	}
	return ret, nil
}

func (r *TelemetryClient) getHTTPRequest(ctx context.Context, qs interface{}, method, path string) (*http.Request, error) {

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

func decodeHTTPMetricsQueryResponse(_ context.Context, r *http.Response) (*MetricsQueryResponse, error) {
	if r.StatusCode != http.StatusOK {
		return nil, errorDecoder(r)
	}
	var resp MetricsQueryResponse
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
