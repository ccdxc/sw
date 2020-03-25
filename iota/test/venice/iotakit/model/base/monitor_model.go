package base

import (
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Events returns all the events from the default tenant and namespace. Max of 1000 events will be returned
func (sm *SysModel) Events() *objects.EventsCollection {
	eventsList, err := sm.ListEvents(&api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace}})
	if err != nil {
		log.Errorf("failed to list events, err: %v", err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}

// LinkUpEventsSince returns all the link up events since the given time from any of the given naples collection
func (sm *SysModel) SystemBootEvents(npc *objects.NaplesCollection) *objects.EventsCollection {
	var naplesNames []string
	for _, naples := range npc.Nodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			naplesNames = append(naplesNames, dsc.Status.PrimaryMAC)
		}
	}

	fieldSelector := fmt.Sprintf("type=%s,object-ref.kind=DistributedServiceCard,object-ref.name in (%v)",
		eventtypes.SYSTEM_COLDBOOT, fmt.Sprintf("%s", strings.Join(naplesNames, ",")))

	log.Infof("%s", fieldSelector)

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}

// LinkUpEventsSince returns all the link up events since the given time from any of the given naples collection
func (sm *SysModel) LinkUpEventsSince(since time.Time, npc *objects.NaplesCollection) *objects.EventsCollection {
	var naplesNames []string
	for _, naples := range npc.Nodes {
		naplesNames = append(naplesNames, naples.Name())
	}

	fieldSelector := fmt.Sprintf("type=%s,meta.mod-time>=%v,object-ref.kind=DistributedServiceCard,object-ref.name in (%v)",
		eventtypes.LINK_UP, since.Format(time.RFC3339Nano), fmt.Sprintf("%s", strings.Join(naplesNames, ",")))

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}

// LinkDownEventsSince returns all the link down events since the given time from any of the given naples collection
func (sm *SysModel) LinkDownEventsSince(since time.Time, npc *objects.NaplesCollection) *objects.EventsCollection {
	var naplesNames []string
	for _, naples := range npc.Nodes {
		naplesNames = append(naplesNames, naples.Name())
	}

	fieldSelector := fmt.Sprintf("type=%s,meta.mod-time>=%v,object-ref.kind=DistributedServiceCard,object-ref.name in (%v)",
		eventtypes.LINK_DOWN, since.Format(time.RFC3339Nano), fmt.Sprintf("%s", strings.Join(naplesNames, ",")))

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}
