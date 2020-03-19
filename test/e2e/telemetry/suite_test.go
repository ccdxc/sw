package telemetry

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/kvstore/store"

	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/meta"

	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"google.golang.org/grpc"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	cinteg "github.com/pensando/sw/test/integ/tsdb/collector"
	stypes "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

var configFile = flag.String("configFile", "./tb_config.json", "Path to JSON Config file")

type teleSuite struct {
	//simClients []halproto.InternalClient
	conns  []*grpc.ClientConn
	broker *broker.Broker
	tt     *cinteg.TimeTable
}

type tbConfig struct {
	NumVeniceNodes int
	NumNaplesHosts int
	FirstVeniceIP  string
	FirstNaplesIP  string
	FirstSimPort   int
	NumSims        int
	Scheme         string
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

	file, err := ioutil.ReadFile(*configFile)
	if err != nil {
		return nil, err
	}
	err = json.Unmarshal(file, &cfg)
	if err != nil {
		return nil, err
	}

	citadelHost, err := getCitadelURLs(cfg.Scheme)
	if err != nil {
		return nil, err
	}

	cluster := meta.DefaultClusterConfig()
	cluster.MetastoreType = store.KVStoreTypeEtcd
	br, err := broker.NewBroker(cluster, citadelHost, log.GetNewLogger(log.GetDefaultConfig("telemetry_suite")))
	if err != nil {
		return nil, err
	}

	//sims, conns, err := getSimClients(&cfg)
	//if err != nil {
	//	return nil, err
	//}

	return &teleSuite{
		//simClients: sims,
		conns:  nil,
		broker: br,
		tt:     cinteg.NewTimeTable("firewall"),
	}, nil
}

func getCitadelURLs(scheme string) (string, error) {
	var result stypes.ServiceList

	for i := 0; i < 20; i++ {
		err := netutils.HTTPGet(scheme+"node1:"+globals.CMDRESTPort+"/api/v1/services", &result)
		if err != nil {
			log.Errorf("%v", err)
			return "", err
		}

		for _, svc := range result.Items {
			if svc.Name == globals.Citadel {
				for _, inst := range svc.Instances {
					h := strings.Split(inst.URL, ":")[0]
					return h, nil
				}
			}
		}

		log.Infof("citadel service not ready, Retrying...")
		time.Sleep(time.Second)
	}

	return "", fmt.Errorf("no citadel service found")
}

//func getSimClients(cfg *tbConfig) ([]halproto.InternalClient, []*grpc.ClientConn, error) {
//	var sims []halproto.InternalClient
//	var conns []*grpc.ClientConn
//	var err error
//
//	naplesIP := net.ParseIP(cfg.FirstNaplesIP).To4()
//	if naplesIP == nil {
//		return nil, nil, fmt.Errorf("Bad IP %v", cfg.FirstNaplesIP)
//	}
//
//	var opts []grpc.DialOption
//	opts = append(opts, grpc.WithInsecure())
//	appendor := func(a string) error {
//		conn, err := grpc.Dial(a, opts...)
//		if err != nil {
//			return err
//		}
//
//		conns = append(conns, conn)
//		sims = append(sims, halproto.NewInternalClient(conn))
//		return nil
//	}
//
//	numNaplesNodes := cfg.NumNaplesHosts
//	if numNaplesNodes > 200 {
//		// limit to allow the following logic
//		numNaplesNodes = 200
//	}
//
//	for ix := 0; ix < numNaplesNodes; ix++ {
//		naplesIP[3] += byte(ix)
//		port := cfg.FirstSimPort
//		addr := fmt.Sprintf("%s:%d", naplesIP.String(), port)
//		err = appendor(addr)
//		if err != nil {
//			return nil, nil, err
//		}
//	}
//
//	return sims, conns, nil
//}

func (ts *teleSuite) DbQuery(ctx context.Context, db, cmd string) (models.Rows, error) {
	response, err := ts.broker.ExecuteQuery(ctx, db, cmd)
	if err != nil {
		return nil, err
	}

	return response[0].Series, nil
}

func (ts *teleSuite) CreateDB(ctx context.Context, db string) error {
	return ts.broker.CreateDatabase(ctx, db)
}

func (ts *teleSuite) DeleteDB(ctx context.Context, db string) error {
	//if err := ts.broker.DeleteDatabase(ctx, db); err != nil {
	//	return err
	//	}

	return nil
}

func (ts *teleSuite) InjectLogs(count int) error {
	//ctx := context.Background()
	stamp := time.Now()
	twoms, err := time.ParseDuration("5ms")
	if err != nil {
		return err
	}
	//simCount := len(ts.simClients)
	//if simCount == 0 {
	//	return fmt.Errorf("No sim clients")
	//}
	for ix := 0; ix < count; ix++ {
		fwe := &halproto.FWEvent{
			Flowaction: halproto.FlowLogEventType(ix % 2),
			IpVer:      4,
			Sipv4:      uint32(rand.Intn(0xc0ffffff)),
			Dipv4:      uint32(rand.Intn(0xc0ffffff)),
			Sport:      uint32(rand.Intn(60000)),
			Dport:      uint32(10000 + ix),
			Timestamp:  stamp.UnixNano(),
		}

		//m := &halproto.LogFlowRequestMsg{
		//	Request: []*halproto.FWEvent{fwe},
		//}

		//clientID := ix % simCount
		//_, err := ts.simClients[clientID].LogFlow(ctx, m)
		//if err != nil {
		//	return errors.Wrapf(err, "simCount: %d, clientID: %d", simCount, clientID)
		//}

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
	ts.tt.AddRow(cinteg.InfluxTS(stamp, time.Nanosecond), tags, fields)
}

func (ts *teleSuite) VerifyQueryAll(db, meas string) error {
	ctx := context.Background()
	res, err := ts.DbQuery(ctx, db, "SELECT * FROM "+meas)
	if err != nil {
		return err
	}

	return ts.tt.MatchQueryRow(res[0])
}

func (ts *teleSuite) CountPoints(db, meas string) (int, error) {
	count := 0
	res, err := ts.DbQuery(context.Background(), db, "SELECT * FROM "+meas)
	if err != nil {
		return 0, err
	}

	for _, i := range res {
		count += len(i.Values)
	}

	return count, nil
}
