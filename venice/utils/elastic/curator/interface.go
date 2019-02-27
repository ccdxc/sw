package curator

// Interface is interface definition for Curator service
// which handles Index retention and associated cleanup
type Interface interface {
	// Start the curator service
	Start()

	// Stop the service
	Stop()

	// SetConfig updates the curator config
	Scan(cfg *Config)
}
