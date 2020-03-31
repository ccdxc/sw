package vcprobe

// DeltaStrSet is a set of strings that allows easy diff
// against a slice of strings. It is NOT thread-safe.
type DeltaStrSet struct {
	// when diffing, false means deleted, true
	// means added. for set membership, only
	// presence in the map is considered
	items map[string]bool
}

// NewDeltaStrSet creates a DeltaStrSet from the slice
func NewDeltaStrSet(l []string) *DeltaStrSet {
	d := &DeltaStrSet{
		items: make(map[string]bool, len(l)),
	}

	for _, s := range l {
		d.items[s] = true
	}

	return d
}

// Count returns the number of items in the DeltaStrSet
func (d *DeltaStrSet) Count() int {
	return len(d.items)
}

// Add adds new items to the set
func (d *DeltaStrSet) Add(l []string) {
	for _, s := range l {
		d.items[s] = true
	}
}

// AddSingle adds a new item to the set
func (d *DeltaStrSet) AddSingle(l string) {
	d.items[l] = true
}

// RemoveSingle removes an item from the set
func (d *DeltaStrSet) RemoveSingle(l string) {
	if _, ok := d.items[l]; ok {
		delete(d.items, l)
	}
}

// Diff returns the difference set(i.e. l - d).
func (d *DeltaStrSet) Diff(l []string) *DeltaStrSet {
	// clone and initialize values to false for old
	// false means deleted, true means added
	diff := &DeltaStrSet{
		items: make(map[string]bool, len(d.items)),
	}

	for k := range d.items {
		diff.items[k] = false
	}

	// remove items that are in l and d.
	// add items that are only in l with value set to true

	for _, k := range l {
		if _, found := diff.items[k]; found {
			delete(diff.items, k)
		} else {
			diff.items[k] = true
		}
	}

	return diff
}

// Additions returns the set of items marked as added
func (d *DeltaStrSet) Additions() []string {
	a := make([]string, 0, len(d.items))
	for k, v := range d.items {
		if v {
			a = append(a, k)
		}
	}

	return a
}

// Deletions returns the set of items marked as deleted
func (d *DeltaStrSet) Deletions() []string {
	a := make([]string, 0, len(d.items))
	for k, v := range d.items {
		if !v {
			a = append(a, k)
		}
	}

	return a
}

// Items returns the items in the set (adds and deletes)
func (d *DeltaStrSet) Items() []string {
	x := make([]string, 0, len(d.items))
	for k := range d.items {
		x = append(x, k)
	}

	return x
}

// ApplyDiff applies the diff to d.
func (d *DeltaStrSet) ApplyDiff(diff *DeltaStrSet) {
	for k, v := range diff.items {
		if v {
			d.items[k] = true
		} else {
			delete(d.items, k)
		}
	}
}

// MarkAllForDeletion marks all elements as false
func (d *DeltaStrSet) MarkAllForDeletion() {
	for k := range d.items {
		d.items[k] = false
	}
}
