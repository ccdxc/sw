// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package monitoring is a auto generated package.
Input file: mirror.proto
*/
package monitoring

import (
	"errors"
	fmt "fmt"

	listerwatcher "github.com/pensando/sw/api/listerwatcher"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"

	validators "github.com/pensando/sw/venice/utils/apigen/validators"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Dummy definitions to suppress nonused warnings
var _ kvstore.Interface
var _ log.Logger
var _ listerwatcher.WatcherClient

var _ validators.DummyVar
var validatorMapMirror = make(map[string]map[string][]func(string, interface{}) error)

// MakeKey generates a KV store key for the object
func (m *MirrorSession) MakeKey(prefix string) string {
	return fmt.Sprint(globals.ConfigRootPrefix, "/", prefix, "/", "MirrorSession/", m.Tenant, "/", m.Name)
}

func (m *MirrorSession) MakeURI(cat, ver, prefix string) string {
	in := m
	return fmt.Sprint("/", cat, "/", prefix, "/", ver, "/tenant/", in.Tenant, "/MirrorSession/", in.Name)
}

// Clone clones the object into into or creates one of into is nil
func (m *AppProtoSelector) Clone(into interface{}) (interface{}, error) {
	var out *AppProtoSelector
	var ok bool
	if into == nil {
		out = &AppProtoSelector{}
	} else {
		out, ok = into.(*AppProtoSelector)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *AppProtoSelector) Defaults(ver string) bool {
	var ret bool
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *MatchRule) Clone(into interface{}) (interface{}, error) {
	var out *MatchRule
	var ok bool
	if into == nil {
		out = &MatchRule{}
	} else {
		out, ok = into.(*MatchRule)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MatchRule) Defaults(ver string) bool {
	var ret bool
	if m.AppProtoSel != nil {
		ret = m.AppProtoSel.Defaults(ver) || ret
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *MatchSelector) Clone(into interface{}) (interface{}, error) {
	var out *MatchSelector
	var ok bool
	if into == nil {
		out = &MatchSelector{}
	} else {
		out, ok = into.(*MatchSelector)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MatchSelector) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *MirrorCollector) Clone(into interface{}) (interface{}, error) {
	var out *MirrorCollector
	var ok bool
	if into == nil {
		out = &MirrorCollector{}
	} else {
		out, ok = into.(*MirrorCollector)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MirrorCollector) Defaults(ver string) bool {
	var ret bool
	ret = true
	switch ver {
	default:
		m.Type = "VENICE"
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *MirrorSession) Clone(into interface{}) (interface{}, error) {
	var out *MirrorSession
	var ok bool
	if into == nil {
		out = &MirrorSession{}
	} else {
		out, ok = into.(*MirrorSession)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MirrorSession) Defaults(ver string) bool {
	m.Kind = "MirrorSession"
	m.Tenant, m.Namespace = "default", "default"
	var ret bool
	ret = m.Spec.Defaults(ver) || ret
	ret = m.Status.Defaults(ver) || ret
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *MirrorSessionSpec) Clone(into interface{}) (interface{}, error) {
	var out *MirrorSessionSpec
	var ok bool
	if into == nil {
		out = &MirrorSessionSpec{}
	} else {
		out, ok = into.(*MirrorSessionSpec)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MirrorSessionSpec) Defaults(ver string) bool {
	var ret bool
	for k := range m.Collectors {
		i := m.Collectors[k]
		ret = i.Defaults(ver) || ret
	}
	for k := range m.MatchRules {
		i := m.MatchRules[k]
		ret = i.Defaults(ver) || ret
	}
	ret = m.StopConditions.Defaults(ver) || ret
	ret = true
	switch ver {
	default:
		for k := range m.PacketFilters {
			m.PacketFilters[k] = "ALL_PKTS"
		}
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *MirrorSessionStatus) Clone(into interface{}) (interface{}, error) {
	var out *MirrorSessionStatus
	var ok bool
	if into == nil {
		out = &MirrorSessionStatus{}
	} else {
		out, ok = into.(*MirrorSessionStatus)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MirrorSessionStatus) Defaults(ver string) bool {
	var ret bool
	ret = true
	switch ver {
	default:
		m.State = "NONE"
	}
	return ret
}

// Clone clones the object into into or creates one of into is nil
func (m *MirrorStartConditions) Clone(into interface{}) (interface{}, error) {
	var out *MirrorStartConditions
	var ok bool
	if into == nil {
		out = &MirrorStartConditions{}
	} else {
		out, ok = into.(*MirrorStartConditions)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MirrorStartConditions) Defaults(ver string) bool {
	return false
}

// Clone clones the object into into or creates one of into is nil
func (m *MirrorStopConditions) Clone(into interface{}) (interface{}, error) {
	var out *MirrorStopConditions
	var ok bool
	if into == nil {
		out = &MirrorStopConditions{}
	} else {
		out, ok = into.(*MirrorStopConditions)
		if !ok {
			return nil, fmt.Errorf("mismatched object types")
		}
	}
	*out = *m
	return out, nil
}

// Default sets up the defaults for the object
func (m *MirrorStopConditions) Defaults(ver string) bool {
	var ret bool
	ret = true
	switch ver {
	default:
		m.ExpiryDuration = "2h"
	}
	return ret
}

// Validators

func (m *AppProtoSelector) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	if vs, ok := validatorMapMirror["AppProtoSelector"][ver]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	} else if vs, ok := validatorMapMirror["AppProtoSelector"]["all"]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	}
	return ret
}

func (m *MatchRule) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	if m.AppProtoSel != nil {
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := path + dlmtr + "AppProtoSel"
		if errs := m.AppProtoSel.Validate(ver, npath, ignoreStatus); errs != nil {
			ret = append(ret, errs...)
		}
	}
	return ret
}

func (m *MatchSelector) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *MirrorCollector) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	if m.ExportCfg != nil {
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := path + dlmtr + "ExportCfg"
		if errs := m.ExportCfg.Validate(ver, npath, ignoreStatus); errs != nil {
			ret = append(ret, errs...)
		}
	}
	if vs, ok := validatorMapMirror["MirrorCollector"][ver]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	} else if vs, ok := validatorMapMirror["MirrorCollector"]["all"]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	}
	return ret
}

func (m *MirrorSession) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	{
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		ret = m.ObjectMeta.Validate(ver, path+dlmtr+"ObjectMeta", ignoreStatus)
	}

	dlmtr := "."
	if path == "" {
		dlmtr = ""
	}
	npath := path + dlmtr + "Spec"
	if errs := m.Spec.Validate(ver, npath, ignoreStatus); errs != nil {
		ret = append(ret, errs...)
	}
	if !ignoreStatus {

		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := path + dlmtr + "Status"
		if errs := m.Status.Validate(ver, npath, ignoreStatus); errs != nil {
			ret = append(ret, errs...)
		}
	}
	return ret
}

func (m *MirrorSessionSpec) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	for k, v := range m.Collectors {
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := fmt.Sprintf("%s%sCollectors[%v]", path, dlmtr, k)
		if errs := v.Validate(ver, npath, ignoreStatus); errs != nil {
			ret = append(ret, errs...)
		}
	}
	for k, v := range m.MatchRules {
		dlmtr := "."
		if path == "" {
			dlmtr = ""
		}
		npath := fmt.Sprintf("%s%sMatchRules[%v]", path, dlmtr, k)
		if errs := v.Validate(ver, npath, ignoreStatus); errs != nil {
			ret = append(ret, errs...)
		}
	}

	dlmtr := "."
	if path == "" {
		dlmtr = ""
	}
	npath := path + dlmtr + "StopConditions"
	if errs := m.StopConditions.Validate(ver, npath, ignoreStatus); errs != nil {
		ret = append(ret, errs...)
	}
	if vs, ok := validatorMapMirror["MirrorSessionSpec"][ver]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	} else if vs, ok := validatorMapMirror["MirrorSessionSpec"]["all"]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	}
	return ret
}

func (m *MirrorSessionStatus) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	if vs, ok := validatorMapMirror["MirrorSessionStatus"][ver]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	} else if vs, ok := validatorMapMirror["MirrorSessionStatus"]["all"]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	}
	return ret
}

func (m *MirrorStartConditions) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	return ret
}

func (m *MirrorStopConditions) Validate(ver, path string, ignoreStatus bool) []error {
	var ret []error
	if vs, ok := validatorMapMirror["MirrorStopConditions"][ver]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	} else if vs, ok := validatorMapMirror["MirrorStopConditions"]["all"]; ok {
		for _, v := range vs {
			if err := v(path, m); err != nil {
				ret = append(ret, err)
			}
		}
	}
	return ret
}

// Transformers

func init() {
	scheme := runtime.GetDefaultScheme()
	scheme.AddKnownTypes(
		&MirrorSession{},
	)

	validatorMapMirror = make(map[string]map[string][]func(string, interface{}) error)

	validatorMapMirror["AppProtoSelector"] = make(map[string][]func(string, interface{}) error)

	validatorMapMirror["AppProtoSelector"]["all"] = append(validatorMapMirror["AppProtoSelector"]["all"], func(path string, i interface{}) error {
		m := i.(*AppProtoSelector)
		for k, v := range m.Ports {
			if !validators.ProtoPort(v) {
				return fmt.Errorf("%v[%v] validation failed", path+"."+"Ports", k)
			}
		}

		return nil
	})

	validatorMapMirror["MirrorCollector"] = make(map[string][]func(string, interface{}) error)
	validatorMapMirror["MirrorCollector"]["all"] = append(validatorMapMirror["MirrorCollector"]["all"], func(path string, i interface{}) error {
		m := i.(*MirrorCollector)

		if _, ok := PacketCollectorType_value[m.Type]; !ok {
			return errors.New("MirrorCollector.Type did not match allowed strings")
		}
		return nil
	})

	validatorMapMirror["MirrorSessionSpec"] = make(map[string][]func(string, interface{}) error)
	validatorMapMirror["MirrorSessionSpec"]["all"] = append(validatorMapMirror["MirrorSessionSpec"]["all"], func(path string, i interface{}) error {
		m := i.(*MirrorSessionSpec)

		for k, v := range m.PacketFilters {
			if _, ok := MirrorSessionSpec_MirrorPacketFilter_value[v]; !ok {
				return fmt.Errorf("%v[%v] did not match allowed strings", path+"."+"PacketFilters", k)
			}
		}
		return nil
	})

	validatorMapMirror["MirrorSessionStatus"] = make(map[string][]func(string, interface{}) error)
	validatorMapMirror["MirrorSessionStatus"]["all"] = append(validatorMapMirror["MirrorSessionStatus"]["all"], func(path string, i interface{}) error {
		m := i.(*MirrorSessionStatus)

		if _, ok := MirrorSessionState_value[m.State]; !ok {
			return errors.New("MirrorSessionStatus.State did not match allowed strings")
		}
		return nil
	})

	validatorMapMirror["MirrorStopConditions"] = make(map[string][]func(string, interface{}) error)
	validatorMapMirror["MirrorStopConditions"]["all"] = append(validatorMapMirror["MirrorStopConditions"]["all"], func(path string, i interface{}) error {
		m := i.(*MirrorStopConditions)
		args := make([]string, 0)
		args = append(args, "0")
		args = append(args, "2h")

		if !validators.Duration(m.ExpiryDuration, args) {
			return fmt.Errorf("%v failed validation", path+"."+"ExpiryDuration")
		}
		return nil
	})

}
