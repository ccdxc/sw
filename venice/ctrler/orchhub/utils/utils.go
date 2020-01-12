package utils

import (
	"fmt"
)

const (
	// Delim delimiter used when creating name when exposing objects between orch and venice
	Delim = "-"
)

var (
	// OrchNameKey is the key used to label the orchestrator being used
	OrchNameKey = "orch-name"
)

// CreateGlobalKey creates a global key
func CreateGlobalKey(orchID, namespace, objName string) string {
	// TODO: Add in datacenter name
	return fmt.Sprintf("%s%s%s%s%s", orchID, Delim, namespace, Delim, objName)
}

// CreateGlobalKeyPrefix creates the key prefix
func CreateGlobalKeyPrefix(orchID, namespace string) string {
	return fmt.Sprintf("%s-%s", orchID, namespace)
}

// AddOrchNameLabel adds orchestrator name into the labels
func AddOrchNameLabel(labels map[string]string, name string) {
	labels[OrchNameKey] = name
}
