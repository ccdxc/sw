package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"runtime/debug"
	"time"

	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif"
	"github.com/pensando/sw/nic/agent/tmagent/state"
	"github.com/pensando/sw/venice/utils/events/recorder"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/nodemetrics"

	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	dproto "github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// reportInterval is how often(in seconds) tmagent sends metrics to TSDB
var reportInterval = 30 * time.Second

// TelemetryAgent keeps the telementry agent state
type TelemetryAgent struct {
	ctx            context.Context
	tpState        *state.PolicyState
	tpClient       *ctrlerif.TpClient
	nodeUUID       string
	resolverClient resolver.Interface
	mode           string
	restServer     *restapi.RestServer
	nodeMetrics    nodemetrics.NodeInterface
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
	nslist := delphiProto.DistributedServiceCardStatusList(s.DelphiClient)
	for _, ns := range nslist {
		s.handleVeniceCoordinates(ns)
	}
}

func (s *service) Name() string {
	return s.name
}

// OnDistributedServiceCardStatusCreate event handler
func (s *service) OnDistributedServiceCardStatusCreate(obj *delphiProto.DistributedServiceCardStatus) {
	s.handleVeniceCoordinates(obj)
}

// OnDistributedServiceCardStatusUpdate event handler
func (s *service) OnDistributedServiceCardStatusUpdate(old, newStat *delphiProto.DistributedServiceCardStatus) {
	s.handleVeniceCoordinates(newStat)
}

// OnDistributedServiceCardStatusDelete event handler
func (s *service) OnDistributedServiceCardStatusDelete(obj *delphiProto.DistributedServiceCardStatus) {
}

func (s *service) handleVeniceCoordinates(obj *delphiProto.DistributedServiceCardStatus) {
	log.Infof("Tmagent reactor called with %v", obj)
	s.tmagent.mode = obj.DistributedServiceCardMode.String()
	s.tmagent.nodeUUID = obj.GetDSCName()
	log.Infof("tmagent uuid: %s", s.tmagent.nodeUUID)
	if obj.DistributedServiceCardMode == delphiProto.DistributedServiceCardStatus_NETWORK_MANAGED_INBAND || obj.DistributedServiceCardMode == delphiProto.DistributedServiceCardStatus_NETWORK_MANAGED_OOB {
		var controllers []string
		var err error

		s.tmagent.tpState.UpdateHostName(obj.GetID())
		s.tmagent.restServer.SetNodeUUID(obj.GetDSCName())
		for _, ip := range obj.Controllers {
			controllers = append(controllers, fmt.Sprintf("%s:%s", ip, globals.CMDResolverPort))
		}

		if s.tmagent.resolverClient != nil {
			log.Infof("Tpclient updating Venice Co-ordinates with %v", controllers)
			s.tmagent.resolverClient.UpdateServers(controllers)
			return
		}

		log.Infof("Populating Venice Co-ordinates with %v", controllers)

		s.tmagent.resolverClient = resolver.New(&resolver.Config{Name: globals.Tmagent, Servers: controllers})

		// Init the TSDB
		if err := s.tmagent.tpState.TsdbInit(s.tmagent.nodeUUID, s.tmagent.resolverClient); err != nil {
			log.Fatalf("failed to init tsdb, err: %v", err)
		}

		// Init ObjectStore(MinIO)
		if err := s.tmagent.tpState.ObjStoreInit(s.tmagent.nodeUUID, s.tmagent.resolverClient, time.Duration(1)*time.Minute, nil); err != nil {
			log.Fatalf("failed to init minio, err: %v", err)
		}

		s.tmagent.tpClient, err = ctrlerif.NewTpClient(s.tmagent.nodeUUID, s.tmagent.tpState, globals.Tpm, s.tmagent.resolverClient)
		if err != nil {
			log.Fatalf("failed to init tmagent controller client, err: %v", err)
		}

		if err := s.tmagent.tpState.FwlogInit(state.FwlogIpcShm); err != nil {
			log.Fatal(err)
		}

		// start reporting metrics
		if err := s.tmagent.reportMetrics(s.tmagent.resolverClient, s.DelphiClient); err != nil {
			log.Fatal(err)
		}
	} else {
		log.Infof("switching to host mode")
		s.tmagent.tpState.UpdateHostName(obj.GetDSCName())
		if err := s.tmagent.tpState.Reset(); err != nil {
			log.Errorf("failed to delete the existing policies, err: %v", err)
		}

		// stop handling venice policy
		if s.tmagent.tpClient != nil {
			s.tmagent.tpClient.Stop()
		}

		if s.tmagent.tpState != nil {
			s.tmagent.tpState.Close()
		}

		// stop node watcher
		if s.tmagent.nodeMetrics != nil {
			s.tmagent.nodeMetrics.Close()
		}

		// stop metrics
		if s.tmagent.restServer != nil {
			s.tmagent.restServer.StopMetrics()
		}

		// shutdown tsdb
		if s.tmagent.tpState != nil {
			s.tmagent.tpState.TsdbCleanup()
		}

		// create new instance
		tpState, err := state.NewTpAgent(s.tmagent.ctx, globals.AgentRESTPort)
		if err != nil {
			log.Fatalf("failed to init tmagent state, err: %v", err)
		}
		s.tmagent.tpState = tpState
		s.tmagent.restServer.SetPolicyHandler(tpState)
	}
}

func (ta *TelemetryAgent) reportMetrics(rc resolver.Interface, dclient clientApi.Client) error {
	// report node metrics
	node := &cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{
			Kind: "DistributedServiceCard",
		},
		ObjectMeta: api.ObjectMeta{
			Name: ta.nodeUUID,
		},
	}

	nm, err := nodemetrics.NewNodeMetrics(ta.ctx, node, reportInterval, log.WithContext("pkg", "nodemetrics"))
	if err != nil {
		return err
	}

	ta.nodeMetrics = nm

	// report delphi metrics
	ta.restServer.ReportMetrics(reportInterval, dclient)
	return nil
}

// periodicFreeMemory forces garbage collection every minute and frees OS memory
func periodicFreeMemory() {
	for {
		select {
		case <-time.After(time.Minute):
			// force GC and free OS memory
			debug.FreeOSMemory()
		}
	}
}

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Tmagent)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		restURL         = flag.String("rest-url", "127.0.0.1:"+globals.TmAGENTRestPort, "specify telemetry agent REST URL")
		datapath        = flag.String("datapath", "hal", "specify the agent datapath type either mock or hal")
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
			MaxSize:    2,
			MaxBackups: 1,
			MaxAge:     7,
		},
	}

	// Initialize logger config
	logger := log.SetConfig(logConfig)
	defer logger.Close()

	// create events recorder
	evtsRecorder, err := recorder.NewRecorder(&recorder.Config{Component: globals.Tmagent}, logger)
	if err != nil {
		log.Fatalf("failed to create events recorder, err: %v", err)
	}
	defer evtsRecorder.Close()

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	tmAgent := &TelemetryAgent{
		ctx: ctx,
	}

	var delphiService = &service{
		name:    globals.Tmagent,
		tmagent: tmAgent,
	}

	tpState, err := state.NewTpAgent(ctx, globals.AgentRESTPort)
	if err != nil {
		log.Fatalf("failed to init tmagent state, err: %v", err)
	}
	defer tpState.Close()

	tmAgent.tpState = tpState

	tmAgent.restServer, err = restapi.NewRestServer(ctx, *restURL, tpState, *datapath)
	if err != nil {
		log.Fatalf("failed to create tmagent rest API server, Err: %v", err)
	}

	delphiClient, err := delphi.NewClient(delphiService)
	if err != nil {
		log.Fatalf("delphi NewClient failed")
	}
	delphiService.DelphiClient = delphiClient
	delphiService.sysmgrClient = sysmgr.NewClient(delphiClient, delphiService.Name())

	// Mount delphi naples status and interface objects
	delphiProto.DistributedServiceCardStatusMount(delphiClient, dproto.MountMode_ReadMode)
	dnetproto.InterfaceMount(delphiClient, dproto.MountMode_ReadMode)

	// Set up watches
	delphiProto.DistributedServiceCardStatusWatch(delphiClient, delphiService)

	// run delphi thread in background
	go delphiClient.Run()

	// set Garbage collection ratio and periodically free OS memory
	debug.SetGCPercent(20)
	go periodicFreeMemory()

	// wait forever
	select {}
}
