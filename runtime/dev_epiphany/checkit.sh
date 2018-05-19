#!/bin/sh
#
# MODULE: epiphany
#   PART: script to check if module can be supported
# ACTION: exits with 1 if the module cannot be make'd
#

# Check if EPIPHANY_HOME and e-gcc are known
if [ -z $EPIPHANY_HOME ] || ! command -v e-gcc > /dev/null ; then
	exit 1
fi
