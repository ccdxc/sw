How to define and implement HAL protos and APIs:

- Decide if you need a new service. 
- If its a new service 
    - Create a new .proto file. 
    - Create your APIs in this proto file.
    - Each API takes in a RequestMsg as input and returns a Response. (Look at existing .protos for examples)
    - The .proto will be automatically picked by Makefile in proto/ and generates the .cc files in gen/protobuf.
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

Tags:

HAL protos use gogoprotobuf (https://github.com/gogo/protobuf) that has more
code generation features. HAL uses "moretags" in the proto files to express
more information about individual fields in the objects. More about "moretags"
can be found here - https://github.com/gogo/protobuf/blob/master/extensions.md

Below is the list of all "moretags" used in HAL protos:

1. key fields - all key fields of an object are annotated with [(gogoproto.moretags) = "venice:\"key\""]

2. reference to another object - [(gogoproto.moretags)] = "venice:ref"

3. immutable fields - [(gogoproto.moretags)] = "venice:immutable"
   This is used to indicate that a field can't be changed once object is created

4. mandatory fields - [(gogoproto.moretags)] = "venice:mandatory"
   This is used to indicate that an object can't be created without this field
   being set to some valid value

5. api status field - [(gogoproto.moretags) = "venice:api_status"]
   This is needed for test tool to validate the responses from HAL. Otherwise,
   functionally this is not required

6. field constraints - [(gogoproto.moretags) = "venice:constraints={C1,C2,..}"
   where C1, C2 etc. are the constraints. The supported constraints are listed
   below:

   a. range checks - [(gogoproto.moretags) = "venice:constraints={range:0-255}"
      This is to express that a field can take only values from a given range

   b. depdendency on other fields - [(gogoproto.moretags) = "venice:constraints={<pkg>.<object>.<field> OP <value>"
      where OP is the operation, supported operations are == (equal) and != (not equal) and
      value can be a constant or enum
