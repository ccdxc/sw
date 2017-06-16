/*
Package cmd is a auto generated package.
Input file: protos/cmd.proto
*/

package cmd

import fmt "fmt"

func (m *Node) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "nodes/", m.Name)
}
func (m *Cluster) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "cluster/", m.Name)
}
