
#!/bin/bash

#
# Generate drivers-esx.tar.xz from the workspace.  The package can be used to do vib install on ESX
#
# Synopsis:
#
# #Generate and copy the package
# platform/tools/drivers-esx.sh
# scp platform/gen/drivers-esx.tar.xz root@some-host:
#
# #Unpack and build on the host
# ssh root@some-host
# tar xaf drivers-esx.tar.xz
# cd drivers-esx
# ./build.sh
#

set -x

# Where am I
TOP=$(readlink -f "$(dirname "$0")/../..")

# Sources for generation
: ${DRIVERS_SRC:="$TOP/platform/drivers/"}

# Products generated

: ${GEN_DIR:="$TOP/platform/gen/drivers-windows"}
: ${GEN_PKG:="$GEN_DIR.zip"}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

 
# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
asset-pull windows-driver 1.0.28 $GEN_PKG
