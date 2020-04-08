
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
: ${DRIVERS_SRC:="$TOP/platform/drivers/windows"}
: ${REMOTE_DRIVERS_SRC:="C:/CurrentBuild"}
: ${REMOTE_DRIVERS_SRCSTR:="\"$REMOTE_DRIVERS_SRC"\"}
: ${REMOTE_DRIVERS_BUILDSCRIPT:="$REMOTE_DRIVERS_SRC/BuildScript/IonicBuild.ps1"}
: ${REMOTE_DRIVERS_SOLUTION:="$REMOTE_DRIVERS_SRC/Pensando Solution/Pensando Solution.sln"}
: ${REMOTE_DRIVERS_SOLSTR:="\"$REMOTE_DRIVERS_SOLUTION"\"}
: ${BUILD_VM_NAME:="192.168.66.35"}
: ${BUILD_VM_USER:="Administrator"}

# Products generated

: ${GEN_DIR:="$TOP/platform/gen/drivers-windows"}
: ${GEN_PKG:="$GEN_DIR.zip"}
: ${GEN_PKG_UNSIGNED:="$TOP/platform/gen/drivers-windows-unsigned.zip"}
: ${GEN_REMOTE_ARTIFACTS:="$REMOTE_DRIVERS_SRC/Pensando Solution/ArtifactsZipped"}
: ${GEN_REMOTE_BUILDLOGS:="$REMOTE_DRIVERS_SRC/Pensando Solution/BuildLogs"}
: ${GEN_REMOTE_ARTIFACTS_STR:="\"$GEN_REMOTE_ARTIFACTS"\"}
: ${GEN_REMOTE_BUILDLOGS_STR:="\"$GEN_REMOTE_BUILDLOGS"\"}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"
rm -f "$GEN_PKG_UNSIGNED"

 
# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi


sshpass -p pen123! ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "rmdir " "$REMOTE_DRIVERS_SRCSTR" "/s /q"

sshpass -p pen123! scp -o StrictHostKeyChecking=no -pr "$DRIVERS_SRC" "$BUILD_VM_USER"@"$BUILD_VM_NAME":"$REMOTE_DRIVERS_SRC"

if [ -z "$VER" ] ; then
	sshpass -p pen123! ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "powershell -f" "$REMOTE_DRIVERS_BUILDSCRIPT" "-SolutionNameAndPath" "$REMOTE_DRIVERS_SOLSTR"
else
	sshpass -p pen123! ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "powershell -f" "$REMOTE_DRIVERS_BUILDSCRIPT" "-SolutionNameAndPath" "$REMOTE_DRIVERS_SOLSTR" "-VerStr " "$VER"
fi

if [ 0 -eq $? ] ; then
    sshpass -p pen123! scp -o StrictHostKeyChecking=no -r "$BUILD_VM_USER"@"$BUILD_VM_NAME":"$GEN_REMOTE_ARTIFACTS_STR" "$GEN_DIR"
fi

sshpass -p pen123! scp -o StrictHostKeyChecking=no -pr "$BUILD_VM_USER"@"$BUILD_VM_NAME":"$GEN_REMOTE_BUILDLOGS_STR" "$GEN_DIR"


# Copy package from the ArtifactsZipped folder
cd "$GEN_DIR/.."
cp "$GEN_DIR/ArtifactsZipped/Artifacts.zip" .
mv -f Artifacts.zip "$GEN_PKG_UNSIGNED"

asset-pull windows-driver 1.1.38 $GEN_PKG

