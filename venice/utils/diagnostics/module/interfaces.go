package module

import (
	"github.com/pensando/sw/api/generated/diagnostics"
)

// Watcher is an interface to watch and process module configuration changes like log level, enable tracing etc. It is a singleton object.
type Watcher interface {
	// Module returns module information associated with the process's singleton module watcher
	Module() diagnostics.Module
	// RegisterOnChangeCb registers a callback to process any module spec changes
	RegisterOnChangeCb(cb OnChangeCb)
	// Start re-initializes Watcher after Stop() has been called
	Start()
	// Stop un-initializes Watcher
	Stop()
}

// OnChangeCb is a callback to process changes in module spec
type OnChangeCb func(module *diagnostics.Module)

// Updater is an abstraction to modify module object. It is a singleton object
type Updater interface {
	// Enqueue enqueues a module configuration along with the operation to be performed on it in the system
	Enqueue(module *diagnostics.Module, oper Oper) error
	// Start re-initializes Updater after Stop() has been called
	Start()
	// Stop indicates Updater to un-initialize
	Stop()
}

// Oper is the operation requested on module object
type Oper string

// Values supported for module Updater
const (
	Create = Oper("Create")
	Update = Oper("Update")
	Delete = Oper("Delete")
)
