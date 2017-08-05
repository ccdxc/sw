package runtime

import (
	"testing"
)

func TestMustUint32(t *testing.T) {
	tests := []struct {
		strVal string
		intVal uint32
	}{
		{
			strVal: "100",
			intVal: 100,
		},
		{
			strVal: "1",
			intVal: 1,
		},
		{
			strVal: "1000",
			intVal: 1000,
		},
	}

	for _, test := range tests {
		if MustUint32(test.strVal) != test.intVal {
			t.Fatalf("Expected %v, got %v", test.intVal, MustUint32(test.strVal))
		}
	}
}

func TestNegativeInt(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Fatalf("MustUint32 did not panic with negative number")
		}
	}()
	MustUint32("-100")
}

func TestNonInt(t *testing.T) {
	defer func() {
		t.Logf("Called")
		if r := recover(); r == nil {
			t.Fatalf("MustUint32 did not panic with non number")
		}
	}()
	_ = MustUint32("abc")
}
