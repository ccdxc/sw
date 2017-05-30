/*
Package bookstore is a auto generated package.
Input file: example.proto
*/

package bookstore

import fmt "fmt"

func (m *Publisher) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "publishers/", m.Name)
}
func (m *Book) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "books/", m.Name)
}
func (m *Order) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "orders/", m.Name)
}
