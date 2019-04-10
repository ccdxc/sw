package techsupport

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"sync/atomic"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/tsm/rpcserver/tsproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"

	action "github.com/pensando/sw/venice/utils/techsupport/actionengine"
	export "github.com/pensando/sw/venice/utils/techsupport/exporter"
	rest "github.com/pensando/sw/venice/utils/techsupport/restapi"
	st "github.com/pensando/sw/venice/utils/techsupport/state"
)

const (
	// kindTechSupportRequest is the kind of SmartNIC objects in string form
	kindTechSupportRequest = string(monitoring.KindTechSupportRequest)
)

// TSMClient keeps the techsupport state
type TSMClient struct {
	sync.Mutex
	waitGrp sync.WaitGroup
	stopped bool

	// the name of the node. Must match the name of the Node/SmartNIC object
	name string
	// the kind of the node: Node (controller) or SmartNIC
	kind string
	// primary mac of the node
	mac string
	// the notifications that have been received
	notifications []*tsproto.TechSupportRequestEvent
	// a flag used to know if agent is watching or not -- access using atomics
	watching int32 // 0 --> not watching, 1 --> watching
	// rpc client used to watch and send updates
	tsGrpcClient *rpckit.RPCClient
	// API client used to watch and send updates
	tsAPIClient tsproto.TechSupportApiClient
	ctx         context.Context
	// cancel function
	cancelFn       context.CancelFunc
	resolverClient resolver.Interface
	State          *st.State
	restServer     *rest.RestServer
}

// NewTSMClient creates a new techsupport agent
func NewTSMClient(name string, mac string, kind string, configPath string, controllers []string) *TSMClient {
	log.Infof("Name : %v MAC : %v Controllers : %v", name, mac, controllers)
	state := st.NewState(configPath)
	if state == nil {
		log.Error("Could not create new tech support state.")
		return nil
	}

	var resolverClient resolver.Interface
	if len(controllers) != 0 {
		resolverClient = resolver.New(&resolver.Config{Name: globals.Tsm, Servers: controllers})
	} else {
		resolverClient = nil
	}

	ctxt, cancel := context.WithCancel(context.Background())
	agent := &TSMClient{
		ctx:            ctxt,
		cancelFn:       cancel,
		State:          state,
		name:           name,
		mac:            mac,
		kind:           kind,
		resolverClient: resolverClient,
		stopped:        false,
	}

	log.Infof("CFG : %v", state.Cfg)

	if state.Cfg.RESTUri != "" {
		log.Info("REST URI is non null")
		agent.restServer = rest.NewRestServer(state.Cfg.RESTUri, state)
	}

	return agent
}

func (ag *TSMClient) isStopped() bool {
	ag.Lock()
	defer ag.Unlock()

	return ag.stopped
}

// Start the operation of the techsupport agent
func (ag *TSMClient) Start() {
	log.Infof("Starting Tech Support TSMClient")

	if ag.resolverClient != nil {
		go ag.runTechSupportWatcher()
	}

	if ag.restServer != nil {
		go ag.restServer.Start()
	}

	go ag.StartWorking()
}

func (ag *TSMClient) handleTechSupportEvents(events *tsproto.TechSupportRequestEventList) {
	log.Infof("Handling Techsupport Event")

	for _, event := range events.Events {
		log.Infof("Event Type is : %v", event.EventType.String())

		switch event.EventType {
		case api.EventType_CreateEvent:
			log.Infof("Create event received")
			tsWork := event.Request
			ag.State.RQ.Put(tsWork)
			log.Infof("Got request %v", tsWork)
			ag.sendUpdate(tsWork, tsproto.TechSupportRequestStatus_Queued)
		}
	}
}

func (ag *TSMClient) runTechSupportWatcher() {
	log.Info("Starting Techsupport Watcher")
	for {
		tsGrpcClient, err := rpckit.NewRPCClient(ag.name, globals.Tsm, rpckit.WithBalancer(balancer.New(ag.resolverClient)))
		if err != nil {
			log.Errorf("Error creating TSM client %s: %v", ag.name, err)

			if ag.isStopped() {
				return
			}

			time.Sleep(time.Second)
			continue
		}
		ag.tsGrpcClient = tsGrpcClient

		// start watch for techsupport requests
		client := tsproto.NewTechSupportApiClient(ag.tsGrpcClient.ClientConn)
		ag.tsAPIClient = client
		watchParams := &tsproto.WatchTechSupportRequestsParameters{
			NodeName: ag.name,
			NodeKind: ag.kind,
		}

		stream, err := client.WatchTechSupportRequests(ag.ctx, watchParams)
		if err != nil {
			log.Errorf("Error setting up watch for TechSupport requests. Err:%v", err)
			if ag.isStopped() {
				return
			}

			time.Sleep(time.Second)
			break
		}

		atomic.StoreInt32(&ag.watching, 1)
		log.Infof("Started watch for TechSupport requests for Node : %v Type : %v", ag.name, ag.kind)
		for {
			evtList, err := stream.Recv()
			if err != nil {
				log.Errorf("Error in receiving stream : %v", err)
				if ag.isStopped() {
					return
				}

				time.Sleep(time.Second)
				break
			}
			ag.notifications = append(ag.notifications, evtList.Events...)
			ag.handleTechSupportEvents(evtList)
		}
		log.Infof("TSMClient %s stopped watch", ag.name)
		atomic.StoreInt32(&ag.watching, 0)
	}

	ag.tsGrpcClient.Close()
	ag.tsGrpcClient = nil
}

func (ag *TSMClient) isWatching() bool {
	return atomic.LoadInt32(&ag.watching) != 0
}

func (ag *TSMClient) sendUpdate(work *tsproto.TechSupportRequest, status tsproto.TechSupportRequestStatus_ActionStatus) {
	update := &tsproto.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: kindTechSupportRequest,
		},
		ObjectMeta: api.ObjectMeta{
			Name: work.ObjectMeta.Name,
		},
		Spec: tsproto.TechSupportRequestSpec{
			InstanceID: work.Spec.InstanceID,
		},
		Status: tsproto.TechSupportRequestStatus{
			Status: status,
		},
	}

	if status == tsproto.TechSupportRequestStatus_InProgress {
		startTime := api.Timestamp{}
		startTime.Parse("now()")
		update.Status.StartTime = &startTime
	} else if status == tsproto.TechSupportRequestStatus_Failed || status == tsproto.TechSupportRequestStatus_Completed {
		endTime := api.Timestamp{}
		err := endTime.Parse("now()")
		if err != nil {
			log.Errorf("Error parsing time.")
			return
		}
		update.Status.EndTime = &endTime
	}

	updParams := &tsproto.UpdateTechSupportResultParameters{
		NodeName: ag.name,
		NodeKind: ag.kind,
		Request:  update,
	}
	ag.tsAPIClient.UpdateTechSupportResult(ag.ctx, updParams)
}

// Stop stops the TSMClient
func (ag *TSMClient) Stop() {
	ag.Lock()
	ag.stopped = true
	ag.Unlock()

	if ag.cancelFn != nil {
		ag.cancelFn()
	}

	if ag.tsGrpcClient != nil {
		ag.tsGrpcClient.Close()
		ag.tsGrpcClient = nil
	}
	ag.tsAPIClient = nil

	if ag.restServer != nil {
		ag.restServer.Stop()
	}
	ag.restServer = nil
}

// StartWorking the work be the worker
func (ag *TSMClient) StartWorking() {
	log.Info("Worker is starting work.")
	for {
		if ag.isStopped() {
			return
		}

		if !ag.State.RQ.IsEmpty() {
			ag.DoWork()
		}
	}
}

// DoWork executes commands for collecting techsupport
func (ag *TSMClient) DoWork() error {
	log.Info("Worker doing work.")
	work := ag.State.RQ.Get()
	ag.sendUpdate(work, tsproto.TechSupportRequestStatus_InProgress)
	err := ag.pre(work)
	if err != nil {
		log.Errorf("Failed to finish pre-work : %v", err)
		ag.sendUpdate(work, tsproto.TechSupportRequestStatus_Failed)
		return err
	}

	err = ag.do(work)
	if err != nil {
		log.Errorf("Failed to finish work : %v", err)
		ag.sendUpdate(work, tsproto.TechSupportRequestStatus_Failed)
		return err
	}

	err = ag.post(work)
	if err != nil {
		log.Errorf("Failed to finish post-work : %v", err)
		ag.sendUpdate(work, tsproto.TechSupportRequestStatus_Failed)
		return err
	}

	ag.sendUpdate(work, tsproto.TechSupportRequestStatus_Completed)
	return nil
}

// Move these into their own directory
func (ag *TSMClient) pre(work *tsproto.TechSupportRequest) error {
	log.Info("PreWork")
	return nil
}

func (ag *TSMClient) do(work *tsproto.TechSupportRequest) error {
	log.Info("Actual Work")
	var instanceID, instanceName string
	if work.Spec.InstanceID == "" {
		instanceID = string(time.Now().Unix())
		instanceID = strings.Replace(instanceID, " ", "-", -1)
		instanceName = "techsupport"
	} else {
		instanceID = work.Spec.InstanceID
		instanceName = work.ObjectMeta.Name
	}
	action.CollectTechSupport(ag.State.Cfg, instanceID)
	export.GenerateTechsupportZip(instanceID, ag.State.Cfg.FileSystemRoot+"/"+instanceID)
	tarballFile := ag.State.Cfg.FileSystemRoot + "/" + instanceID + "/" + instanceID + ".tgz"
	targetID := ag.generateTargetID(instanceID, instanceName)

	for _, destination := range work.Spec.Destinations {
		log.Infof("Destination : %v", destination)
		switch destination.Proto.String() {
		case "Local":
			log.Info("Keep file local")
		case "SCP":
			log.Info("SCP file")
			export.ScpFile(tarballFile, destination.Destination, "root", "docker", destination.Path)
		case "Venice":
			export.SendToVenice(ag.resolverClient, tarballFile, targetID)
		case "HTTPS":
			log.Info("Transfer file using HTTPs")
			export.SendToHTTP(tarballFile, destination.Path, "", "")
		}
	}

	return nil
}

func (ag *TSMClient) post(work *tsproto.TechSupportRequest) error {
	log.Info("Post Work")
	return nil
}

// ListTechSupportRequests List of all techsupport request - complete/incomplete presented
func (ag *TSMClient) ListTechSupportRequests() []*tsproto.TechSupportRequestEvent {
	return ag.notifications
}

func (ag *TSMClient) generateTargetID(instanceID, instanceName string) string {
	return fmt.Sprintf("%s-%s-%s-%s", instanceName, instanceID, ag.kind, ag.name)
}
