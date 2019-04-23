# Apollo Tests

All apollo tests reside in nic/apollo/test

## Directory layout

Each test module has its own directory implementing the test cases for that
module. The test module names are used as the directory names.

There are other misc directories which deal with utilities and tools.
The utils/ directory contains common utility routines which are used across
all test modules and are available through the utils library.
The tools/ directory contains the scripts that are used to run the gtests for
apollo

- [x] Test Modules
```
golden     - Handles basic pipeline tests
framework  - Handles framework tests
device     - Handles device tests
vpc        - Handles VPC tests
subnet     - Handles subnet tests
vnic       - Handles VNIC tests
tep        - Handles TEP tests
mapping    - Handles mapping tests
scale      - Handles scale tests
flow_test  - Handles flow tests
```
- [x] Misc
```
utils      - Utility library
tools      - Test tools
```

## File format

### Testcase file format

Each test module directory has a test cases file that implements the test cases
for that module. The test file is typically formatted in the following blocks.

- [x] Test class for the module
- [x] Test cases
- [x] Other stuff including main()

### Utility file format

Typically, each test module will have a .cc and .hpp for the utility routines
that it provides. The name of the files follow the same module name.

The utility header file (.hpp file) for each test module declares the routines
under a module_util class. The class doesn't solve much purpose as
there is no state associated. It just provides a namespace separation for
medthods defined under that class.

```c++
class module_util {
public:
    static sdk_ret_t func-name(arg-type arg..);
    [more utility function declarations..]
};
```
There are some misc files which implements the base class for the tests.

### Doxygen

Doxygen modelled comments are expected in the following places.

- [x] Test cases inside module/main.cc

      1. A doxygen group defined to group test cases so that we can generate
         test cases per module
      2. Brief description of the test case
      3. Detailed description of the test case

```c++
/// \defgroup MODULE_TEST
/// @{

/// \brief Brief description of the test case
///
/// Detailed description of the test case
TEST_F(module, long_function_name_describing_test_case)
{
}

[more test case definitions as above..]
/// @}
```

- [x] Utility class inside the utilty/module.hpp file

      1. Doxygen header for the class
      2. Doxygen comments for each method as shown in sample below

```c++
/// MODULE test utility class
class module_util {
public:
    /// \brief Brief description of the utility function
    ///
    /// \param <arg-name> Argument description
    ///  [more params..]
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t func-name(arg-type arg..);
};
```
