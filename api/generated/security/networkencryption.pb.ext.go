// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package security is a auto generated package.
Input file: networkencryption.proto
*/
package security

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
func (m *TrafficEncryptionPolicy) MakeKey(prefix string) string {
	return fmt.Sprint(globals.RootPrefix, "/", prefix, "/", "trafficEncryptionPolicy/", m.Tenant, "/", m.Name)
}

func (m *TrafficEncryptionPolicy) MakeURI(cat, ver, prefix string) string {
	in := m
	return fmt.Sprint("/", cat, "/", prefix, "/", ver, "/tenant/", in.Tenant, "/trafficEncryptionPolicy/", in.Name)
}

// Clone clones the object into into or creates one of into is nil
func (m *IPsecProtocolSpec) Clone(into interface{}) (interface{}, error) {
	var out *IPsecProtocolSpec
	var ok bool
	if into == nil {
		out = &IPsecProtocolSpec{}
	} else {
		out, ok = into.(*IPsecProtocolSpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *IPsecProtocolSpec) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *TLSProtocolSpec) Clone(into interface{}) (interface{}, error) {
	var out *TLSProtocolSpec
	var ok bool
	if into == nil {
		out = &TLSProtocolSpec{}
	} else {
		out, ok = into.(*TLSProtocolSpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *TLSProtocolSpec) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *TrafficEncryptionPolicy) Clone(into interface{}) (interface{}, error) {
	var out *TrafficEncryptionPolicy
	var ok bool
	if into == nil {
		out = &TrafficEncryptionPolicy{}
	} else {
		out, ok = into.(*TrafficEncryptionPolicy)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *TrafficEncryptionPolicy) Defaults(ver string) bool {
	m.Kind = "TrafficEncryptionPolicy"
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *TrafficEncryptionPolicySpec) Clone(into interface{}) (interface{}, error) {
	var out *TrafficEncryptionPolicySpec
	var ok bool
	if into == nil {
		out = &TrafficEncryptionPolicySpec{}
	} else {
		out, ok = into.(*TrafficEncryptionPolicySpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *TrafficEncryptionPolicySpec) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *TrafficEncryptionPolicyStatus) Clone(into interface{}) (interface{}, error) {
	var out *TrafficEncryptionPolicyStatus
	var ok bool
	if into == nil {
		out = &TrafficEncryptionPolicyStatus{}
	} else {
		out, ok = into.(*TrafficEncryptionPolicyStatus)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *TrafficEncryptionPolicyStatus) Defaults(ver string) bool {
	return false
}

// Validators

func (m *IPsecProtocolSpec) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *TLSProtocolSpec) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *TrafficEncryptionPolicy) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *TrafficEncryptionPolicySpec) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *TrafficEncryptionPolicyStatus) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes(
		&TrafficEncryptionPolicy{},
	)

}
