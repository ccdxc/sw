package cachedb

// KeyCursor allows iteration through keys in a set of files within a FileStore.
type KeyCursor struct {
	key []byte

	// seeks is all the file locations that we need to return during iteration.
	seeks []*location

	// current is the set of blocks possibly containing the next set of points.
	// Normally this is just one entry, but there may be multiple if points have
	// been overwritten.
	current []*location
	buf     []Value

	// pos is the index within seeks.  Based on ascending, it will increment or
	// decrement through the size of seeks slice.
	pos       int
	ascending bool

	// duplicates is a hint that there are overlapping blocks for this key in
	// multiple files (e.g. points have been overwritten but not fully compacted)
	// If this is true, we need to scan the duplicate blocks and dedup the points
	// as query time until they are compacted.
	duplicates bool
}

// TimeRange holds a min and max timestamp.
type TimeRange struct {
	Min, Max int64
}

type location struct {
}

func (l *location) read() bool {
	return false
}

func (l *location) markRead(min, max int64) {
}

type descLocations []*location

// Sort methods
func (a descLocations) Len() int      { return len(a) }
func (a descLocations) Swap(i, j int) { a[i], a[j] = a[j], a[i] }
func (a descLocations) Less(i, j int) bool {
	return false
}

type ascLocations []*location

// Sort methods
func (a ascLocations) Len() int      { return len(a) }
func (a ascLocations) Swap(i, j int) { a[i], a[j] = a[j], a[i] }
func (a ascLocations) Less(i, j int) bool {
	return false
}

// Close removes all references on the cursor.
func (c *KeyCursor) Close() {
}

// hasOverlappingBlocks returns true if blocks have overlapping time ranges.
// This result is computed once and stored as the "duplicates" field.
func (c *KeyCursor) hasOverlappingBlocks() bool {
	return false
}

// seek positions the cursor at the given time.
func (c *KeyCursor) seek(t int64) {
}

func (c *KeyCursor) seekAscending(t int64) {
}

func (c *KeyCursor) seekDescending(t int64) {
}

// Next moves the cursor to the next position.
// Data should be read by the ReadBlock functions.
func (c *KeyCursor) Next() {
}

func (c *KeyCursor) nextAscending() {
}

func (c *KeyCursor) nextDescending() {
}

func (c *KeyCursor) filterFloatValues(tombstones []TimeRange, values FloatValues) FloatValues {
	return values
}

func (c *KeyCursor) filterIntegerValues(tombstones []TimeRange, values IntegerValues) IntegerValues {
	return values
}

func (c *KeyCursor) filterUnsignedValues(tombstones []TimeRange, values UnsignedValues) UnsignedValues {
	return values
}

func (c *KeyCursor) filterStringValues(tombstones []TimeRange, values StringValues) StringValues {
	return values
}

func (c *KeyCursor) filterBooleanValues(tombstones []TimeRange, values BooleanValues) BooleanValues {
	return values
}
