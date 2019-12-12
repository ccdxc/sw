package techsupport

import (
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
)

func (ag *TSMClient) handleModuleEvent(event *diagnostics.Module) {
	log.Infof("Handling Module event %v", event)

	// TODO : Does the task of enqueing the request into the channel shared between the watcher and worker
}

// StartModuleWorker runs the module events enqueued
func (ag *TSMClient) StartModuleWorker() {
	defer ag.waitGrp.Done()

	for {
		// TODO add stuf here to handle module events appropriately
		time.Sleep(time.Second)

		if ag.isStopped() {
			return
		}

	}
}

// RunModuleWatcher establishes watch
func (ag *TSMClient) RunModuleWatcher() {
	defer ag.waitGrp.Done()

	ag.diagnosticsAPIClient = tsproto.NewDiagnosticsApiClient(ag.tsGrpcClient.ClientConn)
	if ag.diagnosticsAPIClient == nil {
		log.Error("Failed to create diagnostics API Client.")
		return
	}

	opt1 := &api.ListWatchOptions{
		FieldSelector: fmt.Sprintf("status.mac-address=%s", ag.mac),
	}
	opt1.Name = fmt.Sprintf("%s*", ag.mac)

	stream, err := ag.diagnosticsAPIClient.WatchModule(ag.ctx, opt1)
	if err != nil {
		log.Errorf("Failed to establish watch on module. Err : %v", err)
		return
	}

	log.Infof("Started watching module objects for %v", opt1.Name)

	for {
		moduleEvent, err := stream.Recv()

		if err != nil {
			log.Errorf("Error in receiving stream: %v", err)
			if ag.isStopped() {
				log.Log("Agent stopped.")
				return
			}

			time.Sleep(time.Second)
			break
		}

		ag.moduleCh <- *moduleEvent
		ag.handleModuleEvent(moduleEvent)
	}
}
