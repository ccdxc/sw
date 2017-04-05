package env

import (
	"sync"

	"github.com/pensando/sw/cmd/server/options"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/quorum"
	"github.com/pensando/sw/utils/runtime"
)

// CMD global state is held here.
var (
	Options *options.ServerRunOptions
	Scheme  *runtime.Scheme
	KVStore kvstore.Interface
	Quorum  quorum.Interface
	Leader  string
	Mutex   sync.Mutex
)
