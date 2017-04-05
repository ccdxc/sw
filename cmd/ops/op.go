package ops

import (
	"encoding/json"
	"net/http"
	"sync"

	log "github.com/Sirupsen/logrus"
)

var (
	mutex sync.Mutex
)

// Op is a interface to model all cluster operations.
type Op interface {
	// Validate validates the inputs for an operation.
	Validate() error

	// Run executes the operation and returns the result or error.
	Run() (interface{}, error)
}

// Run executes a given operation.
func Run(op Op) (interface{}, error) {
	mutex.Lock()
	defer mutex.Unlock()
	if err := op.Validate(); err != nil {
		return nil, err
	}
	return op.Run()
}

// RunHTTP executes a given operation and writes the result to the client.
func RunHTTP(w http.ResponseWriter, op Op) {
	result, err := Run(op)

	encoder := json.NewEncoder(w)
	if err != nil {
		if err := encoder.Encode(err); err != nil {
			log.Errorf("Failed to encode with error: %v", err)
		}
	} else {
		if err := encoder.Encode(result); err != nil {
			log.Errorf("Failed to encode with error: %v", err)
		}
	}
}
