#!/bin/bash

root=$(dirname $0)/..
template=$(dirname $0)
release=$(dirname $0)/release
build=$(dirname $0)/build
build=$(dirname $0)/temp
arch=$(uname -p)

name=""
binary=""
mode=""

while getopts "hcafn:b:" opt; do
		case $opt in
				n)
						name="$OPTARG"
						;;
				b)
						binary="$OPTARG"
						;;
				c)
						mode="create"
						;;
				a)
						mode="add"
						;;
				f)
						mode="finalize"
						;;
				h)
						echo "usage:"
						echo "  -c            Create new release"
						echo "  -n NAME       Archive name"
						echo "  -b BINARY     Binary name"
						echo ""
						echo "  -a            Add new build to release"
						echo "  -f            Finalize release"
						exit
						;;
		esac
done

function create(){
		if [[ -z "$name" || -z "$binary" ]]; then
				echo "not all arguments set, use -h"
				exit 1
		fi
		
		if [[ -e "$release" ]]; then
				echo "previous release already exists, please remove it"
				exit 1
		fi
		
		echo "Creating release/temp folder"
		mkdir "$release"
		mkdir -p "$build"
		mkdir -p "$temp"
		
		echo "Copying template"
		rsync -av $template \
				--exclude dist.sh \
				--exclude release \
				--exclude build \
				--exclude temp \
				--exclude '*.tar.gz' \
				--exclude '*~' \
				$release

		echo "Frobnicating scripts"
		sed -i "s/BINARY/$binary/g" $release/run.sh

		echo "Creating tarball"
		echo $name > .name
		echo $binary > .binary
		tar cvf "$name-frobnicators.tar" .name .binary "$release" --transform="s#$release#$name#" --show-transformed
		rm .name
		rm .binary

		echo "All done, now use -a to add binaries"
}

function add(){
		name=$(tar xf $2 .name -O)
		binary=$(tar xf $2 .binary -O)

		echo "Building code"
		pushd $build
		../$root/configure --disable-editor --disable-input FULLSCREEN=1 DATA_PATH=.
		make
		popd
		
		echo "Installing binaries"
		strip $build/$binary
		tar rvf $2 "$build/$binary" --transform="s#./temp/#${name}/${arch}/bin/#" --show-transformed
}

function finalize(){
		echo $2
		tar vf $2 --delete .name
		tar vf $2 --delete .binary
		gzip $2
}

if [[ -z "$mode" ]]; then
		echo "No mode specified, see -h for usage"
		exit 1
fi

$mode	$*
