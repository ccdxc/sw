package telemetry

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"math/rand"
	"net"
	"os"
	"reflect"
	"testing"
	"time"

	influx "github.com/influxdata/influxdb/client/v2"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"google.golang.org/grpc"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	cinteg "github.com/pensando/sw/test/integ/tsdb/collector"
	stypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

var configFile string

type teleSuite struct {
	simClients []halproto.FWLogSimClient
	conns      []*grpc.ClientConn
	infClient  influx.Client
	tt         *cinteg.TimeTable
}

type qResult struct {
	results []influx.Result
}

type tbConfig struct {
	NumVeniceNodes int
	NumNaplesHosts int
	FirstVeniceIP  string
	FirstSimPort   int
	NumSims        int
	Scheme         string
}

func init() {
	cfgFile := flag.String("configFile", "./tb_config.json", "Path to JSON Config file")
	flag.Parse()
	configFile = *cfgFile
}
func TestTM(t *testing.T) {
	if os.Getenv("E2E_TEST") == "" {
		return
	}

	RegisterFailHandler(Fail)
	RunSpecs(t, "Telemetry Suite")
}

func newTS() (*teleSuite, error) {
	var cfg tbConfig

	file, err := ioutil.ReadFile(configFile)
	if err != nil {
		return nil, err
	}
	err = json.Unmarshal(file, &cfg)
	if err != nil {
		return nil, err
	}

	infURL, err := getInfluxURL(cfg.Scheme)
	if err != nil {
		return nil, err
	}

	hc := influx.HTTPConfig{
		Addr: cfg.Scheme + infURL,
	}

	ic, err := influx.NewHTTPClient(hc)
	if err != nil {
		return nil, err
	}

	sims, conns, err := getSimClients(&cfg)
	if err != nil {
		return nil, err
	}

	return &teleSuite{
		simClients: sims,
		conns:      conns,
		infClient:  ic,
		tt:         cinteg.NewTimeTable("firewall"),
	}, nil
}

// getInfluxURL fetches the influx backends from resolver
// TODO: migrate when query api is available
func getInfluxURL(scheme string) (string, error) {
	for ix := 0; ix < 10; ix++ {
		icList := getInfluxURLs(scheme + "node1:" + globals.CMDRESTPort + "/api/v1/services")
		if len(icList) > 0 {
			return icList[0], nil
		}

		time.Sleep(5 * time.Second)
		log.Infof("No Influx service found. Retrying...")
	}

	return "", fmt.Errorf("no influx service found")
}

func getInfluxURLs(cmdAddr string) []string {
	var result stypes.ServiceList
	var urls []string
	err := netutils.HTTPGet(cmdAddr, &result)
	if err != nil {
		log.Errorf("%v", err)
		return urls
	}

	for _, svc := range result.Items {
		if svc.Name == "pen-influx" {
			for _, inst := range svc.Instances {
				urls = append(urls, inst.URL)
			}
		} else {
			log.Infof("svc name: %s", svc.Name)
		}

	}
	return urls
}

func getSimClients(cfg *tbConfig) ([]halproto.FWLogSimClient, []*grpc.ClientConn, error) {
	var sims []halproto.FWLogSimClient
	var conns []*grpc.ClientConn
	var err error

	naplesIP := net.ParseIP(cfg.FirstVeniceIP).To4()
	if naplesIP == nil {
		return nil, nil, fmt.Errorf("Bad IP %v", cfg.FirstVeniceIP)
	}
	naplesIP[3] += byte(cfg.NumVeniceNodes)

	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	appendor := func(a string) error {
		conn, err := grpc.Dial(a, opts...)
		if err != nil {
			return err
		}

		conns = append(conns, conn)
		sims = append(sims, halproto.NewFWLogSimClient(conn))
		return nil
	}

	numNaplesNodes := cfg.NumNaplesHosts
	if numNaplesNodes > 200 {
		// limit to allow the following logic
		numNaplesNodes = 200
	}

	for ix := 0; ix < numNaplesNodes; ix++ {
		naplesIP[3] += byte(ix)
		port := cfg.FirstSimPort
		addr := fmt.Sprintf("%s:%d", naplesIP.String(), port)
		err = appendor(addr)
		if err != nil {
			return nil, nil, err
		}
	}

	return sims, conns, nil
}

func (ts *teleSuite) InfluxQuery(db, cmd string) ([]influx.Result, error) {
	var res []influx.Result

	q := influx.Query{
		Command:  cmd,
		Database: db,
	}
	if response, err := ts.infClient.Query(q); err == nil {
		if response.Error() != nil {
			return res, response.Error()
		}
		res = response.Results
	} else {
		return res, err
	}
	return res, nil
}

func (ts *teleSuite) CreateDB(db string) error {
	_, err := ts.InfluxQuery("", fmt.Sprintf("CREATE DATABASE %s", db))
	if err != nil {
		return err
	}

	// verify the database was created
	res, err := ts.InfluxQuery("", "SHOW DATABASES")
	if err != nil {
		return err
	}

	log.Infof("CreateDB: %+v", res)
	qr := &qResult{results: res}
	return qr.mustHaveValue(db)
}

func (ts *teleSuite) DeleteDB(db string) error {
	_, err := ts.InfluxQuery("", fmt.Sprintf("DROP DATABASE %s", db))
	return err
}

func (ts *teleSuite) InjectLogs(count int) error {
	ctx := context.Background()
	stamp := time.Now()
	twoms, err := time.ParseDuration("2ms")
	if err != nil {
		return err
	}
	simCount := len(ts.simClients)
	if simCount == 0 {
		return fmt.Errorf("No sim clients")
	}
	for ix := 0; ix < count; ix++ {
		fwe := &halproto.FWEvent{
			Flowaction: halproto.FlowLogEventType(ix % 2),
			IpVer:      4,
			Sipv4:      uint32(rand.Intn(0xc0ffffff)),
			Dipv4:      uint32(rand.Intn(0xc0ffffff)),
			Sport:      uint32(rand.Intn(60000)),
			Dport:      uint32(rand.Intn(20000)),
			Timestamp:  stamp.UnixNano(),
		}

		m := &halproto.LogFlowRequestMsg{
			Request: []*halproto.FWEvent{fwe},
		}

		clientID := ix % simCount
		_, err := ts.simClients[clientID].LogFlow(ctx, m)
		if err != nil {
			return errors.Wrapf(err, "simCount: %d, clientID: %d", simCount, clientID)
		}

		ts.addTimeTable(fwe)
		stamp = stamp.Add(twoms)
	}
	return nil
}

func (ts *teleSuite) addTimeTable(fwe *halproto.FWEvent) {
	ipSrc := netutils.IPv4Uint32ToString(fwe.Sipv4)
	ipDest := netutils.IPv4Uint32ToString(fwe.Dipv4)
	dPort := fmt.Sprintf("%v", fwe.Dport)
	ipProt := fmt.Sprintf("%v", fwe.IpProt)
	action := fmt.Sprintf("%v", fwe.Flowaction)
	dir := fmt.Sprintf("%v", fwe.Direction)
	stamp := time.Unix(0, fwe.Timestamp)
	tags := map[string]string{"src": ipSrc, "dest": ipDest, "dPort": dPort, "ipProt": ipProt, "action": action, "direction": dir}
	fields := map[string]interface{}{"sPort": int64(fwe.Sport)}
	ts.tt.AddRow(cinteg.InfluxTS(stamp, time.Millisecond), tags, fields)
}

func (ts *teleSuite) VerifyQueryAll(db, meas string) error {
	res, err := ts.InfluxQuery(db, "SELECT * FROM "+meas)
	if err != nil {
		return err
	}

	return ts.tt.MatchQueryRow(res[0].Series[0])
}

func (ts *teleSuite) CountPoints(db, meas string) (int, error) {
	res, err := ts.InfluxQuery(db, "SELECT * FROM "+meas)
	if err != nil {
		return 0, err
	}
	qr := &qResult{results: res}
	return qr.countRows(), nil
}

func (qr *qResult) mustHaveValue(v interface{}) error {
	for _, res := range qr.results {
		for _, s := range res.Series {
			for _, r := range s.Values {
				for _, c := range r {
					if reflect.DeepEqual(c, v) {
						log.Infof("found %v", c)
						return nil
					}
				}
			}
		}
	}

	log.Infof("DB not found")
	return fmt.Errorf("%v Not found", v)
}

func (qr *qResult) countRows() int {
	var count int

	for _, res := range qr.results {
		for _, s := range res.Series {
			count += len(s.Values)
		}
	}

	return count
}
