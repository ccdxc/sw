package usegvlanmgr

import (
	"container/list"
	"fmt"
	"math/rand"
	"sync"
	"time"
)

// VlanMax is the highest allowed VLAN + 1
const VlanMax int = 4096

// Inf defines the vlan manager interface
type Inf interface {
	SetVlanOwner(owner string, vlan int) error
	AssignVlan(owner string) (int, error)
	GetOwner(vlan int) (string, error)
	GetVlan(owner string) (int, error)
	ReleaseVlanOwner(owner string) error
	ReleaseVlan(vlan int) error
	GetFreeVlanCount() int
}

// VlanMgr is a thread safe utility to manage vlan allocations
type VlanMgr struct {
	sync.RWMutex
	// index is vlan - v.StartingIndex
	// value is the owner
	vlans []string
	// Doubly linked list of indexes in vlans array
	randList *list.List
	// Maps from index to a linked list element
	// Used for removing an item efficiently
	randMap map[int]*list.Element
	// map from owner to vlan
	ownerMap     map[string]int
	StartingVlan int
	EndingVlan   int
	randSource   *rand.Rand
}

// NewVlanManager creates a vlan manager
// Only vlans from startingVlan (inclusive) to endingVlan (exclusive) are available
func NewVlanManager(startingVlan, endingVlan int) Inf {
	if startingVlan < 0 || endingVlan < startingVlan {
		return nil
	}

	var randSeed = time.Now().Unix()
	var r = rand.New(rand.NewSource(randSeed))

	s := endingVlan - startingVlan
	mgr := &VlanMgr{
		vlans: make([]string, s),

		StartingVlan: startingVlan,
		EndingVlan:   endingVlan,
		ownerMap:     make(map[string]int),
		randSource:   r,
	}
	mgr.genRandList()
	return mgr
}

// Walks the vlan array and generates a random
// linked list of indexes that are available
func (v *VlanMgr) genRandList() {
	v.Lock()
	defer v.Unlock()

	l := list.New()
	v.randMap = map[int]*list.Element{}

	for _, i := range v.randSource.Perm(len(v.vlans)) {
		if len(v.vlans[i]) == 0 {
			// Unassigned index
			l.PushFront(i)
			v.randMap[i] = l.Front()
		}
	}

	v.randList = l
}

// Walks the rand linked list and removes the item
// with the given index
func (v *VlanMgr) removeFromRandList(index int) {
	v.Lock()
	defer v.Unlock()

	l := v.randList
	l.Remove(v.randMap[index])
	delete(v.randMap, index)
}

func (v *VlanMgr) validateVlan(vlan int) error {
	if vlan < v.StartingVlan || vlan >= v.EndingVlan {
		return fmt.Errorf("supplied vlan %d is an invalid vlan value", vlan)
	}
	return nil
}

func (v *VlanMgr) validateOwner(owner string) error {
	if len(owner) == 0 {
		return fmt.Errorf("owner cannot be empty")
	}
	return nil
}

// SetVlanOwner assigns the given vlan to the given owner if the vlan is available
func (v *VlanMgr) SetVlanOwner(owner string, vlan int) error {
	if err := v.validateVlan(vlan); err != nil {
		return err
	}
	if err := v.validateOwner(owner); err != nil {
		return err
	}

	v.Lock()

	if assignment, ok := v.ownerMap[owner]; ok {
		if vlan == assignment {
			// owner already has the given vlan
			return nil
		}
		return fmt.Errorf("owner %s is already assigned to vlan %d", owner, assignment)
	}
	index := vlan - v.StartingVlan
	err := v.assignVlan(owner, vlan, index)
	if err != nil {
		return err
	}

	v.Unlock()
	v.removeFromRandList(index)
	return nil
}

// Caller must acquire the lock
func (v *VlanMgr) assignVlan(owner string, vlan, index int) error {
	if v.vlans[index] != "" {
		return fmt.Errorf("vlan %d (index %d) is already taken by owner %s", vlan, index, v.vlans[index])
	}
	v.vlans[index] = owner
	v.ownerMap[owner] = vlan
	return nil
}

// AssignVlan assigns a random free vlan
func (v *VlanMgr) AssignVlan(owner string) (int, error) {
	if err := v.validateOwner(owner); err != nil {
		return -1, err
	}

	v.Lock()
	defer v.Unlock()

	// Check if owner already has an assignment
	if assignment, ok := v.ownerMap[owner]; ok {
		return -1, fmt.Errorf("owner is already assigned to vlan %d", assignment)
	}

	if v.randList.Len() == 0 {
		return -1, fmt.Errorf("no vlans available")
	}

	// Pick one at random
	index := v.randList.Remove(v.randList.Front()).(int)
	nextVlan := index + v.StartingVlan

	if err := v.assignVlan(owner, nextVlan, index); err != nil {
		return -1, err
	}

	return nextVlan, nil
}

// GetOwner returns the owner for the given vlan,
func (v *VlanMgr) GetOwner(vlan int) (string, error) {
	if err := v.validateVlan(vlan); err != nil {
		return "", err
	}
	v.RLock()
	defer v.RUnlock()

	index := vlan - v.StartingVlan
	val := v.vlans[index]
	return val, nil
}

// GetVlan gets the vlan assigned to the given owner
func (v *VlanMgr) GetVlan(owner string) (int, error) {
	v.RLock()
	defer v.RUnlock()

	vlan, ok := v.ownerMap[owner]
	if !ok {
		return -1, fmt.Errorf("owner %s has no vlan assignment", owner)
	}
	return vlan, nil
}

// ReleaseVlanOwner frees the vlan allocated to the given owner
func (v *VlanMgr) ReleaseVlanOwner(owner string) error {
	vlan, err := v.GetVlan(owner)
	if err != nil {
		return err
	}
	return v.ReleaseVlan(vlan)
}

// ReleaseVlan frees the given vlan
func (v *VlanMgr) ReleaseVlan(vlan int) error {
	if err := v.validateVlan(vlan); err != nil {
		return err
	}

	v.Lock()
	defer v.Unlock()

	index := vlan - v.StartingVlan

	owner := v.vlans[index]
	if len(owner) != 0 {
		v.vlans[index] = ""
		delete(v.ownerMap, owner)
		// Add back into rand list
		v.randList.PushBack(index)
		v.randMap[index] = v.randList.Back()
	}
	return nil
}

// GetFreeVlanCount returns how many vlans are left for assignment
func (v *VlanMgr) GetFreeVlanCount() int {
	v.RLock()
	defer v.RUnlock()

	return v.randList.Len()
}
