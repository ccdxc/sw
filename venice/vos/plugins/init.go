package plugins

import (
	"github.com/pensando/sw/venice/vos"
)

// RegisterPlugins is called during init of the object store and all per-bucket plugins are to be registered here.
func RegisterPlugins(instance vos.Interface) {
	// Register all needed plugins here
	loggerRegisterCallbacks(instance)
	uploadBundleImageCallbacks(instance)
}
