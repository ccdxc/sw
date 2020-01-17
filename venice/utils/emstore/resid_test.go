package emstore

import (
	"fmt"
	"math/rand"
	"testing"
	"time"

	"os"

	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	NetworkID            = "networkID"
	SecurityGroupID      = "sgID"
	TenantID             = "tenantID"
	concurrencyCount     = 3000
	boltDBPath           = "/tmp/boltdb_resourceid_generation_test.db"
	concurrentTestDBPath = "/tmp/boltdb_concurrent_resourceid_generation_test.db"
)

func TestBoltDBResourceID(t *testing.T) {
	t.Parallel()
	db, err := NewBoltdbStore(boltDBPath)
	defer cleanup(boltDBPath)
	AssertOk(t, err, fmt.Sprintf("Could not create bolt db. Err: %v", err))

	resourceIDAllocationTest(db, t, NetworkID, SecurityGroupID, TenantID)
}

func TestMemDBResourceID(t *testing.T) {
	t.Parallel()
	db, err := NewMemStore()
	AssertOk(t, err, fmt.Sprintf("Could not create bolt db. Err: %v", err))

	resourceIDAllocationTest(db, t, NetworkID, SecurityGroupID, TenantID)
}

func TestBoltDBConcurrentIDGeneration(t *testing.T) {
	t.Parallel()
	defer cleanup(concurrentTestDBPath)
	db, err := NewBoltdbStore(concurrentTestDBPath)
	AssertOk(t, err, fmt.Sprintf("Could not create bolt db. Err: %v", err))

	var genIDs []uint64

	concurrentResourceIDAllocationTest(db, t, genIDs, NetworkID, SecurityGroupID, TenantID)
}

func TestMemDBConcurrentIDGeneration(t *testing.T) {
	t.Parallel()
	db, err := NewMemStore()
	AssertOk(t, err, fmt.Sprintf("Could not create mem db. Err: %v", err))
	var genIDs []uint64

	concurrentResourceIDAllocationTest(db, t, genIDs, NetworkID, SecurityGroupID, TenantID)
}

func validateConcurrency(genids *[]uint64) error {
	idFreqMap := make(map[uint64]int)
	if len(*genids) != 3*concurrencyCount {
		return fmt.Errorf("could not find expected ids: %v", *genids)
	}

	for _, id := range *genids {
		_, ok := idFreqMap[id]
		if !ok {
			idFreqMap[id] = 1
		} else {
			idFreqMap[id]++
		}
	}

	for k, v := range idFreqMap {
		// All ID frequency should be 3, one for each networkID, sgID and tenantID
		if v != 3 {
			log.Infof("Duplicate IDs reported %v", genids)
			return fmt.Errorf("duplicate id found %v", v)
		}
		// All IDs should start from 1. Value 0 happens only on the first store of the key which should not be handed out
		if k == 0 {
			return fmt.Errorf("ids should be non zero. Found %v occurrences of 0 id", v)
		}

	}

	return nil
}

func resourceIDAllocationTest(db Emstore, t *testing.T, n, s, tn ResourceType) {
	// test net id allocation

	nextID, err := db.GetNextID(n, 0)
	AssertOk(t, err, "Could not get the next ID")
	AssertEquals(t, uint64(1), nextID, "Getting next id for network failed.")
	nextID, err = db.GetNextID(n, 0)
	AssertOk(t, err, "Could not get the next ID")
	AssertEquals(t, uint64(2), nextID, "Getting next id for network failed.")

	// test sg id allocation
	nextID, err = db.GetNextID(s, 0)
	AssertOk(t, err, "Could not get the next ID")
	AssertEquals(t, uint64(1), nextID, "Getting next id for network failed.")
	nextID, err = db.GetNextID(s, 0)
	AssertOk(t, err, "Could not get the next ID")
	AssertEquals(t, uint64(2), nextID, "Getting next id for network failed.")

	// test tenant id allocation
	nextID, err = db.GetNextID(tn, 0)
	AssertOk(t, err, "Could not get the next ID")
	AssertEquals(t, uint64(1), nextID, "Getting next id for network failed.")
	nextID, err = db.GetNextID(tn, 0)
	AssertOk(t, err, "Could not get the next ID")
	AssertEquals(t, uint64(2), nextID, "Getting next id for network failed.")
}

func concurrentResourceIDAllocationTest(db Emstore, t *testing.T, genIDs []uint64, n, s, tn ResourceType) {
	randSrc := rand.NewSource(time.Now().UnixNano())
	randGen := rand.New(randSrc)

	idChannel := make(chan uint64, concurrencyCount*3)
	defer close(idChannel)

	// request 10000 net, sg and tenant object ids
	for i := 0; i < concurrencyCount; i++ {
		randomUsec := randGen.Intn(100)
		go func(iter int) {
			id, err := db.GetNextID(n, 0)
			AssertOk(t, err, fmt.Sprintf("could not generate network ID. Iteration: %v, Err: %v", iter, err))
			idChannel <- id
			time.Sleep(time.Microsecond * time.Duration(randomUsec))
		}(i)

		go func(iter int) {
			id, err := db.GetNextID(s, 0)
			AssertOk(t, err, fmt.Sprintf("Could not generate security group ID. Iteration: %v, Err: %v", iter, err))
			idChannel <- id
			time.Sleep(time.Microsecond * time.Duration(randomUsec))
		}(i)

		go func(iter int) {
			id, err := db.GetNextID(tn, 0)
			AssertOk(t, err, fmt.Sprintf("Could not generate tenant ID. Iteration: %v, Err: %v", iter, err))
			idChannel <- id
			time.Sleep(time.Microsecond * time.Duration(randomUsec))
		}(i)
	}

	for i := 0; i < concurrencyCount*3; i++ {
		genIDs = append(genIDs, <-idChannel)
	}

	err := validateConcurrency(&genIDs)
	AssertOk(t, err, fmt.Sprintf("Failed concurrency validation. %v", err))
}

// cleanup is used to delete the database
func cleanup(dbPath string) {
	os.Remove(dbPath)
}
