package globals

// Module names
const (
	APIGw                 = "pen-apigw"
	APIServer             = "pen-apiserver"
	Cmd                   = "pen-cmd"
	CmdNICUpdatesSvc      = "pen-cmd-nic-updates" // This a separate service running exclusively on leader CMD
	Ckm                   = "pen-ckm"
	Etcd                  = "pen-etcd"
	Filebeat              = "pen-filebeat"
	Ntp                   = "pen-ntp"
	VCHub                 = "pen-vchub"
	Npm                   = "pen-npm"
	Influx                = "pen-influx"
	EvtsMgr               = "pen-evtsmgr"
	Spyglass              = "pen-spyglass"
	ElasticSearch         = "pen-elastic"
	EvtsProxy             = "pen-evtsproxy"
	KubeAPIServer         = "pen-kube-apiserver"
	KubeScheduler         = "pen-kube-scheduler"
	KubeControllerManager = "pen-kube-controller-manager"
	Kubelet               = "pen-kubelet"
	Tpm                   = "pen-tpm"
	Tsm                   = "pen-tsm"
	Nmd                   = "pen-nmd"
	Netagent              = "pen-netagent"
	K8sAgent              = "pen-k8sagent"
	Vos                   = "pen-vos"
	VosMinio              = "pen-vos-minio"
	VosHTTP               = "pen-vos-http"
	Aggregator            = "pen-aggregator"
	Citadel               = "pen-citadel"
	Collector             = "pen-collector"
	Tmagent               = "pen-tmagent"
	Rollout               = "pen-rollout"
)
