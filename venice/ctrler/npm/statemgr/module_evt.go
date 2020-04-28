package statemgr

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/ctkit"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics"
)

//GetModuleWatchOptions gets options
func (sm *Statemgr) GetModuleWatchOptions() *api.ListWatchOptions {
	opts := api.ListWatchOptions{}
	opts.FieldChangeSelector = []string{"Spec"}
	return &opts
}

// OnModuleUpdate updates the log level on module update event
func (sm *Statemgr) OnModuleUpdate(oldObj *ctkit.Module, newObj *diagapi.Module) error {
	sm.logger.ResetFilter(diagnostics.GetLogFilter(newObj.Spec.LogLevel))
	return nil
}

// OnModuleCreate updates the log level on module create event
func (sm *Statemgr) OnModuleCreate(obj *ctkit.Module) error {
	sm.logger.ResetFilter(diagnostics.GetLogFilter(obj.Spec.LogLevel))
	return nil
}

// OnModuleDelete is a no-op
func (sm *Statemgr) OnModuleDelete(obj *ctkit.Module) error {
	return nil
}

// OnModuleReconnect is called when ctkit reconnects to apiserver
func (sm *Statemgr) OnModuleReconnect() {
	return
}
