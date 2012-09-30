#!/bin/bash

# frobnicator demo maker ldd test script
#

# exit on failures
set +e

# detect arch
arch=$(uname -p)

# setup directories
root=$(dirname $0)
if [[ $arch =~ i[3456]86 ]]; then
		libdir="${root}/i386/libs"
		bindir="${root}/i386/bin"
elif [[ $arch == "x86_64" ]]; then
		libdir="${root}/x86_64/libs"
		bindir="${root}/x86_64/bin"
else
		echo "unknown arch $arch"
		exit 1
fi

export LD_LIBRARY_PATH="$libdir"
ldd $bindir/duststorm
