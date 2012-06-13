#!/bin/bash
[ -e $HOME/gobuilds/tools/jenkins-env.sh ] && source $HOME/gobuilds/tools/jenkins-env.sh
[ -z $CLIBS_HOME ] && export CLIBS_HOME=$HOME/moovweb/clibs
[ ! -d $CLIBS_HOME ] && mkdir -p $CLIBS_HOME

./configure --prefix=$CLIBS_HOME
make install
