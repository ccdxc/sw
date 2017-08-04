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
1. create a protobuf definition file in pensando/sw/api/protos directory.
2. Execute make in the pensando/sw/api directory.
    (__Fun Fact__: Make will generate all needed code and plug it into the API Gateway and API Server. The code is generated in pensando/sw/api/generated/{pkgname} directory where {pkgname} is the package name specified in the .proto file).
3. (Optional) Add any hooks needed in the pensando/sw/api/hooks directory. Registering of hooks is explained later.
4. Celebrate!

## Protobuf specification
A protobuf specification is made up of
1. Message definitions
2. Service definitions
3. Method definitions under the Service  
  
For a functional example protobuf example please look at sw/api/protos/example.proto. The example specifies an API definition for a hypothetical bookstore. That is used as an example in the following sections.

### Message definition
Messages define the object structure. A message defines the structure of every unique request or response or an object that needs to be written to the KV store. Messages can be annotated with some predefined options. A later section lists the supported options. Protobuf supports nested structures. The snippet below shows a message definition. 


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

The option specified on the next line (venice.objectPrefix) is a custom pensando option. This option controls the path for the object when it is written to KV store. In this case the path would be /<version>/venice/<service prefix>/publishers/<Name from the Object Meta>. Version and "service prefix" used above are defined in the service definition covered next.

The objectPrefix definition can be more complicated for example it could have been something like "tenant-{Tenant}/publishers" in which case the key might have been "/v1/venice/bookstore/tenant-Amazon/publishers/Oriely" for example. Here the tenant would have been from the ObjectMeta. Anything specified within "{..}" is parsed as a field from the object and it is filled in at runtime. The field could be in ObjectMeta or in the message itself. 

All objects that need to be written to KV store need a "venice.objectPrefix" definition.

### Service definition
A Service is a collection of a group of API endpoints of the same version. Multiple Services could exist in the same proto file for example when there are different versions of the API in use. Each service contains one or more methods which corresponds to the API endpoints of the service. Below is a snippet of a Service definition along with relevant message definitions for context.


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

        // The apiGrpcCrudService Option generates CRUD + List, Watch APIs for the specified message.
        option (venice.apiGrpcCrudService) = "Order";
        option (venice.apiGrpcCrudService) = "Book";
        option (venice.apiGrpcCrudService) = "Publisher";

        // REST resources exposed by the service. The Option specifies
        //   Object  - The resource
        //   Method  - REST methods allowed on the resource (post, put, get, delete, list)
        //   Pattern - Pattern for the URI
        option (venice.apiRestService) = {
            Object: "Order"
            Method: [ "put", "get", "delete" ]
            Pattern: "/orders/{O.Name}"
        };
        option (venice.apiRestService) = {
            Object: "Order"
            Method: [ "post", "list" ]
            Pattern: "/orders"
        };
    }

Resources can be exposed for CRUD operations by the service either via gRPC and optionally via REST.

Using the "venice.apiGrpcCrudService" option with a message name as a parameter instructs the code generator to generate CRUD apis (Create, Read, Update, Delete, List, Watch). These APIs can then be used via the generated GRPC client code to operate on the objects.

The "venice.apiRestService" options specifies that the API Gateway expose the resource via REST. The option specifies the Object, the methods allowed and the pattern for the URI where the object is to be exposed. Any parameters in URL that map to the request object are specified inside "{..}" like the {O.Name} specified in the example above "put, get, delete" operation for example. This means that anything specified in the URI overwrites the Name filed in the object meta field in the OrderSpec.

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


        ├── generated
        │   ├── bookstore // --- One directory per package defined. --//
        │   │   ├── example.pb.ext.go
        │   │   ├── example.pb.go
        │   │   ├── example.pb.gw.go
        │   │   ├── example_doc.go
        │   │   ├── example_endpoints.go
        │   │   ├── example_services.go
        │   │   ├── example_transport.go
        │   │   ├── gateway //-- Gets plugged into the the API Gateway --//
        │   │   │   ├── example_gateway.go
        │   │   │   └── rice-box.go
        │   │   ├── grpc
        │   │   │   ├── client //-- gRPC client bindings that can be used by clients --//
        │   │   │   │   └── example_client.go
        │   │   │   └── server //--- Gets plugged into the API server --//
        │   │   │       └── example_server.go
        │   │   └── swagger //-- Swagger definition exported into the API Gateway and served from the API Gateway --//
        │   │       └── example.swagger.json
        │   ├── exports //-- imports to facilitate API Server and API Gateway importing generated code. --//
        │   │   ├── apigw
        │   │   │   └── services.go
        │   │   └── apiserver
        │   │       └── backends.go
        │   ├── manifest
        ├── hooks //-- All handwritten Hooks go in this directory --//
        │   └── bookstore.go
        ├── protos //-- Protobuf definitions go in this directory --//


All generated code is in the generated/ directory.

All hand edited code goes in the hooks/ directory. This is an optional step. When the service needs to influence the API server handling of specific APIs via plugins, the service owner can handwrite hooks and register them with the API server. 

## Controlling API server handling of requests with Hooks
The API server allows the registering of the following hooks. All hooks are registered via hand written code placed in the hooks/ directory. Please look at sw/api/hooks/bookstore.go for a reference implementation.

### Message hooks
#### Version Transform
Messages can be transformed from the request version to the native version of the API server and vice-versa by registered TransformFuncs. Transformation functions are registered by specifying the from and to versions. Each registration is unidirectional. For example if a service creates a new version of the API, "v2" due to an incompatible change ("v1" being the original version), the service registers a transform function for ("v1" -> "v2") and a transform function for ("v2" -> "v1").

#### Defaulting 
If non-standard defaults are to be applied then a Defaulter function registered can mutate the object as desired.

#### Validation Function
If semantic validations are desired then a validation hook can be registered. The Validate function returns nil when validation passes and an error when validation fails. If message validation fails the API request fails.

### Method hooks
#### Pre Commit hook
This is invoked before the KV store operation is performed. The Hook can perform any operations (for example allocating IDs etc) needed before updating KV store, modify the request object and/or signal the API server to not perform the KV store operation.

#### Post Commit Hook
This is invoked by the API server after the KV store operation is performed. 

#### Response writer
Hook to modify the returned response. The hook can return a completely new type in the response if needed.

The snippet below shows an example of registering a post commit hook to a method (please see the hooks/bookstore.go for a reference example)


    func (r *bookstoreHooks) RegisterHooks(logger log.Logger, svc apiserver.Service) {
        ...
        svc.GetMethod("DeleteBook").WithPostCommitHook(r.processDelBook)
    }


With this the APIs defined in the protobuf should be functional and API call should should be able to perform CRUD operations on the KV store objects. Service controllers would typically watch the config objects in the KV store and take business logic actions.

#Using the generated GRPC client
The code generator generates a API server client with with CRUD apis for objects tagged with the "venice.apiCrudService" and explicit gRPC endpoints specified in the definition. The snippet below shows a sample CRUD operation on a auto generated object.

        apicl, err := cache.NewGrpcUpstream(url, l, grpc.WithInsecure(), grpc.WithTimeout(time.Second))
        ..
        ret, err := apicl.BookstoreV1().Publisher().Create(ctx, pub)
        ..
        meta := api.ObjectMeta{Name: "Sahara"}
        ret, err = apicl.BookstoreV1().Publisher().Get(ctx, meta)
        ..
        opts := api.ListWatchOptions{}
        pubs, err := apicl.BookstoreV1().Publisher().List(ctx, opts)