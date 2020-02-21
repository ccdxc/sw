package usegvlanmgr

import (
	"container/list"
	"fmt"
	"math/rand"
	"sync"
)

// VlanMax is the highest allowed VLAN + 1
// Vlans of 0 and 4095 are reserved
const VlanMax int = 4095

// Inf defines the vlan manager interface
type Inf interface {
	SetVlanOwner(owner string, vlan int) error
	AssignVlan(owner string) (int, error)
	GetOwner(vlan int) (string, error)
	GetVlan(owner string) (int, error)
	ReleaseVlanOwner(owner string) error
	ReleaseVlan(vlan int) error
	GetFreeVlanCount() int
	Debug(map[string]string) (interface{}, error)
}

// VlanMgr is a thread safe utility to manage vlan allocations
type VlanMgr struct {
	sync.RWMutex
	// index is vlan - v.StartingIndex
	// value is the owner
	vlans []string
	// Whether vlan assignments should be random or sequential
	assignRandom bool
	// Doubly linked list of indexes in vlans array
	itemList *list.List
	// Maps from index to a linked list element
	// Used for removing an item efficiently
	indexMap map[int]*list.Element
	// map from owner to vlan
	ownerMap     map[string]int
	StartingVlan int
	EndingVlan   int
	randSource   *rand.Rand
}

// NewVlanManager creates a vlan manager
// Only vlans from startingVlan (inclusive) to endingVlan (exclusive) are available
func NewVlanManager(startingVlan, endingVlan int, assignRandom bool, seed int64) Inf {
	if startingVlan < 0 || endingVlan < startingVlan {
		return nil
	}

	var r = rand.New(rand.NewSource(seed))

	s := endingVlan - startingVlan
	mgr := &VlanMgr{
		vlans: make([]string, s),

		StartingVlan: startingVlan,
		EndingVlan:   endingVlan,
		ownerMap:     make(map[string]int),
		randSource:   r,
		assignRandom: assignRandom,
	}
	mgr.genItemList()
	return mgr
}

// Walks the vlan array and generates a random
// linked list of indexes that are available
func (v *VlanMgr) genItemList() {
	v.Lock()
	defer v.Unlock()

	l := list.New()
	v.indexMap = map[int]*list.Element{}
	var intList []int
	if v.assignRandom {
		intList = v.randSource.Perm(len(v.vlans))
	} else {
		for i := range v.vlans {
			intList = append(intList, i)
		}
	}

	for _, i := range intList {
		if len(v.vlans[i]) == 0 {
			// Unassigned index
			l.PushBack(i)
			v.indexMap[i] = l.Back()
		}
	}

	v.itemList = l
}

// Walks the rand linked list and removes the item
// with the given index
func (v *VlanMgr) removeFromitemList(index int) {
	v.Lock()
	defer v.Unlock()

	l := v.itemList
	l.Remove(v.indexMap[index])
	delete(v.indexMap, index)
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
			v.Unlock()
			return nil
		}
		v.Unlock()
		return fmt.Errorf("owner %s is already assigned to vlan %d", owner, assignment)
	}
	index := vlan - v.StartingVlan
	err := v.assignVlan(owner, vlan, index)
	if err != nil {
		v.Unlock()
		return err
	}

	v.Unlock()
	v.removeFromitemList(index)
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

	if v.itemList.Len() == 0 {
		return -1, fmt.Errorf("no vlans available")
	}

	// Pick one at random
	index := v.itemList.Remove(v.itemList.Front()).(int)
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
		v.itemList.PushBack(index)
		v.indexMap[index] = v.itemList.Back()
	}
	return nil
}

// GetFreeVlanCount returns how many vlans are left for assignment
func (v *VlanMgr) GetFreeVlanCount() int {
	v.RLock()
	defer v.RUnlock()

	return v.itemList.Len()
}

// Debug returns debug info of vlan assignments
func (v *VlanMgr) Debug(params map[string]string) (interface{}, error) {
	return v.ownerMap, nil
}
