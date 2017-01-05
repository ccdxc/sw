package api

// TypeMeta contains the metadata about kind and version for all API objects.
type TypeMeta struct {
	// Kind represents the type of the API object.
	Kind string `json:"kind,omitempty"`

	// APIVersion defines the version of the API object.
	APIVersion string `json:"apiVersion,omitempty"`
}

// ListMeta contains the metadata for list of objects.
type ListMeta struct {
	// Resource version of object store at the time of list generation.
	ResourceVersion string `json:"resourceVersion,omitempty"`
}

// ObjectMeta contains metadata that all objects stored in kvstore must have.
type ObjectMeta struct {
	// Name of the object, unique within a Namespace for scoped objects.
	Name string `json:"name,omitempty"`

	// Namespace of the object, for scoped objects.
	Namespace string `json:"namespace,omitempty"`

	// Resource version in the object store. This can only be set by the server.
	ResourceVersion string `json:"resourceVersion,omitempty"`

	// TODO: Add timestamps, labels etc.
}

// ObjectRef contains identifying information about an object.
type ObjectRef struct {
	// Kind represents the type of the API object.
	Kind string `json:"kind,omitempty"`

	// Namespace of the object, for scoped objects.
	Namespace string `json:"namespace,omitempty"`

	// Name of the object, unique within a Namespace for scoped objects.
	Name string `json:"name,omitempty"`
}

// StatusResult contains possible statuses for a NIC.
type StatusResult string

const (
	// StatusResultOk indicates that the operation succeeded (http code 200).
	StatusResultOk StatusResult = "Ok"

	// StatusResultExpired indicates that the data being requested has expired
	// (http code 410).
	StatusResultExpired StatusResult = "Expired"

	// StatusResultInternalError indicates that the server had an internal error
	// processing the request (http code 500).
	StatusResultInternalError StatusResult = "Internal Error"
)

// Status is returned for calls that dont return objects.
type Status struct {
	TypeMeta `json:",inline"`

	// Result contains the status of the operation, success or failure.
	Result StatusResult `json:"reason,omitempty"`

	// Message contains human readable form of the error.
	Message string `json:"message,omitempty"`

	// Code is the HTTP status code.
	Code int32 `json:"code,omitempty"`

	// Reference to the object (optional) for which this status is being sent.
	Ref *ObjectRef `json:"ref,omitempty"`
}
