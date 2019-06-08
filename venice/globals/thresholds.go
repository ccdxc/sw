package globals

import "fmt"

const (
	// MemHighThreshold tolerable limit for memory usage; CRITICAL events will be created when the usage exceeds.
	MemHighThreshold = 80

	// CPUHighThreshold tolerable limit for CPU usage; CRITICAL events will be created when the usage exceeds.
	CPUHighThreshold = 80

	// DiskHighThreshold tolerable limit for disk usage; CRITICAL events will be created when the usage exceeds.
	DiskHighThreshold = 80
)

var (
	// MemHighThresholdMessage message to be included in the mem. threshold event
	MemHighThresholdMessage = fmt.Sprintf("Memory threshold exceeded %v%%", MemHighThreshold)

	// CPUHighThresholdMessage message to be included in the CPU threshold event
	CPUHighThresholdMessage = fmt.Sprintf("CPU threshold exceeded %v%%", CPUHighThreshold)

	// DiskHighThresholdMessage message to be included in the disk threshold event
	DiskHighThresholdMessage = fmt.Sprintf("Disk threshold exceeded %v%%", DiskHighThreshold)
)
