#!/bin/bash



echo -e "building uis..."

cd ui
for file in *.ui
do
        outfile=ui_${file%\.ui}.h

        echo -e "${file} -> ${outfile}"
        uic ${file} -o ${outfile}
done
cd ..

echo -e "\n"




echo -e "building mocs..."

moc main/mainwnd.h -o main/mainwnd.moc
moc main/subwnd.h -o main/subwnd.moc

moc plot/plot.h -o plot/plot.moc
moc plot/plot2d.h -o plot/plot2d.moc
moc plot/plot3d.h -o plot/plot3d.moc
moc plot/plot4d.h -o plot/plot4d.moc

moc dialogs/ListDlg.h -o dialogs/ListDlg.moc
moc dialogs/CombineDlg.h -o dialogs/CombineDlg.moc
moc dialogs/SettingsDlg.h -o dialogs/SettingsDlg.moc
moc dialogs/FitDlg.h -o dialogs/FitDlg.moc
moc dialogs/AboutDlg.h -o dialogs/AboutDlg.moc
moc dialogs/RoiDlg.h -o dialogs/RoiDlg.moc
moc dialogs/ComboDlg.h -o dialogs/ComboDlg.moc
moc dialogs/PsdPhaseDlg.h -o dialogs/PsdPhaseDlg.moc
moc dialogs/RadialIntDlg.h -o dialogs/RadialIntDlg.moc
moc dialogs/LatticeDlg.h -o dialogs/LatticeDlg.moc
moc dialogs/PlotPropDlg.h -o dialogs/PlotPropDlg.moc
moc dialogs/RebinDlg.h -o dialogs/RebinDlg.moc
moc dialogs/ExportDlg.h -o dialogs/ExportDlg.moc
moc dialogs/InfoDock.h -o dialogs/InfoDock.moc
moc dialogs/NormDlg.h -o dialogs/NormDlg.moc

moc tools/res/ResoDlg.h -o tools/res/ResoDlg.moc
moc tools/formula/FormulaDlg.h -o tools/formula/FormulaDlg.moc
moc tools/taz/taz.h -o tools/taz/taz.moc
moc tools/taz/scattering_triangle.h -o tools/taz/scattering_triangle.moc
moc tools/taz/tas_layout.h -o tools/taz/tas_layout.moc

echo -e "\n"
