package rpcserver

import (
	"errors"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/ctrler/tsm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/ref"
)

// diagnosticsServer is the diagnostics RPC server
type diagnosticsServer struct {
	stateMgr *statemgr.Statemgr
}

func (d *diagnosticsServer) WatchModule(options *api.ListWatchOptions, stream tsproto.DiagnosticsApi_WatchModuleServer) error {
	moduleEventChannel := make(chan memdb.Event, memdb.WatchLen)
	defer close(moduleEventChannel)
	d.stateMgr.WatchObjects(statemgr.KindModule, moduleEventChannel)
	defer d.stateMgr.StopWatchObjects(statemgr.KindModule, moduleEventChannel)

	modules := d.stateMgr.ListModuleState()
	for _, moduleState := range modules {
		moduleState.Lock()
		if !d.matchesModule(moduleState.Module, options) {
			moduleState.Unlock()
			continue
		}
		moduleCopy := ref.DeepCopy(moduleState.Module).(*diagnostics.Module)
		moduleState.Unlock()
		stream.Send(moduleCopy)
	}

	// Receives notifications from StateMgr when a Module object is created/updated
	ctx := stream.Context()
	for {
		select {

		case evt, ok := <-moduleEventChannel:
			if !ok {
				log.Errorf("Error reading from Module update channel")
				return errors.New("error reading from Module update channel")
			}

			eventType := memdbToAPIEventType(evt.EventType)

			moduleState := evt.Obj.(*statemgr.ModuleState)
			moduleState.Lock()
			if !d.matchesModule(moduleState.Module, options) {
				moduleState.Unlock()
				continue
			}

			moduleCopy := ref.DeepCopy(moduleState.Module).(*diagnostics.Module)
			moduleState.Unlock()
			log.Infof("Sending event %v to node %s for Module %s", eventType, moduleCopy.Status.Node, moduleCopy.Name)
			err := stream.Send(moduleCopy)
			if err != nil {
				log.Errorf("Error sending event %v to node %s for Module %s", eventType, moduleCopy.Status.Node, moduleCopy.Name)
			}

		case <-ctx.Done():
			return ctx.Err()
		}
	}
}

func (d *diagnosticsServer) matchesModule(modObj *diagnostics.Module, options *api.ListWatchOptions) bool {
	if modObj.Status.Category != diagnostics.ModuleStatus_Naples.String() {
		return false
	}
	if options.FieldSelector != "" {
		selector, err := fields.ParseWithValidation(fmt.Sprintf("%s.%s", "diagnostics", string(diagnostics.KindModule)), options.FieldSelector)
		if err != nil {
			log.Errorf("Error parsing field selector string, err: %v", err)
			return false
		}
		if !selector.MatchesObj(modObj) {
			return false
		}
	}
	return true
}

// NewDiagnosticsServer returns a RPC server for Diagnostics API
func NewDiagnosticsServer(stateMgr *statemgr.Statemgr) tsproto.DiagnosticsApiServer {
	return &diagnosticsServer{stateMgr: stateMgr}
}
