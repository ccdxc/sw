package useg

import (
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/venice/ctrler/orchhub/utils/usegvlanmgr"
)

/**
 * useg allocator manages useg allocations for a single dvs

 we have one useg allocator per host
 we randomly pick a useg for each workload

 one usegmanager keeps pvlan allocations
 reserved user space is first 1000 vlans
 the rest is useg space.

 Each host gets a usegmanager
 given a workload we pick a vlan for it by getting what host its on and picking useg

 TODO: implement the following
 when a migration happens, we check if there is a conlflict
 1. no conflict, - nothing happens
 2. We switch the useg for that workload and set the new override


 Order of events:
 1. VCHub comes up and initializes vlan assignments based on existing vms/workloads
 2. VM comes up in vcenter
 3. vm reaches probe, probe sends to store
 4. In store, we make an assignment call to useg.go
	- useg saves the assigned vlan.
	- check that we haven't already tried to do an assignment

 5. store sends write event to probe
 6. probe writes to vcenter
 7. watch event comes from vcenter
 8. check that it has the same value as expected in useg. If not, issue new write request.
 9. If it's correct, we write the useg into statemgr
 10. When a VM is deleted we must remove from useg.go even if the workload isn't created completely.

 Restart behavior:
 On restart, for every VM we get, we check statemgr first to see if it has an assigned vlan already.
 If it has a conflict, we will reassign one of the vlans

*/

// Inf is the interface
type Inf interface {
	AssignVlanForVnic(vnicKey string, host string) (int, error)
	ReleaseVlanForVnic(vnicKey string, host string) error
	GetVlanForVnic(vnicKey string, host string) (int, error)
	SetVlanForVnic(vnicKey string, host string, vlan int) error
	// TODO: support workload migration
	// MigrationEvent(key string, oldHost string, newHost string)
	AssignVlansForPG(pg string) (int, int, error)
	GetVlansForPG(pg string) (int, int, error)
	SetVlansForPG(pg string, vlan int) error
	ReleaseVlansForPG(pg string) error
}

// Allocator implements Inf
type Allocator struct {
	pgLock    sync.Mutex
	pgVlanMgr usegvlanmgr.Inf
	hostLock  sync.Mutex
	hostMgrs  map[string]usegvlanmgr.Inf
}

// NewUsegAllocator creates a new useg allocator
func NewUsegAllocator() (Inf, error) {
	allocator := &Allocator{
		hostMgrs: make(map[string]usegvlanmgr.Inf),
	}
	err := allocator.newPGVlanAllocator()
	return allocator, err
}

// for 500 PGs, we need the first 1000 vlans
// PG space is FirstUsegVlan = FirstPGVlan

// FirstPGVlan is the first vlan value used for PGs
// 0 and 1  are not used
const FirstPGVlan = int(2)

// FirstUsegVlan is the lowest value
// that is used for useg assignment
const FirstUsegVlan = int(1002)

// IsPGVlanSecondary returns whether the given vlan is primary or secondary pvlan
func IsPGVlanSecondary(vlan int) bool {
	// first PG Value is primary so mod 2 should
	// not be equal if it is secondary
	return FirstPGVlan%2 != vlan%2
}

// AssignVlanForVnic assings a vlan for the given vnic
func (u *Allocator) AssignVlanForVnic(vnicKey string, host string) (int, error) {
	u.hostLock.Lock()
	defer u.hostLock.Unlock()
	vlanMgr, ok := u.hostMgrs[host]
	if !ok {
		if err := u.newHostVlanAllocator(host); err != nil {
			return -1, err
		}
		vlanMgr = u.hostMgrs[host]
	}
	return vlanMgr.AssignVlan(vnicKey)
}

// ReleaseVlanForVnic release a vlan for the given vnic
func (u *Allocator) ReleaseVlanForVnic(vnicKey string, host string) error {
	u.hostLock.Lock()
	defer u.hostLock.Unlock()
	vlanMgr, ok := u.hostMgrs[host]
	if !ok {
		return nil
	}

	return vlanMgr.ReleaseVlanOwner(vnicKey)
}

// SetVlanForVnic assings a vlan for the given vnic
func (u *Allocator) SetVlanForVnic(vnicKey string, host string, vlan int) error {
	u.hostLock.Lock()
	defer u.hostLock.Unlock()
	vlanMgr, ok := u.hostMgrs[host]
	if !ok {
		if err := u.newHostVlanAllocator(host); err != nil {
			return err
		}
		vlanMgr = u.hostMgrs[host]
	}
	return vlanMgr.SetVlanOwner(vnicKey, vlan)
}

// GetVlanForVnic returns the vlan for the given vnic
func (u *Allocator) GetVlanForVnic(vnicKey string, host string) (int, error) {
	u.hostLock.Lock()
	defer u.hostLock.Unlock()
	vlanMgr, ok := u.hostMgrs[host]
	if !ok {
		return -1, fmt.Errorf("No assignment was found for vnic %s, host %s", vnicKey, host)
	}

	return vlanMgr.GetVlan(vnicKey)
}

// AssignVlansForPG assigns a primary and secondary vlan for the given PG
func (u *Allocator) AssignVlansForPG(pg string) (int, int, error) {
	key1, key2 := createPGKeys(pg)

	out1 := -1
	out2 := -2
	var err error

	u.pgLock.Lock()
	defer u.pgLock.Unlock()
	if u.pgVlanMgr.GetFreeVlanCount() < 2 {
		return -1, -1, fmt.Errorf("PG limit reached")
	}

	val, err := u.pgVlanMgr.GetVlan(key1)
	if err != nil || val == -1 {
		// Assign a value
		out1, err = u.pgVlanMgr.AssignVlan(key1)
		if err != nil {
			return -1, -1, err
		}
	}

	val, err = u.pgVlanMgr.GetVlan(key2)
	if err != nil || val == -1 {
		// Assign a value
		out2, err = u.pgVlanMgr.AssignVlan(key2)
		if err != nil {
			// Clean up other assignment first
			u.pgVlanMgr.ReleaseVlan(out1)
			return -1, -1, err
		}
	}

	return out1, out2, nil
}

// SetVlansForPG sets the vlans as belonging to the given PG. Primary or secondary vlan
// can be supplied
func (u *Allocator) SetVlansForPG(pg string, vlan int) error {
	primary := vlan
	secondary := vlan + 1
	if IsPGVlanSecondary(vlan) {
		primary = vlan - 1
		secondary = vlan
	}
	return u.setVlansForPG(pg, primary, secondary)
}

func (u *Allocator) setVlansForPG(pg string, primary int, secondary int) error {
	key1, key2 := createPGKeys(pg)

	var err error

	u.pgLock.Lock()
	defer u.pgLock.Unlock()

	// Set primary
	err = u.pgVlanMgr.SetVlanOwner(key1, primary)
	if err != nil {
		return err
	}

	// Set secondary
	err = u.pgVlanMgr.SetVlanOwner(key2, secondary)
	if err != nil {
		// Clean up other
		u.pgVlanMgr.ReleaseVlan(primary)
		return err
	}

	return err
}

// GetVlansForPG returns the assigned primary and secondary pvlans
func (u *Allocator) GetVlansForPG(pg string) (int, int, error) {
	key1, key2 := createPGKeys(pg)

	u.pgLock.Lock()
	defer u.pgLock.Unlock()
	out1, err := u.pgVlanMgr.GetVlan(key1)
	if err != nil {
		return -1, -1, err
	}
	out2, err := u.pgVlanMgr.GetVlan(key2)
	if err != nil {
		return -1, -1, err
	}

	return out1, out2, nil
}

// ReleaseVlansForPG releases the primary and secondary vlan assignments for the given PG
func (u *Allocator) ReleaseVlansForPG(pg string) error {
	key1, key2 := createPGKeys(pg)

	u.pgLock.Lock()
	defer u.pgLock.Unlock()

	err := u.pgVlanMgr.ReleaseVlanOwner(key1)
	if err != nil {
		return err
	}
	err = u.pgVlanMgr.ReleaseVlanOwner(key2)
	if err != nil {
		return err
	}
	return nil
}

// newHostVlanAllocator creates a new vlan mgr for the given host name
// Caller must have the lock
func (u *Allocator) newHostVlanAllocator(host string) error {
	if _, ok := u.hostMgrs[host]; ok {
		return nil
	}
	seed := time.Now().Unix()
	vlanMgr := usegvlanmgr.NewVlanManager(FirstUsegVlan, usegvlanmgr.VlanMax, true, seed)
	u.hostMgrs[host] = vlanMgr
	return nil
}

// newPGVlanAllocator creates a new vlan mgr for just managing pvlan
// allocations for PGs.
// Caller must have the lock
func (u *Allocator) newPGVlanAllocator() error {
	mgr := usegvlanmgr.NewVlanManager(FirstPGVlan, FirstUsegVlan, false, 0)
	u.pgVlanMgr = mgr
	return nil
}

// createPGKeys returns the formatted keys for the PG's primary and secondary vlan assignments
func createPGKeys(key string) (string, string) {
	return fmt.Sprintf("%s-primary", key), fmt.Sprintf("%s-secondary", key)
}
