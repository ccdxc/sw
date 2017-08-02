How to define Proto

- Decide if you need a new service. 
- If its a new service 
    - Create a new .proto file. 
    - Create your APIs in this proto file.
    - Each API takes in a RequestMsg as input and returns a Response. (Look at existing .protos for examples)
    - The .proto will be automatically picked by Makefile in proto/ and generates the .cc files in gen/protobuf.
      Add <service>.pb.cc and <service>.grpc.pb.cc into Makefile.proto in proto/
    - Register the new service in main.cc in hal/. Look for server_builder.RegisterService(). Follow the existing examples.
    - Create a new .cc and .hpp in hal/svc. In this you will inherit the new service and override the APIs.
      Look at the existing services for examples.
    - Add the new <service>\_svc.cc to the Makefile in hal/
    - From <service>\_svc.cc the code branches into the source in hal/src/
- If its an existing service
    - Change the attributes or add new APIs in the respective .proto file. 
    - Make changes in the respective file  <service>\_svc.cc
    - Branch out to in hal/src and make changes.


Gtests:
- The directory nic/hal/test/gtests/ has the gtests to test Hal. 
- Look at existing tests written for some interface objects.

