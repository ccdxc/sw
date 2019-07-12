package mock

import (
	"fmt"

	"github.com/pensando/sw/venice/utils/quorum"
)

type member struct {
	quorum.Member
	health bool
}

// Quorum is a mock implementation of the Quorum interface
type Quorum struct {
	members map[uint64]*member
}

// List returns the current quorum members.
func (q *Quorum) List() ([]quorum.Member, error) {
	var list []quorum.Member
	for _, m := range q.members {
		list = append(list, m.Member)
	}
	return list, nil
}

// Add adds a new member to the quorum.
func (q *Quorum) Add(m *quorum.Member) error {
	_, ok := q.members[m.ID]
	if ok {
		return fmt.Errorf("Member %x already exists", m.ID)
	}
	q.members[m.ID] = &member{Member: *m, health: true}
	return nil
}

// Defrag defrag db of a quorum member.
func (q *Quorum) Defrag(m *quorum.Member) error {
	_, ok := q.members[m.ID]
	if !ok {
		return fmt.Errorf("Member %x not found", m.ID)
	}
	return nil
}

// Remove removes an existing quorum member.
func (q *Quorum) Remove(id uint64) error {
	_, ok := q.members[id]
	if !ok {
		return fmt.Errorf("Member %x not found", id)
	}
	delete(q.members, id)
	return nil
}

// GetStatus returns the status of the member
func (q *Quorum) GetStatus(member *quorum.Member) (interface{}, error) {
	return nil, fmt.Errorf("Not implemented")
}

// GetHealth returns the health of the member
func (q *Quorum) GetHealth(member *quorum.Member) (bool, error) {
	m, ok := q.members[member.ID]
	if !ok {
		return false, fmt.Errorf("Member %x not found", member.ID)
	}
	return m.health, nil
}

// SetHealth seths the health of the member
func (q *Quorum) SetHealth(m *quorum.Member, health bool) error {
	_, ok := q.members[m.ID]
	if !ok {
		return fmt.Errorf("Member %x not found", m.ID)
	}
	q.members[m.ID].health = health
	return nil
}

// NewQuorum returns a new instance of the mock quorum
func NewQuorum() *Quorum {
	return &Quorum{
		members: make(map[uint64]*member),
	}
}
