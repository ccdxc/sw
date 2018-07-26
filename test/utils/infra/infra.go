package infra

import (
	"sync"
)

// Context - created by one binary usually or possibly one thread of a test binary
type Context interface {
	FindRemoteEntity(kind string) []RemoteEntity

	GetAppCatalog(match string) []string
}

// ctx is instantiation of Context interface, used by test cases to fetch infra details
type infraCtx struct {
	res   map[string]*remoteEntity
	mutex sync.Mutex
}

// Config specifies the configuration parameters when initializing the package
type Config struct {
}

// New initializes/returns an instance of the global context
func New(cfg Config) (Context, error) {
	return &infraCtx{}, nil
}

// FindRemoteEntity - finds a remote entity of a specified kind, if none specified it returns all remote entities
func (ctx *infraCtx) FindRemoteEntity(kind string) []RemoteEntity {
	newRe := &remoteEntity{}
	newRes := []RemoteEntity{}
	newRes = append(newRes, newRe)

	return newRes
}

// GetAppCatalog - gets all pre-defined apps from the catalog, if match is non null, it returns only apps that contains provided match value
func (ctx *infraCtx) GetAppCatalog(match string) []string {
	return []string{}
}
