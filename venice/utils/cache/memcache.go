// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package cache

import (
	"time"

	gcache "github.com/patrickmn/go-cache"
)

const (
	// DefaultCleanupInterval expired items are cleaned up in this interval
	DefaultCleanupInterval = 5 * time.Minute

	// NoExpiration - cache items never expire
	NoExpiration = gcache.NoExpiration // -1
)

// MemCache wrapper around go-cache, in-memory cache
type MemCache struct {
	// default expiry to be set on items that don't carry expiration
	expiration time.Duration

	// cache ensure thread safety
	c *gcache.Cache
}

// EvictCallback function to be called while evicting an item
type EvictCallback func(key string, value interface{})

// New creates a new cache with the given expiry (acts as the default expiry for
// all the items that don't carry explicit expiration), cleanup interaval
// and evict call backs. Invalid values are hanled like below:
// if the given expiry is invalid, then it is set to `NoExpiration`
// if the given cleanupInterval is invalid, then it is set to `DefaultCleanupInterval`
func New(defaultExpiry time.Duration, cleanupInterval time.Duration, onEvict EvictCallback) Cache {
	if defaultExpiry <= 0 {
		defaultExpiry = NoExpiration
	}

	if cleanupInterval <= 0 {
		cleanupInterval = DefaultCleanupInterval
	}

	mCache := &MemCache{expiration: defaultExpiry}

	// Expired items are deleted once in 5 minutes by default
	mCache.c = gcache.New(gcache.NoExpiration, cleanupInterval)

	// set evict call back
	if onEvict != nil {
		mCache.c.OnEvicted(onEvict)
	}

	return mCache
}

// Add adds an item to the cache with the default expiration.
// consecutive calls to `Add` with the same key, replaces the existing item.
func (mc *MemCache) Add(key string, value interface{}) {
	mc.c.Set(key, value, mc.expiration)
}

// AddWithExpiration adds an item to the cache with the given expiration.
// consecutive calls to `AddWithExpiration` with the same key, replaces the existing item.
func (mc *MemCache) AddWithExpiration(key string, value interface{}, expiry time.Duration) {
	// if the expiry is invalid, set it to never expire
	if expiry <= 0 {
		expiry = NoExpiration
	}

	mc.c.Set(key, value, expiry)
}

// Get returns the given key's value from the cache along with bool which
// indicates whether the item was found or not.
func (mc *MemCache) Get(key string) (interface{}, bool) {
	return mc.c.Get(key)
}

// Remove removes the given key from the cache. And, does nothing if the key
// is not found in the cache.
func (mc *MemCache) Remove(key string) {
	mc.c.Delete(key)
}

// Size returns the number of items in the cache.
func (mc *MemCache) Size() int {

	return mc.c.ItemCount()
}

// Keys returns the list of keys from the cache
func (mc *MemCache) Keys() []string {
	items := mc.c.Items()

	keys := make([]string, len(items))
	i := 0
	for key := range items {
		keys[i] = key
		i++
	}

	return keys
}

// Items returns all the cache items
func (mc *MemCache) Items() map[string]interface{} {
	cItems := mc.c.Items()
	items := make(map[string]interface{}, len(cItems))

	for key, value := range cItems {
		items[key] = value.Object
	}

	return items
}

// Flush deletes all the items from the cache
func (mc *MemCache) Flush() {
	mc.c.Flush()
}
