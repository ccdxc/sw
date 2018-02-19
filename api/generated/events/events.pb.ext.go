// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package events is a auto generated package.
Input file: protos/events.proto
*/
package events

import (
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/globals"
	validators "github.com/pensando/sw/venice/utils/apigen/validators"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

var _ validators.DummyVar
var funcMapEvents = make(map[string]map[string][]func(interface{}) bool)

// MakeKey generates a KV store key for the object
func (m *Event) MakeKey(prefix string) string {
	return fmt.Sprint(globals.RootPrefix, "/", prefix, "/", "events/", m.Tenant, "/", m.Name)
}

// MakeKey generates a KV store key for the object
func (m *EventPolicy) MakeKey(prefix string) string {
	return fmt.Sprint(globals.RootPrefix, "/", prefix, "/", "eventPolicy/", m.Tenant, "/", m.Name)
}

// MakeKey generates a KV store key for the object
func (m *EventPolicyList) MakeKey(prefix string) string {
	obj := EventPolicy{}
	return obj.MakeKey(prefix)
}

// MakeKey generates a KV store key for the object
func (m *AutoMsgEventPolicyWatchHelper) MakeKey(prefix string) string {
	obj := EventPolicy{}
	return obj.MakeKey(prefix)
}

func (m *AutoMsgEventPolicyWatchHelper) Clone(into interface{}) error {
	out, ok := into.(*AutoMsgEventPolicyWatchHelper)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *Event) Clone(into interface{}) error {
	out, ok := into.(*Event)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *EventAttributes) Clone(into interface{}) error {
	out, ok := into.(*EventAttributes)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *EventPolicy) Clone(into interface{}) error {
	out, ok := into.(*EventPolicy)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *EventPolicyList) Clone(into interface{}) error {
	out, ok := into.(*EventPolicyList)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *EventPolicySpec) Clone(into interface{}) error {
	out, ok := into.(*EventPolicySpec)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *EventPolicyStatus) Clone(into interface{}) error {
	out, ok := into.(*EventPolicyStatus)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

func (m *EventSource) Clone(into interface{}) error {
	out, ok := into.(*EventSource)
	if !ok {
		return fmt.Errorf("mismatched object types")
	}
	*out = *m
	return nil
}

// Validators

func (m *AutoMsgEventPolicyWatchHelper) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *Event) Validate(ver string, ignoreStatus bool) bool {
	if !m.EventAttributes.Validate(ver, ignoreStatus) {
		return false
	}
	return true
}

func (m *EventAttributes) Validate(ver string, ignoreStatus bool) bool {
	if vs, ok := funcMapEvents["EventAttributes"][ver]; ok {
		for _, v := range vs {
			if !v(m) {
				return false
			}
		}
	} else if vs, ok := funcMapEvents["EventAttributes"]["all"]; ok {
		for _, v := range vs {
			if !v(m) {
				return false
			}
		}
	}
	return true
}

func (m *EventPolicy) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *EventPolicyList) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *EventPolicySpec) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *EventPolicyStatus) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *EventSource) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func init() {
	funcMapEvents = make(map[string]map[string][]func(interface{}) bool)

	funcMapEvents["EventAttributes"] = make(map[string][]func(interface{}) bool)
	funcMapEvents["EventAttributes"]["all"] = append(funcMapEvents["EventAttributes"]["all"], func(i interface{}) bool {
		m := i.(*EventAttributes)

		if _, ok := SeverityLevel_value[m.Severity]; !ok {
			return false
		}
		return true
	})

}
