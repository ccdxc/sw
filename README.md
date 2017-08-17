# sw


##Developer/Test tools

### Developer's Guide
Follow the instructions at [Developer Guide](docs/dev.md) to create
developer environment on a Mac or Linux machine.

### Modifying Developer Environment
Follow the instructions at [Packer Changes](docs/packer.md) to make
changes to the install time build environments. For other runtime changes
appropriate Vagrantfile, as found in [vagrant-files] directory, would need
to be modified.

## Container Make Tasks

These tasks can be used to replicate the CI environment for this directory:

* `docker-test` will run the unit tests in a container.
* `test-debug` will drop you into a shell in the container.

