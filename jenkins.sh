#!/bin/bash
[ -e $HOME/gobuilds/tools/jenkins-env.sh ] && source $HOME/gobuilds/tools/jenkins-env.sh
[ -z $CLIBS_HOME ] && export CLIBS_HOME=$HOME/moovweb/clibs
[ ! -d $CLIBS_HOME ] && mkdir -p $CLIBS_HOME

git pull upstream master
git push origin master
make install PREFIX=$CLIBS_HOME
