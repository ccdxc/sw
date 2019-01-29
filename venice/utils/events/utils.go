package events

import (
	"fmt"
	"strings"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils"
)

// GetSourceKey helper function to construct the source key given the event
// source. This key is used for maintaining a separate cache for each source.
func GetSourceKey(source *evtsapi.EventSource) string {
	if source == nil {
		return ""
	}

	return fmt.Sprintf("%s-%s", source.GetNodeName(), source.GetComponent())
}

// ValidateEvent validates the given event to ensure the manadatory fields are there.
func ValidateEvent(event *evtsapi.Event) error {
	if event.GetSource() == nil ||
		utils.IsEmpty(strings.TrimSpace(event.GetType())) ||
		utils.IsEmpty(strings.TrimSpace(event.GetSeverity())) {
		return NewError(ErrMissingEventAttributes, "source/type/severity is required")
	}

	return nil
}

// GetEventKey helper function to create a hashkey for the event. This is
// used for storing events in the cache.
func GetEventKey(event *evtsapi.Event) string {
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
			event.GetObjectRef().GetTenant(),
			event.GetObjectRef().GetNamespace(),
			event.GetObjectRef().GetKind(),
			event.GetObjectRef().GetName()}...)
	}

	return strings.Join(keyComponents, "")
}

// Minify returns the formatted strings containing name, message and count for each event.
// It will be logged instead of the whole event; helpful for debugging issues.
func Minify(events []*evtsapi.Event) []string {
	var res []string
	for _, evt := range events {
		res = append(res, MinifyEvent(evt))
	}

	return res
}

// MinifyEvent returns the formatted string containing event details
func MinifyEvent(evt *evtsapi.Event) string {
	return fmt.Sprintf("%s: {%s, count: %d, source: %s}", evt.GetName(), evt.GetMessage(), evt.GetCount(),
		fmt.Sprintf("%s-%s", evt.GetSource().GetNodeName(), evt.GetSource().GetComponent()))
}
