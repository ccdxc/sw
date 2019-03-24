// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package cluster is a auto generated package.
Input file: svc_cluster.proto
*/
package cluster

const (
	ElectionStarted            = "ElectionStarted"
	ElectionCancelled          = "ElectionCancelled"
	ElectionNotificationFailed = "ElectionNotificationFailed"
	ElectionStopped            = "ElectionStopped"
	LeaderElected              = "LeaderElected"
	LeaderLost                 = "LeaderLost"
	LeaderChanged              = "LeaderChanged"
	NodeJoined                 = "NodeJoined"
	NodeDisjoined              = "NodeDisjoined"
	NICAdmitted                = "NICAdmitted"
	NICHealthUnknown           = "NICHealthUnknown"
	NICHealthy                 = "NICHealthy"
	NICUnhealthy               = "NICUnhealthy"
	AutoGeneratedTLSCert       = "AutoGeneratedTLSCert"
)

// GetEventTypes returns the list of event types; this will be set of event types supported,
// anything beyond this list will not be supported by the events recorder.
func GetEventTypes() []string {
	return []string{
		ElectionStarted,
		ElectionCancelled,
		ElectionNotificationFailed,
		ElectionStopped,
		LeaderElected,
		LeaderLost,
		LeaderChanged,
		NodeJoined,
		NodeDisjoined,
		NICAdmitted,
		NICHealthUnknown,
		NICHealthy,
		NICUnhealthy,
		AutoGeneratedTLSCert,
	}
}
