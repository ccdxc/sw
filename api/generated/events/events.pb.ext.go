// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package events is a auto generated package.
Input file: events.proto
*/
package events

import (
	"errors"
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"

	validators "github.com/pensando/sw/venice/utils/apigen/validators"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

var _ validators.DummyVar
var validatorMapEvents = make(map[string]map[string][]func(string, interface{}) error)

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
	ret = m.EventAttributes.Defaults(ver) || ret
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
		m.Severity = "INFO"
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *EventList) Clone(into interface{}) (interface{}, error) {
	var out *EventList
	var ok bool
	if into == nil {
		out = &EventList{}
	} else {
		out, ok = into.(*EventList)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *EventList) Defaults(ver string) bool {
	var ret bool
	for k := range m.Items {
		if m.Items[k] != nil {
			i := m.Items[k]
			ret = i.Defaults(ver) || ret
		}
	}
	return ret
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

// Validators

func (m *Event) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error

	dlmtr := "."
	if path == "" {
		dlmtr = ""
	}
	npath := path + dlmtr + "EventAttributes"
	if errs := m.EventAttributes.Validate(ver, npath, ignoreStatus); errs != nil {
		ret = append(ret, errs...)
	}
	return ret
}

func (m *EventAttributes) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	if vs, ok := validatorMapEvents["EventAttributes"][ver]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	} else if vs, ok := validatorMapEvents["EventAttributes"]["all"]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	}
	return ret
}

func (m *EventList) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	for k, v := range m.Items {
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := fmt.Sprintf("%s%sItems[%v]", path, dlmtr, k)
		if errs := v.Validate(ver, npath, ignoreStatus); errs != nil {
			ret = append(ret, errs...)
		}
	}
	return ret
}

func (m *EventSource) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

// Transformers

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes()

	validatorMapEvents = make(map[string]map[string][]func(string, interface{}) error)

	validatorMapEvents["EventAttributes"] = make(map[string][]func(string, interface{}) error)
	validatorMapEvents["EventAttributes"]["all"] = append(validatorMapEvents["EventAttributes"]["all"], func(path string, i interface{}) error {
		m := i.(*EventAttributes)

		if _, ok := SeverityLevel_value[m.Severity]; !ok {
			return errors.New("EventAttributes.Severity did not match allowed strings")
		}
		return nil
	})

}
