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
