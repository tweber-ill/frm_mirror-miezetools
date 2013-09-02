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

moc mainwnd.h -o mainwnd.moc
moc subwnd.h -o subwnd.moc

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
moc dialogs/ResoDlg.h -o dialogs/ResoDlg.moc
moc dialogs/PsdPhaseDlg.h -o dialogs/PsdPhaseDlg.moc
moc dialogs/RadialIntDlg.h -o dialogs/RadialIntDlg.moc
moc dialogs/FormulaDlg.h -o dialogs/FormulaDlg.moc
moc dialogs/PlotPropDlg.h -o dialogs/PlotPropDlg.moc
moc dialogs/ExportDlg.h -o dialogs/ExportDlg.moc
moc dialogs/InfoDock.h -o dialogs/InfoDock.moc

echo -e "\n"
