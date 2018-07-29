package configs

import (
	"path"

	"k8s.io/client-go/tools/clientcmd"
	"k8s.io/client-go/tools/clientcmd/api"

	"github.com/pensando/sw/venice/utils/certs"
)

func generateKubeconfig(kubeAPIServerAddr string, kubeAPIServerPort string, user, confDir, pkiDir string) error {
	tlsCertFile, tlsPrivateKeyFile, clientCACertFile := certs.GetTLSCredentialsPaths(pkiDir)
	config := api.Config{
		Clusters: map[string]*api.Cluster{
			"kubernetes": {
				Server:               "https://" + kubeAPIServerAddr + ":" + kubeAPIServerPort,
				CertificateAuthority: clientCACertFile,
			},
		},
		Contexts: map[string]*api.Context{
			"kubernetes": {
				Cluster:  "kubernetes",
				AuthInfo: user,
			},
		},
		CurrentContext: "kubernetes",
		AuthInfos: map[string]*api.AuthInfo{
			user: {
				ClientCertificate: tlsCertFile,
				ClientKey:         tlsPrivateKeyFile,
			},
		},
	}
	return clientcmd.WriteToFile(config, path.Join(confDir, kubeconfigFileName))
}

func removeKubeconfig(dir string) {
	removeFiles([]string{path.Join(dir, kubeconfigFileName)})
}
