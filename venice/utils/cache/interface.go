// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package cache

import "time"

// Cache interface; any type of cache implementation should adhere to this.
type Cache interface {
	// Adds a value to the cache, replacing any existing item if found.
	// any item that gets replaced, will be set with the new expiry
	// (way to extend the item's lifetime).
	Add(key string, value interface{})

	// AddWithExpiration adds an item to the cache with the given expiry.
	// i.e. it honors the given expiry over the default expiration set with New(..)
	// or NewWithEvict(..)
	AddWithExpiration(key string, value interface{}, expiry time.Duration)

	// Returns key's value from the cache and a bool indication whether the item was
	// found in the cache.
	Get(key string) (value interface{}, ok bool)

	// Keys returns the list of keys from the cache
	Keys() []string

	// Items returns all the cache items (key, value pairs)
	Items() map[string]interface{}

	// Removes a key from the cache.
	Remove(key string)

	// Returns the number of items in the cache.
	Size() int

	// Flushes all the items from the cache.
	Flush()
}
