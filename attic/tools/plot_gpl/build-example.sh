#!/bin/bash
# tw

cd src/qtterminal

moc embed_example.h > embed_example.moc.cpp

gcc -o embed_example embed_example.cpp embed_example.moc.cpp \
-lstdc++ -I /usr/include/qt4 -I /usr/include/qt4/QtCore -I /usr/include/qt4/QtGui \
-I /usr/include/qt4/QtSvg -I /usr/include/qt4/QtNetwork -L /usr/lib64/qt4 \
-lQtCore -lQtGui -lQtSvg -lQtNetwork -lstdc++ ./gnuplot-qt.so
