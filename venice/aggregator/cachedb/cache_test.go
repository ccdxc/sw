package cachedb

import (
	"bytes"
	"errors"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"reflect"
	"runtime"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
)

func TestCache_NewCache(t *testing.T) {
	c := NewCache(100, "")
	if c == nil {
		t.Fatalf("failed to create new cache")
	}

	if c.MaxSize() != 100 {
		t.Fatalf("new cache max size not correct")
	}
	if c.Size() != 0 {
		t.Fatalf("new cache size not correct")
	}
	if len(c.Keys()) != 0 {
		t.Fatalf("new cache keys not correct: %v", c.Keys())
	}
}

func TestCache_CacheWrite(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(3*valuesSize, "")

	if err := c.Write([]byte("foo"), values); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if err := c.Write([]byte("bar"), values); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if n := c.Size(); n != 2*valuesSize+6 {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", 2*valuesSize, n)
	}

	if exp, keys := [][]byte{[]byte("bar"), []byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}

	s := c.Statistics(nil)
	x := s[0].Values[statCacheWriteErr]
	we := x.(int64)
	if we != 0 {
		t.Fatalf("Expected 0 WriteErr, got %d", we)
	}
	c.Free()
	if c.Count() != 0 {
		t.Fatalf("Free failed")
	}
}

func TestCache_CacheWrite_TypeConflict(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, int(64))
	values := Values{v0, v1}
	valuesSize := v0.Size() + v1.Size()

	c := NewCache(uint64(2*valuesSize), "")

	if err := c.Write([]byte("foo"), values[:1]); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}

	if err := c.Write([]byte("foo"), values[1:]); err == nil {
		t.Fatalf("expected field type conflict")
	}

	if exp, got := uint64(v0.Size())+3, c.Size(); exp != got {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", exp, got)
	}
}

func TestCache_CacheWriteMulti(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(30*valuesSize, "")

	if err := c.WriteMulti(map[string][]Value{"foo": values, "bar": values}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if n := c.Size(); n != 2*valuesSize+6 {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", 2*valuesSize, n)
	}

	if exp, keys := [][]byte{[]byte("bar"), []byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}
}

// Tests that the cache stats and size are correctly maintained during writes.
func TestCache_WriteMulti_Stats(t *testing.T) {
	limit := uint64(1)
	c := NewCache(limit, "")
	ms := NewTestStore()
	c.store = ms

	// Not enough room in the cache.
	v := NewValue(1, 1.0)
	values := map[string][]Value{"foo": {v, v}}
	if got, exp := c.WriteMulti(values), ErrCacheMemorySizeLimitExceeded(uint64(v.Size()*2), limit); !reflect.DeepEqual(got, exp) {
		t.Fatalf("got %q, expected %q", got, exp)
	}

	// Fail one of the values in the write.
	c = NewCache(50, "")
	c.init()
	c.store = ms

	ms.writef = func(key []byte, v Values) (bool, error) {
		if bytes.Equal(key, []byte("foo")) {
			return false, errors.New("write failed")
		}
		return true, nil
	}

	values = map[string][]Value{"foo": {v, v}, "bar": {v}}
	if got, exp := c.WriteMulti(values), errors.New("write failed"); !reflect.DeepEqual(got, exp) {
		t.Fatalf("got %v, expected %v", got, exp)
	}

	// Cache size decreased correctly.
	if got, exp := c.Size(), uint64(16)+3; got != exp {
		t.Fatalf("got %v, expected %v", got, exp)
	}

	// Write stats updated
	if got, exp := c.stats.WriteDropped, int64(1); got != exp {
		t.Fatalf("got %v, expected %v", got, exp)
	} else if got, exp := c.stats.WriteErr, int64(1); got != exp {
		t.Fatalf("got %v, expected %v", got, exp)
	}
}

func TestCache_CacheWriteMulti_TypeConflict(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, int64(3))
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(3*valuesSize, "")

	if err := c.WriteMulti(map[string][]Value{"foo": values[:1], "bar": values[1:]}); err == nil {
		t.Fatalf(" expected field type conflict")
	}

	if exp, got := uint64(v0.Size())+3, c.Size(); exp != got {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", exp, got)
	}

	if exp, keys := [][]byte{[]byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}
}

func TestCache_Cache_DeleteRange(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(30*valuesSize, "")

	if err := c.WriteMulti(map[string][]Value{"foo": values, "bar": values}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if n := c.Size(); n != 2*valuesSize+6 {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", 2*valuesSize, n)
	}

	if exp, keys := [][]byte{[]byte("bar"), []byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}
}

func TestCache_DeleteRange_NoValues(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(3*valuesSize, "")

	if err := c.WriteMulti(map[string][]Value{"foo": values}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if n := c.Size(); n != valuesSize+3 {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", 2*valuesSize, n)
	}

	if exp, keys := [][]byte{[]byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}
}

func TestCache_Cache_Delete(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(30*valuesSize, "")

	if err := c.WriteMulti(map[string][]Value{"foo": values, "bar": values}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if n := c.Size(); n != 2*valuesSize+6 {
		t.Fatalf("cache size incorrect after 2 writes, exp %d, got %d", 2*valuesSize, n)
	}

	if exp, keys := [][]byte{[]byte("bar"), []byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}
}

func TestCache_Cache_Delete_NonExistent(t *testing.T) {
	c := NewCache(1024, "")

	c.Delete([][]byte{[]byte("bar")})

	if got, exp := c.Size(), uint64(0); exp != got {
		t.Fatalf("cache size incorrect exp %d, got %d", exp, got)
	}
}

// This tests writing two batches to the same series.  The first batch
// is sorted.  The second batch is also sorted but contains duplicates.
func TestCache_CacheWriteMulti_Duplicates(t *testing.T) {
	v0 := NewValue(2, 1.0)
	v1 := NewValue(3, 1.0)
	values0 := Values{v0, v1}

	v3 := NewValue(4, 2.0)
	v4 := NewValue(5, 3.0)
	v5 := NewValue(5, 3.0)
	values1 := Values{v3, v4, v5}

	c := NewCache(0, "")

	if err := c.WriteMulti(map[string][]Value{"foo": values0}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}

	if err := c.WriteMulti(map[string][]Value{"foo": values1}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}

	if exp, keys := [][]byte{[]byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after 2 writes, exp %v, got %v", exp, keys)
	}

	expAscValues := Values{v0, v1, v3, v5}
	if exp, got := len(expAscValues), len(c.Values([]byte("foo"))); exp != got {
		t.Fatalf("value count mismatch: exp: %v, got %v", exp, got)
	}
	if deduped := c.Values([]byte("foo")); !reflect.DeepEqual(expAscValues, deduped) {
		t.Fatalf("deduped ascending values for foo incorrect, exp: %v, got %v", expAscValues, deduped)
	}
}

func TestCache_CacheValues(t *testing.T) {
	v0 := NewValue(1, 0.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	v3 := NewValue(1, 1.0)
	v4 := NewValue(4, 4.0)

	c := NewCache(512, "")
	if deduped := c.Values([]byte("no such key")); deduped != nil {
		t.Fatalf("Values returned for no such key")
	}

	if err := c.Write([]byte("foo"), Values{v0, v1, v2, v3}); err != nil {
		t.Fatalf("failed to write 3 values, key foo to cache: %s", err.Error())
	}
	if err := c.Write([]byte("foo"), Values{v4}); err != nil {
		t.Fatalf("failed to write 1 value, key foo to cache: %s", err.Error())
	}

	expAscValues := Values{v3, v1, v2, v4}
	if deduped := c.Values([]byte("foo")); !reflect.DeepEqual(expAscValues, deduped) {
		t.Fatalf("deduped ascending values for foo incorrect, exp: %v, got %v", expAscValues, deduped)
	}
}

func TestCache_CacheWriteMemoryExceeded(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)

	c := NewCache(uint64(v1.Size()), "")

	if err := c.Write([]byte("foo"), Values{v0}); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if exp, keys := [][]byte{[]byte("foo")}, c.Keys(); !reflect.DeepEqual(keys, exp) {
		t.Fatalf("cache keys incorrect after writes, exp %v, got %v", exp, keys)
	}
	if err := c.Write([]byte("bar"), Values{v1}); err == nil || !strings.Contains(err.Error(), "cache-max-memory-size") {
		t.Fatalf("wrong error writing key bar to cache: %v", err)
	}

	// Grab snapshot, write should still fail since we're still using the memory.
	_, err := c.Snapshot()
	if err != nil {
		t.Fatalf("failed to snapshot cache: %v", err)
	}
	if err := c.Write([]byte("bar"), Values{v1}); err == nil || !strings.Contains(err.Error(), "cache-max-memory-size") {
		t.Fatalf("wrong error writing key bar to cache: %v", err)
	}
}

func TestCache_Deduplicate_Concurrent(t *testing.T) {
	if testing.Short() || os.Getenv("GORACE") != "" || os.Getenv("APPVEYOR") != "" {
		t.Skip("Skipping test in short, race, appveyor mode.")
	}

	values := make(map[string][]Value)

	for i := 0; i < 1000; i++ {
		for j := 0; j < 100; j++ {
			vv := values[fmt.Sprintf("cpu%d", i)]
			values[fmt.Sprintf("cpu%d", i)] = append(vv, NewValue(int64(i+j)+int64(rand.Intn(10)), float64(i)))
		}
	}

	wg := sync.WaitGroup{}
	c := NewCache(1000000, "")

	wg.Add(1)
	c.WriteMulti(values)
	c.WriteMulti(values)
	go func() {
		defer wg.Done()
		for i := 0; i < 1000; i++ {
			c.WriteMulti(values)
		}
	}()

	wg.Add(1)
	go func() {
		defer wg.Done()
		for i := 0; i < 1000; i++ {
			c.Deduplicate()
		}
	}()

	wg.Wait()
}

func TestCache_Split(t *testing.T) {
	v0 := NewValue(1, 1.0)
	v1 := NewValue(2, 2.0)
	v2 := NewValue(3, 3.0)
	values := Values{v0, v1, v2}
	valuesSize := uint64(v0.Size() + v1.Size() + v2.Size())

	c := NewCache(0, "")

	if err := c.Write([]byte("foo"), values); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}
	if err := c.Write([]byte("bar"), values); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}

	if err := c.Write([]byte("baz"), values); err != nil {
		t.Fatalf("failed to write key foo to cache: %s", err.Error())
	}

	if n := c.Size(); n != 3*valuesSize+9 {
		t.Fatalf("cache size incorrect after 3 writes, exp %d, got %d", 3*valuesSize*9, n)
	}

	splits := c.Split(3)
	keys := make(map[string]int)
	for _, s := range splits {
		for _, k := range s.Keys() {
			keys[string(k)] = s.Values(k).Size()
		}
	}

	for _, key := range []string{"foo", "bar", "baz"} {
		if _, ok := keys[key]; !ok {
			t.Fatalf("missing key, exp %s, got %v", key, nil)
		}
	}
}

func TestCache_entry(t *testing.T) {
	values := make([]Value, 10)

	for i := 0; i < 10; i++ {
		values[i] = NewValue(int64(i+1), float64(i))
	}

	entry, err := newEntryValues(values, 0) // Will use default allocation size.
	if err != nil {
		t.Fatal(err)
	}

	if err := entry.add(values); err != nil {
		t.Fatal(err)
	}

	entry.filter(0, 16000)
	if entry.count() != 0 {
		t.Fatalf("Expected 0, got %d", entry.count())
	}

	if err := entry.add([]Value{NewValue(int64(1), float64(1))}); err != nil {
		t.Fatal(err)
	}
	entry.deduplicate()

	otherValues := make([]Value, 2)
	for i := 0; i < 2; i++ {
		otherValues[i] = NewValue(int64(i+1), float64(i))
	}

	if err := entry.add(otherValues); err != nil {
		t.Fatal(err)
	}

	if entry.count() != 3 {
		t.Fatalf("Expected 3, got %d", entry.count())
	}

	confValues := make([]Value, 2)
	for i := 0; i < 2; i++ {
		confValues[i] = NewValue(int64(i+1), int64(i))
	}

	if err := entry.add(confValues); err == nil {
		t.Fatal("Expected conflict, got none")
	}

	dupValues := make([]Value, 2)
	for i := 0; i < 2; i++ {
		dupValues[i] = NewValue(int64(i+1), float64(i))
	}

	if err := entry.add(dupValues); err != nil {
		t.Fatal(err)
	}

	if entry.count() != 5 {
		t.Fatalf("Expected 5, got %d", entry.count())
	}

	entry.deduplicate()
	if entry.count() != 2 {
		t.Fatalf("Expected 2, got %d", entry.count())
	}
}

func mustTempDir() string {
	dir, err := ioutil.TempDir("", "tsm1-test")
	if err != nil {
		panic(fmt.Sprintf("failed to create temp dir: %v", err))
	}
	return dir
}

func mustTempFile(dir string) *os.File {
	f, err := ioutil.TempFile(dir, "tsm1test")
	if err != nil {
		panic(fmt.Sprintf("failed to create temp file: %v", err))
	}
	return f
}

// TestStore implements the storer interface and can be used to mock out a
// Cache's storer implememation.
type TestStore struct {
	entryf       func(key []byte) *entry
	writef       func(key []byte, values Values) (bool, error)
	addf         func(key []byte, entry *entry)
	removef      func(key []byte)
	keysf        func(sorted bool) [][]byte
	applyf       func(f func([]byte, *entry) error) error
	applySerialf func(f func([]byte, *entry) error) error
	resetf       func()
	splitf       func(n int) []storer
	countf       func() int
}

func NewTestStore() *TestStore                                      { return &TestStore{} }
func (s *TestStore) entry(key []byte) *entry                        { return s.entryf(key) }
func (s *TestStore) write(key []byte, values Values) (bool, error)  { return s.writef(key, values) }
func (s *TestStore) add(key []byte, entry *entry)                   { s.addf(key, entry) }
func (s *TestStore) remove(key []byte)                              { s.removef(key) }
func (s *TestStore) keys(sorted bool) [][]byte                      { return s.keysf(sorted) }
func (s *TestStore) apply(f func([]byte, *entry) error) error       { return s.applyf(f) }
func (s *TestStore) applySerial(f func([]byte, *entry) error) error { return s.applySerialf(f) }
func (s *TestStore) reset()                                         { s.resetf() }
func (s *TestStore) split(n int) []storer                           { return s.splitf(n) }
func (s *TestStore) count() int                                     { return s.countf() }

var fvSize = uint64(NewValue(1, float64(1)).Size())

func BenchmarkCacheFloatEntries(b *testing.B) {
	cache := NewCache(uint64(b.N)*fvSize, "")
	vals := make([][]Value, b.N)
	for i := 0; i < b.N; i++ {
		vals[i] = []Value{NewValue(1, float64(i))}
	}
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		if err := cache.Write([]byte("test"), vals[i]); err != nil {
			b.Fatal("err:", err, "i:", i, "N:", b.N)
		}
	}
}

type points struct {
	key  []byte
	vals []Value
}

func BenchmarkCacheParallelFloatEntries(b *testing.B) {
	c := b.N * runtime.GOMAXPROCS(0)
	cache := NewCache(uint64(c)*fvSize*10, "")
	vals := make([]points, c)
	for i := 0; i < c; i++ {
		v := make([]Value, 10)
		for j := 0; j < 10; j++ {
			v[j] = NewValue(1, float64(i+j))
		}
		vals[i] = points{key: []byte(fmt.Sprintf("cpu%v", rand.Intn(20))), vals: v}
	}
	i := int32(-1)
	b.ResetTimer()

	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			j := atomic.AddInt32(&i, 1)
			v := vals[j]
			if err := cache.Write(v.key, v.vals); err != nil {
				b.Fatal("err:", err, "j:", j, "N:", b.N)
			}
		}
	})
}

func BenchmarkEntry_add(b *testing.B) {
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			b.StopTimer()
			values := make([]Value, 10)
			for i := 0; i < 10; i++ {
				values[i] = NewValue(int64(i+1), float64(i))
			}

			otherValues := make([]Value, 10)
			for i := 0; i < 10; i++ {
				otherValues[i] = NewValue(1, float64(i))
			}

			entry, err := newEntryValues(values, 0) // Will use default allocation size.
			if err != nil {
				b.Fatal(err)
			}

			b.StartTimer()
			if err := entry.add(otherValues); err != nil {
				b.Fatal(err)
			}
		}
	})
}
