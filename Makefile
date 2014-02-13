CC = gcc
INC = -I/usr/include/qt4 -I/usr/local/include -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I./tools/plot_gpl/src/qtterminal
LIB_DIRS = -L/usr/lib64 -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib

#DEFINES = -DUSE_GPL -DUSE_FFTW
DEFINES = -DUSE_FFTW

FLAGS = ${INC} -O2 -march=native -std=c++11 -DNDEBUG ${DEFINES}
#FLAGS = ${INC} -std=c++11 -ggdb ${DEFINES}

STD_LIBS = -lsupc++ -lstdc++ -lm
MATH_LIBS = -lMinuit2 -lfftw3
#MISC_LIBS = ./gnuplot-qt.so
MISC_LIBS = 
LAPACK_LIBS = -L/usr/local/lib64 -llapacke -llapack -lblas -lgfortran
QT_LIBS = -L/usr/lib64/qt4 -L/usr/lib/x86_64-linux-gnu -L /usr/lib/qt4/lib \
	-lQtCore -lQtGui -lQtXml -lQtXmlPatterns -lQtOpenGL \
	-lGL -lGLU -lX11
LIBS_RESO = -L/usr/lib64 -lstdc++ -lm -lboost_iostreams-mt ${QT_LIBS} ${LAPACK_LIBS}
LIBS_TAZ = -L/usr/lib64 -lstdc++ -lm ${QT_LIBS}
LIBS_FORMULA = -L/usr/lib64 -lstdc++ -lm -lboost_iostreams-mt ${QT_LIBS}
LIBS = ${LIB_DIRS} -fopenmp -lboost_iostreams-mt ${MATH_LIBS} ${QT_LIBS} ${LAPACK_LIBS} ${MISC_LIBS} ${STD_LIBS}


cattus: obj/main.o obj/mainwnd.o obj/mainwnd_files.o obj/mainwnd_session.o obj/mainwnd_mdi.o \
	obj/subwnd.o obj/settings.o obj/data.o obj/data1.o obj/data2.o obj/data3.o obj/data4.o \
	obj/FormulaDlg.o obj/CombineDlg.o obj/ComboDlg.o obj/FitDlg.o obj/ListDlg.o obj/ResoDlg.o \
	obj/RoiDlg.o obj/SettingsDlg.o obj/PsdPhaseDlg.o obj/RadialIntDlg.o obj/ExportDlg.o \
	obj/PlotPropDlg.o obj/fourier.o obj/string.o obj/xml.o obj/loadcasc.o obj/loadnicos.o \
	obj/loadtxt.o obj/plotgl.o obj/plot.o obj/plot2d.o obj/plot3d.o obj/plot4d.o obj/roi.o \
	obj/cn.o obj/pop.o obj/chi2.o obj/fitter.o obj/functions.o obj/parser.o obj/freefit.o \
	obj/freefit-nd.o obj/gauss.o obj/gauss-nd.o obj/msin.o obj/interpolation.o obj/ellipse.o \
	obj/linalg.o obj/blob.o obj/export.o obj/fit_data.o obj/formulas.o obj/file.o obj/comp.o \
	obj/rand.o obj/InfoDock.o obj/NormDlg.o obj/LatticeDlg.o obj/RebinDlg.o obj/taz.o \
	obj/scattering_triangle.o obj/lattice.o
	${CC} ${FLAGS} -o bin/cattus obj/main.o obj/mainwnd.o obj/mainwnd_files.o obj/mainwnd_session.o \
			obj/mainwnd_mdi.o obj/subwnd.o obj/settings.o obj/data.o obj/data1.o obj/data2.o \
			obj/data3.o obj/data4.o obj/FormulaDlg.o obj/CombineDlg.o obj/ComboDlg.o \
			obj/FitDlg.o obj/ListDlg.o obj/ResoDlg.o obj/RoiDlg.o obj/SettingsDlg.o \
			obj/PsdPhaseDlg.o obj/RadialIntDlg.o obj/ExportDlg.o obj/PlotPropDlg.o \
			obj/fourier.o obj/string.o obj/xml.o obj/loadcasc.o obj/loadnicos.o \
			obj/loadtxt.o obj/plotgl.o obj/plot.o obj/plot2d.o obj/plot3d.o obj/plot4d.o \
			obj/roi.o obj/cn.o obj/pop.o obj/chi2.o obj/fitter.o obj/functions.o \
			obj/parser.o obj/freefit.o obj/freefit-nd.o obj/gauss.o obj/gauss-nd.o obj/msin.o \
			obj/interpolation.o obj/ellipse.o obj/linalg.o obj/blob.o obj/export.o \
			obj/fit_data.o obj/formulas.o obj/file.o obj/comp.o obj/rand.o obj/InfoDock.o \
			obj/NormDlg.o obj/LatticeDlg.o obj/RebinDlg.o \
			obj/taz.o obj/scattering_triangle.o obj/lattice.o \
			${LIBS}
	strip bin/cattus

reso: obj/settings.o obj/data.o obj/data1.o obj/ResoDlg_prog.o obj/string.o obj/xml.o \
	obj/plot_nopars.o obj/cn.o obj/pop.o obj/ellipse.o obj/roi.o obj/plotgl.o \
	obj/linalg.o obj/blob.o obj/comp.o
	${CC} ${FLAGS} -o bin/reso obj/settings.o obj/data.o obj/data1.o obj/ResoDlg_prog.o \
			obj/string.o obj/xml.o obj/plot_nopars.o obj/cn.o obj/pop.o \
			obj/ellipse.o obj/roi.o obj/plotgl.o obj/linalg.o obj/blob.o \
			obj/comp.o \
			${LIBS_RESO}
	strip bin/reso

taz: obj/taz_prog.o obj/scattering_triangle.o obj/lattice.o
	${CC} ${FLAGS} -o bin/taz obj/taz_prog.o obj/scattering_triangle.o obj/lattice.o \
			${LIBS_TAZ}
	strip bin/taz

formula: obj/FormulaDlg_prog.o obj/formulas.o obj/string.o obj/settings.o obj/plot_nopars.o \
	obj/data.o obj/data1.o obj/blob.o obj/roi.o obj/xml.o obj/comp.o obj/export.o obj/data2.o
	${CC} ${FLAGS} -o bin/formula obj/FormulaDlg_prog.o obj/formulas.o obj/string.o obj/settings.o \
			obj/plot_nopars.o obj/data.o obj/data1.o obj/blob.o obj/roi.o obj/xml.o \
			obj/comp.o obj/export.o obj/data2.o \
			${LIBS_FORMULA}
	strip bin/formula


obj/main.o: main.cpp
	${CC} ${FLAGS} -c -o obj/main.o main.cpp

obj/mainwnd.o: mainwnd.cpp mainwnd.h
	${CC} ${FLAGS} -c -o obj/mainwnd.o mainwnd.cpp

obj/mainwnd_files.o: mainwnd_files.cpp mainwnd.h
	${CC} ${FLAGS} -c -o obj/mainwnd_files.o mainwnd_files.cpp

obj/mainwnd_session.o: mainwnd_session.cpp mainwnd.h
	${CC} ${FLAGS} -c -o obj/mainwnd_session.o mainwnd_session.cpp

obj/mainwnd_mdi.o: mainwnd_mdi.cpp mainwnd.h
	${CC} ${FLAGS} -c -o obj/mainwnd_mdi.o mainwnd_mdi.cpp

obj/subwnd.o: subwnd.cpp subwnd.h
	${CC} ${FLAGS} -c -o obj/subwnd.o subwnd.cpp

obj/settings.o: settings.cpp settings.h
	${CC} ${FLAGS} -c -o obj/settings.o settings.cpp


obj/data.o: data/data.cpp data/data.h
	${CC} ${FLAGS} -c -o obj/data.o data/data.cpp

obj/data1.o: data/data1.cpp data/data1.h
	${CC} ${FLAGS} -c -o obj/data1.o data/data1.cpp

obj/data2.o: data/data2.cpp data/data2.h
	${CC} ${FLAGS} -c -o obj/data2.o data/data2.cpp

obj/data3.o: data/data3.cpp data/data3.h
	${CC} ${FLAGS} -c -o obj/data3.o data/data3.cpp

obj/data4.o: data/data4.cpp data/data4.h
	${CC} ${FLAGS} -c -o obj/data4.o data/data4.cpp

obj/fit_data.o: data/fit_data.cpp data/fit_data.h
	${CC} ${FLAGS} -c -o obj/fit_data.o data/fit_data.cpp

obj/export.o: data/export.cpp data/export.h
	${CC} ${FLAGS} -c -o obj/export.o data/export.cpp


obj/CombineDlg.o: dialogs/CombineDlg.cpp dialogs/CombineDlg.h
	${CC} ${FLAGS} -c -o obj/CombineDlg.o dialogs/CombineDlg.cpp

obj/ComboDlg.o: dialogs/ComboDlg.cpp dialogs/ComboDlg.h
	${CC} ${FLAGS} -c -o obj/ComboDlg.o dialogs/ComboDlg.cpp

obj/FitDlg.o: dialogs/FitDlg.cpp dialogs/FitDlg.h
	${CC} ${FLAGS} -c -o obj/FitDlg.o dialogs/FitDlg.cpp

obj/ListDlg.o: dialogs/ListDlg.cpp dialogs/ListDlg.h
	${CC} ${FLAGS} -c -o obj/ListDlg.o dialogs/ListDlg.cpp

obj/ResoDlg.o: tools/res/ResoDlg.cpp tools/res/ResoDlg.h
	${CC} ${FLAGS} -c -o obj/ResoDlg.o tools/res/ResoDlg.cpp

obj/ResoDlg_prog.o: tools/res/ResoDlg.cpp tools/res/ResoDlg.h
	${CC} ${FLAGS} -c -DSTANDALONE_RESO -o obj/ResoDlg_prog.o tools/res/ResoDlg.cpp

obj/FormulaDlg.o: tools/formula/FormulaDlg.cpp tools/formula/FormulaDlg.h
	${CC} ${FLAGS} -c -o obj/FormulaDlg.o tools/formula/FormulaDlg.cpp

obj/FormulaDlg_prog.o: tools/formula/FormulaDlg.cpp tools/formula/FormulaDlg.h
	${CC} ${FLAGS} -c -DSTANDALONE_FORMULA -o obj/FormulaDlg_prog.o tools/formula/FormulaDlg.cpp

obj/taz_prog.o: tools/taz/taz.cpp tools/taz/taz.h
	${CC} ${FLAGS} -c -DSTANDALONE_TAZ -o obj/taz_prog.o tools/taz/taz.cpp

obj/taz.o: tools/taz/taz.cpp tools/taz/taz.h
	${CC} ${FLAGS} -c -o obj/taz.o tools/taz/taz.cpp

obj/scattering_triangle.o: tools/taz/scattering_triangle.cpp tools/taz/scattering_triangle.h
	${CC} ${FLAGS} -c -o obj/scattering_triangle.o tools/taz/scattering_triangle.cpp

obj/PsdPhaseDlg.o: dialogs/PsdPhaseDlg.cpp dialogs/PsdPhaseDlg.h
	${CC} ${FLAGS} -c -o obj/PsdPhaseDlg.o dialogs/PsdPhaseDlg.cpp

obj/RoiDlg.o: dialogs/RoiDlg.cpp dialogs/RoiDlg.h
	${CC} ${FLAGS} -c -o obj/RoiDlg.o dialogs/RoiDlg.cpp

obj/SettingsDlg.o: dialogs/SettingsDlg.cpp dialogs/SettingsDlg.h
	${CC} ${FLAGS} -c -o obj/SettingsDlg.o dialogs/SettingsDlg.cpp

obj/RadialIntDlg.o: dialogs/RadialIntDlg.cpp dialogs/RadialIntDlg.h
	${CC} ${FLAGS} -c -o obj/RadialIntDlg.o dialogs/RadialIntDlg.cpp

obj/LatticeDlg.o: dialogs/LatticeDlg.cpp dialogs/LatticeDlg.h
	${CC} ${FLAGS} -c -o obj/LatticeDlg.o dialogs/LatticeDlg.cpp

obj/ExportDlg.o: dialogs/ExportDlg.cpp dialogs/ExportDlg.h
	${CC} ${FLAGS} -c -o obj/ExportDlg.o dialogs/ExportDlg.cpp

obj/PlotPropDlg.o: dialogs/PlotPropDlg.cpp dialogs/PlotPropDlg.h
	${CC} ${FLAGS} -c -o obj/PlotPropDlg.o dialogs/PlotPropDlg.cpp

obj/InfoDock.o: dialogs/InfoDock.cpp dialogs/InfoDock.h
	${CC} ${FLAGS} -c -o obj/InfoDock.o dialogs/InfoDock.cpp

obj/NormDlg.o: dialogs/NormDlg.cpp dialogs/NormDlg.h
	${CC} ${FLAGS} -c -o obj/NormDlg.o dialogs/NormDlg.cpp

obj/RebinDlg.o: dialogs/RebinDlg.cpp dialogs/RebinDlg.h
	${CC} ${FLAGS} -c -o obj/RebinDlg.o dialogs/RebinDlg.cpp


obj/fourier.o: helper/fourier.cpp helper/fourier.h
	${CC} ${FLAGS} -c -o obj/fourier.o helper/fourier.cpp

obj/string.o: helper/string.cpp helper/string.h
	${CC} ${FLAGS} -c -o obj/string.o helper/string.cpp

obj/xml.o: helper/xml.cpp helper/xml.h
	${CC} ${FLAGS} -c -o obj/xml.o helper/xml.cpp

obj/linalg.o: helper/linalg.cpp helper/linalg.h
	${CC} ${FLAGS} -c -o obj/linalg.o helper/linalg.cpp

obj/lattice.o: helper/lattice.cpp helper/lattice.h
	${CC} ${FLAGS} -c -o obj/lattice.o helper/lattice.cpp

obj/blob.o: helper/blob.cpp helper/blob.h
	${CC} ${FLAGS} -c -o obj/blob.o helper/blob.cpp

obj/formulas.o: helper/formulas.cpp helper/neutrons.hpp
	${CC} ${FLAGS} -c -o obj/formulas.o helper/formulas.cpp

obj/comp.o: helper/comp.cpp helper/comp.h
	${CC} ${FLAGS} -c -o obj/comp.o helper/comp.cpp

obj/file.o: helper/file.cpp helper/file.h
	${CC} ${FLAGS} -c -o obj/file.o helper/file.cpp

obj/rand.o: helper/rand.cpp helper/rand.h
	${CC} ${FLAGS} -c -o obj/rand.o helper/rand.cpp


obj/loadcasc.o: loader/loadcasc.cpp loader/loadcasc.h
	${CC} ${FLAGS} -c -o obj/loadcasc.o loader/loadcasc.cpp

obj/loadnicos.o: loader/loadnicos.cpp loader/loadnicos.h
	${CC} ${FLAGS} -c -o obj/loadnicos.o loader/loadnicos.cpp

obj/loadtxt.o: loader/loadtxt.cpp loader/loadtxt.h
	${CC} ${FLAGS} -c -o obj/loadtxt.o loader/loadtxt.cpp




obj/plot.o: plot/plot.cpp plot/plot.h
	${CC} ${FLAGS} -c -o obj/plot.o plot/plot.cpp

obj/plot_nopars.o: plot/plot.cpp plot/plot.h
	${CC} ${FLAGS} -DNO_PARSER -c -o obj/plot_nopars.o plot/plot.cpp

obj/plotgl.o: plot/plotgl.cpp plot/plotgl.h
	${CC} ${FLAGS} -c -o obj/plotgl.o plot/plotgl.cpp

obj/plot2d.o: plot/plot2d.cpp plot/plot2d.h
	${CC} ${FLAGS} -c -o obj/plot2d.o plot/plot2d.cpp

obj/plot3d.o: plot/plot3d.cpp plot/plot3d.h
	${CC} ${FLAGS} -c -o obj/plot3d.o plot/plot3d.cpp

obj/plot4d.o: plot/plot4d.cpp plot/plot4d.h
	${CC} ${FLAGS} -c -o obj/plot4d.o plot/plot4d.cpp




obj/roi.o: roi/roi.cpp roi/roi.h
	${CC} ${FLAGS} -c -o obj/roi.o roi/roi.cpp




obj/cn.o: tools/res/cn.cpp tools/res/cn.h
	${CC} ${FLAGS} -c -o obj/cn.o tools/res/cn.cpp

obj/pop.o: tools/res/pop.cpp tools/res/pop.h
	${CC} ${FLAGS} -c -o obj/pop.o tools/res/pop.cpp

obj/ellipse.o: tools/res/ellipse.cpp tools/res/ellipse.h
	${CC} ${FLAGS} -c -o obj/ellipse.o tools/res/ellipse.cpp



obj/chi2.o: fitter/chi2.cpp fitter/chi2.h
	${CC} ${FLAGS} -c -o obj/chi2.o fitter/chi2.cpp

obj/fitter.o: fitter/fitter.cpp fitter/fitter.h
	${CC} ${FLAGS} -c -o obj/fitter.o fitter/fitter.cpp

obj/functions.o: fitter/functions.cpp fitter/functions.h
	${CC} ${FLAGS} -c -o obj/functions.o fitter/functions.cpp

obj/parser.o: fitter/parser.cpp fitter/parser.h
	${CC} ${FLAGS} -c -o obj/parser.o fitter/parser.cpp


obj/freefit.o: fitter/models/freefit.cpp fitter/models/freefit.h
	${CC} ${FLAGS} -c -o obj/freefit.o fitter/models/freefit.cpp

obj/freefit-nd.o: fitter/models/freefit-nd.cpp fitter/models/freefit-nd.h
	${CC} ${FLAGS} -c -o obj/freefit-nd.o fitter/models/freefit-nd.cpp

obj/gauss.o: fitter/models/gauss.cpp fitter/models/gauss.h
	${CC} ${FLAGS} -c -o obj/gauss.o fitter/models/gauss.cpp

obj/gauss-nd.o: fitter/models/gauss-nd.cpp fitter/models/gauss-nd.h
	${CC} ${FLAGS} -c -o obj/gauss-nd.o fitter/models/gauss-nd.cpp

obj/msin.o: fitter/models/msin.cpp fitter/models/msin.h
	${CC} ${FLAGS} -c -o obj/msin.o fitter/models/msin.cpp

obj/interpolation.o: fitter/models/interpolation.cpp fitter/models/interpolation.h
	${CC} ${FLAGS} -c -o obj/interpolation.o fitter/models/interpolation.cpp




clean:
	find bin -regex 'bin/[_a-zA-Z0-9]*' | xargs rm -f
	rm -f obj/*.o
	rm -f ui/*.h
	rm -f *.moc
	rm -f dialogs/*.moc
	rm -f tools/res/*.moc
	rm -f tools/taz/*.moc
	rm -f plot/*.moc
