### Buildroot Development

This prepares the buildroot cross compiler build environment to
build the aarch64 (arm64) cross compiler tool chain.  Most developers
will NOT need to do this.  You might need to run this buildrootdev
environment only if you need to change some options of the tool chain,
for example to customize compiler options, or use custom kernel headers,
or to change commands built into the BusyBox image.  The output of this
build is the buildroot cross compiler tool chain that can be installed
in a developers dev environment.

### Files

The config files included here document all of the (non-default)
configuration options for building the cross compiler tool chain from
buildroot sources.

### User's Guide

Create the environment, connect, then run make to download/build
the tool chain.

```
vagrant up
vagrant ssh
make
```

The build installs the toolchain in /buildroot-* in the vagrant VM,
then packages to tools into the package in /vagrant.  Use this package
to install on a development system/VM.

```
ls -l /vagrant/buildroot-*-aarch64.tar.gz
```

If you want to modify the buildroot configuration, use the buildroot
configuration system.

```
cd buildroot-*
make menuconfig
```

Export the modified buildroot config file.  There are similar
buildroot options to export the BusyBox config or other package configs.

```
make savedefconfig BR2_DEFCONFIG=/vagrant/br-arm64-defconfig
```
