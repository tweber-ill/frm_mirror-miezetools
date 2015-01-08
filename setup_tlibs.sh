#!/bin/bash

TLIBS=tlibs-master.tar.bz2

rm ${TLIBS}

if ! wget http://forge.frm2.tum.de/cgit/cgit.cgi/frm2/mira/tlibs.git/snapshot/${TLIBS}
then
	echo -e "Error: Cannot download tlibs.";
	exit -1;
fi

if ! tar -xjvf ${TLIBS}
then
	echo -e "Error: Cannot extract tlibs.";
	exit -1;
fi

mv tlibs-master tlibs
rm ${TLIBS}
