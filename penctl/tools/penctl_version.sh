
#!/bin/bash

set -x

# Where am I
TOP=$(readlink -f "$(dirname "$0")/../..")

# Set version string
if [ -n "$SW_VERSION" ] ; then
	PENVER=$SW_VERSION
else
	PENVER=`git describe --tags`
fi
echo $PENVER > $TOP/penctl/version.txt
