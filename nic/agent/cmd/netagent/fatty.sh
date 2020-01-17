#!/bin/sh
eval `go build -tags iris -work -a 2>&1`
find $WORK -type f -name "*.a" | xargs -I{} du -hxs "{}" | sort -rh > /tmp/dat.out
rawpkgs=$(cat /tmp/dat.out | awk '{print $2}')
for i in $rawpkgs; do
	pkgname=$(grep -R "$i" "$WORK" | cut -d " " -f2 | uniq | cut -d "=" -f1)
	sed -i "s|$i|$pkgname|g" /tmp/dat.out
done
cat /tmp/dat.out | head -n "$1"
