package services

import (
	"path"

	"github.com/pensando/sw/api"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"
)

func getComponentConfigVolume(compName string) *protos.ModuleSpec_Volume {
	ret := configVolume
	ret.HostPath = path.Join(ret.HostPath, compName)
	ret.MountPath = path.Join(ret.MountPath, compName)
	return &ret
}

// configVolume is a reusable volume definition for Pensando configs.
var configVolume = protos.ModuleSpec_Volume{
	Name:      "configs",
	HostPath:  "/etc/pensando",
	MountPath: "/etc/pensando",
}

// logVolume is a reusable volume definition for Pensando logs.
var logVolume = protos.ModuleSpec_Volume{
	Name:      "logs",
	HostPath:  "/var/log/pensando",
	MountPath: "/var/log/pensando",
}

// runVolume is a volume to keep run time configs.
var runVolume = protos.ModuleSpec_Volume{
	Name:      "run",
	HostPath:  "/var/run/pensando/",
	MountPath: "/var/run/pensando/",
}

// eventsVolume is a reusable volume definition for Pensando events.
var eventsVolume = protos.ModuleSpec_Volume{
	Name:      "events",
	HostPath:  "/var/lib/pensando/events/offset",
	MountPath: "/var/lib/pensando/events/offset",
}

// k8sModules contain definitions of controller objects that need to deployed
// through k8s.
var k8sModules = map[string]protos.Module{
	globals.APIGw: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.APIGw,
		},
		Spec: &protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.APIGw,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.APIGw,
							Port: runtime.MustUint32(globals.APIGwRESTPort),
						},
					},
					Args: []string{"-resolver-urls", "$RESOLVER_URLS"},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.Filebeat: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Filebeat,
		},
		Spec: &protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Filebeat,
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				// Volume definition for Filebeat config
				{
					Name:      "configs",
					HostPath:  globals.FilebeatConfigFile,
					MountPath: "/usr/share/filebeat/filebeat.yml",
				},
				&logVolume,
			},
		},
	},
	globals.Ntp: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Ntp,
		},
		Spec: &protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name:       globals.Ntp,
					Privileged: true,
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				getComponentConfigVolume("ntp"),
			},
		},
	},
	globals.APIServer: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.APIServer,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.APIServer,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.APIServer,
							Port: runtime.MustUint32(globals.APIServerPort),
						},
					},
					Args: []string{
						"-kvdest", "$KVSTORE_URL",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				getComponentConfigVolume("apiserver"),
				&logVolume,
			},
		},
	},
	globals.VCHub: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.VCHub,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.VCHub,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.VCHub,
							Port: runtime.MustUint32(globals.VCHubAPIPort),
						},
					},
					Args: []string{
						// TODO: This should be removed when VCenter Object is implemented.
						"-vcenter-list", "http://user:pass@192.168.30.10:8989/sdk",
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.Npm: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Npm,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Npm,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Npm,
							Port: runtime.MustUint32(globals.NpmRPCPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.Influx: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Influx,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Influx,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Influx,
							Port: runtime.MustUint32(globals.InfluxHTTPPort),
						},
					},
					Args: []string{
						"-config", "/etc/pensando/influxdb/influxdb.conf",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				getComponentConfigVolume("influxdb"),
				&logVolume,
			},
		},
	},
	globals.Collector: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Collector,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Collector,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Collector,
							Port: runtime.MustUint32(globals.CollectorAPIPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.ElasticSearch: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Spec: &protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.ElasticSearch,
					// TODO
					// Because of https://github.com/kubernetes/kubernetes/pull/48986
					// we cant have environment variables with special chars in kube
					// So commenting out the code below and have a special Elastic image with
					// these options as part of Dockerfile.
					// Once we upgrade to latest kube, we can use official elastic image
					// and pass the first 2 environment variables explicitly below
					EnvVars: map[string]string{
						//"cluster.name":           "pen-elasticcluster",
						//"xpack.security.enabled": "false",
						"ES_JAVA_OPTS": "-Xms256m -Xmx256m",
					},
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.ElasticSearch,
							Port: runtime.MustUint32(globals.ElasticsearchRESTPort),
						},
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				// Volume definition for Elastic Discovery.
				{
					Name:      "discovery",
					HostPath:  "/etc/pensando/elastic/elastic-discovery",
					MountPath: "/usr/share/elasticsearch/config/discovery-file",
				},
				// Volume definition for sourcing env variables
				{
					Name:      "envvars",
					HostPath:  globals.ElasticMgmtConfigFile,
					MountPath: "/usr/share/elasticsearch/mgmt_env.sh",
				},
				// Volume definition for Elastic data storage
				{
					Name:      "data",
					HostPath:  globals.ElasticDataVolumeDir,
					MountPath: "/usr/share/elasticsearch/data",
				},
			},
		},
	},
	globals.Tpm: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Tpm,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Tpm,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Tpm,
							Port: runtime.MustUint32(globals.TpmRPCPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
				&runVolume,
			},
		},
	},
	globals.Spyglass: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Spyglass,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Spyglass,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Spyglass,
							Port: runtime.MustUint32(globals.SpyglassRPCPort),
						},
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.EvtsMgr: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.EvtsMgr,
		},
		Spec: &protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.EvtsMgr,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.EvtsMgr,
							Port: runtime.MustUint32(globals.EvtsMgrRPCPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.Tsm: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Tsm,
		},
		Spec: &protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name: globals.Tsm,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Tsm,
							Port: runtime.MustUint32(globals.TsmRPCPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
			},
		},
	},
	globals.EvtsProxy: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.EvtsProxy,
		},
		Spec: &protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []*protos.ModuleSpec_Submodule{
				{
					Name:  globals.EvtsProxy,
					Image: globals.EvtsProxy,
					Services: []*protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.EvtsProxy,
							Port: runtime.MustUint32(globals.EvtsProxyRPCPort),
						},
					},
				},
			},
			Volumes: []*protos.ModuleSpec_Volume{
				&logVolume,
				&eventsVolume,
			},
		},
	},
}

// ContainerInfoMap is the map of name to containerInfo
var ContainerInfoMap = map[string]utils.ContainerInfo{}
