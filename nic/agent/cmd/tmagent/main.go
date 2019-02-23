package main

import (
	"context"
	"flag"
	"fmt"
	"net"
	"path/filepath"
	"time"

	"github.com/pensando/sw/venice/utils/tsdb"

	"github.com/pensando/sw/nic/agent/tmagent/state"
	"github.com/pensando/sw/nic/agent/tpa/ctrlerif"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/nodewatcher"

	"github.com/pensando/sw/nic/agent/ipc"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	dproto "github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/resolver"
)

// reportInterval is how often(in seconds) tmagent sends metrics to TSDB
const reportInterval = 30

// TelemetryAgent keeps the telementry agent state
type TelemetryAgent struct {
	tpCtrler     *state.PolicyState
	tpClient     *ctrlerif.TpClient
	nodeUUID     string
	resolverURLs string
	mode         string
	restServer   *restapi.RestServer
}

type service struct {
	name         string
	sysmgrClient *sysmgr.Client
	DelphiClient clientApi.Client
	tmagent      *TelemetryAgent
}

func (s *service) OnMountComplete() {
	log.Printf("OnMountComplete() done for %s\n", s.name)
	s.sysmgrClient.InitDone()

	// walk naples status object
	nslist := delphiProto.NaplesStatusList(s.DelphiClient)
	for _, ns := range nslist {
		s.handleVeniceCoordinates(ns)
	}
}

func (s *service) Name() string {
	return s.name
}

// OnNaplesStatusCreate event handler
func (s *service) OnNaplesStatusCreate(obj *delphiProto.NaplesStatus) {
	s.handleVeniceCoordinates(obj)
	return
}

// OnNaplesStatusUpdate event handler
func (s *service) OnNaplesStatusUpdate(old, new *delphiProto.NaplesStatus) {
	s.handleVeniceCoordinates(new)
	return
}

// OnNaplesStatusDelete event handler
func (s *service) OnNaplesStatusDelete(obj *delphiProto.NaplesStatus) {
	return
}

func (s *service) handleVeniceCoordinates(obj *delphiProto.NaplesStatus) {
	log.Infof("Tmagent reactor called with %v", obj)
	if obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND || obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_OOB {
		var controllers []string
		var err error

		for _, ip := range obj.Controllers {
			controllers = append(controllers, fmt.Sprintf("%s:%s", ip, globals.CMDGRPCAuthPort))
		}

		if s.tmagent.tpClient != nil {
			log.Infof("Tpclient already started. ignoring...")
			return
		}

		log.Infof("Populating Venice Co-ordinates with %v", controllers)

		cfg := &resolver.Config{
			Name:    globals.Tmagent,
			Servers: controllers,
		}

		// start tsdb export
		rc := resolver.New(cfg)
		tsdb.Start(rc)

		s.tmagent.tpClient, err = ctrlerif.NewTpClient(s.tmagent.nodeUUID, s.tmagent.tpCtrler, globals.Tpm, rc)
		if err != nil {
			log.Fatalf("failed to init tmagent controller client, err: %v", err)
		}

		// start reporting metrics
		if err := s.tmagent.reportMetrics(context.Background(), rc); err != nil {
			log.Fatal(err)
		}
	}
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
		nodeUUID:     macAddr.String(),
		resolverURLs: *resolverURLs,
		mode:         *mode,
	}

	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	shm, err := ipc.NewSharedMem(mSize, instCount, "/fwlog_ipc_shm")
	if err != nil {
		log.Fatal(err)
	}

	var delphiService = &service{
		name:    "tpmagent_" + macAddr.String(),
		tmagent: tmAgent,
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	opts := &tsdb.Opts{
		ClientName:              delphiService.name,
		Collector:               globals.Collector,
		DBName:                  "default",
		SendInterval:            time.Duration(30) * time.Second,
		ConnectionRetryInterval: 100 * time.Millisecond,
	}
	// Init the TSDB
	tsdb.Init(ctx, opts)

	tmAgent.tpCtrler, err = state.NewTpAgent(ctx, globals.AgentRESTPort)
	if err != nil {
		log.Fatalf("failed to init tmagent state, err: %v", err)
	}
	defer tmAgent.tpCtrler.Close()

	tmAgent.restServer, err = restapi.NewRestServer(ctx, *restURL)
	if err != nil {
		log.Fatalf("failed to create tmagent rest API server, Err: %v", err)
	}

	for ix := 0; ix < instCount; ix++ {
		ipc := shm.IPCInstance()
		go ipc.Receive(context.Background(), tmAgent.tpCtrler.ProcessFWEvent)
	}

	delphiClient, err := delphi.NewClient(delphiService)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	delphiService.DelphiClient = delphiClient
	delphiService.sysmgrClient = sysmgr.NewClient(delphiClient, delphiService.Name())

	// Mount delphi naples status object
	delphiProto.NaplesStatusMount(delphiClient, dproto.MountMode_ReadMode)

	// Set up watches
	delphiProto.NaplesStatusWatch(delphiClient, delphiService)

	// run delphi thread in background
	go delphiClient.Run()

	// wait forever
	select {}
}
