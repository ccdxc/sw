package curator

// Interface is interface definition for Curator service
// which handles Index retention and associated cleanup
type Interface interface {
	// Start the service
	Start()

	// Stop the service
	Stop()

	// SetConfig updates the curator config
	SetConfig(c *Config) error

	// GetConfig return the current curator config
	GetConfig() Config
}
