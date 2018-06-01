package quorum

import (
	"crypto"
	"crypto/x509"
)

// Config contains configuration to create a KV store with quorum.
type Config struct {
	// Type of KV storage backend, e.g. "etcd"
	Type string
	// ID of the cluster.
	ID string
	// DataDir is the directory for storing data.
	DataDir string
	// CfgFile is the path for configuration file.
	CfgFile string
	// UnitFile is the path for systemd unit file.
	UnitFile string
	// MemberName is the name of this member.
	MemberName string
	// Existing indicates whether the cluster already exists.
	Existing bool
	// Members is the list of quorum members.
	Members []Member
	// Peer mTLS auth: if true, all Peer* parameters below need to be set
	PeerAuthEnabled bool
	// Certificate used both for client and server TLS
	// Needs to have both clientAuth and serverAuth in extendedKeyUsage
	PeerCert *x509.Certificate
	// Key used for client/server TLS connections
	// Must match the public key in PeerCert
	PeerPrivateKey crypto.PrivateKey
	// Trust bundle used to verify peer's certificate
	PeerCATrustBundle []*x509.Certificate
}

// Member contains information about a quorum member.
type Member struct {
	// ID is the unique id for the quorum member.
	ID uint64
	// Name of the quorum member.
	Name string
	// PeerURLs is the list of URLs for other quorum members to talk to this member.
	PeerURLs []string
	// ClientURLs is the list of URLs for clients to talk to this member.
	ClientURLs []string
}

// Interface is an interface to manage the quorum membership of the cluster.
type Interface interface {
	// List returns the current quorum members.
	List() ([]Member, error)

	// Add adds a new member to the quorum.
	Add(member *Member) error

	// Remove removes an existing quorum member.
	Remove(id uint64) error
}
