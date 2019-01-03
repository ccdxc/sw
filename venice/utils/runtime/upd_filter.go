package runtime

import (
	"fmt"
	"reflect"
)

// This file contains a helper function to address the following scenario:
// - ApiServer has the full copy of an object (Meta, Spec, Status)
// - An agent sends an update for the object which:
// -- does NOT contain certain fields that are immutable or not relevant for the agent (e.g. meta and spec, PrimaryMACAddress, ...)
// -- contains field updates that are interesting for a controller but do not need to be propagated back to ApiServer
//    for example, agent may update a timestamp even if there is no actual change in the corresponding field
//
// The filterEqual functions takes two generic types and performs deep comparison using the following rules:
// - if a field name appears in the "ignoreFields" list, it is ignored
// - if a field of the "update" parameter is set to the zero value (nil pointer, empty string, empty slice, etc.)
//   it is assumed that it is intentionally missing from the update, and so it is ignored, unless the
//   field name appears in the "forceFields" list.
// - if a field appears both in the "ignoreFields" and "forceField" list, it is ignored
//
// The rules are applied at each level of the recursive comparison tree
//
// Notes:
// - the function does not track visited fields. If there are circular dependencies, it will not terminate.

func filterEqual(refVal, updVal reflect.Value, ignoreFields, forceFields map[string]bool, fieldName string, depth int) bool {
	if ignoreFields[fieldName] {
		// field is ignored, skip
		return true
	}
	force := forceFields[fieldName]

	switch updVal.Kind() {

	case reflect.Slice:
		if !force && (updVal.IsNil() || updVal.Len() == 0) {
			// We assume empty is "unspecified"
			return true
		}
		if updVal.Len() != refVal.Len() {
			return false
		}
		for i := 0; i < updVal.Len(); i++ {
			if !filterEqual(refVal.Index(i), updVal.Index(i), ignoreFields, forceFields, fieldName, depth+1) {
				return false
			}
		}
		return true

	case reflect.Struct:
		for i, n := 0, updVal.NumField(); i < n; i++ {
			if !filterEqual(refVal.Field(i), updVal.Field(i), ignoreFields, forceFields, updVal.Type().Field(i).Name, depth+1) {
				return false
			}
		}
		return true

	case reflect.Interface:
		if !force && updVal.IsNil() {
			return true
		}
		return filterEqual(refVal.Elem(), updVal.Elem(), ignoreFields, forceFields, fieldName, depth+1)

	case reflect.Ptr:
		if !force && updVal.IsNil() {
			return true
		}
		if updVal.Pointer() == refVal.Pointer() {
			return true
		}
		return filterEqual(refVal.Elem(), updVal.Elem(), ignoreFields, forceFields, fieldName, depth+1)

	case reflect.Map:
		if !force && updVal.IsNil() {
			return true
		}
		if updVal.Len() != refVal.Len() {
			return false
		}
		if updVal.Pointer() == refVal.Pointer() {
			return true
		}
		for _, k := range updVal.MapKeys() {
			if !updVal.MapIndex(k).IsValid() || !refVal.MapIndex(k).IsValid() || !filterEqual(refVal.MapIndex(k), updVal.MapIndex(k), ignoreFields, forceFields, fieldName, depth+1) {
				return false
			}
		}
		return true

	case reflect.Chan, reflect.Array, reflect.Func:
		panic(fmt.Sprintf("Unsupported kind %+v", updVal))

	default:
		if !force && (updVal.Interface() == reflect.Zero(updVal.Type()).Interface()) {
			// attribute is not specified in update, skip
			return true
		}
		return updVal.Interface() == refVal.Interface()

	}
}

// FilterUpdate compares a reference object and an update object and returns true if
// the update can be safely filtered, i.e. should NOT sent back to APIServer.
// See comment at the top of the file for a description of the comparison rules and
// function parameters.
func FilterUpdate(refObj, updObj interface{}, ignoreFields, forceFields []string) bool {

	// nil update can always be filtered
	if updObj == nil {
		return true
	}

	if reflect.TypeOf(refObj) != reflect.TypeOf(updObj) {
		return false
	}

	ifMap := make(map[string]bool, len(ignoreFields))
	for _, f := range ignoreFields {
		ifMap[f] = true
	}

	ffMap := make(map[string]bool, len(forceFields))
	for _, f := range forceFields {
		ffMap[f] = true
	}

	return filterEqual(reflect.ValueOf(refObj), reflect.ValueOf(updObj), ifMap, ffMap, "", 0)
}
