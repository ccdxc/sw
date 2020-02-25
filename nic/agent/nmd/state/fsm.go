package state

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/looplab/fsm"

	"github.com/pensando/sw/nic/agent/nmd/cmdif"
	"github.com/pensando/sw/nic/agent/nmd/rolloutif"
	nmdProto "github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// NewNMDStateMachine returns an instance of NMD's state machine
func NewNMDStateMachine() *NMDStateMachine {
	log.Info("Initializing new NMD State Machine")
	s := NMDStateMachine{
		FSM: fsm.NewFSM(
			"init",
			fsm.Events{
				{Name: "doStatic", Src: []string{"init"}, Dst: "doStatic"},
				{Name: "doDynamic", Src: []string{"init"}, Dst: "doDynamic"},
				{Name: "doNTP", Src: []string{"doDynamic", "doStatic"}, Dst: "doNTP"},
				{Name: "doAdmission", Src: []string{"doNTP"}, Dst: "doAdmission"},
				{Name: "doUpgrade", Src: []string{"doAdmission"}, Dst: "doUpgrade"},
				{Name: "rebootPending", Src: []string{"doAdmission", "doUpgrade"}, Dst: "rebootPending"},
			},
			fsm.Callbacks{
				"doStatic": func(e *fsm.Event) {
					log.Infof("Entered State: %v", e.Event)
					nmd, ok := e.Args[0].(*NMD)
					if !ok {
						log.Error("Failed to cast event args to type NMD.")
						e.Err = errors.New("failed to cast event args to type NMD")
						return
					}
					_, err := nmd.IPClient.DoStaticConfig()
					if err != nil {
						log.Errorf("Failed to perform static configuration. Err: %v", err)
						e.Err = err
						return
					}
					nmd.config.Status.IPConfig = nmd.config.Spec.IPConfig
				},

				"doDynamic": func(e *fsm.Event) {
					log.Infof("Entered State: %v", e.Event)
					nmd, ok := e.Args[0].(*NMD)
					if !ok {
						log.Error("Failed to cast event args to type NMD.")
						e.Err = errors.New("failed to cast event args to type NMD")
						return
					}

					if err := nmd.IPClient.DoDHCPConfig(); err != nil {
						log.Errorf("Failed to do DHCP Config. Err: %v", err)
						e.Err = err
						return
					}

					if nmd.Pipeline != nil && nmd.Pipeline.GetPipelineType() != globals.NaplesPipelineApollo {
						// TODO We need to temporarily need to run dhclient here to ensure that the default routes are installed. Move this to dhcp config eventually
						e.Err = runCmd(fmt.Sprintf("dhclient %s", nmd.IPClient.GetIPClientIntf()))
					}
				},

				"doNTP": func(e *fsm.Event) {
					log.Infof("Entered State: %v", e.Event)
					nmd, ok := e.Args[0].(*NMD)
					if !ok {
						log.Error("Failed to cast event args to type NMD.")
						e.Err = errors.New("failed to cast event args to type NMD")
						return
					}
					e.Err = nmd.IPClient.DoNTPSync()
					return
				},

				"doAdmission": func(e *fsm.Event) {
					log.Infof("Entered State: %v", e.Event)
					nmd, ok := e.Args[0].(*NMD)
					if !ok {
						log.Error("Failed to cast event args to type NMD.")
						e.Err = errors.New("failed to cast event args to type NMD")
						return
					}
					// Update CMD Client with the new resolvers obtained statically
					veniceIPs := nmd.config.Status.Controllers
					if len(veniceIPs) == 0 {
						log.Error("Missing venice co-ordinates")
						e.Err = fmt.Errorf("missing venice co-ordinates")
						return
					}
					var resolverURLs []string
					registrationURL := fmt.Sprintf("%s:%s", veniceIPs[0], globals.CMDSmartNICRegistrationPort)
					for _, v := range veniceIPs {
						resolverURLs = append(resolverURLs, fmt.Sprintf("%s:%s", v, globals.CMDGRPCAuthPort))
					}
					// Update NMD's resolver client.
					nmd.resolverClient = resolver.New(&resolver.Config{
						Name:    fmt.Sprintf("%s-%s", globals.Nmd, nmd.GetAgentID()),
						Servers: resolverURLs,
					})

					// Re-populate remote certs URLs
					nmd.remoteCertsURLs = []string{}
					for _, i := range veniceIPs {
						nmd.remoteCertsURLs = append(nmd.remoteCertsURLs, fmt.Sprintf("%s:%s", i, globals.CMDAuthCertAPIPort))
					}

					// Init TSDB
					// initialize netagent's tsdb client
					opts := &tsdb.Opts{
						ClientName:              "nmd_" + nmd.GetAgentID(),
						ResolverClient:          nmd.resolverClient,
						Collector:               globals.Collector,
						DBName:                  "default",
						SendInterval:            time.Duration(30) * time.Second,
						ConnectionRetryInterval: 100 * time.Millisecond,
					}
					ctx, cancel := context.WithCancel(context.Background())
					tsdb.Init(ctx, opts)
					nmd.tsdbCancel = cancel

					cmdAPI, err := cmdif.NewCmdClient(nmd, registrationURL, nmd.resolverClient)
					if err != nil {
						log.Errorf("Failed to instantiate CMD Client. Err: %v", err)
						e.Err = err
						return
					}
					nmd.cmd = cmdAPI
					roClient, err := rolloutif.NewRoClient(nmd, nmd.resolverClient)
					if err != nil {
						log.Errorf("Failed to instantiate Rollout Client. Err: %v", err)
						e.Err = err
						return
					}
					nmd.rollout = roClient

					nmd.modeChange.Lock()
					err = nmd.initTLSProvider()
					nmd.modeChange.Unlock()
					if err != nil {
						log.Errorf("Error initializing TLS provider: %v", err)
						e.Err = fmt.Errorf("error initializing TLS provider: %v", err)
						return
					}
					go nmd.AdmitNaples()
					return
				},

				"doUpgrade": func(e *fsm.Event) {
					log.Infof("Entered State: %v", e.Event)
					_, ok := e.Args[0].(*NMD)
					if !ok {
						log.Error("Failed to cast event args to type NMD.")
						e.Err = errors.New("failed to cast event args to type NMD")
						return
					}
					// TODO hook up Upgrade
				},

				"rebootPending": func(e *fsm.Event) {
					log.Infof("Entered State: %v", e.Event)
					nmd, ok := e.Args[0].(*NMD)
					if !ok {
						log.Error("Failed to cast event args to type NMD.")
						e.Err = errors.New("failed to cast event args to type NMD")
						return
					}
					log.Info("Setting the transition phase to reboot pending")
					nmd.config.Status.TransitionPhase = nmdProto.DistributedServiceCardStatus_REBOOT_PENDING.String()
				},
			},
		),
	}
	return &s
}
