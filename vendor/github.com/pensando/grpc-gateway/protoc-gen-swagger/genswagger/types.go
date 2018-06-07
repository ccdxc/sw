package genswagger

import (
	"bytes"
	"encoding/json"

	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
)

// Finalizer is a list of finalizer callbacks for parts of the spec. THis is called after the spec
//  element is generated. The finalizer then has an oppurtunity to modify the spec element as needed.
type Finalizer struct {
	// Init returns any Messages and Enums to be inserted into the spec.
	Init      func(reg *descriptor.Registry) ([]*descriptor.Message, []*descriptor.Enum)
	Spec      func(obj *SwaggerObject, file *descriptor.File, reg *descriptor.Registry) error
	Method    func(obj *SwaggerPathItemObject, path *string, method *descriptor.Method, reg *descriptor.Registry) error
	Def       func(obj *SwaggerSchemaObject, message *descriptor.Message, reg *descriptor.Registry) error
	Field     func(obj *SwaggerSchemaObject, field *descriptor.Field, reg *descriptor.Registry) error
	FieldName func(field *descriptor.Field) string
}

type param struct {
	*descriptor.File
	reg *descriptor.Registry
}

type binding struct {
	*descriptor.Binding
}

// http://swagger.io/specification/#infoObject
type SwaggerInfoObject struct {
	Title          string `json:"title"`
	Description    string `json:"description,omitempty"`
	TermsOfService string `json:"termsOfService,omitempty"`
	Version        string `json:"version"`

	Contact      *SwaggerContactObject               `json:"contact,omitempty"`
	License      *SwaggerLicenseObject               `json:"license,omitempty"`
	ExternalDocs *SwaggerExternalDocumentationObject `json:"externalDocs,omitempty"`
}

// http://swagger.io/specification/#contactObject
type SwaggerContactObject struct {
	Name  string `json:"name,omitempty"`
	URL   string `json:"url,omitempty"`
	Email string `json:"email,omitempty"`
}

// http://swagger.io/specification/#licenseObject
type SwaggerLicenseObject struct {
	Name string `json:"name,omitempty"`
	URL  string `json:"url,omitempty"`
}

// http://swagger.io/specification/#externalDocumentationObject
type SwaggerExternalDocumentationObject struct {
	Description string `json:"description,omitempty"`
	URL         string `json:"url,omitempty"`
}

// http://swagger.io/specification/#SwaggerObject
type SwaggerObject struct {
	Swagger     string                   `json:"swagger"`
	Info        SwaggerInfoObject        `json:"info"`
	Host        string                   `json:"host,omitempty"`
	BasePath    string                   `json:"basePath,omitempty"`
	Schemes     []string                 `json:"schemes"`
	Consumes    []string                 `json:"consumes"`
	Produces    []string                 `json:"produces"`
	Paths       SwaggerPathsObject       `json:"paths"`
	Definitions SwaggerDefinitionsObject `json:"definitions"`
}

// http://swagger.io/specification/#pathsObject
type SwaggerPathsObject map[string]SwaggerPathItemObject

// http://swagger.io/specification/#pathItemObject
type SwaggerPathItemObject struct {
	Get    *SwaggerOperationObject `json:"get,omitempty"`
	Delete *SwaggerOperationObject `json:"delete,omitempty"`
	Post   *SwaggerOperationObject `json:"post,omitempty"`
	Put    *SwaggerOperationObject `json:"put,omitempty"`
	Patch  *SwaggerOperationObject `json:"patch,omitempty"`
}

// http://swagger.io/specification/#operationObject
type SwaggerOperationObject struct {
	Summary     string                  `json:"summary,omitempty"`
	Description string                  `json:"description,omitempty"`
	OperationID string                  `json:"operationId"`
	Responses   SwaggerResponsesObject  `json:"responses"`
	Parameters  SwaggerParametersObject `json:"parameters,omitempty"`
	Tags        []string                `json:"tags,omitempty"`

	ExternalDocs *SwaggerExternalDocumentationObject `json:"externalDocs,omitempty"`
}

type SwaggerParametersObject []SwaggerParameterObject

// http://swagger.io/specification/#parameterObject
type SwaggerParameterObject struct {
	Name        string              `json:"name"`
	Description string              `json:"description,omitempty"`
	In          string              `json:"in,omitempty"`
	Required    bool                `json:"required"`
	Type        string              `json:"type,omitempty"`
	Format      string              `json:"format,omitempty"`
	Items       *SwaggerItemsObject `json:"items,omitempty"`
	Enum        []string            `json:"enum,omitempty"`
	Default     string              `json:"default,omitempty"`

	// Or you can explicitly refer to another type. If this is defined all
	// other fields should be empty
	Schema *SwaggerSchemaObject `json:"schema,omitempty"`
}

// core part of schema, which is common to itemsObject and schemaObject.
// http://swagger.io/specification/#itemsObject
type SchemaCore struct {
	Type   string `json:"type,omitempty"`
	Format string `json:"format,omitempty"`
	Ref    string `json:"$ref,omitempty"`

	Items *SwaggerItemsObject `json:"items,omitempty"`

	// If the item is an enumeration include a list of all the *NAMES* of the
	// enum values.  I'm not sure how well this will work but assuming all enums
	// start from 0 index it will be great. I don't think that is a good assumption.
	Enum    []string `json:"enum,omitempty"`
	Default string   `json:"default,omitempty"`

	// Extended Properties
	Minimum   int    `json:"minimum,omitempty"`
	Maximum   int    `json:"maximum,omitempty"`
	MinLength int    `json:"minLength,omitempty"`
	MaxLength int    `json:"maxLength,omitempty"`
	Example   string `json:"example,omitempty"`

	// Extenstions
	XUiHints map[string]string `json:"x-ui-hints,omitempty"`
}

type SwaggerItemsObject SchemaCore

// http://swagger.io/specification/#responsesObject
type SwaggerResponsesObject map[string]SwaggerResponseObject

// http://swagger.io/specification/#responseObject
type SwaggerResponseObject struct {
	Description string              `json:"description"`
	Schema      SwaggerSchemaObject `json:"schema"`
}

type KeyVal struct {
	Key   string
	Value interface{}
}

type SwaggerSchemaObjectProperties []KeyVal

func (op SwaggerSchemaObjectProperties) MarshalJSON() ([]byte, error) {
	var buf bytes.Buffer
	buf.WriteString("{")
	for i, kv := range op {
		if i != 0 {
			buf.WriteString(",")
		}
		key, err := json.Marshal(kv.Key)
		if err != nil {
			return nil, err
		}
		buf.Write(key)
		buf.WriteString(":")
		val, err := json.Marshal(kv.Value)
		if err != nil {
			return nil, err
		}
		buf.Write(val)
	}

	buf.WriteString("}")
	return buf.Bytes(), nil
}

// http://swagger.io/specification/#schemaObject
type SwaggerSchemaObject struct {
	SchemaCore
	// Properties can be recursively defined
	Properties           SwaggerSchemaObjectProperties `json:"properties,omitempty"`
	AdditionalProperties *SwaggerSchemaObject          `json:"additionalProperties,omitempty"`

	Description string `json:"description,omitempty"`
	Title       string `json:"title,omitempty"`
}

// http://swagger.io/specification/#referenceObject
type SwaggerReferenceObject struct {
	Ref string `json:"$ref"`
}

// http://swagger.io/specification/#definitionsObject
type SwaggerDefinitionsObject map[string]SwaggerSchemaObject

// Internal type mapping from FQMN to descriptor.Message. Used as a set by the
// findServiceMessages function.
type messageMap map[string]*descriptor.Message

// Internal type mapping from FQEN to descriptor.Enum. Used as a set by the
// findServiceMessages function.
type enumMap map[string]*descriptor.Enum
