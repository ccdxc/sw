package configs

// Interface for the configs package
type Interface interface {
	GenerateKubeletConfig(virtualIP string) error
	RemoveKubeletConfig()
	GenerateAPIServerConfig() error
	RemoveAPIServerConfig()
	GenerateFilebeatConfig(virtualIP string) error
	RemoveFilebeatConfig()
	GenerateKubeMasterConfig(virtualIP string) error
	RemoveKubeMasterConfig()
}

type configs struct{}

func (c *configs) GenerateKubeletConfig(virtualIP string) error {
	return GenerateKubeletConfig(virtualIP)
}
func (c *configs) RemoveKubeletConfig() {
	RemoveKubeletConfig()
}
func (c *configs) GenerateAPIServerConfig() error {
	return GenerateAPIServerConfig()
}
func (c *configs) RemoveAPIServerConfig() {
	RemoveAPIServerConfig()
}
func (c *configs) GenerateFilebeatConfig(virtualIP string) error {
	return GenerateFilebeatConfig(virtualIP)
}
func (c *configs) RemoveFilebeatConfig() {
	RemoveAPIServerConfig()
}
func (c *configs) GenerateKubeMasterConfig(virtualIP string) error {
	return GenerateKubeMasterConfig(virtualIP)
}
func (c *configs) RemoveKubeMasterConfig() {
	RemoveKubeMasterConfig()
}

// New object implementing the Interface
func New() Interface {
	return &configs{}
}
