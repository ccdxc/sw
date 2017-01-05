package dh

import (
	"testing"
)

func TestDHSecretForAllGroups(t *testing.T) {
	groups := []GroupID{Group1, Group2, Group5, Group14, Group15}

	for _, group := range groups {
		k1, err := NewKey(group)
		if err != nil {
			t.Fatalf("Failed to create DH key for group %v", group)
		}

		k2, err := NewKey(group)
		if err != nil {
			t.Fatalf("Failed to create DH key for group %v", group)
		}

		secret1, err := k1.ComputeSecret(k2.gx)
		if err != nil {
			t.Fatalf("Failed to compute secret for group %v", group)
		}

		secret2, err := k2.ComputeSecret(k1.gx)
		if err != nil {
			t.Fatalf("Failed to compute secret for group %v", group)
		}

		if secret1.Cmp(secret2) != 0 {
			t.Fatalf("Secrets did not match for group %v: %v, %v", group, secret1.String(), secret2.String())
		}
		t.Logf("Secret matched for group %v", group)
	}
}
