// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"time"

	memcache "github.com/pensando/sw/venice/utils/cache"
)

// global dedup cache encapsulating the cache for each event source.
// each event source gets it's own cache. It helps reduce contention and also,
// it gives the flexibility to dedup at each source. Source of the event is
// inferred from the recorder. The cache items are expired once TTL is met.
// the items (events) are deduped as long as it lives in the cache (i.e. TTL).
// Expiration is handled by the underlying memecache (-> go-cache).
// NOTE: the caller will need to ensure thread safety.
type cache struct {
	m   map[string]memcache.Cache // cache for each source; the source keys are removed from the map only during flush
	ttl time.Duration             // time to live or expiration duration for each item in the cache; this is the same as dedup-interval
}

// newDedupCache creates a new dedup cache
func newDedupCache(ttl time.Duration) *cache {
	return &cache{m: map[string]memcache.Cache{}, ttl: ttl}
}

// getSourceCache returns the underlying cache of the given source key.
// it creates, if it does not exists already.
// make sure to perform this opertion under lock to avoid race conditions.
func (c *cache) getSourceCache(sourceKey string) memcache.Cache {
	if c.m[sourceKey] == nil {
		c.m[sourceKey] = memcache.New(c.ttl, 0, nil)
	}

	return c.m[sourceKey]
}
