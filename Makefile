CC = gcc
INC = -I/usr/include/qt4 -I/usr/local/include
FLAGS = ${INC} -O2 -march=native -std=c++11 -DNDEBUG
LAPACK_LIBS = -L/usr/local/lib64 -llapack -llapacke
QT_LIBS = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -lQtCore -lQtGui -lQtXml -lQtXmlPatterns -lQtOpenGL -lGLU
LIBS_RESO = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -lstdc++ -lm ${QT_LIBS} ${LAPACK_LIBS}
LIBS = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -lstdc++ -lm -fopenmp -lMinuit2 -lfftw3 ${QT_LIBS} ${LAPACK_LIBS}


cattus: main.o mainwnd.o settings.o data.o CombineDlg.o ComboDlg.o FitDlg.o ListDlg.o ResoDlg.o RoiDlg.o SettingsDlg.o fourier.o string.o xml.o loadcasc.o loadnicos.o loadtxt.o plotgl.o plot.o plot2d.o plot3d.o plot4d.o roi.o cn.o pop.o chi2.o fitter.o functions.o parser.o freefit.o freefit-nd.o gauss.o gauss-nd.o msin.o interpolation.o ellipse.o
	${CC} ${FLAGS} -o cattus main.o mainwnd.o settings.o data.o CombineDlg.o ComboDlg.o FitDlg.o ListDlg.o ResoDlg.o RoiDlg.o SettingsDlg.o fourier.o string.o xml.o loadcasc.o loadnicos.o loadtxt.o plotgl.o plot.o plot2d.o plot3d.o plot4d.o roi.o cn.o pop.o chi2.o fitter.o functions.o parser.o freefit.o freefit-nd.o gauss.o gauss-nd.o msin.o interpolation.o ellipse.o ${LIBS}
	strip cattus

reso: settings.o data.o ResoDlg_prog.o string.o xml.o plot.o cn.o pop.o ellipse.o roi.o plotgl.o
	${CC} ${FLAGS} -o reso settings.o data.o ResoDlg_prog.o string.o xml.o plot.o cn.o pop.o ellipse.o roi.o plotgl.o ${LIBS_RESO}
	strip reso


main.o: main.cpp
	${CC} ${FLAGS} -c -o main.o main.cpp

mainwnd.o: mainwnd.cpp mainwnd.h
	${CC} ${FLAGS} -c -o mainwnd.o mainwnd.cpp

settings.o: settings.cpp settings.h
	${CC} ${FLAGS} -c -o settings.o settings.cpp



data.o: data/data.cpp data/data.h
	${CC} ${FLAGS} -c -o data.o data/data.cpp



CombineDlg.o: dialogs/CombineDlg.cpp dialogs/CombineDlg.h
	${CC} ${FLAGS} -c -o CombineDlg.o dialogs/CombineDlg.cpp

ComboDlg.o: dialogs/ComboDlg.cpp dialogs/ComboDlg.h
	${CC} ${FLAGS} -c -o ComboDlg.o dialogs/ComboDlg.cpp

FitDlg.o: dialogs/FitDlg.cpp dialogs/FitDlg.h
	${CC} ${FLAGS} -c -o FitDlg.o dialogs/FitDlg.cpp

ListDlg.o: dialogs/ListDlg.cpp dialogs/ListDlg.h
	${CC} ${FLAGS} -c -o ListDlg.o dialogs/ListDlg.cpp

ResoDlg.o: dialogs/ResoDlg.cpp dialogs/ResoDlg.h
	${CC} ${FLAGS} -c -o ResoDlg.o dialogs/ResoDlg.cpp

ResoDlg_prog.o: dialogs/ResoDlg.cpp dialogs/ResoDlg.h
	${CC} ${FLAGS} -c -DSTANDALONE_RESO -o ResoDlg_prog.o dialogs/ResoDlg.cpp

RoiDlg.o: dialogs/RoiDlg.cpp dialogs/RoiDlg.h
	${CC} ${FLAGS} -c -o RoiDlg.o dialogs/RoiDlg.cpp

SettingsDlg.o: dialogs/SettingsDlg.cpp dialogs/SettingsDlg.h
	${CC} ${FLAGS} -c -o SettingsDlg.o dialogs/SettingsDlg.cpp



fourier.o: helper/fourier.cpp helper/fourier.h
	${CC} ${FLAGS} -c -o fourier.o helper/fourier.cpp

string.o: helper/string.cpp helper/string.h
	${CC} ${FLAGS} -c -o string.o helper/string.cpp

xml.o: helper/xml.cpp helper/xml.h
	${CC} ${FLAGS} -c -o xml.o helper/xml.cpp




loadcasc.o: loader/loadcasc.cpp loader/loadcasc.h
	${CC} ${FLAGS} -c -o loadcasc.o loader/loadcasc.cpp

loadnicos.o: loader/loadnicos.cpp loader/loadnicos.h
	${CC} ${FLAGS} -c -o loadnicos.o loader/loadnicos.cpp

loadtxt.o: loader/loadtxt.cpp loader/loadtxt.h
	${CC} ${FLAGS} -c -o loadtxt.o loader/loadtxt.cpp




plot.o: plot/plot.cpp plot/plot.h
	${CC} ${FLAGS} -c -o plot.o plot/plot.cpp

plotgl.o: plot/plotgl.cpp plot/plotgl.h
	${CC} ${FLAGS} -c -o plotgl.o plot/plotgl.cpp

plot2d.o: plot/plot2d.cpp plot/plot2d.h
	${CC} ${FLAGS} -c -o plot2d.o plot/plot2d.cpp

plot3d.o: plot/plot3d.cpp plot/plot3d.h
	${CC} ${FLAGS} -c -o plot3d.o plot/plot3d.cpp

plot4d.o: plot/plot4d.cpp plot/plot4d.h
	${CC} ${FLAGS} -c -o plot4d.o plot/plot4d.cpp




roi.o: roi/roi.cpp roi/roi.h
	${CC} ${FLAGS} -c -o roi.o roi/roi.cpp




cn.o: tools/res/cn.cpp tools/res/cn.h
	${CC} ${FLAGS} -c -o cn.o tools/res/cn.cpp

pop.o: tools/res/pop.cpp tools/res/pop.h
	${CC} ${FLAGS} -c -o pop.o tools/res/pop.cpp

ellipse.o: tools/res/ellipse.cpp tools/res/ellipse.h
	${CC} ${FLAGS} -c -o ellipse.o tools/res/ellipse.cpp



chi2.o: fitter/chi2.cpp fitter/chi2.h
	${CC} ${FLAGS} -c -o chi2.o fitter/chi2.cpp

fitter.o: fitter/fitter.cpp fitter/fitter.h
	${CC} ${FLAGS} -c -o fitter.o fitter/fitter.cpp

functions.o: fitter/functions.cpp fitter/functions.h
	${CC} ${FLAGS} -c -o functions.o fitter/functions.cpp

parser.o: fitter/parser.cpp fitter/parser.h
	${CC} ${FLAGS} -c -o parser.o fitter/parser.cpp


freefit.o: fitter/models/freefit.cpp fitter/models/freefit.h
	${CC} ${FLAGS} -c -o freefit.o fitter/models/freefit.cpp

freefit-nd.o: fitter/models/freefit-nd.cpp fitter/models/freefit-nd.h
	${CC} ${FLAGS} -c -o freefit-nd.o fitter/models/freefit-nd.cpp

gauss.o: fitter/models/gauss.cpp fitter/models/gauss.h
	${CC} ${FLAGS} -c -o gauss.o fitter/models/gauss.cpp

gauss-nd.o: fitter/models/gauss-nd.cpp fitter/models/gauss-nd.h
	${CC} ${FLAGS} -c -o gauss-nd.o fitter/models/gauss-nd.cpp

msin.o: fitter/models/msin.cpp fitter/models/msin.h
	${CC} ${FLAGS} -c -o msin.o fitter/models/msin.cpp

interpolation.o: fitter/models/interpolation.cpp fitter/models/interpolation.h
	${CC} ${FLAGS} -c -o interpolation.o fitter/models/interpolation.cpp





clean:
	rm -f *.o
	rm -f cattus
	rm -f reso
	
	rm -f ui/*.h
	rm -f *.moc
	rm -f dialogs/*.moc
	rm -f plot/*.moc
