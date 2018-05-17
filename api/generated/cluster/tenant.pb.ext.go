// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package cluster is a auto generated package.
Input file: tenant.proto
*/
package cluster

import (
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"

	"github.com/pensando/sw/venice/globals"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

// MakeKey generates a KV store key for the object
func (m *Tenant) MakeKey(prefix string) string {
	return fmt.Sprint(globals.RootPrefix, "/", prefix, "/", "tenants/", m.Name)
}

// Clone clones the object into into or creates one of into is nil
func (m *Tenant) Clone(into interface{}) (interface{}, error) {
	var out *Tenant
	var ok bool
	if into == nil {
		out = &Tenant{}
	} else {
		out, ok = into.(*Tenant)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *Tenant) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *TenantSpec) Clone(into interface{}) (interface{}, error) {
	var out *TenantSpec
	var ok bool
	if into == nil {
		out = &TenantSpec{}
	} else {
		out, ok = into.(*TenantSpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *TenantSpec) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *TenantStatus) Clone(into interface{}) (interface{}, error) {
	var out *TenantStatus
	var ok bool
	if into == nil {
		out = &TenantStatus{}
	} else {
		out, ok = into.(*TenantStatus)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *TenantStatus) Defaults(ver string) bool {
	return false
}

// Validators

func (m *Tenant) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *TenantSpec) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *TenantStatus) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes(
		&Tenant{},
	)

}
