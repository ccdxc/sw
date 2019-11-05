package apiutils

import (
	"context"
	"encoding/json"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestDryRun(t *testing.T) {
	ctx := context.Background()
	nctx := setDryRun(ctx, 100)
	if !IsDryRun(nctx) {
		t.Fatalf("expecting to return true")
	}
	if IsDryRun(ctx) {
		t.Fatalf("expecting to return false")
	}
}

func TestObjectMetaValidation(t *testing.T) {
	meta := api.ObjectMeta{}
	goodNames := []string{
		"ThisSsaGoodName23", "123.This", "order-2", "Aasdad_EREA", "00.00.11", "coke-vm-1", "coke_VM-1.23", "coke--VM-1.23",
	}
	for _, c := range goodNames {
		meta.Name = c
		if err := meta.Validate("", "", false, false); len(err) != 0 {
			t.Fatalf("expecting to succeed [%v](%s)", c, err)
		}
	}

	badNames := []string{
		"This is not a good name", "-NietherIsThis", ".OrThis", "Not#still", "coke-vm-1.", "coke-vm-1-", "coke-vm-1_", "tooLooooooooooooooooooooooooooooooooooooooooooooooooooooooooooonnng",
		"_tenant", "+tenant", ".", "..", "ten/ant/", "t*enant", "t<en>", "t//enant", "ten|ant", "-", "coke:VM-1.23",
	}
	for _, c := range badNames {
		meta.Name = c
		if err := meta.Validate("", "", false, false); len(err) == 0 {
			t.Fatalf("expecting to fail [%v](%s)", c, err)
		}
	}
}

type testStruct struct {
	Values []*api.Interface
}

func TestIntf(t *testing.T) {
	obj := &testStruct{
		Values: []*api.Interface{
			&api.Interface{Value: &api.Interface_Interfaces{
				Interfaces: &api.InterfaceSlice{
					Values: []*api.Interface{
						&api.Interface{Value: &api.Interface_Str{Str: "Test Sting"}},
						&api.Interface{Value: &api.Interface_Str{Str: "Another string"}},
						&api.Interface{Value: &api.Interface_Bool{Bool: true}},
						nil,
						&api.Interface{Value: &api.Interface_Float{Float: 10.111}},
					},
				},
			},
			},
			&api.Interface{Value: &api.Interface_Interfaces{
				Interfaces: &api.InterfaceSlice{
					Values: []*api.Interface{
						&api.Interface{Value: &api.Interface_Str{Str: "Test Sting2"}},
						&api.Interface{Value: &api.Interface_Str{Str: "yet Another string"}},
						&api.Interface{Value: &api.Interface_Bool{Bool: true}},
						&api.Interface{Value: &api.Interface_Float{Float: 10.222}},
						nil,
					},
				},
			},
			},
		},
	}
	m, err := json.Marshal(obj)
	if err != nil {
		t.Fatalf("Could not marshal json (%s)", err)
	}
	exp := "{\"Values\":[[\"Test Sting\",\"Another string\",true,null,10.111],[\"Test Sting2\",\"yet Another string\",true,10.222,null]]}"
	if exp != string(m) {
		t.Fatalf("did not get expected marshalled value want:\n%v\ngot\n%v", exp, string(m))
	}
}

func TestSetGetVar(t *testing.T) {
	ctx := context.Background()
	_, ok := GetVar(ctx, "var1")
	if ok {
		t.Errorf("Found var1 on empty")
	}
	ctx = SetVar(ctx, "var1", "one")
	vi, ok := GetVar(ctx, "var1")
	if !ok {
		t.Errorf("did not find var1")
	}
	if vi.(string) != "one" {
		t.Errorf("mismatch - want [one] got [%v]", vi.(string))
	}
}

type var1 struct{}
type var2 struct{}
type var3 struct{}
type var4 struct{}
type var5 struct{}
type dummy struct {
	a string
}

func BenchmarkGetVarOneCtx(b *testing.B) {
	ctx := context.Background()
	ctx = SetVar(ctx, "var1", "one")
	SetVar(ctx, "var2", "two")
	SetVar(ctx, "var3", "three")
	SetVar(ctx, "var4", "four")
	SetVar(ctx, "var5", "five")
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		GetVar(ctx, "var3")
	}
}

func BenchmarkGetVarChildContext(b *testing.B) {
	ctx := context.Background()
	ctx = SetVar(ctx, "var1", "one")
	SetVar(ctx, "var2", "two")
	SetVar(ctx, "var3", "three")
	SetVar(ctx, "var4", "four")
	SetVar(ctx, "var5", "five")
	key := dummy{"Dummy1"}
	ctx = context.WithValue(ctx, key, "")
	key.a = "Dummy2"
	ctx = context.WithValue(ctx, key, "")
	key.a = "Dummy3"
	ctx = context.WithValue(ctx, key, "")
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		GetVar(ctx, "var3")
	}
}

func BenchmarkContext5Deep(b *testing.B) {
	ctx := context.Background()
	ctx = context.WithValue(ctx, var1{}, "one")
	ctx = context.WithValue(ctx, var2{}, "two")
	ctx = context.WithValue(ctx, var3{}, "three")
	ctx = context.WithValue(ctx, var4{}, "four")
	ctx = context.WithValue(ctx, var5{}, "five")

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		ctx.Value(var3{})
	}
}

func BenchmarkMapGet(b *testing.B) {
	m := make(map[string]string)
	m["var1"] = "one"
	m["var2"] = "one"
	m["var3"] = "one"
	m["var4"] = "one"
	m["var5"] = "one"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = m["var3"]
	}
}

func BenchmarkVarGet(b *testing.B) {
	var x = "One"
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = x
	}

}

func TestGetQueryStringFromListWatchOptions(t *testing.T) {
	opts := api.ListWatchOptions{}
	r := GetQueryStringFromListWatchOptions(&opts)
	if r != "" {
		t.Fatalf("expecting empty string got [%s]", r)
	}

	opts.Name = "foo"
	opts.Tenant = "tenant1"
	opts.FieldChangeSelector = []string{"Spec.ABC", "status.in"}
	exp := "name=foo&tenant=tenant1&field-change-selector=Spec.ABC&field-change-selector=status.in"
	r = GetQueryStringFromListWatchOptions(&opts)
	if r != exp {
		t.Fatalf("expecting empty string got [%s]", r)
	}
	opts.FieldSelector = "x.x=test,y.y>=6,z.z<7"
	opts.LabelSelector = "x in (a,b),y notin (a,b)"
	opts.MaxResults = 2
	opts.From = 10
	exp = "name=foo&tenant=tenant1&label-selector=x in (a,b),y notin (a,b)&field-selector=x.x=test,y.y>=6,z.z<7&field-change-selector=Spec.ABC&field-change-selector=status.in&from=10&max-results=2"
	r = GetQueryStringFromListWatchOptions(&opts)
	if r != exp {
		t.Fatalf("expecting empty string got [%s]", r)
	}
}

func TestRequirements(t *testing.T) {
	ctx := context.Background()

	v, err := GetRequirements(ctx)
	if err == nil {
		t.Errorf("not expected to succeed")
	}
	if v != nil {
		t.Errorf("recevied non nil value on error")
	}
	ctx = SetRequirements(ctx, nil)
	v, err = GetRequirements(ctx)
	if err != nil {
		t.Errorf("expected to succeed")
	}
	if v != nil {
		t.Errorf("recevied non nil value")
	}

	ctx = SetRequirements(ctx, "success")
	v, err = GetRequirements(ctx)
	if err != nil {
		t.Errorf("expected to succeed")
	}
	if v == nil {
		t.Errorf("recevied nil value")
	}
	if v1, ok := v.(string); !ok {
		t.Fatalf("retrieved wrong kind")
	} else {
		if v1 != "success" {
			t.Fatalf("wrong value [%v]", v1)
		}
	}
}

func TestRelMapper(t *testing.T) {
	types := map[string]*api.Struct{
		"grp1.Type1": &api.Struct{
			Kind:     "Type1",
			APIGroup: "grp1",
			Scopes:   []string{"Tenant"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "test.Type2"},
				"Fld2": api.Field{Name: "Fld2", JSONTag: "fld2", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp1.Type2": &api.Struct{
			Kind:     "Type2",
			APIGroup: "grp1",
			Scopes:   []string{"Tenant"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp2.Type3": &api.Struct{
			Kind:     "Type3",
			APIGroup: "grp2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp1.Type4": &api.Struct{
			Kind:     "Type4",
			APIGroup: "grp1",
			Scopes:   []string{"Tenant"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp2.Type5": &api.Struct{
			Kind:     "Type5",
			APIGroup: "grp2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp2.Type6": &api.Struct{
			Kind:     "Type6",
			APIGroup: "grp2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp2.Type7": &api.Struct{
			Kind:     "Type7",
			APIGroup: "grp2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp1.Type8": &api.Struct{
			Kind:     "Type8",
			APIGroup: "grp1",
			Scopes:   []string{"Tenant"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp2.Type9": &api.Struct{
			Kind:     "Type9",
			APIGroup: "grp2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
		"grp2.Type10": &api.Struct{
			Kind:     "Type10",
			APIGroup: "grp2",
			Scopes:   []string{"Cluster"},
			Fields: map[string]api.Field{
				"Fld1": api.Field{Name: "Fld1", JSONTag: "fld1", Pointer: false, Slice: true, Map: false, Type: "TYPE_INT32"},
			},
		},
	}
	sch := runtime.NewScheme()
	sch.AddSchema(types)
	t.Logf("kinds are %+v", sch.Kinds())
	relMap := map[string][]apiintf.ObjRelation{
		"grp1.Type1": []apiintf.ObjRelation{
			{Type: apiintf.NamedReference, To: "grp1.Type2"},
			{Type: apiintf.NamedReference, To: "grp2.Type3"},
			{Type: apiintf.WeakReference, To: "grp1.Type4"},
		},
		"grp2.Type3": []apiintf.ObjRelation{
			{Type: apiintf.NamedReference, To: "grp2.Type6"},
		},
		"grp2.Type5": []apiintf.ObjRelation{
			{Type: apiintf.NamedReference, To: "grp2.Type6"},
			{Type: apiintf.NamedReference, To: "grp2.Type7"},
		},
		"grp2.Type6": []apiintf.ObjRelation{
			{Type: apiintf.NamedReference, To: "grp1.Type8"},
		},
		"grp2.Type7": []apiintf.ObjRelation{
			{Type: apiintf.NamedReference, To: "grp2.Type9"},
		},
	}
	expDeps := map[string][]string{
		"grp1.Type1":     []string{"grp1.Type1", "grp1.Type2", "grp2.Type3", "grp2.Type6", "grp1.Type8"},
		"grp1.Type2":     []string{"grp1.Type2"},
		"grp2.Type3":     []string{"grp2.Type3", "grp2.Type6", "grp1.Type8"},
		"grp1.Type4":     []string{"grp1.Type4"},
		"grp2.Type5":     []string{"grp2.Type5", "grp2.Type6", "grp2.Type7", "grp1.Type8", "grp2.Type9"},
		"grp2.Type6":     []string{"grp2.Type6", "grp1.Type8"},
		"grp2.Type7":     []string{"grp2.Type7", "grp2.Type9"},
		"grp1.Type8":     []string{"grp1.Type8"},
		"grp2.Type9":     []string{"grp2.Type9"},
		"grp2.Type10":    []string{"grp2.Type10"},
		"cluster.Tenant": []string{"cluster.Tenant", "grp1.Type1", "grp1.Type2", "grp2.Type3", "grp1.Type4", "grp2.Type6", "grp1.Type8"},
	}
	orm := &ObjRelMapper{}
	orm.init(sch, relMap)
	t.Logf("Ordered List is [%v]", orm.orderedKind)
	if len(orm.orderedKind) != len(expDeps) {
		t.Fatalf("mismatched number of entries [%v/%v]", len(orm.orderedKind), len(expDeps))
	}
	ordIndex := make(map[string]int)
	for i, k := range orm.GetOrderedKinds() {
		ordIndex[k] = i
	}
	validateKind := func(k string, olist []string) {
		exp := expDeps[k]
		actual, err := orm.GetDependencies(k)
		if err != nil {
			t.Fatalf("kind not found [%v]", k)
		}
		sort.Strings(exp)
		sort.Strings(actual)
		if len(exp) != len(actual) {
			t.Fatalf("number of dependencies dont match [%v][%v/%v]", k, exp, actual)
		}

		if len(exp) > 0 && !reflect.DeepEqual(exp, actual) {
			t.Fatalf("Dependencies dont match for [%s] a:[%v] e:[%v]", k, actual, exp)
		}
		found := false
		for i, v := range orm.orderedKind {
			if v != k {
				continue
			}
			found = true
			for _, v1 := range actual {
				if i1, ok := ordIndex[v1]; !ok {
					t.Fatalf("did not find kind in ordered list [%v]", v1)
				} else {
					if i1 > i {
						t.Fatalf("Wrong ordered list for [%v]->[%v] in [%v]", k, v1, orm.orderedKind)
					}
				}
			}
		}
		if !found {
			t.Fatalf("kind not found in ordered List [%v]", k)
		}
	}

	for _, k := range orm.orderedKind {
		validateKind(k, orm.orderedKind)
	}

	// Unknown kind
	_, err := orm.GetDependencies("type.Unknown")
	if err == nil {
		t.Errorf("expected to fail")
	}

	// Add a circular dependency
	relMap["grp2.Type6"] = append(relMap["grp2.Type6"], apiintf.ObjRelation{Type: apiintf.NamedReference, To: "cluster.Tenant"})

	func() {
		defer func() {
			if r := recover(); r == nil {
				t.Fatalf("The code did not panic")
			}
		}()
		orm.init(sch, relMap)
	}()
}
