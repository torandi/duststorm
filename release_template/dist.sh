#!/bin/bash

root=$(dirname $0)/..
template=$(dirname $0)
release=$(dirname $0)/release
build=$(dirname $0)/build
arch=$(uname -m)

name=""
binary=""
mode=""

# exit on failures
set -e

while getopts "hcafin:b:" opt; do
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
				i)
						mode="info"
						;;
				h)
						echo "usage:"
						echo "  -c            Create new release"
						echo "  -n NAME       Archive name"
						echo "  -b BINARY     Binary name"
						echo ""
						echo "  -a            Add new build to release"
						echo "  -f            Finalize release"
						echo "  -i            Information about release"
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
		
		echo "Creating release/build folder"
		mkdir -p "$release"
		mkdir -p "$build"
		
		echo "Copying template"
		rsync -av $template \
				--exclude .gitignore \
				--exclude dist.sh \
				--exclude release \
				--exclude build \
				--exclude '*.tar' \
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
		local name=$(tar xf $2 .name -O)
		local binary=$(tar xf $2 .binary -O)
		local have_arch="no"

		# test if overwriting existing arch
		if [[ ! -z $(tar -tf $2 | grep .arch) ]]; then
				have_arch="yes"
				local match="no"
				while read -r line; do
						if [[ "$arch" == $line ]]; then
								match="yes"
								break
						fi
				done < <(tar xf $2 .arch -O)

				if [[ $match == "yes" ]]; then
						read -n 1 -p "Archive already contains binaries for $arch, continue? [yN]"
						echo # force newline
						if [[ $REPLY != "y" && $REPLY != "Y" ]]; then
								exit
						fi
				fi
		fi

		echo "Building code"
		pushd $build
		../$root/configure --disable-editor --disable-input --enable-loading FULLSCREEN=1 DATA_PATH=.
		make
		popd
		
		echo "Installing binaries"
		strip $build/$binary
		tar rvf $2 "$build/$binary" --transform="s#./build/#${name}/${arch}/bin/#" --show-transformed

		echo "Installing libraries"
		tar rvf $2 "${root}/vendor/libs/${arch}" --transform="s#vendor/libs/${arch}#${name}/${arch}/libs#" --show-transformed

		# record arch
		if [[ $have_arch == "yes" ]]; then
				tar xf $2 .arch
		fi
		echo "$arch" >> .arch
		tar rvf $2 ".arch"
		rm .arch
}

function finalize(){
		echo $2
		tar vf $2 --delete .name
		tar vf $2 --delete .binary
		tar vf $2 --delete .arch
		gzip $2
		rm -rf $build
		rm -rf $release
}

function info(){
		local name=$(tar xf $2 .name -O)
		local binary=$(tar xf $2 .binary -O)

		echo "Release name: $name"
		echo "Binary name: $binary"
		echo "Installed architectures:"
		while read -r line; do
				echo " * $line"
		done < <(tar xf $2 .arch -O)
}

if [[ -z "$mode" ]]; then
		echo "No mode specified, see -h for usage"
		exit 1
fi

$mode	"$@"
