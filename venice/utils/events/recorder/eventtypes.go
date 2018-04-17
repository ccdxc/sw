// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package recorder

// constructEventTypesMap helper function to construct the map from given slice
// this map helps to reduce the time complexity of event types validation.
func constructEventTypesMap(eventTypes []string) map[string]struct{} {
	eventTypesMap := map[string]struct{}{}

	for _, eventType := range eventTypes {
		eventTypesMap[eventType] = struct{}{}
	}

	return eventTypesMap
}
