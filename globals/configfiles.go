package globals

const (

	// FilebeatConfigFile is used by Filebeat
	FilebeatConfigFile = "/etc/pensando/filebeat.yml"

	// EtcdConfigFile is written by CMD when starting cluster
	EtcdConfigFile = "/etc/pensando/etcd.conf"

	// APIServerConfigFile is used by ApiServer
	APIServerConfigFile = "/etc/pensando/apiserver.conf"
)
