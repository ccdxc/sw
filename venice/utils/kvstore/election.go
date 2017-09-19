package kvstore

// ElectionEventType defines possible types of events for an election.
type ElectionEventType string

const (
	// Elected is an event to indicate this node won the election.
	Elected ElectionEventType = "Elected"
	// Lost is an event to indicate this node lost the election after
	// having won it.
	Lost ElectionEventType = "Lost"
	// Changed is an event to indicate the leader has changed..
	Changed ElectionEventType = "Changed"
	// ElectionError is an event to indicate there is an error with the leader
	// election.
	ElectionError ElectionEventType = "Error"
)

// ElectionEvent contains information about a single event with leader election.
type ElectionEvent struct {
	// Type of the election event.
	Type ElectionEventType

	// Leader is populated with the current leader, if there is one.
	Leader string
}

// Election is an interface that can be implemented to elect a leader using the
// key value store. A contender becomes a leader by successful atomic creation
// of a key.
//
// Any implementation of this interface must do the following:
// - Start contending when the object is created.
// - Renew the lease for the leader automatically.
type Election interface {
	// EventChan returns results of leader election.
	EventChan() <-chan *ElectionEvent

	// Stop participating in leader election.
	Stop()

	// ID returns the identifier of the contender.
	ID() string

	// Leader returns the id of the leader.
	Leader() string

	// IsLeader returns true if the current node is the leader, false
	// otherwise.
	IsLeader() bool
}
