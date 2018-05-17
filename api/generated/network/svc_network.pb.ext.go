// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package network is a auto generated package.
Input file: svc_network.proto
*/
package network

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

// MakeKey generates a KV store key for the object
func (m *LbPolicyList) MakeKey(prefix string) string {
	obj := LbPolicy{}
	return obj.MakeKey(prefix)
}

// MakeKey generates a KV store key for the object
func (m *NetworkList) MakeKey(prefix string) string {
	obj := Network{}
	return obj.MakeKey(prefix)
}

// MakeKey generates a KV store key for the object
func (m *ServiceList) MakeKey(prefix string) string {
	obj := Service{}
	return obj.MakeKey(prefix)
}

// MakeKey generates a KV store key for the object
func (m *AutoMsgLbPolicyWatchHelper) MakeKey(prefix string) string {
	obj := LbPolicy{}
	return obj.MakeKey(prefix)
}

// MakeKey generates a KV store key for the object
func (m *AutoMsgNetworkWatchHelper) MakeKey(prefix string) string {
	obj := Network{}
	return obj.MakeKey(prefix)
}

// MakeKey generates a KV store key for the object
func (m *AutoMsgServiceWatchHelper) MakeKey(prefix string) string {
	obj := Service{}
	return obj.MakeKey(prefix)
}

// Clone clones the object into into or creates one of into is nil
func (m *AutoMsgLbPolicyWatchHelper) Clone(into interface{}) (interface{}, error) {
	var out *AutoMsgLbPolicyWatchHelper
	var ok bool
	if into == nil {
		out = &AutoMsgLbPolicyWatchHelper{}
	} else {
		out, ok = into.(*AutoMsgLbPolicyWatchHelper)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AutoMsgLbPolicyWatchHelper) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *AutoMsgLbPolicyWatchHelper_WatchEvent) Clone(into interface{}) (interface{}, error) {
	var out *AutoMsgLbPolicyWatchHelper_WatchEvent
	var ok bool
	if into == nil {
		out = &AutoMsgLbPolicyWatchHelper_WatchEvent{}
	} else {
		out, ok = into.(*AutoMsgLbPolicyWatchHelper_WatchEvent)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AutoMsgLbPolicyWatchHelper_WatchEvent) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *AutoMsgNetworkWatchHelper) Clone(into interface{}) (interface{}, error) {
	var out *AutoMsgNetworkWatchHelper
	var ok bool
	if into == nil {
		out = &AutoMsgNetworkWatchHelper{}
	} else {
		out, ok = into.(*AutoMsgNetworkWatchHelper)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AutoMsgNetworkWatchHelper) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *AutoMsgNetworkWatchHelper_WatchEvent) Clone(into interface{}) (interface{}, error) {
	var out *AutoMsgNetworkWatchHelper_WatchEvent
	var ok bool
	if into == nil {
		out = &AutoMsgNetworkWatchHelper_WatchEvent{}
	} else {
		out, ok = into.(*AutoMsgNetworkWatchHelper_WatchEvent)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AutoMsgNetworkWatchHelper_WatchEvent) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *AutoMsgServiceWatchHelper) Clone(into interface{}) (interface{}, error) {
	var out *AutoMsgServiceWatchHelper
	var ok bool
	if into == nil {
		out = &AutoMsgServiceWatchHelper{}
	} else {
		out, ok = into.(*AutoMsgServiceWatchHelper)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AutoMsgServiceWatchHelper) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *AutoMsgServiceWatchHelper_WatchEvent) Clone(into interface{}) (interface{}, error) {
	var out *AutoMsgServiceWatchHelper_WatchEvent
	var ok bool
	if into == nil {
		out = &AutoMsgServiceWatchHelper_WatchEvent{}
	} else {
		out, ok = into.(*AutoMsgServiceWatchHelper_WatchEvent)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AutoMsgServiceWatchHelper_WatchEvent) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *LbPolicyList) Clone(into interface{}) (interface{}, error) {
	var out *LbPolicyList
	var ok bool
	if into == nil {
		out = &LbPolicyList{}
	} else {
		out, ok = into.(*LbPolicyList)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *LbPolicyList) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *NetworkList) Clone(into interface{}) (interface{}, error) {
	var out *NetworkList
	var ok bool
	if into == nil {
		out = &NetworkList{}
	} else {
		out, ok = into.(*NetworkList)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *NetworkList) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *ServiceList) Clone(into interface{}) (interface{}, error) {
	var out *ServiceList
	var ok bool
	if into == nil {
		out = &ServiceList{}
	} else {
		out, ok = into.(*ServiceList)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *ServiceList) Defaults(ver string) bool {
	return false
}

// Validators

func (m *AutoMsgLbPolicyWatchHelper) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *AutoMsgLbPolicyWatchHelper_WatchEvent) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *AutoMsgNetworkWatchHelper) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *AutoMsgNetworkWatchHelper_WatchEvent) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *AutoMsgServiceWatchHelper) Validate(ver string, ignoreStatus bool) bool {
	for _, v := range m.Events {
		if !v.Validate(ver, ignoreStatus) {
			return false
		}
	}
	return true
}

func (m *AutoMsgServiceWatchHelper_WatchEvent) Validate(ver string, ignoreStatus bool) bool {
	if m.Object != nil && !m.Object.Validate(ver, ignoreStatus) {
		return false
	}
	return true
}

func (m *LbPolicyList) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *NetworkList) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *ServiceList) Validate(ver string, ignoreStatus bool) bool {
	for _, v := range m.Items {
		if !v.Validate(ver, ignoreStatus) {
			return false
		}
	}
	return true
}

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes()

}
