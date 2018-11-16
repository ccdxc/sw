package apiutils

import (
	"context"
	"encoding/json"
	"testing"

	"github.com/pensando/sw/api"
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
		"ThisSsaGoodName23", "123.This", "order-2", "Aasdad_EREA", "00.00.11", "coke-vm-1", "coke_VM-1.23", "coke::VM-1.23",
	}
	for _, c := range goodNames {
		meta.Name = c
		if err := meta.Validate("", "", false); len(err) != 0 {
			t.Fatalf("expecting to succeed [%v](%s)", c, err)
		}
	}

	badNames := []string{
		"This is not a good name", "-NietherIsThis", ".OrThis", "Not#still", "coke-vm-1.", "coke-vm-1-", "coke-vm-1_", "tooLooooooooooooooooooooooooooooooooooooooooooooooooooooooooooonnng",
	}
	for _, c := range badNames {
		meta.Name = c
		if err := meta.Validate("", "", false); len(err) == 0 {
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
