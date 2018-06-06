The Delphi objects are wrappers around the protobuffer messages. To keep things
simple and in a single file, we are using the original go protbuffer compiler
and add some functionality specific to Delphi.

The main change is that we append and underscore in all the original protobuf
messages. So if the user would define a message called "InterfaceSpec", the
protobuf message type would be "InterfaceSpec_", and the Delphi wrapper around
it would be "InterfaceSpec". This happens in the generator/DelphiRename() 
function.

The reasons for doing this dual:

1. We can prevent the user from having direct access to the field, and enfornce
all access and mutation throught Get/Set functions.

2. We can add extra state, such as mutexes, handle to the client, etc.

The main code changes are under the "//Delphi change start here" comment in the
generator file.

