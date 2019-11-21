## HAL Coding Style (C/C++/protobuf only)

* Except for the protobuf generated files, don't use camel case for anything
  including class names, function names, variable names etc.
* For global variables, use g_ prefix
* For constants (that are not macros), you can use k_ prefix.
* For private variables in classes and structures, use _ as suffix and expose
  get/set methods
* Don't expose a constructor for a class as is, use factory method instead.
  There are several benefits to this approach over using constructor directly:
  * we can use in place constructors in these factory methods where memory is
    coming from slabs instead of heap
  * we can detect memory allocation failures and return NULL, which a
    constructor can't do
  * if class instantiation fails in the middle (some buffers are allocated but
    some other failed), we can back off and unitialize cleanly
* Implement a destroy() method as well for every class
* Don't use malloc()/calloc() anywhere, instead use HAL_MALLOC instead
   * we may endup overloading new operator eventually to track allocs and frees
     to pin point mem leaks
   * same goes with direct usage of new, ideally we should do HAL_MALLOC and
     use inplace constructor instead
   * refer to hal.hpp file
* Use `SDK_ASSERT_XXX()` macros instead of directly using assert()
* For protobuf, use 2 space indentation (not 4 space)
* For C/C++, use 4 space indentation
* No tabs (except probably Makefile)
* No blank spaces at the end of each line
* No unnecessary blank lines in the code
* Don't leave space after function name in C style function declarations
* Leave space after function name in C style function definitions
* Functions that are local to file must be declared static and for static
  functions avoid declarations even within in the .cc file
* Don't expose internal functions in header files by default. This only makes
  module interactions confusing. If a real use case arises, then move the function
  declaration to header files instead of putting all declarations in header
  file.
* Don't use C++ STL or boost etc. in the config or data path unless you are
  absolutely sure that there is no other way to do it. For example, it is ok to
  parse some config files in .json when HAL starts
* Don't use fancy C++ features that come with performance penalty e.g., virtual
  functions etc. in the data path. C like C++ is perfectly fine
* Namespce your code, double namespacing is perfectly fine too !!  e.g.,
  hal::utils, hal::pd etc.
* Make sure each header file can compile fully by itself, i.e., each header
  file should include all header files it depends on. Ensure there are no
  circular dependencies
* please use C++ commenting style instead of C commenting style
* Function declarations
   - return type and rest of the declaration should be on same line
   - no space required after the function name
   - if declaration exceeds 80 chars, next function parameter should
     be in next line aligned with 1st parameter
   - if function doesn't take any args, use 'void' explicity
* Function definitions
  - C++ class methods don't need space between function name and arguments
  - C++ class methods needs { in same line as last parameter
  - C function definitions need space between function name and arguments
  - C function definitions need { in separate line
  - return type must be in separate line
  - if parameters are spread across multiple lines, parameters from 2nd line
    onwards must be aligned with 1st parameter (not 4 space indentantion from
    start of the next line)
* "if" block must have 4 space indentation and must always be inside curly
  braces (even if there is only one statement in the block)
* if-else must always be cuddled (e.g., } else { in same line)
* switch & case in switch-case statement should be aligned in same column


## Building and making sure HAL build is fine
1. Under sw/, do "make pull-assets"
2. Under sw/nic, do "make v=1" and this should build everything needed for HAL
3. To ensure all is well, under sw/nic, do "./run.py"

## Container/CI tasks

### Bootstrapping your development environment

This must be done on a host that does not have docker installed already. You
must have root access.

If you already have a version of docker installed, and you need help using the
development tasks below, please contact <erikh@pensando.io>.

* `curl -sSL https://get.docker.com | sudo bash`
* `gpasswd -aG docker <your username>`
* edit `/etc/docker/daemon.json` to have the following content:

```json
{
    "insecure-registries" : ["srv1.pensando.io:5000", "registry.test.pensando.io:5000"],
    "ipv6": true,
    "fixed-cidr-v6": "2001:db8:1::/64"
}
```

* restart docker (`sudo systemctl restart docker` usually)

### Doing development in the CI/Container environment

**NOTE**: There are several tasks in the
[Makefile](https://github.com/pensando/sw/blob/master/nic/Makefile) which you
can read for additional information should this documentation be lacking.

To get started, you can `make shell`. The first time you do this it will:

* pull a large docker image
* install the [box](https://box-builder.github.io/box) tool (and require root
  to install it if it doesn't exist)
* build a temporary image for you to use and run a shell inside of it

Note that in any situation, your directories are bind mounted (somewhat like VM
file sharing) into the container so you are able to work both inside and
outside of the container. e.g., you can vim or emacs or whatever you prefer
outside the container and your changes will be reflected inside too. Likewise
in the opposite direction.

Because of this, and the UID differences between inside and outside of the
container, you may run into permission issues with your build artifacts
specifically.

You can run `make clean-docker` to run the standard `make clean` process in a
container so it can be removed safely.

If you just want to test the build, or the gtest suite, `make build` and `make
test` do that respectively. Your latest changes will reflect in the container.

### If you need to change the dependencies image

The "big image" mentioned in the previous section is a large image built from
dependencies that are required to build things in the `nic/` and `dol/`
directories.

It is very important that you exercise caution in testing, before you push the
final release image. The following instructions allow you to do the testing.

Note that you will need 10-20GB of storage to build this image successfully.
The image builds fine with just the makefiles and docker in any VM or bare
metal host.

The regular make tasks, for ease of use in general cases automatically pull the
latest dependencies image. However, if you are *building* the image this pull
will overwrite your changes. Therefore the tasks below were created to ease up
this problem a little bit. None of the below tasks invoke `docker pull` at all.

These make targets were created for testing:

* `make deps` - makes a *test* dependencies image named `srv1.pensando.io/pensando/nic:dependencies`
* `make deps-test-shell` - just like `make shell`, but tests against your freshly-built image.
* `make deps-release` - makes a *release* dependencies image tagged with the
  same name.  This image is flattened to reduce size.
* `make deps-test-build` - just like `make build`.

Once you're done testing, and you have a **RELEASE** image (replace 'x' below with the corresponding version):

```
$ docker push registry.test.pensando.io:5000/pensando/nic:1.<x>
```

This will push the image to the image repository. It will make it immediately
available for use.
