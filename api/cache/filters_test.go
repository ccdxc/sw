package cache

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestFilters(t *testing.T) {
	getDefObject := func() *testObj {
		return &testObj{
			ObjectMeta: api.ObjectMeta{
				Name:            "exampleBook",
				Tenant:          "exampleTenant",
				Namespace:       "exampleNamespace",
				ResourceVersion: "10001",
				Labels:          make(map[string]string),
			},
		}
	}
	cases := []struct {
		name    string
		count   int
		result  bool
		getOpts func() api.ListWatchOptions
		getObjs func() (runtime.Object, runtime.Object)
	}{
		{
			name:   "FromVer Opts[pass exact",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.ResourceVersion = "10001"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "FromVer Opts [fail]",
			count:  1,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.ResourceVersion = "10011"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "FromVer Opts [pass later]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.ResourceVersion = "999"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name Opts [pass]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.Name = "exampleBook"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name Opts [fail]",
			count:  1,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.Name = "anotherBook"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name Namespace [pass]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.Namespace = "exampleNamespace"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name Namespace [fail]",
			count:  1,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.Namespace = "anotherNamespace"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name Tenant [pass]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.Tenant = "exampleTenant"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name Tenant [fail]",
			count:  1,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.Tenant = "anotherTenant"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) { return getDefObject(), nil },
		},
		{
			name:   "Name label selector [pass]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.LabelSelector = "label2 in (value2, value3)"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Labels["label1"] = "value1"
				o.Labels["label2"] = "value2"
				return o, nil
			},
		},
		{
			name:   "Name label selector [fail]",
			count:  1,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.LabelSelector = "label1 in (value2, value3)"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Labels["label1"] = "value1"
				o.Labels["label2"] = "value2"
				return o, nil
			},
		},
		{
			name:   "Name field change selector [pass]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.FieldChangeSelector = []string{"ObjectMeta.Name"}
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Name = "anotherObject"
				return getDefObject(), o
			},
		},
		{
			name:   "Name field change selector multiple [pass]",
			count:  1,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.FieldChangeSelector = []string{"ObjectMeta.Namespace", "ObjectMeta.Name"}
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Name = "anotherObject"
				return getDefObject(), o
			},
		},
		{
			name:   "Name field change selector multiple [fail]",
			count:  1,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.FieldChangeSelector = []string{"ObjectMeta.Namespace", "ObjectMeta.Tenant"}
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Name = "anotherObject"
				return getDefObject(), o
			},
		},
		{
			name:   "multiple filters [pass]",
			count:  4,
			result: true,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.ResourceVersion = "10001"
				opts.Name = "exampleBook"
				opts.Namespace = "exampleNamespace"
				opts.FieldChangeSelector = []string{"ObjectMeta.Name", "ObjectMeta.Tenant"}
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Name = "anotherObject"
				return getDefObject(), o
			},
		},
		{
			name:   "multiple filters [fail]",
			count:  3,
			result: false,
			getOpts: func() api.ListWatchOptions {
				opts := api.ListWatchOptions{}
				opts.ResourceVersion = "10001"
				opts.Name = "exampleBook"
				opts.Namespace = "exampleNamespace"
				return opts
			},
			getObjs: func() (runtime.Object, runtime.Object) {
				o := getDefObject()
				o.Name = "anotherObject"
				return o, nil
			},
		},
	}
	for _, c := range cases {
		t.Logf(" -> Test [ %s ]", c.name)
		opts := c.getOpts()
		obj, prev := c.getObjs()
		filters, err := getFilters(opts)
		if err != nil {
			t.Fatalf("   ** [%s] getFilters failed (%s)", c.name, err)
		}
		if len(filters) != c.count {
			t.Errorf("   ** [%s] expecting [%d] filter got [%d] instead", c.name, c.count, len(filters))
		}
		passFilter := true
		for _, fn := range filters {
			passFilter = passFilter && fn(obj, prev)
		}
		if passFilter != c.result {
			t.Errorf("   ** [%s] expecting to [%v] result got [%v] instead", c.name, c.result, passFilter)
		}
	}
}
