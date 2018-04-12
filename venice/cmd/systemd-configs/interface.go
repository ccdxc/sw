package configs

// Interface for the configs package
type Interface interface {
	GenerateKubeletConfig(nodeid, apiServerAddr string, kubeAPIServerPort string) error
	RemoveKubeletConfig()
	GenerateAPIServerConfig() error
	RemoveAPIServerConfig()
	GenerateFilebeatConfig(elasticServerAddrs []string) error
	RemoveFilebeatConfig()
	GenerateKubeMasterConfig(apiServerAddr string) error
	RemoveKubeMasterConfig()
	GenerateElasticDiscoveryConfig(elasticServerAddrs []string) error
	RemoveElasticDiscoveryConfig()
	GenerateElasticMgmtConfig(mgmtAddr string, quorumSize int) error
	RemoveElasticMgmtConfig()
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
func (c *configs) GenerateFilebeatConfig(elasticServerAddrs []string) error {
	return GenerateFilebeatConfig(elasticServerAddrs)
}
func (c *configs) RemoveFilebeatConfig() {
	RemoveFilebeatConfig()
}
func (c *configs) GenerateKubeMasterConfig(apiServerAddr string) error {
	return GenerateKubeMasterConfig(apiServerAddr)
}
func (c *configs) RemoveKubeMasterConfig() {
	RemoveKubeMasterConfig()
}
func (c *configs) GenerateElasticDiscoveryConfig(elasticServerAddrs []string) error {
	return GenerateElasticDiscoveryConfig(elasticServerAddrs)
}
func (c *configs) RemoveElasticDiscoveryConfig() {
	RemoveElasticDiscoveryConfig()
}
func (c *configs) GenerateElasticMgmtConfig(mgmtAddr string, quorumSize int) error {
	return GenerateElasticMgmtConfig(mgmtAddr, quorumSize)
}
func (c *configs) RemoveElasticMgmtConfig() {
	RemoveElasticMgmtConfig()
}

// New object implementing the Interface
func New() Interface {
	return &configs{}
}
