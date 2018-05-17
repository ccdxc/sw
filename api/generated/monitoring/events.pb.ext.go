// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package monitoring is a auto generated package.
Input file: events.proto
*/
package monitoring

import (
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"

	validators "github.com/pensando/sw/venice/utils/apigen/validators"

	"github.com/pensando/sw/venice/globals"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

var _ validators.DummyVar
var validatorMapEvents = make(map[string]map[string][]func(interface{}) bool)

// MakeKey generates a KV store key for the object
func (m *Event) MakeKey(prefix string) string {
	return fmt.Sprint(globals.RootPrefix, "/", prefix, "/", "events/", m.Tenant, "/", m.Name)
}

// MakeKey generates a KV store key for the object
func (m *EventPolicy) MakeKey(prefix string) string {
	return fmt.Sprint(globals.RootPrefix, "/", prefix, "/", "eventPolicy/", m.Tenant, "/", m.Name)
}

// Clone clones the object into into or creates one of into is nil
func (m *Event) Clone(into interface{}) (interface{}, error) {
	var out *Event
	var ok bool
	if into == nil {
		out = &Event{}
	} else {
		out, ok = into.(*Event)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *Event) Defaults(ver string) bool {
	var ret bool
	ret = ret || m.EventAttributes.Defaults(ver)
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *EventAttributes) Clone(into interface{}) (interface{}, error) {
	var out *EventAttributes
	var ok bool
	if into == nil {
		out = &EventAttributes{}
	} else {
		out, ok = into.(*EventAttributes)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventAttributes) Defaults(ver string) bool {
	var ret bool
	ret = true
	switch ver {
	default:
		m.Severity = SeverityLevel_name[0]
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *EventExport) Clone(into interface{}) (interface{}, error) {
	var out *EventExport
	var ok bool
	if into == nil {
		out = &EventExport{}
	} else {
		out, ok = into.(*EventExport)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventExport) Defaults(ver string) bool {
	var ret bool
	ret = true
	switch ver {
	default:
		m.Format = MonitoringExportFormat_name[0]
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *EventPolicy) Clone(into interface{}) (interface{}, error) {
	var out *EventPolicy
	var ok bool
	if into == nil {
		out = &EventPolicy{}
	} else {
		out, ok = into.(*EventPolicy)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventPolicy) Defaults(ver string) bool {
	var ret bool
	ret = ret || m.Spec.Defaults(ver)
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *EventPolicySpec) Clone(into interface{}) (interface{}, error) {
	var out *EventPolicySpec
	var ok bool
	if into == nil {
		out = &EventPolicySpec{}
	} else {
		out, ok = into.(*EventPolicySpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventPolicySpec) Defaults(ver string) bool {
	var ret bool
	for k := range m.Exports {
		if m.Exports[k] != nil {
			ret = ret || m.Exports[k].Defaults(ver)
		}
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *EventPolicyStatus) Clone(into interface{}) (interface{}, error) {
	var out *EventPolicyStatus
	var ok bool
	if into == nil {
		out = &EventPolicyStatus{}
	} else {
		out, ok = into.(*EventPolicyStatus)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventPolicyStatus) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *EventSource) Clone(into interface{}) (interface{}, error) {
	var out *EventSource
	var ok bool
	if into == nil {
		out = &EventSource{}
	} else {
		out, ok = into.(*EventSource)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventSource) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *EventsList) Clone(into interface{}) (interface{}, error) {
	var out *EventsList
	var ok bool
	if into == nil {
		out = &EventsList{}
	} else {
		out, ok = into.(*EventsList)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventsList) Defaults(ver string) bool {
	var ret bool
	for k := range m.Events {
		if m.Events[k] != nil {
			ret = ret || m.Events[k].Defaults(ver)
		}
	}
	return ret
}

// Validators

func (m *Event) Validate(ver string, ignoreStatus bool) bool {
	if !m.EventAttributes.Validate(ver, ignoreStatus) {
		return false
	}
	return true
}

func (m *EventAttributes) Validate(ver string, ignoreStatus bool) bool {
	if vs, ok := validatorMapEvents["EventAttributes"][ver]; ok {
		for _, v := range vs {
			if !v(m) {
				return false
			}
		}
	} else if vs, ok := validatorMapEvents["EventAttributes"]["all"]; ok {
		for _, v := range vs {
			if !v(m) {
				return false
			}
		}
	}
	return true
}

func (m *EventExport) Validate(ver string, ignoreStatus bool) bool {
	if m.Selector != nil && !m.Selector.Validate(ver, ignoreStatus) {
		return false
	}
	if vs, ok := validatorMapEvents["EventExport"][ver]; ok {
		for _, v := range vs {
			if !v(m) {
				return false
			}
		}
	} else if vs, ok := validatorMapEvents["EventExport"]["all"]; ok {
		for _, v := range vs {
			if !v(m) {
				return false
			}
		}
	}
	return true
}

func (m *EventPolicy) Validate(ver string, ignoreStatus bool) bool {
	if !m.Spec.Validate(ver, ignoreStatus) {
		return false
	}
	return true
}

func (m *EventPolicySpec) Validate(ver string, ignoreStatus bool) bool {
	for _, v := range m.Exports {
		if !v.Validate(ver, ignoreStatus) {
			return false
		}
	}
	return true
}

func (m *EventPolicyStatus) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *EventSource) Validate(ver string, ignoreStatus bool) bool {
	return true
}

func (m *EventsList) Validate(ver string, ignoreStatus bool) bool {
	for _, v := range m.Events {
		if !v.Validate(ver, ignoreStatus) {
			return false
		}
	}
	return true
}

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes(
		&Event{},
		&EventPolicy{},
	)

	validatorMapEvents = make(map[string]map[string][]func(interface{}) bool)

	validatorMapEvents["EventAttributes"] = make(map[string][]func(interface{}) bool)
	validatorMapEvents["EventAttributes"]["all"] = append(validatorMapEvents["EventAttributes"]["all"], func(i interface{}) bool {
		m := i.(*EventAttributes)

		if _, ok := SeverityLevel_value[m.Severity]; !ok {
			return false
		}
		return true
	})

	validatorMapEvents["EventExport"] = make(map[string][]func(interface{}) bool)
	validatorMapEvents["EventExport"]["all"] = append(validatorMapEvents["EventExport"]["all"], func(i interface{}) bool {
		m := i.(*EventExport)

		if _, ok := MonitoringExportFormat_value[m.Format]; !ok {
			return false
		}
		return true
	})

}
