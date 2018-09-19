package collectorinteg

import (
	"time"

	"github.com/gogo/protobuf/types"
	ic "github.com/influxdata/influxdb/client/v2"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/test/integ/tsdb/aggutils"
	tec "github.com/pensando/sw/venice/citadel/collector"
	"github.com/pensando/sw/venice/citadel/collector/influxdb"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	log "github.com/pensando/sw/venice/utils/log"
)

// Suite defines a collector testbed with backends
type Suite struct {
	backends aggutils.Server
	col      *tec.Collector
	rpcSrv   *rpcserver.CollRPCSrv
	clients  ic.Client
}

// NewSuite sets up influx backends, collector and rpc
func NewSuite(url string) *Suite {
	be := aggutils.OpenDefaultServer(aggutils.NewConfig())
	log.Infof("use influx {%+v} ", be.URL())
	cl, err := influxclient.NewInfluxClient(&influxclient.InfluxConfig{
		Addr: be.URL(),
	})
	if err != nil {
		log.Fatal(err)
	}

	c := tec.NewCollector(cl)
	s := &Suite{
		col: c,
	}

	s.backends = be
	client, err := ic.NewHTTPClient(ic.HTTPConfig{
		Addr: be.URL(),
	})

	if err != nil {
		log.Fatal(err)
	}
	s.clients = client

	srv, err := rpcserver.NewCollRPCSrv(url, c)
	if err != nil {
		log.Fatal(err)
	}

	s.rpcSrv = srv
	return s
}

// CollectorURL gets the collectors grpc url
func (s *Suite) CollectorURL() string {
	return s.rpcSrv.URL()
}

// BackendURLs returns the URLs of the influx backends
func (s *Suite) BackendURLs() []string {
	return []string{s.backends.URL()}
}

// CreateDB creates a database on the backend
func (s *Suite) CreateDB(db string) error {
	_, err := s.backends.CreateDatabase(db)
	if err != nil {
		return err
	}

	return nil
}

// Backends returns the influx backends
func (s *Suite) Backends() aggutils.Server {
	return s.backends
}

// Clients returns the influx clients
func (s *Suite) Clients() ic.Client {
	return s.clients
}

// Query queries the specifed be
func (s *Suite) Query(db, cmd string) ([]ic.Result, error) {

	q := ic.Query{
		Command:  cmd,
		Database: db,
	}
	response, err := s.clients.Query(q)
	if err != nil {
		return nil, err
	}
	if response.Error() != nil {
		return nil, response.Error()
	}

	return response.Results, nil
}

// TearDown stops all servers
func (s *Suite) TearDown() {
	s.rpcSrv.Stop()
	s.backends.Close()
	s.clients.Close()
}

// GetMetricPoint creates a MetricPoint struct
func GetMetricPoint(name string, tags map[string]string, fields map[string]interface{}, ts time.Time) *metric.MetricPoint {
	f := make(map[string]*metric.Field, len(fields))

	for k, v := range fields {
		switch v.(type) {
		case int64:
			f[k] = &metric.Field{
				F: &metric.Field_Int64{
					Int64: v.(int64),
				},
			}
		case int:
			f[k] = &metric.Field{
				F: &metric.Field_Int64{
					Int64: int64(v.(int)),
				},
			}
		case float64:
			f[k] = &metric.Field{
				F: &metric.Field_Float64{
					Float64: v.(float64),
				},
			}
		case string:
			f[k] = &metric.Field{
				F: &metric.Field_String_{
					String_: v.(string),
				},
			}
		case bool:
			f[k] = &metric.Field{
				F: &metric.Field_Bool{
					Bool: v.(bool),
				},
			}

		default:
			return nil
		}
	}

	tt, _ := types.TimestampProto(ts)
	mp := &metric.MetricPoint{
		Name:   name,
		Tags:   tags,
		Fields: f,
		When: &api.Timestamp{
			Timestamp: *tt,
		},
	}

	return mp
}
