package globals

import "fmt"

const (
	// DefaultDiskHighThreshold default disk usage threshold for creating CRITICAL events
	DefaultDiskHighThreshold = 75.0
)

var (
	// DiskHighThreshold tolerable limit for disk usage; CRITICAL events will be created when the usage exceeds.
	DiskHighThreshold = DefaultDiskHighThreshold

	// ThresholdEventConfig setting to generate event when threshold is exceeded
	ThresholdEventConfig = true

	// DiskHighThresholdMessage message to be included in the disk threshold event
	DiskHighThresholdMessage = fmt.Sprintf("Disk threshold exceeded %v%%", DiskHighThreshold)

	// DiskHighThresholdEventStatus event already raised or not
	DiskHighThresholdEventStatus = false
)
