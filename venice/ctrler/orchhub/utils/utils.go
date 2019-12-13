package utils

import (
	"fmt"
)

var (
	// OrchNameKey is the key used to label the orchestrator being used
	OrchNameKey = "orch-name"
)

// CreateGlobalKey creates a global key
func CreateGlobalKey(orchID, objName string) string {
	// TODO: Add in datacenter name
	return fmt.Sprintf("%s-%s", orchID, objName)
}

// AddOrchNameLabel adds orchestrator name into the labels
func AddOrchNameLabel(labels map[string]string, name string) {
	labels[OrchNameKey] = name
}
