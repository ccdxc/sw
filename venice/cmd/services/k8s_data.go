package services

import (
	"fmt"
	"path"
	"strconv"

	"github.com/pensando/sw/api"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

func getComponentConfigVolume(compName string) protos.ModuleSpec_Volume {
	ret := configVolume
	ret.HostPath = path.Join(ret.HostPath, compName)
	ret.MountPath = path.Join(ret.MountPath, compName)
	return ret
}

// configVolume is a reusable volume definition for Pensando configs.
var configVolume = protos.ModuleSpec_Volume{
	Name:      "configs",
	HostPath:  "/etc/pensando",
	MountPath: "/etc/pensando",
}

// etcdClientCredsVolume is a reusable volume containing credentials for direct access to etcd
var etcdClientCredsVolume = protos.ModuleSpec_Volume{
	Name:      "etcd-client-credentials",
	HostPath:  globals.EtcdClientAuthDir,
	MountPath: globals.EtcdClientAuthDir,
}

// elasticClientCredsVolume is a reusable volume containing credentials for direct access to Elastic
var elasticClientCredsVolume = protos.ModuleSpec_Volume{
	Name:      "elastic-client-credentials",
	HostPath:  globals.ElasticClientAuthDir,
	MountPath: globals.ElasticClientAuthDir,
}

// logVolume is a reusable volume definition for Pensando logs.
var logVolume = protos.ModuleSpec_Volume{
	Name:      "logs",
	HostPath:  globals.LogDir,
	MountPath: globals.LogDir,
}

// runVolume is a volume to keep run time configs.
var runVolume = protos.ModuleSpec_Volume{
	Name:      "run",
	HostPath:  globals.RuntimeDir,
	MountPath: globals.RuntimeDir,
}

// eventsVolume is a reusable volume definition for Pensando events.
var eventsVolume = protos.ModuleSpec_Volume{
	Name:      "events",
	HostPath:  globals.EventsDir,
	MountPath: globals.EventsDir,
}

// objstoreVolume1 is a reusable volume definition for Pensando object store.
var objstoreVolume1 = protos.ModuleSpec_Volume{
	Name:      "disk1",
	HostPath:  "/data/minio/disk1",
	MountPath: "/disk1",
}

// objstoreVolume2 is a reusable volume definition for Pensando object store.
var objstoreVolume2 = protos.ModuleSpec_Volume{
	Name:      "disk2",
	HostPath:  "/data/minio/disk2",
	MountPath: "/disk2",
}

// citadelDbVolume is a reusable volume definition for citadel.
var citadelDbVolume = protos.ModuleSpec_Volume{
	Name:      "citadeldb",
	HostPath:  globals.CitadelDbDir,
	MountPath: globals.CitadelDbDir,
}

// kubernetesPKIVolume is a reusable volume definition for kuberenete API Client PKI directory.
var kubernetesPKIVolume = protos.ModuleSpec_Volume{
	Name:      "kubepki",
	HostPath:  globals.KubeletPKIDir,
	MountPath: globals.KubeletPKIDir,
}

// varLogVolume is a reusable volume definition for kuberenete API Client PKI directory.
var varLogVolume = protos.ModuleSpec_Volume{
	Name:      "varlog",
	HostPath:  "/var/log",
	MountPath: "/var/syslog",
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
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.APIGw,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.APIGw,
							Port: runtime.MustUint32(globals.APIGwRESTPort),
						},
					},
					Args: []string{"-resolver-urls", "$RESOLVER_URLS"},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
				elasticClientCredsVolume,
			},
		},
	},
	// We don't want to run filebeat on Venice logs to avoid bloating Elastic.
	// Howerver, filebeat may be useful for other purposes as well, so we are
	// leaving the manifest here and keeping the CMD code to generate and update
	// config files.
	/*
		globals.Filebeat: {
			TypeMeta: api.TypeMeta{
				Kind: "Module",
			},
			ObjectMeta: api.ObjectMeta{
				Name: globals.Filebeat,
			},
			Spec: protos.ModuleSpec{
				Type: protos.ModuleSpec_DaemonSet,
				Submodules: []protos.ModuleSpec_Submodule{
					{
						Name: globals.Filebeat,
					},
				},
				Volumes: []protos.ModuleSpec_Volume{
					// Volume definition for Filebeat config
					{
						Name:      "configs",
						HostPath:  globals.FilebeatConfigFile,
						MountPath: "/usr/share/filebeat/filebeat.yml",
					},
					// Volume definition for Filebeat fields config.
					{
						Name:      "fields",
						HostPath:  globals.FilebeatFieldsFile,
						MountPath: "/usr/share/filebeat/logging_fields.yml",
					},
					logVolume,
					eventsVolume,
					{
						Name:      "elastic-client-credentials",
						HostPath:  globals.ElasticClientAuthDir,
						MountPath: globals.FilebeatElasticClientAuthDir,
					},
				},
			},
		},
	*/
	globals.Ntp: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Ntp,
		},
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:       globals.Ntp,
					Privileged: true,
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				getComponentConfigVolume(globals.Ntp),
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
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.APIServer,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.APIServer,
							Port: runtime.MustUint32(globals.APIServerPort),
						},
					},
					Args: []string{
						"-kvdest", "$KVSTORE_URL",
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				getComponentConfigVolume(globals.APIServer),
				etcdClientCredsVolume,
				logVolume,
				eventsVolume,
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
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.Npm,
					Services: []protos.ModuleSpec_Submodule_Service{
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
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
				elasticClientCredsVolume,
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
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.ElasticSearch,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.ElasticSearch,
							Port: runtime.MustUint32(globals.ElasticsearchRESTPort),
						},
					},
					ReadinessProbe: &protos.ModuleSpec_Submodule_Probe{
						Handler: &protos.ModuleSpec_Submodule_Handler{
							Exec: &protos.ModuleSpec_Submodule_ExecAction{
								Command: []string{"/bin/sh", "-c",
									fmt.Sprintf("wget -O- --private-key=%s --certificate=%s --ca-certificate=%s %s",
										fmt.Sprintf("%s/key.pem", globals.ElasticClientAuthDir),
										fmt.Sprintf("%s/cert.pem", globals.ElasticClientAuthDir),
										fmt.Sprintf("%s/ca-bundle.pem", globals.ElasticClientAuthDir),
										fmt.Sprintf("https://$KUBERNETES_POD_IP:%s", globals.ElasticsearchRESTPort))},
							},
						},
						InitialDelaySeconds: 5,
						PeriodSeconds:       10,
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				// Volume definition for Elastic Discovery.
				{
					Name:      "discovery",
					HostPath:  "/etc/pensando/" + globals.ElasticSearch + "/elastic-discovery",
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
				// Volume definition for Elastic node authentication
				{
					Name:      "auth",
					HostPath:  globals.ElasticNodeAuthDir,
					MountPath: "/usr/share/elasticsearch/config/auth-node",
				},
				// Volume definition for Elastic node authentication
				{
					Name:      "https",
					HostPath:  globals.ElasticHTTPSAuthDir,
					MountPath: "/usr/share/elasticsearch/config/auth-https",
				},
				// Volume definition for Elastic client authentication
				{
					Name:      "elastic-client-credentials",
					HostPath:  globals.ElasticClientAuthDir,
					MountPath: globals.ElasticClientAuthDir,
				},
				logVolume,
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
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.Tpm,
					Services: []protos.ModuleSpec_Submodule_Service{
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
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				runVolume,
				eventsVolume,
				elasticClientCredsVolume,
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
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.Spyglass,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Spyglass,
							Port: runtime.MustUint32(globals.SpyglassRPCPort),
						},
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
				elasticClientCredsVolume,
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
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.EvtsMgr,
					Services: []protos.ModuleSpec_Submodule_Service{
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
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
				elasticClientCredsVolume,
			},
		},
	},
	globals.VeniceTechSupport: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.VeniceTechSupport,
		},
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:  globals.VeniceTechSupport,
					Image: globals.VeniceTechSupport,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.VeniceTechSupport,
							Port: 20000,
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS", "-config", "/bin/vtsa.json",
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
				configVolume,
				runVolume,
				kubernetesPKIVolume,
				varLogVolume,
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
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name: globals.Tsm,
					Services: []protos.ModuleSpec_Submodule_Service{
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
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
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
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:  globals.EvtsProxy,
					Image: globals.EvtsProxy,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.EvtsProxy,
							Port: runtime.MustUint32(globals.EvtsProxyRPCPort),
						},
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
			},
		},
	},
	// object store
	globals.Vos: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Vos,
		},
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_DaemonSet,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:  globals.Vos,
					Image: globals.Vos,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Vos,
							Port: runtime.MustUint32(globals.VosGRPcPort),
						},
						{
							Name: globals.VosHTTP,
							Port: runtime.MustUint32(globals.VosHTTPPort),
						},
						{
							Name: globals.VosMinio,
							Port: runtime.MustUint32(globals.VosMinioPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
						"-cluster-nodes", "$QUORUM_NODES",
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				objstoreVolume1,
				objstoreVolume2,
				logVolume,
				eventsVolume,
				{
					Name:      "minio-credentials",
					HostPath:  globals.VosHTTPSAuthDir,
					MountPath: "/root/.minio/certs",
				},
			},
			RestrictNodes: "$QUORUM_NODES",
		},
	},
	// citadel
	globals.Citadel: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Citadel,
		},
		Spec: protos.ModuleSpec{
			Type: protos.ModuleSpec_DaemonSet,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:  globals.Citadel,
					Image: globals.Citadel,
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Citadel,
							Port: runtime.MustUint32(globals.CitadelQueryRPCPort),
						},
						{
							Name: globals.Collector,
							Port: runtime.MustUint32(globals.CollectorRPCPort),
						},
					},
					ReadinessProbe: &protos.ModuleSpec_Submodule_Probe{
						Handler: &protos.ModuleSpec_Submodule_Handler{
							HTTPGet: &protos.ModuleSpec_Submodule_HTTPGetAction{
								Path: "/healthz",
								Port: func() int32 {
									p, err := strconv.Atoi(globals.CitadelHTTPPort)
									if err != nil {
										log.Fatalf("failed to parse %v port %v", globals.Citadel, globals.CitadelHTTPPort)
									}
									return int32(p)
								}(),
								Scheme: "HTTP",
							},
						},
						InitialDelaySeconds: 5,
						PeriodSeconds:       10,
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				etcdClientCredsVolume,
				citadelDbVolume,
				logVolume,
				eventsVolume,
				elasticClientCredsVolume,
			},
		},
	},
	// rollout
	globals.Rollout: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Rollout,
		},
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:    globals.Rollout,
					EnvVars: map[string]string{},
					Args:    []string{},
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.Rollout,
							Port: runtime.MustUint32(globals.RolloutRPCPort),
						},
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				elasticClientCredsVolume,
			},
		},
	},
	globals.OrchHub: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.OrchHub,
		},
		Spec: protos.ModuleSpec{
			Type:      protos.ModuleSpec_DaemonSet,
			NumCopies: 1,
			Submodules: []protos.ModuleSpec_Submodule{
				{
					Name:    globals.OrchHub,
					EnvVars: map[string]string{},
					Args:    []string{"-resolver-urls", "$RESOLVER_URLS"},
					Services: []protos.ModuleSpec_Submodule_Service{
						{
							Name: globals.OrchHub,
							Port: runtime.MustUint32(globals.OrchHubAPIPort),
						},
					},
				},
			},
			Volumes: []protos.ModuleSpec_Volume{
				logVolume,
				eventsVolume,
				elasticClientCredsVolume,
			},
		},
	},
}
