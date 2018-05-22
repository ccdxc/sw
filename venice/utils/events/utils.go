package events

import (
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils"
)

// GetSourceKey helper function to construct the source key given the event
// source. This key is used for maintaining a separate cache for each source.
func GetSourceKey(source *monitoring.EventSource) string {
	if source == nil {
		return ""
	}

	return fmt.Sprintf("%s-%s", source.GetNodeName(), source.GetComponent())
}

// ValidateEvent validates the given event to ensure the manadatory fields are there.
func ValidateEvent(event *monitoring.Event) error {
	if event.GetSource() == nil ||
		utils.IsEmpty(strings.TrimSpace(event.GetType())) ||
		utils.IsEmpty(strings.TrimSpace(event.GetSeverity())) {
		return NewError(ErrMissingEventAttributes, "source/type/severity is required")
	}

	return nil
}

// GetEventKey helper function to create a hashkey for the event. This is
// used for storing events in the cache.
func GetEventKey(event *monitoring.Event) string {
	keyComponents := []string{
		event.GetType(),
		event.GetSeverity(),
		event.GetMessage(),
	}

	if event.GetSource() != nil {
		keyComponents = append(keyComponents, []string{
			event.GetSource().GetNodeName(),
			event.GetSource().GetComponent()}...)
	}

	if event.GetObjectRef() != nil {
		keyComponents = append(keyComponents, []string{
			event.GetTenant(),
			event.GetNamespace(),
			event.GetKind()}...)
	}

	return strings.Join(keyComponents, "")
}
