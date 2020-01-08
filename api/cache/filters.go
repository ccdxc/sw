package cache

import (
	"fmt"
	"strconv"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ref"
	"github.com/pensando/sw/venice/utils/runtime"
)

func fromVersionFilterFn(fromVer uint64) filterFn {
	return func(obj, prev runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		ver, err := strconv.ParseUint(meta.ResourceVersion, 10, 64)
		if err != nil {
			log.Fatalf("unable to parse version string [%s](%s)", meta.ResourceVersion, err)
		}
		return ver >= fromVer
	}
}

func nameFilterFn(name string) filterFn {
	return func(obj, prev runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		return meta.Name == name
	}
}

func tenantFilterFn(tenant string) filterFn {
	return func(obj, prev runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		return meta.Tenant == tenant
	}
}

func namespaceFilterFn(namespace string) filterFn {
	return func(obj, prev runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		return meta.Namespace == namespace
	}
}

func labelSelectorFilterFn(selector *labels.Selector) filterFn {
	return func(obj, prev runtime.Object) bool {
		meta, _ := apiutils.MustGetObjectMetaVersion(obj)
		labels := labels.Set(meta.Labels)
		return selector.Matches(labels)
	}
}

func fieldSelectorFilterFn(selector *fields.Selector) filterFn {
	return func(obj, prev runtime.Object) bool {
		return selector.MatchesObj(obj)
	}
}

func fieldChangeSelectorFilterFn(selectors []string) filterFn {
	return func(obj, prev runtime.Object) bool {
		diffs, ok := ref.ObjDiff(obj, prev)
		if !ok {
			return false
		}
		for _, f := range selectors {
			if diffs.Lookup(f) {
				return true
			}
		}
		return false
	}
}

func getFilters(opts api.ListWatchOptions, schemaType string) ([]filterFn, error) {
	var filters []filterFn
	if opts.ResourceVersion != "" {
		ver, err := strconv.ParseUint(opts.ResourceVersion, 10, 64)
		if err != nil {
			log.Fatalf("unable to parse version string [%s](%s)", opts.ResourceVersion, err)
		}
		filters = append(filters, fromVersionFilterFn(ver))
	}

	if opts.Name != "" {
		filters = append(filters, nameFilterFn(opts.Name))
	}

	if opts.Tenant != "" {
		filters = append(filters, tenantFilterFn(opts.Tenant))
	}

	if opts.Namespace != "" {
		filters = append(filters, namespaceFilterFn(opts.Namespace))
	}

	if opts.LabelSelector != "" {
		selector, err := labels.Parse(opts.LabelSelector)
		if err != nil {
			return nil, fmt.Errorf("invalid label selector specification(%s)", err)
		}
		filters = append(filters, labelSelectorFilterFn(selector))
	}

	if opts.FieldSelector != "" {
		var selector *fields.Selector
		var err error
		if schemaType != "" {
			selector, err = fields.ParseWithValidation(schemaType, opts.FieldSelector)
			if err != nil {
				return nil, fmt.Errorf("invalid field selector specification(%s)", err)
			}
		} else {
			selector, err = fields.Parse(opts.FieldSelector)
			if err != nil {
				return nil, fmt.Errorf("invalid field selector specification(%s)", err)
			}
		}
		filters = append(filters, fieldSelectorFilterFn(selector))
	}
	if len(opts.FieldChangeSelector) != 0 {
		filters = append(filters, fieldChangeSelectorFilterFn(opts.FieldChangeSelector))
	}
	return filters, nil
}
