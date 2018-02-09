# sw

### Developer's Guide
Follow the instructions at [Developer Guide](docs/dev.md) to create
developer environment on a Mac or Linux machine.

### Modifying Developer Environment
Follow the instructions at [Packer Changes](docs/packer.md) to make
changes to the install time build environments. For other runtime changes
appropriate Vagrantfile, as found in [vagrant-files] directory, would need
to be modified.

## Most commonly used make targets.

These tasks can be used to replicate the CI environment for this directory:

* `make` Default target which runs everything.
* `ws-tools` This will build all the binaries that we need for code styling and building. Always run this to ensure that you have a consistent toolchain.
* `checks` Runs goimports, lint and vet. These tools will ensure that code meets style guidelines by running formatting and validates source code
* `gen` Runs all auto generated code. Including .pb.go, REST and CRUD handlers, CLI handlers.
* `build` Builds all source code.
* `unit-test-cover` Runs all unit tests in coverage mode. This will fail if the coverage on the package is < 75%
* `cluster` Builds all the binaries, containerizes everything and deploys on kubernetes.
* `make dev` Brings up the standard development vagrant VMs.
* `e2e` runs an end to end cluster tests.

## Guidelines/Sample Workflows/FAQs.
#### I want to import a new package
Follow changes in [vendoring guidelines](https://github.com/pensando/sw/blob/master/docs/vendor.md)

### I have unrelated auto-generated diffs in my PR
This is usually due to inconsistent versions of gen tools. Always run `make ws-tools` and ensure that
all the binaries are from `$GOPATH/bin/`. Avoid doing a `go get -u <package>`. [Vendor](https://github.com/pensando/sw/blob/master/docs/vendor.md) it and add it to `TO_INSTALL` directive in the Makefile. This ensures that everyone will have the same version.

### I need to containerize my binary.
Add this to `TO_DOCKERIZE` directive in the makefile.

### I have a package that I don't want to build.
Add this to `EXCLUDE_PATTERNS` directive in the makefile.

### I need to upgrade CI Base images
We use [jobd](http://jobd) Every top level target has a `box-base.rb` and `.job.yml`
Use `box-base.rb` to declare what environment you need and `.job.yml` to declare what your targets are.
If you're trying to install a new package, avoid `go get`. Instead [vendor](https://github.com/pensando/sw/blob/master/docs/vendor.md) and add it `TO_INSTALL` directive

### I need to version and upgrade nic sanities base images
Run the following
```
1. Make changes in nic/box-deps.rb and in nic/Vagrantfile
2. Upgrade all the version numbers in nic/Makefile, nic/box.rb and any other targets that depend on pensando/nic
3. make -C nic deps-release
4. Push the new container to the registry.test.pensando.io:5000
5. Create a PR and also test locally with a make -C nic shell
```

### I have questions that are not answered here.
Shoot an email to `venice-dev@pensando.io` or reach out to `#sw` slack channel.
