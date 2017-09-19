package runtime

import (
	"testing"
)

func TestScheme(t *testing.T) {
	s := NewScheme()
	s.AddKnownTypes(&TestObj{})
	if _, err := s.New("TestObj"); err != nil {
		t.Fatalf("New failed, error: %v", err)
	}
	if _, err := s.New("testobj2"); err == nil {
		t.Fatalf("New passed when it should have failed")
	}
}
