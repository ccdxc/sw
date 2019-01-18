package main

import (
	"context"
	"flag"
	"fmt"
	"net"
	"path/filepath"
	"strings"
	"time"

	tsdb "github.com/pensando/sw/venice/utils/ntsdb"

	"github.com/pensando/sw/nic/agent/tmagent/state"
	"github.com/pensando/sw/nic/agent/tpa/ctrlerif"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/nodewatcher"

	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
)

// reportInterval is how often(in seconds) tmagent sends metrics to TSDB
const reportInterval = 30

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
}

var srv = &service{
	name: "tmagent",
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
	s.sysmgrClient.InitDone()
}

func (s *service) Name() string {
	return s.name
}

// TelemetryAgent keeps the telementry agent state
type TelemetryAgent struct {
	tpCtrler   *state.PolicyState
	tpClient   *ctrlerif.TpClient
	nodeUUID   string
	restServer *restapi.RestServer
}

func (ta *TelemetryAgent) reportMetrics(ctx context.Context, rc resolver.Interface) error {
	// report node metrics
	node := &cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name: ta.nodeUUID,
		},
	}

	if err := nodewatcher.NewNodeWatcher(ctx, node, rc, reportInterval, log.WithContext("pkg", "nodewatcher")); err != nil {
		return err
	}

	// report delphi metrics
	go ta.restServer.ReportMetrics(reportInterval)
	return nil
}

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		primaryMAC      = flag.String("primary-mac", "", "Primary MAC address")
		hostIf          = flag.String("hostif", "ntrunk0", "Host facing interface")
		mode            = flag.String("mode", "host", "specify the agent mode either host or network")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Tmagent)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		resolverURLs    = flag.String("resolver-urls", ":"+globals.CMDResolverPort, "comma separated list of resolver URLs <IP:Port>")
		restURL         = flag.String("rest-url", ":"+globals.TmAGENTRestPort, "specify Agent REST URL")
	)
	flag.Parse()

	// Fill logger config params
	logConfig := &log.Config{
		Module:      globals.Tmagent,
		Format:      log.JSONFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       *debugflag,
		CtxSelector: log.ContextAll,
		LogToStdout: *logToStdoutFlag,
		LogToFile:   true,
		FileCfg: log.FileConfig{
			Filename:   *logToFile,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	var macAddr net.HardwareAddr

	if *primaryMAC != "" {
		mac, err := net.ParseMAC(*primaryMAC)
		if err != nil {
			log.Fatalf("invalid primary-mac %v", *primaryMAC)
		}
		macAddr = mac
	} else {
		mac, err := netutils.GetIntfMac(*hostIf)
		if err != nil {
			log.Fatalf("Error getting host interface's mac addr. Err: %v", err)
		}
		macAddr = mac
	}

	tmAgent := &TelemetryAgent{
		nodeUUID: macAddr.String(),
	}

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		log.Fatal(err)
	}

	rList := strings.Split(*resolverURLs, ",")
	cfg := &resolver.Config{
		Name:    globals.Tmagent,
		Servers: rList,
	}

	rc := resolver.New(cfg)

	opts := &tsdb.Opts{
		ClientName:              macAddr.String(),
		ResolverClient:          rc,
		Collector:               globals.Collector,
		DBName:                  "default",
		SendInterval:            time.Duration(30) * time.Second,
		ConnectionRetryInterval: 100 * time.Millisecond,
	}

	ctx, cancel := context.WithCancel(context.Background())
	// Init the TSDB
	tsdb.Init(ctx, opts)

	tmAgent.tpCtrler, err = state.NewTpAgent(ctx, globals.AgentRESTPort)
	if err != nil {
		log.Fatalf("failed to init tmagent state, err: %v", err)
	}
	defer tmAgent.tpCtrler.Close()
	defer cancel()

	tmAgent.restServer, err = restapi.NewRestServer(ctx, *restURL)
	if err != nil {
		log.Fatalf("failed to create tmagent rest API server, Err: %v", err)
	}

	if *mode == "network" {

		tmAgent.tpClient, err = ctrlerif.NewTpClient(tmAgent.nodeUUID, tmAgent.tpCtrler, globals.Tpm, rc)
		if err != nil {
			log.Fatalf("failed to init tmagent controller, err: %v", err)
		}

		if err := tmAgent.reportMetrics(ctx, rc); err != nil {
			log.Fatal(err)
		}
	}

	for ix := 0; ix < instCount; ix++ {
		ipc := shm.IPCInstance()
		go ipc.Receive(context.Background(), tmAgent.tpCtrler.ProcessFWEvent)
	}

	delphiClient, err := delphi.NewClient(srv)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	srv.sysmgrClient = sysmgr.NewClient(delphiClient, srv.Name())

	// run delphi thread in background
	go delphiClient.Run()

	// wait forever
	select {}
}
