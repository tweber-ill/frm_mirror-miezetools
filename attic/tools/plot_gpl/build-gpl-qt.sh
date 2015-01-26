#!/bin/bash
# tw

cd src/qtterminal

uic QtGnuplotSettings.ui > ui_QtGnuplotSettings.h
moc QtGnuplotWidget.h > QtGnuplotWidget.moc.cpp
moc QtGnuplotScene.h > QtGnuplotScene.moc.cpp
moc QtGnuplotEvent.h > QtGnuplotEvent.moc.cpp
moc QtGnuplotInstance.h > QtGnuplotInstance.moc.cpp

#gcc -o embed_example embed_example.cpp QtGnuplotWidget.cpp QtGnuplotInstance.cpp QtGnuplotEvent.cpp \
#QtGnuplotScene.cpp QtGnuplotItems.cpp embed_example.moc.cpp QtGnuplotWidget.moc.cpp \
#QtGnuplotScene.moc.cpp QtGnuplotEvent.moc.cpp QtGnuplotInstance.moc.cpp -lstdc++ -I /usr/include/qt4 \
#-I /usr/include/qt4/QtCore -I /usr/include/qt4/QtGui -I /usr/include/qt4/QtSvg -I \
#/usr/include/qt4/QtNetwork -L /usr/lib64/qt4 -lQtCore -lQtGui -lQtSvg -lQtNetwork -lstdc++


gcc -O2 -march=native -o gnuplot-qt.so -fPIC -shared \
QtGnuplotWidget.cpp QtGnuplotInstance.cpp QtGnuplotEvent.cpp \
QtGnuplotScene.cpp QtGnuplotItems.cpp QtGnuplotWidget.moc.cpp \
QtGnuplotScene.moc.cpp QtGnuplotEvent.moc.cpp QtGnuplotInstance.moc.cpp -lstdc++ \
-I /usr/include/qt4 \
-I /usr/include/qt4/QtCore -I /usr/include/qt4/QtGui -I /usr/include/qt4/QtSvg -I \
/usr/include/qt4/QtNetwork -L /usr/lib64/qt4 -lQtCore -lQtGui -lQtSvg -lQtNetwork -lstdc++
