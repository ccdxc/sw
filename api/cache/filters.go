package cache

import (
	"strconv"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

func fromVersionFilterFn(fromVer uint64) filterFn {
	return func(obj runtime.Object) bool {
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
	return func(obj runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		return meta.Name == name
	}
}

func tenantFilterFn(tenant string) filterFn {
	return func(obj runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		return meta.Tenant == tenant
	}
}

func namespaceFilterFn(namespace string) filterFn {
	return func(obj runtime.Object) bool {
		objm := obj.(runtime.ObjectMetaAccessor)
		meta := objm.GetObjectMeta()
		return meta.Namespace == namespace
	}
}

func getFilters(opts api.ListWatchOptions) []filterFn {
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

	return filters
}
