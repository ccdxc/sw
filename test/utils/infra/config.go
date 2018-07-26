package infra

// ConfigBuffer defines configuration manipulation routines to help upload a specific configuration
type ConfigBuffer interface {
	Load(config string) error

	LoadFile(configPath string) error

	Merge(config string) error

	MergeFile(configPath string) error

	FindObject(kind string) []interface{}

	GenerateObject(kind string, numInsances int) ([]interface{}, error)

	SaveObject(obj interface{}) error
}

type configBuffer struct {
	config string
}

// NewConfigBuffer - creates a new configuration buffer
func NewConfigBuffer() ConfigBuffer {
	return &configBuffer{}
}

// Load - loads and possibly overwrites configuration from a json string
// - config contains string json that represent configuration to be augmented
func (cfg *configBuffer) Load(config string) error {
	return nil
}

// LoadFile - loads and possibly overwrites configuration from a file
// - configPath contains the configuration to be augmented
func (cfg *configBuffer) LoadFile(configPath string) error {
	return nil
}

// Merge - merges configuration into the buffer
// - config is the specific configuration (json string) to be patched in
func (cfg *configBuffer) Merge(config string) error {
	return nil
}

// MergeFile - merges configuration into the buffer from a file
func (cfg *configBuffer) MergeFile(configPath string) error {
	return nil
}

// GenerateConfiguration - generates/returns object configuration of the specified kind, save the config in buffer
func (cfg *configBuffer) GenerateConfig(kind string, numInstances int) error {
	return nil
}

// FindObjects - finds objects of a specified kind and return entire list
func (cfg *configBuffer) FindObject(kind string) []interface{} {
	return []interface{}{}
}

// GenerateObject - creates an objects of a given kind
func (cfg *configBuffer) GenerateObject(kind string, numInsances int) ([]interface{}, error) {
	return []interface{}{}, nil
}

// SaveObject - overwrites an object in the configuration with a specified object
func (cfg *configBuffer) SaveObject(obj interface{}) error {
	return nil
}
