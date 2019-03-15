package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	"github.com/pensando/sw/nic/agent/tmagent/state"
	"github.com/pensando/sw/nic/agent/tpa/ctrlerif"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/nodewatcher"

	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	"github.com/pensando/sw/nic/agent/tmagent/ctrlerif/restapi"
	delphi "github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	dproto "github.com/pensando/sw/nic/delphi/proto/delphi"
	sysmgr "github.com/pensando/sw/nic/sysmgr/golib"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// reportInterval is how often(in seconds) tmagent sends metrics to TSDB
const reportInterval = 30

// TelemetryAgent keeps the telementry agent state
type TelemetryAgent struct {
	ctx            context.Context
	tpState        *state.PolicyState
	tpClient       *ctrlerif.TpClient
	nodeUUID       string
	resolverClient resolver.Interface
	mode           string
	restServer     *restapi.RestServer
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
	s.tmagent.mode = obj.NaplesMode.String()
	if obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_INBAND || obj.NaplesMode == delphiProto.NaplesStatus_NETWORK_MANAGED_OOB {
		var controllers []string
		var err error

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

		// todo: move to the uniqe name published by NMD when it is ready
		s.tmagent.nodeUUID = obj.Hostname + obj.Fru.MacStr
		log.Infof("tmagent uuid: %s", s.tmagent.nodeUUID)

		// Init the TSDB
		if err := s.tmagent.tpState.TsdbInit(s.tmagent.nodeUUID, s.tmagent.resolverClient); err != nil {
			log.Fatalf("failed to init tsdb, err: %v", err)
		}

		s.tmagent.tpClient, err = ctrlerif.NewTpClient(s.tmagent.nodeUUID, s.tmagent.tpState, globals.Tpm, s.tmagent.resolverClient)
		if err != nil {
			log.Fatalf("failed to init tmagent controller client, err: %v", err)
		}

		if err := s.tmagent.tpState.FwlogInit(state.FwlogIpcShm); err != nil {
			log.Fatal(err)
		}

		// start reporting metrics
		if err := s.tmagent.reportMetrics(s.tmagent.resolverClient); err != nil {
			log.Fatal(err)
		}
	}
}

func (ta *TelemetryAgent) reportMetrics(rc resolver.Interface) error {
	// report node metrics
	node := &cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name: ta.nodeUUID,
		},
	}

	if err := nodewatcher.NewNodeWatcher(ta.ctx, node, rc, reportInterval, log.WithContext("pkg", "nodewatcher")); err != nil {
		return err
	}

	// report delphi metrics
	go ta.restServer.ReportMetrics(reportInterval)
	return nil
}

func main() {

	var (
		debugflag       = flag.Bool("debug", false, "Enable debug mode")
		logToFile       = flag.String("logtofile", fmt.Sprintf("%s.log", filepath.Join(globals.LogDir, globals.Tmagent)), "Redirect logs to file")
		logToStdoutFlag = flag.Bool("logtostdout", false, "enable logging to stdout")
		restURL         = flag.String("rest-url", "127.0.0.1:"+globals.TmAGENTRestPort, "specify telemetry agent REST URL")
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

	tmAgent.restServer, err = restapi.NewRestServer(ctx, *restURL, tpState)
	if err != nil {
		log.Fatalf("failed to create tmagent rest API server, Err: %v", err)
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
