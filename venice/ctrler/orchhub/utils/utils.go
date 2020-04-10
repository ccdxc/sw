package utils

import (
	"fmt"
	"strings"

	"github.com/pensando/sw/venice/globals"
)

// Global constants
const (
	// Delim delimiter used when creating name when exposing objects between orch and venice
	Delim = "-"
	// Indicates managing all datacenters
	ManageAllDcs = "all_namespaces"
)

var (
	// OrchNameKey is the key used to label the orchestrator being used
	OrchNameKey = fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "orch-name")
	// NamespaceKey is the namespace in the orchestrator
	NamespaceKey = fmt.Sprintf("%s%s", globals.SystemLabelPrefix, "namespace")
)

// CreateGlobalKey creates a global key
func CreateGlobalKey(orchID, namespace, objName string) string {
	return fmt.Sprintf("%s%s%s%s%s", orchID, Delim, namespace, Delim, objName)
}

// ParseGlobalKey returns objName from globalKey
func ParseGlobalKey(orchID, namespace, globalKey string) (key string) {
	return strings.TrimPrefix(globalKey, CreateGlobalKey(orchID, namespace, ""))
}

// CreateGlobalKeyPrefix creates the key prefix
func CreateGlobalKeyPrefix(orchID, namespace string) string {
	return fmt.Sprintf("%s-%s", orchID, namespace)
}

// AddOrchNameLabel adds orchestrator name into the labels
func AddOrchNameLabel(labels map[string]string, name string) {
	labels[OrchNameKey] = name
}

// AddOrchNamespaceLabel adds namespace into the labels
func AddOrchNamespaceLabel(labels map[string]string, name string) {
	labels[NamespaceKey] = name
}

// IsObjForOrch returns whether the labels indicate the object belongs to the orchID.
// If namespace is supplied, it will also check if it belongs to that namespace.
func IsObjForOrch(labels map[string]string, orchID string, namespace string) bool {
	if labels == nil {
		return false
	}
	orch, ok := labels[OrchNameKey]
	if !ok || orch != orchID {
		return false
	}
	if len(namespace) > 0 {
		ns, ok := labels[NamespaceKey]
		if !ok || ns != namespace {
			return false
		}
	}
	return true
}

// GetOrchNameFromObj returns the orch associated with the given labels
func GetOrchNameFromObj(labels map[string]string) (string, bool) {
	orch, ok := labels[OrchNameKey]
	return orch, ok
}

// GetOrchNamespaceFromObj returns the orch namespace associated with the given labels
func GetOrchNamespaceFromObj(labels map[string]string) (string, bool) {
	ns, ok := labels[NamespaceKey]
	return ns, ok
}

// DiffNamespace returns lists of diff between two string list
func DiffNamespace(oldList []string, newList []string) (added []string, deleted []string, nochange []string) {
	deletedNamespaces := oldList
	addedNamespaces := []string{}
	nochangeNamespaces := []string{}

	for _, newNamespace := range newList {
		found := false
		for i, oldNamespace := range deletedNamespaces {
			if newNamespace == oldNamespace {
				found = true
				nochangeNamespaces = append(nochangeNamespaces, newNamespace)
				// if an element is found in new and old, then it is not deleted, so remove from the
				// deleted list
				deletedNamespaces[i] = deletedNamespaces[len(deletedNamespaces)-1]
				deletedNamespaces = deletedNamespaces[:len(deletedNamespaces)-1]
				break
			}
		}

		if !found {
			addedNamespaces = append(addedNamespaces, newNamespace)
		}
	}

	return addedNamespaces, deletedNamespaces, nochangeNamespaces
}
