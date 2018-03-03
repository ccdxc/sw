// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package cache

import (
	"fmt"
	"runtime"
	"sync"
	"testing"
	"time"

	tu "github.com/pensando/sw/venice/utils/testutils"
)

// Benchmarking results on add and get operations
// Workers: 8
//|--------------------------------------------------------------------------------------|
//| Operation               |#.of.operations | operations/worker | avg.time taken/worker |
//|--------------------------------------------------------------------------------------|
//| AddWithExpiry           |    3000000     |   375000          |  413 ns/op            |
//| AddWithNoExpiry         |    5000000     |   625000          |  382 ns/op            |
//| GetItems(with expiry)   |   20000000     |  2500000          |   70.4 ns/op          |
//| GetItems(w/o expiry)    |   20000000     |  2500000          |   69.6 ns/op          |
//| InvalidGet(non-existing)|   20000000     |  2500000          |   68.8 ns/op          |
//|-------------------------|----------------|-------------------|-----------------------|

// TestCache tests the basic add/get cache functionalities
func TestCache(t *testing.T) {
	// items never expire; 5 mins - default cleanup interval
	mc := New(NoExpiration, DefaultCleanupInterval, nil)

	a, found := mc.Get("a")
	tu.Assert(t, !found && a == nil, "found item that shouldn't exist")

	b, found := mc.Get("b")
	tu.Assert(t, !found && b == nil, "found item that shouldn't exist")

	c, found := mc.Get("c")
	tu.Assert(t, !found && c == nil, "found item that shouldn't exist")

	// test int values
	mc.Add("a", 1)
	mc.Add("b", 2)

	aVal, found := mc.Get("a")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "a"))
	tu.Assert(t, aVal != nil && aVal.(int) == 1, "expected value not found")

	bVal, found := mc.Get("b")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "b"))
	tu.Assert(t, bVal != nil && bVal.(int) == 2, "expected value not found")

	// test string values
	mc.Add("c", "c-string")

	cVal, found := mc.Get("c")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "c"))
	tu.Assert(t, cVal != nil && cVal.(string) == "c-string", "expected value not found")

	// test float values
	mc.Add("d", 1.0)

	dVal, found := mc.Get("d")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "d"))
	tu.Assert(t, dVal != nil && dVal.(float64) == 1.0, "expected value not found")

	// replace an existing item
	mc.Add("a", "a-val")

	aVal, found = mc.Get("a")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "a"))
	tu.Assert(t, aVal != nil && aVal.(string) == "a-val", "expected value not found")

	// test empty keys
	mc.Add("", "foo")
	emptyVal, found := mc.Get("")
	tu.Assert(t, found, "expected empty key not found")
	tu.Assert(t, emptyVal != nil && emptyVal.(string) == "foo", "expected value not found")

	mc.Add("", "")
	emptyVal, found = mc.Get("")
	tu.Assert(t, found, "expected empty key not found")
	tu.Assert(t, emptyVal != nil && emptyVal.(string) == "", "expected value not found")

	// test empty value
	mc.Add("foo", "")
	fooVal, found := mc.Get("foo")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "foo"))
	tu.Assert(t, fooVal != nil && emptyVal.(string) == "", "expected empty value not found")

	// test nil value
	mc.Add("nil", nil)
	nilVal, found := mc.Get("nil")
	tu.Assert(t, found, fmt.Sprintf("expected key %s not found", "nil"))
	tu.Assert(t, nilVal == nil, "expected `nil` value not found")
}

// TestCacheExpiry test cache expiry
func TestCacheExpiry(t *testing.T) {
	var found bool

	// items are expired after 3s and the expired items are cleaned up every 30ms
	mc := New(3*time.Second, 30*time.Millisecond, nil)

	// gets the default expiry of 3s
	mc.Add("a", 1)

	// add an item with no expiry
	mc.AddWithExpiration("b", 2, NoExpiration)

	// expire below items at different sec
	mc.AddWithExpiration("c", 3, 2*time.Second)
	mc.AddWithExpiration("d", 4, 4*time.Second)

	<-time.After((2 * time.Second) + (100 * time.Millisecond))
	_, found = mc.Get("c") // 2s expiry
	tu.Assert(t, !found, fmt.Sprintf("key %s should have been expired", "c"))

	<-time.After(1 * time.Second)
	_, found = mc.Get("a") // 3s expiry
	tu.Assert(t, !found, fmt.Sprintf("key %v should have been expired", "a"))

	_, found = mc.Get("b") // never expires
	tu.Assert(t, found,
		fmt.Sprintf("Couldn't find key %v that should have been never expired", "b"))

	<-time.After(1 * time.Second)
	_, found = mc.Get("d")
	tu.Assert(t, !found,
		fmt.Sprintf("key %v should have been expired", "d"))

	// test extending the life of the item by adding the same key multiple times
	// with higher expiration
	mc.AddWithExpiration("foo", "bar", 2*time.Second)
	<-time.After(1 * time.Second)
	// extend the lifetime of the item
	mc.AddWithExpiration("foo", "bar", 4*time.Second)
	// item should have been expired after 2s (it's initial expiry)
	<-time.After(2 * time.Second)
	_, found = mc.Get("foo")
	tu.Assert(t, found, "key `foo` should have not been expired")

	<-time.After(10 * time.Millisecond)
	_, found = mc.Get("foo")
	tu.Assert(t, found, "key `foo` should have not been expired")

	// item should have been expired after ~4s
	<-time.After(2 * time.Second)
	_, found = mc.Get("foo")
	tu.Assert(t, !found, "key `foo` should have been expired")
}

// TestCacheWithEvict tests whether the evict call backs are involved during eviction
func TestCacheWithEvict(t *testing.T) {
	evictCounter := 0
	// to protect the evict counter
	mutex := &sync.RWMutex{}

	onEvicted := func(k string, v interface{}) {
		mutex.Lock()
		evictCounter++
		mutex.Unlock()
	}

	mc := New(3*time.Second, 30*time.Millisecond, onEvicted)

	// gets the default expiry of 3s
	mc.Add("a", 1)

	// add an item with no expiry
	mc.AddWithExpiration("b", 2, NoExpiration)

	// expire below items at different secs
	mc.AddWithExpiration("c", 3, 2*time.Second)
	mc.AddWithExpiration("d", 4, 4*time.Second)

	<-time.After((2 * time.Second) + (100 * time.Millisecond))
	_, found := mc.Get("c") // 2s expiry
	tu.Assert(t, !found, fmt.Sprintf("key %v should have been expired", "c"))
	mutex.RLock()
	tu.Assert(t, evictCounter >= 1,
		fmt.Sprintf("invalid eviction counter, expected: >=%v, got: %v", 1, evictCounter))
	mutex.RUnlock()

	<-time.After(1 * time.Second)
	_, found = mc.Get("a") // 3s expiry
	tu.Assert(t, !found, fmt.Sprintf("key %v should have been expired", "a"))
	mutex.RLock()
	tu.Assert(t, evictCounter >= 2,
		fmt.Sprintf("invalid eviction counter, expected: >=%v, got: %v", 2, evictCounter))
	mutex.RUnlock()

	_, found = mc.Get("b") // never expires and the counter remains the same
	tu.Assert(t, found,
		fmt.Sprintf("Couldn't find key %v that should have been never expired", "b"))
	mutex.RLock()
	tu.Assert(t, evictCounter >= 2,
		fmt.Sprintf("invalid eviction counter, expected: >=%v, got: %v", 2, evictCounter))
	mutex.RUnlock()

	<-time.After(2 * time.Second)
	_, found = mc.Get("d")
	tu.Assert(t, !found,
		fmt.Sprintf("key %v should have been expired", "d"))
	mutex.RLock()
	tu.Assert(t, evictCounter == 3,
		fmt.Sprintf("invalid eviction counter, expected: %v, got: %v", 3, evictCounter))
	mutex.RUnlock()
}

// TestCacheReplace tests replace an item and it's behavior
func TestCacheReplace(t *testing.T) {
	mc := New(NoExpiration, 0, nil)

	// add foo = bar
	mc.Add("foo", "bar")
	val, found := mc.Get("foo")
	tu.Assert(t, found && val.(string) == "bar",
		fmt.Sprintf("found inconsistency, expected: %v, got: %v", "bar", val.(string)))

	// replace foo = bar -> foo = bar1 and make sure the get returns the updated value
	mc.Add("foo", "bar1")
	val, found = mc.Get("foo")
	tu.Assert(t, found && val.(string) == "bar1",
		fmt.Sprintf("found inconsistency, expected: %v, got: %v", "bar1", val.(string)))

	// revert back to bar
	mc.Add("foo", "bar")
	val, found = mc.Get("foo")
	tu.Assert(t, found && val.(string) == "bar",
		fmt.Sprintf("found inconsistency, expected: %v, got: %v", "bar", val.(string)))
}

// TestCacheRemove tests the delete/remove functionality
func TestCacheRemove(t *testing.T) {
	mc := New(NoExpiration, 0, nil)

	mc.Add("foo", "bar")
	val, found := mc.Get("foo")
	tu.Assert(t, found && val.(string) == "bar",
		fmt.Sprintf("found inconsistency, expected: %v, got: %v", "bar", val.(string)))

	// remove the item and make sure it doesn't exist in the cache
	mc.Remove("foo")
	val, found = mc.Get("foo")
	tu.Assert(t, !found && val == nil,
		fmt.Sprintf("found inconsitency, expected: %v, got: %v", nil, val))
}

// TestCacheFlush tests the flush/purge functionality
func TestCacheFlush(t *testing.T) {
	mc := New(NoExpiration, 0, nil)

	// add some items to the cache
	mc.Add("foo", "bar")
	mc.Add("baz", "qux")

	// flush the cache
	mc.Flush()

	// ensure the items are not there in the cache
	foo, found := mc.Get("foo")
	tu.Assert(t, !found && foo == nil,
		fmt.Sprintf("%v was found, but it should have been flushed", "foo"))

	baz, found := mc.Get("baz")
	tu.Assert(t, !found && baz == nil,
		fmt.Sprintf("%v was found, but it should have been flushed", "baz"))
}

// TestCacheSize ensures the cache updates are reflected on the Size(..) call
func TestCacheSize(t *testing.T) {
	mc := New(3*time.Second, 30*time.Millisecond, nil)

	// gets the default expiry of 3s
	mc.Add("a", 1)

	// add an item with no expiry
	mc.AddWithExpiration("b", 2, NoExpiration)

	// expire below items at different secs
	mc.AddWithExpiration("c", 3, 2*time.Second)
	mc.AddWithExpiration("d", 4, 4*time.Second)

	// ensure size is correct
	size := mc.Size()
	tu.Assert(t, size == 4, fmt.Sprintf("invalid size, expected: %v, got: %v", 4, size))

	<-time.After((2 * time.Second) + (100 * time.Millisecond))
	_, found := mc.Get("c") // 2s expiry
	tu.Assert(t, !found, fmt.Sprintf("key %v should have been expired", "c"))

	// ensure len after expiry of an item
	size = mc.Size()
	tu.Assert(t, size <= 3, fmt.Sprintf("invalid size, expected: <=%v, got: %v", 3, size))

	<-time.After(1 * time.Second)
	_, found = mc.Get("a") // 3s expiry
	tu.Assert(t, !found, fmt.Sprintf("key %v should have been expired", "a"))
	// ensure size after expiry of an item
	size = mc.Size()
	tu.Assert(t, size <= 2, fmt.Sprintf("invalid size, expected: <=%v, got: %v", 2, size))

	// never expires and the counter remains the same
	_, found = mc.Get("b")
	tu.Assert(t, found,
		fmt.Sprintf("Couldn't find key %v that should have been never expired", "b"))

	<-time.After(1 * time.Second)
	_, found = mc.Get("d")
	tu.Assert(t, !found,
		fmt.Sprintf("key %v should have been expired", "d"))
	// ensure size after expiry of an item
	size = mc.Size()
	tu.Assert(t, size == 1, fmt.Sprintf("invalid size, expected: %v, got: %v", 1, size))
}

// TestCacheKeys tests the behavior of `Keys()`
func TestCacheKeys(t *testing.T) {
	mc := New(NoExpiration, 0, nil)

	mc.Add("a", 1)

	// add an item with no expiry
	mc.AddWithExpiration("b", 2, NoExpiration)

	// expire below items at different ms
	mc.AddWithExpiration("c", 3, 10*time.Millisecond)
	mc.AddWithExpiration("d", 4, 50*time.Millisecond)

	keys := mc.Keys()
	tu.Assert(t, len(keys) == 4,
		fmt.Sprintf("invalid number of keys, expected: %v, got: %v", 4, len(keys)))

	for _, key := range keys {
		tu.Assert(t, key == "a" || key == "b" || key == "c" || key == "d",
			fmt.Sprintf("invalid key found %v", key))
	}

	// remove a key and ensure the keys are correct
	mc.Remove("d")

	keys = mc.Keys()
	tu.Assert(t, len(keys) == 3,
		fmt.Sprintf("invalid number of keys, expected: %v, got: %v", 3, len(keys)))

	for _, key := range keys {
		tu.Assert(t, key == "a" || key == "b" || key == "c",
			fmt.Sprintf("invalid key found %v", key))
	}
}

// TestCacheItems tests the behavior of `Items()`
func TestCacheItems(t *testing.T) {
	mc := New(NoExpiration, 0, nil)
	mc.Add("a", 1)

	// add an item with no expiry
	mc.AddWithExpiration("b", 2, NoExpiration)

	// expire below items at different ms
	mc.AddWithExpiration("c", 3, 10*time.Millisecond)
	mc.AddWithExpiration("d", 4, 50*time.Millisecond)

	items := mc.Items()
	tu.Assert(t, len(items) == 4,
		fmt.Sprintf("invalid number of items, expected: %v, got: %v", 4, len(items)))

	for key, value := range items {
		tu.Assert(t, key == "a" || key == "b" || key == "c" || key == "d",
			fmt.Sprintf("invalid key found %v", key))
		tu.Assert(t, value == 1 || value == 2 || value == 3 || value == 4,
			fmt.Sprintf("invalid value found %v for key %v", value, key))
	}

	// remove a key and ensure the keys are correct
	mc.Remove("d")

	items = mc.Items()
	tu.Assert(t, len(items) == 3,
		fmt.Sprintf("invalid number of items, expected: %v, got: %v", 3, len(items)))

	for key, value := range items {
		tu.Assert(t, key == "a" || key == "b" || key == "c",
			fmt.Sprintf("invalid key found %v", key))
		tu.Assert(t, value == 1 || value == 2 || value == 3,
			fmt.Sprintf("invalid value found %v for key %v", value, key))
	}
}

// benchmarks

// test concurrent adds
func BenchmarkCacheAddWithExpiry(b *testing.B) {
	benchmarkCacheAdd(b, 5*time.Minute)
}

func BenchmarkCacheAddWithNoExpiry(b *testing.B) {
	benchmarkCacheAdd(b, NoExpiration)
}

func benchmarkCacheAdd(b *testing.B, exp time.Duration) {
	b.StopTimer()
	mc := New(exp, 0, nil)
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	each := b.N / workers
	wg.Add(workers)

	// start the workers
	b.StartTimer()
	for i := 0; i < workers; i++ {
		go func(threadID int) {
			for j := 0; j < each; j++ {
				mc.Add(fmt.Sprintf("%d", j+threadID), j+threadID)
			}
			wg.Done()
		}(i)
	}

	// wait for workers
	wg.Wait()
}

// test concurrent gets
func BenchmarkCacheGetWithExpiry(b *testing.B) {
	benchmarkCacheGet(b, 5*time.Minute)
}

func BenchmarkCacheGetWithNoExpiry(b *testing.B) {
	benchmarkCacheGet(b, NoExpiration)
}

func benchmarkCacheGet(b *testing.B, exp time.Duration) {
	b.StopTimer()
	mc := New(exp, 0, nil)
	mc.Add("foo", "bar")
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	each := b.N / workers
	wg.Add(workers)

	// start the workers
	b.StartTimer()
	for i := 0; i < workers; i++ {
		go func() {
			for j := 0; j < each; j++ {
				mc.Get("foo")
			}
			wg.Done()
		}()
	}

	// wait for workers
	wg.Wait()
}

// benchmark gets on random keys that don't exists
func BenchmarkCacheInvalidGet(b *testing.B) {
	b.StopTimer()
	mc := New(NoExpiration, 0, nil)
	wg := new(sync.WaitGroup)
	workers := runtime.NumCPU()
	each := b.N / workers
	wg.Add(workers)

	// start the workers
	b.StartTimer()
	for i := 0; i < workers; i++ {
		go func(threadID int) {
			for j := 0; j < each; j++ {
				mc.Get(fmt.Sprintf("%d", j+threadID))
			}
			wg.Done()
		}(i)
	}

	// wait for workers
	wg.Wait()
}
