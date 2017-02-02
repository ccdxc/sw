package quorum

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
	// MemberName is the name of this member.
	MemberName string
	// Existing indicates whether the cluster already exists.
	Existing bool
	// Members is the list of quorum members.
	Members []Member
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
