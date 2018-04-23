# Delphi Example

This is an example service written using delphi framework.

Here are the description of files:

- **example.proto:** This file contains delphi object definitions for the example
- **example.hpp and example.cc:** These two files contain the reactor class and service class.
    - ExampleService: this is the service object.
    - InterfaceMgr: this is the reactor class for `InterfaceSpec` object.
- **example_main.cc:** This file contains the main function that instantiates the service object.
- **example_test.cc:** This file contains the unit test code to test `InterfaceMgr` reactor class.
