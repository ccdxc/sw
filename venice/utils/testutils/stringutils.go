package testutils

import (
	"math/rand"
	"sort"
)

const letterBytes = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

// CreateAlphabetString creates a random string of given size consisting of lowercase or uppercase letters
func CreateAlphabetString(n int) string {
	b := make([]byte, n)
	for i := range b {
		b[i] = letterBytes[rand.Int63()%int64(len(letterBytes))]
	}
	return string(b)
}

// SortErrors sort errors by the error text
func SortErrors(errs []error) {
	sort.Slice(errs, func(i, j int) bool {
		return errs[i].Error() < errs[j].Error()
	})
}
