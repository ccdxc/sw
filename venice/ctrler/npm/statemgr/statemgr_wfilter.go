package statemgr

import (
	"context"
	"fmt"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/featureflags"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/netutils"
)

var netAgentObjects = []string{"App", "Endpoint", "Network", "SecurityProfile", "NetworkSecurityPolicy", "NetworkInterface", "IPAMPolicy", "RoutingConfig", "Vrf", "RouteTable"}

func (sm *Statemgr) setWatchFilterFlags() {
	// set the watch filter behavior based on feature flags
	for _, obj := range netAgentObjects {
		switch obj {
		case "IPAMPolicy", "Network", "NetworkSecurityPolicy", "Vrf", "RouteTable", "RoutingConfig", "SecurityProfile":
			if featureflags.IsOVerlayRoutingEnabled() == true {
				sm.logger.Infof("Setting controller filter flags: %s", obj)
				sm.WatchFilterFlags[obj] = memdb.ControllerWatchFilter | memdb.DefWatchFilterBlock
			} else {
				sm.logger.Info("OverlayRouting not Enabled")
			}
		default:
			// explicitly setting it to 0 even though 0 is uninitialized value
			sm.WatchFilterFlags[obj] = 0
		}
	}

	// set the flags for memdb to use
	sm.mbus.SetWatchFilterFlags(sm.WatchFilterFlags)
}

// GetAgentWatchFilter is called when filter get is happening based on netagent watchoptions
func (sm *Statemgr) GetAgentWatchFilter(ctx context.Context, kind string, opts *api.ListWatchOptions) []memdb.FilterFn {

	var filters []memdb.FilterFn
	// the object kind has "netproto." prefix which is needed for Fieldselector fitlers to work, since the object
	// name clashes with the names in api/protos
	strs := strings.Split(kind, ".")
	objKind := strs[len(strs)-1]

	// if for this kind, the watch filters are controller based, just return
	if sm.WatchFilterFlags[objKind]&memdb.ControllerWatchFilter == memdb.ControllerWatchFilter {
		sm.logger.Info("GetAgentWatchFilter: controller based filter")
		return filters
	}

	// FIX for VS-1305, with Naples running release A code and Venice running newer code, Naples will send
	// no watchoptions, fix the watchoptions here
	if kind == ".Endpoint" {
		if opts.FieldSelector == "" {
			str := fmt.Sprintf("spec.node-uuid=%s", netutils.GetNodeUUIDFromCtx(ctx))
			opts.FieldSelector = str
			kind = "netproto" + kind
			sm.logger.Infof("GetAgentWatchFilter Update watch options for %s | to: %v", kind, opts)
		}
	}
	return sm.mbus.GetWatchFilter(kind, opts)
}
