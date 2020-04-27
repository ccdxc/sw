package testutils

import (
	"fmt"
	"reflect"
	"sort"

	"github.com/pensando/sw/venice/utils/authz"
)

func sortOperations(operations []authz.Operation) {
	sort.Slice(operations, func(i, j int) bool {
		if operations[i] == nil {
			return true
		}
		if operations[j] == nil {
			return false
		}
		return fmt.Sprintf("%s%s%s", operations[i].GetResource().GetKind(), operations[i].GetResource().GetName(), operations[i].GetAction()) < fmt.Sprintf("%s%s%s", operations[j].GetResource().GetKind(), operations[j].GetResource().GetName(), operations[j].GetAction())
	})
}

// AreOperationsEqual compares two authz.Operation slices by first sorting them by kind, name and action
func AreOperationsEqual(expected []authz.Operation, returned []authz.Operation) bool {
	sortOperations(expected)
	sortOperations(returned)
	if len(expected) != len(returned) {
		return false
	}
	for i := 0; i < len(expected); i++ {
		if expected[i] == returned[i] {
			continue
		}
		if expected[i] == nil || returned[i] == nil {
			return false
		}
		if !(expected[i].GetAuditAction() == returned[i].GetAuditAction() &&
			expected[i].GetAction() == returned[i].GetAction() &&
			reflect.DeepEqual(expected[i].GetResource(), returned[i].GetResource())) {
			return false
		}
	}
	return true
}
