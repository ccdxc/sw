package env

const (
	// ElectionStarted represents the leader election process
	ElectionStarted string = "ElectionStarted"

	// ElectionCancelled represents the election cancellation
	ElectionCancelled string = "ElectionCancelled"

	// ElectionStopped represents the leader election stop
	ElectionStopped string = "ElectionStopped"

	// LeaderElected represents the successful leader election
	LeaderElected string = "LeaderElected"

	// LeaderLost represents the existing leader giving up the leadership
	LeaderLost string = "LeaderLost"

	// LeaderChanged represents the leader change event
	LeaderChanged string = "LeaderChanged"
)

// GetEventTypes returns the list of event types; this will be set of event types supported,
// anything beyond this list will not be supported by the events recorder.
func GetEventTypes() []string {
	return []string{
		ElectionStarted,
		ElectionCancelled,
		ElectionStopped,
		LeaderElected,
		LeaderLost,
		LeaderChanged,
	}
}
