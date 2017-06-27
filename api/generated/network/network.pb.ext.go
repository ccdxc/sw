/*
Package network is a auto generated package.
Input file: protos/network.proto
*/

package network

import fmt "fmt"

func (m *Tenant) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "tenant/", m.Name)
}
func (m *Network) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "network/", m.Name)
}
func (m *SecurityGroup) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "security-group/", m.Name)
}
func (m *Sgpolicy) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "sgpolicy/", m.Name)
}
func (m *Service) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "service/", m.Name)
}
func (m *LbPolicy) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "lb-policy/", m.Name)
}
func (m *Endpoint) MakeKey(prefix string) string {
	return fmt.Sprint("/venice/", prefix, "/", "endpoint/", m.Name)
}
