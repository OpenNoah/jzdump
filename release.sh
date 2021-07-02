#!/bin/bash -ex
base=release/jzdump_0.2.1_upgrade

run()
{
	local type="$1"
	TYPE=$type make clean
	TYPE=$type make -j8
	tar acvf ${base}_${type}.tar.xz upgrade_${type}.bin
}

run np1380
#run np1501
#run np2150
