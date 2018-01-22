package options

import (
	"github.com/pensando/sw/venice/globals"
)

// KVStoreOptions contains configuration for starting a K/V store. These options are abstracted out to
// enable integration tests, where multiple instances can run on the same node.
type KVStoreOptions struct {
	// PeerPort is where the K/V store talks to its peers.
	PeerPort string

	// ClientPort is where the clients can talk to K/V store.
	ClientPort string

	// ConfigFile contains the configuration file sourced by systemd unit for K/V store.
	ConfigFile string

	// UnitFile contains the name of the systemd unit file for K/V store.
	UnitFile string

	// DataDir contains the directory where K/V store can save its configuration or runtime state.
	DataDir string
}

// ServerRunOptions contains the configuration for starting cmd. These options are abstracted out to
// enable integration tests, where multiple instances can run on the same node.
type ServerRunOptions struct {
	// KVStore contains the options for K/V store.
	KVStore *KVStoreOptions

	// RESTPort is the port for REST server.
	RESTPort string

	// GRPCUnauthPort is the port for the unauthenticated gRPC server.
	// Does not require TLS. Used for cluster formation, NIC admission and certificates issuance.
	GRPCUnauthPort string

	// GRPCAuthPort is the port for the authenticated gRPC server.
	// Requires TLS and valid client certificate. Used by resolver.
	GRPCAuthPort string

	// ConfigDir is where cmd can persist its configuration.
	ConfigDir string

	// ClusterConfigFile identifies the file name for persisting cluster configuration.
	ClusterConfigFile string
}

// NewServerRunOptions creates the default options for cmd.
func NewServerRunOptions() *ServerRunOptions {
	kvStoreOptions := &KVStoreOptions{
		PeerPort:   globals.KVStorePeerPort,
		ClientPort: globals.KVStoreClientPort,
		ConfigFile: globals.EtcdConfigFile,
		UnitFile:   globals.EtcdServiceFile,
		DataDir:    globals.EtcdDataDir,
	}

	return &ServerRunOptions{
		KVStore:           kvStoreOptions,
		RESTPort:          globals.CMDRESTPort,
		GRPCAuthPort:      globals.CMDGRPCAuthPort,
		GRPCUnauthPort:    globals.CMDGRPCUnauthPort,
		ConfigDir:         globals.ConfigDir,
		ClusterConfigFile: "cluster.conf",
	}
}
