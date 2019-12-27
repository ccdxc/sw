package env

import (
	"github.com/pensando/sw/venice/cmd/server/options"
	"github.com/pensando/sw/venice/perseus/types"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// Perseus global state is held here.
var (
	Logger            log.Logger
	CfgWatcherService types.CfgWatcherService
	ResolverClient    resolver.Interface
	Options           *options.ServerRunOptions
)
