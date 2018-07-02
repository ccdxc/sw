// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package events is a auto generated package.
Input file: svc_events.proto
*/
package events

import (
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

// Clone clones the object into into or creates one of into is nil
func (m *GetEventRequest) Clone(into interface{}) (interface{}, error) {
	var out *GetEventRequest
	var ok bool
	if into == nil {
		out = &GetEventRequest{}
	} else {
		out, ok = into.(*GetEventRequest)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *GetEventRequest) Defaults(ver string) bool {
	return false
}

// Validators

func (m *GetEventRequest) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes()

}
