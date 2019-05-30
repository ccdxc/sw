package globals

const (
	// MemHighThreshold tolerable limit for memory usage; CRITICAL events will be created when the usage exceeds.
	MemHighThreshold = 80

	// CPUHighThreshold tolerable limit for CPU usage; CRITICAL events will be created when the usage exceeds.
	CPUHighThreshold = 80

	// DiskHighThreshold tolerable limit for disk usage; CRITICAL events will be created when the usage exceeds.
	DiskHighThreshold = 80
)
