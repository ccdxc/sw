package api

import (
	swapi "github.com/pensando/sw/api"
)

// ObjectRlnType is
type ObjectRlnType int

const (
	// NamedRef is
	NamedRef ObjectRlnType = iota + 1
	// SelectorRef is
	SelectorRef
	// BackRef is
	BackRef
)

// ObjectRln is
type ObjectRln struct {
	// Location is package where an object is defined
	Location string
	// Type is
	Type ObjectRlnType
	// ToObj is
	ToObj string
	// Field is
	Field string
}

// GenObjStruct defines generation attributes
type GenObjStruct struct {
	Name string
}

// ObjectInfo is
type ObjectInfo struct {
	Name        string
	Package     string
	GrpcService string
	URL         string
	Perms       string
	Rlns        []ObjectRln
	Structs     []GenObjStruct
}

// Objs are
var Objs = map[string]ObjectInfo{
	"node": {Name: "node", Package: "cmd", GrpcService: "cmd", URL: "/v1/cmd/nodes", Perms: "rw",
		Structs: []GenObjStruct{{Name: "NodeCondition"}, {Name: "PortCondition"}, {Name: "NodeRole"}, {Name: "NodeConditionType"}, {Name: "ConditionStatus"}, {Name: "SmartNICPhase"}, {Name: "SmartNICConditionType"}, {Name: "PortConditionType"}, {Name: "NodePhase"}}},
	"cluster":       {Name: "cluster", Package: "cmd", GrpcService: "cmd", URL: "/v1/cmd/cluster", Perms: "rw"},
	"tenant":        {Name: "tenant", Package: "network", GrpcService: "tenant", URL: "/v1/tenants/tenants", Perms: "rw"},
	"network":       {Name: "network", Package: "network", GrpcService: "network", URL: "/v1/networks/:tenant/networks", Perms: "rw"},
	"securityGroup": {Name: "securityGroup", Package: "network", GrpcService: "securityGroup", URL: "/v1/security-groups/:tenant/security-groups", Perms: "rw"},
	"sgpolicy": {Name: "sgpolicy", Package: "network", GrpcService: "sgpolicy", URL: "/v1/sgpolicy/:tenant/sgpolicy", Perms: "rw",
		Structs: []GenObjStruct{{Name: "SGRule"}}},
	"service": {Name: "service", Package: "network", GrpcService: "service", URL: "/v1/services/:tenant/services", Perms: "rw"},
	"lbPolicy": {Name: "lbPolicy", Package: "network", GrpcService: "lbPolicy", URL: "/v1/lb-policy/:tenant/lb-policy", Perms: "rw",
		Structs: []GenObjStruct{{Name: "HealthCheckSpec"}}},
	"endpoint": {Name: "endpoint", Package: "network", GrpcService: "endpoint", URL: "/v1/endpoints/:tenant/endpoints", Perms: "rw"},
	"user": {Name: "user", Package: "api", GrpcService: "api", URL: "/user", Perms: "rw",
		Structs: []GenObjStruct{{Name: "UserAuditLog"}},
		Rlns: []ObjectRln{
			{Type: NamedRef, ToObj: "role", Field: "Spec.Roles"}}},
	"role": {Name: "role", Package: "api", GrpcService: "api", URL: "/role", Perms: "rw",
		Rlns: []ObjectRln{
			{Type: NamedRef, ToObj: "permission", Field: "Spec.Permissions"},
			{Type: BackRef, ToObj: "user", Field: "Status.Users"}}},
	"permission": {Name: "permission", Package: "api", GrpcService: "api", URL: "/permission", Perms: "rw",
		Rlns: []ObjectRln{
			{Type: SelectorRef, ToObj: "any", Field: "Spec.MatchLabels"},
			{Type: SelectorRef, ToObj: "any", Field: "Spec.MatchFields"},
			{Type: BackRef, ToObj: "role", Field: "Status.Roles"}}},
}

// ObjectHeader is
type ObjectHeader struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ObjectMeta is
	swapi.ObjectMeta `json:"meta"`
}

// ListHeader is
type ListHeader struct {
	// TypeMeta is
	swapi.TypeMeta `json:"T"`
	// ListMeta is
	swapi.ListMeta `json:"ListMeta"`
	// Items is
	Items []ObjectHeader
}

// TODO: following definitions are expected to be move under api/protos

// User is
type User struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ObjectMeta is
	swapi.ObjectMeta `json:"meta"`
	// Spec is
	Spec UserSpec `json:"spec,omitempty"`
	// Status is
	Status UserStatus `json:"status,omitempty"`
}

// UserSpec is
type UserSpec struct {
	// Roles are
	Roles []string `json:"roles,omitempty"`
}

// UserStatus is
type UserStatus struct {
	// AuditTrail is
	AuditTrail []UserAuditLog `json:"userAuditLog,omitempty"`
}

// UserAuditLog is
type UserAuditLog struct {
	// FromIPAddress is
	FromIPAddress string `json:"fromIpAddress,omitempty"`
	// LoginTime is
	LoginTime string `json:"loginTime,omitempty"`
	// CrudLogs are
	CrudLogs []string `json:"crudLog,omitempty"`
}

// UserList is
type UserList struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ListMeta is
	swapi.ListMeta `json:"meta"`
	// Items is
	Items []User
}

// Role is
type Role struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ObjectMeta is
	swapi.ObjectMeta `json:"meta"`
	// Spec is
	Spec RoleSpec `json:"spec,omitempty"`
	// Status is
	Status RoleStatus `json:"status,omitempty"`
}

// RoleSpec is
type RoleSpec struct {
	// Permissions are
	Permissions []string `json:"permissions,omitempty"`
}

// RoleStatus is
type RoleStatus struct {
	// Users are
	Users []string `json:"users,omitempty"`
}

// RoleList is
type RoleList struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ListMeta is
	swapi.ListMeta `json:"meta"`
	// Items are
	Items []Role `json:"items,omitempty"`
}

// Permission is
type Permission struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ObjectMeta is
	swapi.ObjectMeta `json:"meta"`
	// Spec is
	Spec PermissionSpec `json:"spec,omitempty"`
	// Status is
	Status PermissionStatus `json:"status,omitempty"`
}

// PermissionSpec is
type PermissionSpec struct {
	// Action is
	Action string `json:"action,omitempty"`
	// ObjectSelector map objects kind, to names (names can be regex)
	ObjectSelector map[string]string `json:"objectSelector,omitempty"`
	// ValidUntil is
	ValidUntil string `json:"validUntil,omitempty"`
}

// PermissionStatus is
type PermissionStatus struct {
	// CreationTime is
	CreationTime string `json:"creationTime,omitempty" venice:"sskip"`
	// Roles are
	Roles []string `json:"roles,omitempty"`
	// Users are
	Users []string `json:"users,omitempty"`
}

// PermissionList is
type PermissionList struct {
	// TypeMeta is
	swapi.TypeMeta `json:",inline"`
	// ListMeta is
	swapi.ListMeta `json:"meta"`
	// Items are
	Items []Permission `json:"items,omitempty"`
}
