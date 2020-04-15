
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
: ${TEMP_BUILDDIR:="$(uuidgen)"}
: ${DRIVERS_SRC:="$TOP/platform/drivers/windows"}
: ${REMOTE_DRIVERS_SRC:="C:/Builds/$TEMP_BUILDDIR"}
: ${REMOTE_DRIVERS_SRCSTR:="\"$REMOTE_DRIVERS_SRC"\"}
: ${REMOTE_DRIVERS_BUILDSCRIPT:="$REMOTE_DRIVERS_SRC/BuildScript/IonicBuild.ps1"}
: ${REMOTE_DRIVERS_SOLUTION:="$REMOTE_DRIVERS_SRC/Pensando Solution/Pensando Solution.sln"}
: ${REMOTE_DRIVERS_SOLSTR:="\"$REMOTE_DRIVERS_SOLUTION"\"}
: ${BUILD_VM_NAME:="192.168.66.35"}
: ${BUILD_VM_USER:="Administrator"}
: ${BUILD_VM_PW:="pen123!"}

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

# Remove all remote builds older than 1 (one) day on the build VM
# TODO: replace with a Task Scheduler task on the build VM 
sshpass -p $BUILD_VM_PW ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "FORFILES /P C:\\Builds /S /D -1 /C \"cmd /c IF @isdir == TRUE rmdir /s /q @path\""

# Remove remote build source folder in case it exists (probably never since its name is based on a uuid).
sshpass -p $BUILD_VM_PW ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "rmdir " "$REMOTE_DRIVERS_SRCSTR" "/s /q"

# Copy source code to the remote build source folder.
sshpass -p $BUILD_VM_PW scp -o StrictHostKeyChecking=no -pr "$DRIVERS_SRC" "$BUILD_VM_USER"@"$BUILD_VM_NAME":"$REMOTE_DRIVERS_SRC"

# Start remote Build script
if [ -z "$VER" ] ; then
	sshpass -p $BUILD_VM_PW ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "powershell -f" "$REMOTE_DRIVERS_BUILDSCRIPT" "-SolutionNameAndPath" "$REMOTE_DRIVERS_SOLSTR"
else
	sshpass -p $BUILD_VM_PW ssh -o StrictHostKeyChecking=no "$BUILD_VM_USER"@"$BUILD_VM_NAME" "powershell -f" "$REMOTE_DRIVERS_BUILDSCRIPT" "-SolutionNameAndPath" "$REMOTE_DRIVERS_SOLSTR" "-VerStr " "$VER"
fi

if [ 0 -eq $? ] ; then
# Copy remote build artifacts
    sshpass -p $BUILD_VM_PW scp -o StrictHostKeyChecking=no -r "$BUILD_VM_USER"@"$BUILD_VM_NAME":"$GEN_REMOTE_ARTIFACTS_STR" "$GEN_DIR"
fi

# Copy remote build logs
sshpass -p $BUILD_VM_PW scp -o StrictHostKeyChecking=no -pr "$BUILD_VM_USER"@"$BUILD_VM_NAME":"$GEN_REMOTE_BUILDLOGS_STR" "$GEN_DIR"

# Copy package from the ArtifactsZipped folder
cd "$GEN_DIR/.."
cp "$GEN_DIR/ArtifactsZipped/Artifacts.zip" .
mv -f Artifacts.zip "$GEN_PKG_UNSIGNED"

asset-pull windows-driver 1.1.44.0 $GEN_PKG

