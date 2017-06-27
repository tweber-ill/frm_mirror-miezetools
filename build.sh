#!/bin/bash
#
# Mieze-Tools
# @author Tobias Weber <tobias.weber@tum.de>
# @license GPLv3
#

if [ ! -d tlibs ] || [ ! -f tlibs/AUTHORS ]
then
	echo -e "Error: tlibs not installed. Use ./setup_tlibs.sh"
	exit -1;
fi




echo -e "Prebuilding..."
if ! ./prebuild.sh
then
	echo -e "Error: Prebuild failed";
	exit -1;
fi


echo -e "\nBuilding..."
make -j4 -f themakefile
