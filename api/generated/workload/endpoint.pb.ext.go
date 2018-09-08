// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package workload is a auto generated package.
Input file: endpoint.proto
*/
package workload

import (
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

// MakeKey generates a KV store key for the object
func (m *Endpoint) MakeKey(prefix string) string {
	return fmt.Sprint(globals.ConfigRootPrefix, "/", prefix, "/", "endpoints/", m.Tenant, "/", m.Name)
}

func (m *Endpoint) MakeURI(cat, ver, prefix string) string {
	in := m
	return fmt.Sprint("/", cat, "/", prefix, "/", ver, "/tenant/", in.Tenant, "/endpoints/", in.Name)
}

// Clone clones the object into into or creates one of into is nil
func (m *Endpoint) Clone(into interface{}) (interface{}, error) {
	var out *Endpoint
	var ok bool
	if into == nil {
		out = &Endpoint{}
	} else {
		out, ok = into.(*Endpoint)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *Endpoint) Defaults(ver string) bool {
	m.Kind = "Endpoint"
	m.Tenant, m.Namespace = "default", "default"
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *EndpointSpec) Clone(into interface{}) (interface{}, error) {
	var out *EndpointSpec
	var ok bool
	if into == nil {
		out = &EndpointSpec{}
	} else {
		out, ok = into.(*EndpointSpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EndpointSpec) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *EndpointStatus) Clone(into interface{}) (interface{}, error) {
	var out *EndpointStatus
	var ok bool
	if into == nil {
		out = &EndpointStatus{}
	} else {
		out, ok = into.(*EndpointStatus)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EndpointStatus) Defaults(ver string) bool {
	return false
}

// Validators

func (m *Endpoint) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	{
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		ret = m.ObjectMeta.Validate(ver, path+dlmtr+"ObjectMeta", ignoreStatus)
	}
	return ret
}

func (m *EndpointSpec) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *EndpointStatus) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

// Transformers

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes(
		&Endpoint{},
	)

}
