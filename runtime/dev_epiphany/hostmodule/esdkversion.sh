#!/bin/sh
# Quick and dirty script to get the right compiler and linker flags. :-)

if [ `e-gcc --version | grep -c 2015 | cut -c 1` -eq 1 ]; then
	if [ $1 = "cflags" ]; then
		echo 2015
	else
		echo "-le-hal -le-loader"
	fi
else
	if [ $1 = "cflags" ]; then
		echo 2013
	else
		echo "-le-hal"
	fi
fi
