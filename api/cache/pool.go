package cache

import "sync"

type connPool struct {
	// total is the number of entries in the pool.
	total int
	// pool is the pool of connections
	pool []interface{}
	// next is the index of the next entry to be used in the pool.
	next int
	// nextMutex protects next
	nextMutex sync.Mutex
}

// AddToPool adds an entry to the pool
func (c *connPool) AddToPool(i interface{}) {
	defer c.nextMutex.Unlock()
	c.nextMutex.Lock()
	c.pool = append(c.pool, i)
	c.total = len(c.pool)
}

// GetFromPool gets an entry from the pool.
func (c *connPool) GetFromPool() interface{} {
	defer c.nextMutex.Unlock()
	c.nextMutex.Lock()
	if c.total == 0 {
		return nil
	}
	c.next = (c.next + 1) % c.total
	return c.pool[c.next]
}

// DelFromPool deletes an item from the pool.
func (c *connPool) DelFromPool(in interface{}) bool {
	defer c.nextMutex.Unlock()
	c.nextMutex.Lock()
	for i := range c.pool {
		if c.pool[i] == in {
			c.pool[i] = c.pool[len(c.pool)-1]
			c.pool = c.pool[:len(c.pool)-1]
			c.total = len(c.pool)
			return true
		}
	}
	return false
}

// DelAll deletes an item from the pool.
func (c *connPool) DelAll() {
	defer c.nextMutex.Unlock()
	c.nextMutex.Lock()
	c.pool = nil
	c.total = 0
}
