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

	// DiskPHighThresholdMessagePrefix prefix of the message to be included in the disk partition threshold event
	// Example of disk partition threshold event message:
	//   "Disk partition exceeded threshold 75.0%, partition /etc/config/, current usage 65.1%"
	DiskPHighThresholdMessagePrefix = fmt.Sprintf("Disk partition exceeded threshold %v%%", DiskHighThreshold)

	// DiskPHighThresholdMessageSuffix suffix of the message to be included in the disk partition threshold event
	DiskPHighThresholdMessageSuffix = fmt.Sprintf(", current usage")

	// DiskHighThresholdEventStatus event already raised or not
	DiskHighThresholdEventStatus = false
)
