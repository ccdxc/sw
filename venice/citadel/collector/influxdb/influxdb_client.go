package influxclient

import (
	"crypto/tls"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"sync"

	influx "github.com/influxdata/influxdb/client/v2"

	"context"
	"time"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/venice/citadel/collector/types"
	"github.com/pensando/sw/venice/utils/runtime"
)

// InfluxConfig contains URL/username/password to access influx DB
type InfluxConfig struct {
	Addr     string
	Username string
	Password string
	Timeout  time.Duration
}

// Implementation of Influx backend for the collector
type influxClient struct {
	sync.WaitGroup
	influxClient influx.Client
	URL          *url.URL
	httpClient   *http.Client
}

// NewInfluxClient returns an instance of the influx backend
func NewInfluxClient(conf *InfluxConfig) (types.TSDBIntf, error) {
	cl := &influxClient{}

	hc := influx.HTTPConfig{
		Addr:     conf.Addr,
		Username: conf.Username,
		Password: conf.Password,
		Timeout:  conf.Timeout,
	}

	c, err := influx.NewHTTPClient(hc)
	if err != nil {
		return nil, err
	}
	cl.influxClient = c

	cl.URL, _ = url.Parse(conf.Addr)
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{},
	}
	cl.httpClient = &http.Client{
		Timeout:   conf.Timeout,
		Transport: tr,
	}

	return cl, nil
}

// Stop stop the client and waits for all pending go routines to finish
func (cl *influxClient) Stop() {
	cl.influxClient.Close()
	cl.Wait()
}

func (cl *influxClient) CreateDatabase(ctx context.Context, database string) error {
	return nil
}

func (cl *influxClient) ExecuteQuery(ctx context.Context, database string, qry string) ([]*query.Result, error) {
	return []*query.Result{}, nil
}

func (cl *influxClient) WritePoints(ctx context.Context, database string, points []models.Point) error {
	// Create a new point batch
	bp, err := influx.NewBatchPoints(influx.BatchPointsConfig{
		Database:  database,
		Precision: "ms",
	})
	if err != nil {
		return err
	}

	// TODO: implement batch processing if we plan to use influx
	for _, p := range points {
		bp.AddPoint(influx.NewPointFrom(p))
	}

	return cl.influxClient.Write(bp)
}

func (cl *influxClient) WriteLines(ctx context.Context, db string, lines []string) error {
	return cl.writeLP(db, lines)
}

func (cl *influxClient) writeLP(db string, lines []string) error {
	var err error
	sr := runtime.NewStringsReader(lines)

	URL := cl.URL
	URL.Path = "write"
	req, err := http.NewRequest("POST", URL.String(), sr)
	if err != nil {
		return err
	}
	req.ContentLength = sr.Size()

	params := req.URL.Query()
	params.Set("db", db)
	params.Set("rp", "")
	params.Set("precision", "ns")
	req.URL.RawQuery = params.Encode()

	resp, err := cl.httpClient.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return err
	}

	if resp.StatusCode != http.StatusNoContent && resp.StatusCode != http.StatusOK {
		return fmt.Errorf(string(body))
	}
	return nil
}
