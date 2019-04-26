// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package k8s

import (
	"os"
)

// Make Pod information available to Controllers using Kubernetes Downward API
// https://kubernetes.io/docs/tasks/inject-data-application/environment-variable-expose-pod-information/

// Environment variable names defined in sw/venice/cmd/services/k8s.go

// GetPodName returns the pod name (metadata.name)
func GetPodName() string {
	return os.Getenv("KUBERNETES_POD_NAME")
}

// GetPodIP returns the pod IP address (status.podIP)
func GetPodIP() string {
	return os.Getenv("KUBERNETES_POD_IP")
}

// GetNodeName returns the node name (spec.nodeName)
func GetNodeName() string {
	return os.Getenv("KUBERNETES_NODE_NAME")
}

// GetHostIP returns the host IP address (status.hostIP)
func GetHostIP() string {
	return os.Getenv("KUBERNETES_HOST_IP")
}

// GetK8sInfo returns a map containing Kubernetes Pod/Node information
func GetK8sInfo() map[string]string {
	return map[string]string{
		"PodName":  GetPodName(),
		"PodIP":    GetPodIP(),
		"NodeName": GetNodeName(),
		"HostIP":   GetHostIP(),
	}
}
