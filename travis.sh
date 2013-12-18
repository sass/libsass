#!/bin/bash

set -e

if [ "x$AUTOTOOLS" == "xnewer" ]; then
	sudo add-apt-repository -y ppa:rbose-debianizer/automake &> /dev/null
	sudo apt-get -qq update
	sudo apt-get -qq install automake
	AUTOTOOLS=yes
fi

if [ "x$AUTOTOOLS" == "xyes" ]; then
	autoreconf
	./configure --enable-tests \
	            --with-sassc-dir=$SASS_SASSC_PATH \
	            --with-sass-spec-dir=$SASS_SPEC_PATH
	make
fi

make LOG_FLAGS=--skip VERBOSE=1 test

