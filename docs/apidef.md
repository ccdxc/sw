# Overview of API Handling

                     +--------------+             +--------------+       +----------+          +-------------+
	REST APIs        |              |  gRPC       |              |       |          |          |             |
     <-------------> |    API GW    |<----------->|  API Server  |<----->| KV Store |<-------->| Controller  |
                     |              |             |              |       |          |          |             |
                     +--------------+             +--------------+       +----------+          +-------------+

The diagram above shows the different layers involved in the handling of API calls. 

In a typical API request the REST call is handled by the API gateway and routed to the appropriate API server via a gRPC call. The API server processes the request by performing any transformations required on the request and then performs a KV store action on the resource, like creating, updating, deleting or retrieving the resource. The domain specific controller (like the network controller, the storage controller etc.) would typically watch for KV store updates and act on those notifications. 

The write up below walks through the definitions of the API endpoints and code generation that generates code to plumb the API call from the API Gateway till KV store operation performed by the API server. This writeup does not cover the actions on the controller.
	
# Defining API and KV store objects
KV store objects and API endpoints are defined in a protobuf definition file. This document will walk through defining API endpoints, request and response structures and any objects that are to be saved to KV store.  

## Workflow steps
1. Create the directory where the service will reside, and copy the Makefile (from sw/utils/apigen/example/) to the directory
2. Create a service.proto as explained below.
3. Execute make, which generates a directory structure and needed code for the API gateway and the API server.
4. Optionally add any hooks needed in the impl/ directory. ServiceHooks object definition is explained later.
5. Include the service in the sw/apiserver/cmd/services.go and sw/apigw/cmd/backends.go
6. Celebrate!

## Protobuf specification
A protobuf specification is made up of
1. Message definitions
2. Service definitions
3. Method definitions under the Service  
  
For a functional example protobuf example please look at sw/utils/apigen/example/service.proto. The example specifies a API for a hypothetical bookstore. That is used as an example in the following sections.

### Message definition
Messages define the object structure. Define a message to define the structure of every unique request or response or an object that needs to be written to the KV store. The messages can be annotated with some predefined options. A later section lists the supported options. Protobuf supports nested structures. The snippet below shows a message definition. 


    message Publisher {
        // Mandatory fields for top level messages. Not needed for nested inner messages
        api.TypeMeta T = 1  [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = ",inline"];
        api.ObjectMeta O = 2  [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = "metadata,inline"];

        // Option to define writing to KV store and the key structure.
        option (venice.objectPrefix) = "publishers";

        PublisherSpec Spec = 3;
    }

    message PublisherSpec {
        string Name = 1;
        string Id  = 2;
        string Address = 3;
    }

In the above definition the TypeMeta and ObjectMeta are embedded messages that are mandatory for top level objects. No customization is needed and one could just copy paste these 2 definitions.

The option specified on the next line (venice.objectPrefix) is a custom pensando option. This option controls the path for the object when it is written to KV store. In this case the path would be /<version>/venice/<service prefix>/publishers/<Name from the Object Meta>. Version and "service prefix" are defined in the Service definition covered next.

The objectPrefix definition can be more complicated for example it could have been something like "tenant-{Tenant}/publishers" in which case the key might have been "/v1/venice/bookstore/tenant-Amazon/publishers/Oriely" for example. Here the tenant would have been from the ObjectMeta. Anything specified within "{..}" is parsed as a field from the object and it is filled in at runtime. The field could be in ObjectMeta or in message itself. 

All objects that need to be written to KV store need a "venice.objectPrefix" definition.

### Service definition
A Service is a collection of a group of API endpoints of same version. Multiple Services could exist when there are different versions of the API in use. Each service contains one or more methods which corresponds to the API endpoints of the service. Below is a snippet of a Service definition along with relevant message definitions for context.


    message Order {
      api.TypeMeta T = 1  [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = ",inline"];
      api.ObjectMeta O = 2  [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = "metadata,inline"];

      option (venice.objectPrefix) = "orders";

      // Spec is not allowed to be Null hence disabling nullable.
      OrderSpec Spec = 3 [(gogoproto.nullable) = false];
      OrderStatus Status = 4;
    }

    message OrderSpec {
        string Id = 1;
        repeated OrderItem Order = 2;
    }

    message OrderItem {
        string ISBNId = 1;
        uint32 Quantity = 2;
    }

    message OrderStatus {
        string Status = 1;
        repeated OrderSpec Filled = 2;
    }

    service BookstoreV1 {
        // ---- Service level parameters ---- //    
        // Prefix for all resources served by this service.
        option (venice.apiPrefix) = "bookstore";
        // API Version.
        option (venice.apiVersion) = "v1";

        // --- Publisher resource --- //
        // This is not exposed to the end user and hence there are only gRPC endpoints.
        rpc AddPublisher(Publisher) returns(Publisher) {
            // This option is specified to define the flavor of the KV operation needed.
	    // grpc only methods can specifiy one method_oper per method. oper include (create, update, delete, get)
            option (venice.methodOper) = "create";
        }

        // --- Order Resource--- //
        // This resource is exposed to the user for all CRUD operations
        rpc OrderOper(Order) returns(Order) {
            option (google.api.http) = {
            // Create
            post: "/orders"
            body: "*"
            additional_bindings: {
                // Update
                put: "/orders/{Spec.Id}"
            }
            additional_bindings: {
                // Delete
                delete: "/orders/{Spec.Id}"
            }
            additional_bindings: {
                //Read
                get: "/orders/{Spec.Id}"
            }
            };
        }
    }

When a API is exposed externally via REST by the API Gateway, the allowed methods are specified via the (google.api.http) option. The URI for the resource is specified in the option. Any parameters in URL that map to the request object are specified inside "{..}" like the Spec.Id specified in the OrderOper PUT operation for example. This means that anything specified in the URI overwrites the Id field in the OrderSpec.

All REST endpoints are exposed by the API gateway and all defined APIs (REST and gRPC only) are exposed by the API server.

All pensando specific options can be found in utils/apigen/annotations/penext.proto

## Code Generation
Three code generation plugins are invoked by the Makefile. 
    1. protobuf
    2. gateway and API server plugins
    3. Swagger definition

### Protobuf code generation
This is the unaltered protobuf code generation. This generates all the go structures and base gRPC server and client transport definition.

### Gateway and API server Plugins
This code generation step, generates plugins that plug into the API Gateway and the API Server. The directory structure is shown below.

	├── Makefile
	├── generated
	│   ├── doc.go
	│   ├── endpoint.go
	│   ├── gateway // Code related to the API Gateway
	│   │   ├── gateway.go
	│   │   └── rice-box.go
	│   ├── grpc 
	│   │   ├── client // Instrumented gRPC client code.
	│   │   │   └── client.go
	│   │   └── server // Code that plugs in to the API server.
	│   │       └── server.go
	│   ├── service.pb.ext.go
	│   ├── service.pb.go
	│   ├── service.pb.gw.go
	│   ├── services.go
	│   ├── swagger // Swagger Spec
	│   │   └── service.swagger.json
	│   └── transport.go
	├── impl // All handwritten code for hooks go here.
	└── service.proto.

All generated code is in the generated/ directory.

All hand edited code goes in the impl/ directory. The below code is the bare skeleton needed in a .go file in this directory. If any hooks are to be registered, then the hook code and registration goes here.
    package impl

    import (
        "context"
        "github.com/pensando/sw/apiserver"
        "github.com/pensando/sw/utils/log"
    )

    func NewBookstoreV1Hooks() apiserver.ServiceHooks {
        return &bookstoreHooks{}
    }

    type bookstoreHooks struct {}

    func (r *bookstoreHooks) RegisterHooks(logger log.Logger, svc apiserver.Service) {}

## Controlling API server handling of requests with Hooks
The API server allows the registering of the following hooks. All hooks are registered via hand written code placed in the impl/ directory. Please look at at sw/utils/apigen/example/impl/ for a reference implementation.

### Message hooks
#### Version Transform
Messages can be transformed from the request version to the native version of the API server and vice-versa by registered TransformFuncs. Transformation functions are registered by specifying the from and to versions. Each registration is unidirectional. For example if a service creates a new version of the APi "v2" due to an incompatible change ("v1" being the original version), the service registers a transform function for ("v1" -> "v2") and a transform function for ("v2" -> "v1").

#### Defaulting 
If non stanadard defaults are to be applied then the a Defaulter function registered can mutate the object as desired.

#### Validation Function
If semantic validations are desired then a validation hook can be registered. The Validate function returns nil when validation passes and an error when validation fails. If message validation fails the API request fails.

### Method hooks
#### Pre Commit hook
This is invoked before the KV store operation is performed. The Hook can perform any operations (for example allocating IDs etc) needed before updating KV store, modify the request object and/or signal the API server to not perform the KV store operation.

#### Post Commit Hook
This is invoked by the API server after the KV store operation is performed. 

#### Response writer
Hook to modify the returned response. The hook can return a completely new type in the response if needed.

The snippet below shows an example of registering a post commit hook to a method (please see the impl/server.go for the functional example)


    func (r *bookstoreHooks) RegisterHooks(logger log.Logger, svc apiserver.Service) {
        ...
        svc.GetMethod("DeleteBook").WithPostCommitHook(r.processDelBook)
    }

## Add the service to the API server and API Gateway
The generated code plugs into the API server and API gateway. To facilitate this the service needs to be added to the API server and API gateway imports. For the API Gateway this happens in the apigw/cmd/services.go.

    import (
        _ "github.com/pensando/sw/utils/apigen/example/grpc/server"
    )

For the API Server this happens vie the apiserver/cmd/backends.go

    import (
        _ "github.com/pensando/sw/utils/apigen/example/gateway"
    )

With this the APIs defined in the protobuf should be functional and API call should should be able to perform CRUD operations on the KV store objects. Service controllers would typically watch the config objects in the KV store and take business logic actions.
