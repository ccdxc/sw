package apiutils

import (
	"context"
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
