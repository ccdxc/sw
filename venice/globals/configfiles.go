package globals

const (

	// RegistryConfigFile is used to store the docker registry
	RegistryConfigFile = "/etc/pensando/registry.conf"

	// FilebeatConfigFile is used by Filebeat
	FilebeatConfigFile = "/etc/pensando/filebeat.yml"

	// ElasticDiscoveryConfigFile is used by Elastic for Node discovery
	ElasticDiscoveryConfigFile = "/etc/pensando/elastic-discovery/unicast_hosts.txt"

	// ElasticMgmtConfigFile is used by Elastic for binding to MgmtAddr and publishing to peers
	ElasticMgmtConfigFile = "/etc/pensando/mgmt_env.sh"

	// ElasticDataVolumeDir is used by Elastic for storing indexed data
	ElasticDataVolumeDir = "/var/lib/pensando/elasticsearch/data"

	// EtcdConfigFile is written by CMD when starting cluster
	EtcdConfigFile = "/etc/pensando/etcd.conf"

	// APIServerConfigFile is used by ApiServer
	APIServerConfigFile = "/etc/pensando/apiserver.conf"

	// NtpConfigFile is generated by NTP service for chronyd format
	NtpConfigFile = "/etc/pensando/chrony.conf"

	// InfluxConfigFile is used by Influx service
	InfluxConfigFile = "/etc/pensando/influxdb.conf"
)
