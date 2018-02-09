package configs

// Interface for the configs package
type Interface interface {
	GenerateKubeletConfig(nodeid, apiServerAddr string, kubeAPIServerPort string) error
	RemoveKubeletConfig()
	GenerateAPIServerConfig() error
	RemoveAPIServerConfig()
	GenerateFilebeatConfig(elasticServerAddr string) error
	RemoveFilebeatConfig()
	GenerateKubeMasterConfig(apiServerAddr string) error
	RemoveKubeMasterConfig()
}

type configs struct{}

func (c *configs) GenerateKubeletConfig(nodeID, kubeAPIServerAddr string, kubeAPIServerPort string) error {
	return GenerateKubeletConfig(nodeID, kubeAPIServerAddr, kubeAPIServerPort)
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
func (c *configs) GenerateFilebeatConfig(elasticServerAddr string) error {
	return GenerateFilebeatConfig(elasticServerAddr)
}
func (c *configs) RemoveFilebeatConfig() {
	RemoveAPIServerConfig()
}
func (c *configs) GenerateKubeMasterConfig(apiServerAddr string) error {
	return GenerateKubeMasterConfig(apiServerAddr)
}
func (c *configs) RemoveKubeMasterConfig() {
	RemoveKubeMasterConfig()
}

// New object implementing the Interface
func New() Interface {
	return &configs{}
}
