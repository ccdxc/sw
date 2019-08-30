package events

import "github.com/pensando/sw/events/generated/eventtypes"

var eventsTobeSuppressed = map[string]struct{}{
	eventtypes.SERVICE_RUNNING.String():        {},
	eventtypes.SERVICE_STOPPED.String():        {},
	eventtypes.NAPLES_SERVICE_STOPPED.String(): {},
	eventtypes.SERVICE_STARTED.String():        {},
	eventtypes.SYSTEM_COLDBOOT.String():        {},

	eventtypes.LEADER_CHANGED.String():   {},
	eventtypes.LEADER_LOST.String():      {},
	eventtypes.LEADER_ELECTED.String():   {},
	eventtypes.ELECTION_STARTED.String(): {},

	eventtypes.QUORUM_MEMBER_ADD.String():       {},
	eventtypes.QUORUM_MEMBER_HEALTHY.String():   {},
	eventtypes.QUORUM_MEMBER_REMOVE.String():    {},
	eventtypes.QUORUM_MEMBER_UNHEALTHY.String(): {},
	eventtypes.UNSUPPORTED_QUORUM_SIZE.String(): {},
	eventtypes.QUORUM_UNHEALTHY.String():        {},

	eventtypes.NODE_HEALTHY.String():     {},
	eventtypes.NODE_UNREACHABLE.String(): {},
	eventtypes.NODE_JOINED.String():      {},

	eventtypes.DSC_HEALTHY.String():     {},
	eventtypes.DSC_UNHEALTHY.String():   {},
	eventtypes.DSC_UNREACHABLE.String(): {},
	eventtypes.DSC_ADMITTED.String():    {},

	eventtypes.LINK_UP.String():   {},
	eventtypes.LINK_DOWN.String(): {},

	eventtypes.AUTO_GENERATED_TLS_CERT.String(): {},
}

// SuppressDuringUpgrade return true if the given event has to be suppressed during upgrade
func SuppressDuringUpgrade(eType string) bool {
	_, ok := eventsTobeSuppressed[eType]
	return ok
}
