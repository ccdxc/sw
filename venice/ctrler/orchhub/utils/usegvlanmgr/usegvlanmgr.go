package usegvlanmgr

import (
	"fmt"
	"sync"
)

const vlanMax int = 4096

// Inf defines the vlan manager interface
type Inf interface {
	AssignVlan(owner string, vlan int) error
	AssignNextVlan(owner string) (int, error)
	GetOwner(vlan int) (string, error)
	GetVlan(owner string) (int, error)
	ReleaseVlanOwner(owner string) error
	ReleaseVlan(vlan int) error
	getFreeVlanCount() int
}

// VlanMgr is a thread safe utility to manage vlan allocations
type VlanMgr struct {
	sync.RWMutex
	// array of vlan(index) to owner
	vlans *[vlanMax]string
	// map from owner to vlan
	ownerMap map[string]int
}

// NewVlanManager creates a vlan manager
func NewVlanManager() Inf {
	return &VlanMgr{
		vlans:    &[vlanMax]string{},
		ownerMap: make(map[string]int),
	}
}

func (v *VlanMgr) validateVlan(vlan int) error {
	if vlan < 0 || vlan >= vlanMax {
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

// AssignVlan assigns the given vlan to the given owner if the vlan is available
func (v *VlanMgr) AssignVlan(owner string, vlan int) error {
	if err := v.validateVlan(vlan); err != nil {
		return err
	}
	if err := v.validateOwner(owner); err != nil {
		return err
	}

	v.Lock()
	defer v.Unlock()

	if assignment, ok := v.ownerMap[owner]; ok {
		if vlan == assignment {
			// owner already has the given vlan
			return nil
		}
		return fmt.Errorf("owner %s is already assigned to vlan %d", owner, assignment)
	}
	return v.assignVlan(owner, vlan)
}

// Caller must acquire the lock
func (v *VlanMgr) assignVlan(owner string, vlan int) error {
	if v.vlans[vlan] != "" {
		return fmt.Errorf("vlan %d is already taken by owner %s", vlan, v.vlans[vlan])
	}
	v.vlans[vlan] = owner
	v.ownerMap[owner] = vlan
	return nil
}

// AssignNextVlan assigns the given owner a vlan
func (v *VlanMgr) AssignNextVlan(owner string) (int, error) {
	if err := v.validateOwner(owner); err != nil {
		return -1, err
	}

	v.Lock()
	defer v.Unlock()

	// Check if owner already has an assignment
	if assignment, ok := v.ownerMap[owner]; ok {
		return -1, fmt.Errorf("owner is already assigned to vlan %d", assignment)
	}

	// Find next free vlan
	nextVlan := -1
	for i, val := range v.vlans {
		if len(val) == 0 {
			nextVlan = i
			break
		}
	}
	if nextVlan == -1 {
		return -1, fmt.Errorf("no vlans available")
	}
	if err := v.assignVlan(owner, nextVlan); err != nil {
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

	val := v.vlans[vlan]
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

	owner := v.vlans[vlan]
	if len(owner) != 0 {
		v.vlans[vlan] = ""
		delete(v.ownerMap, owner)
	}
	return nil
}

// getFreeVlanCount returns how many vlans are left for assignment
func (v *VlanMgr) getFreeVlanCount() int {
	v.RLock()
	defer v.RUnlock()

	count := 0
	for _, val := range v.vlans {
		if len(val) == 0 {
			count++
		}
	}
	return count
}
