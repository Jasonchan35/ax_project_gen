#!/bin/sh
#stop when error
set -e
#change current directory to this file
cd `dirname $0`
echo pwd = `pwd`

../../bin/ax_gen ws=Hello.axworkspace -gen
