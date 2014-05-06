#!/bin/bash


UIC=uic-qt4
MOC=moc-qt4



echo -e "building uis..."

cd ui
for file in *.ui
do
        outfile=ui_${file%\.ui}.h

        echo -e "${file} -> ${outfile}"
        ${UIC} ${file} -o ${outfile}
done
cd ..

echo -e "\n"




echo -e "building mocs..."

${MOC} main/mainwnd.h -o main/mainwnd.moc
${MOC} main/subwnd.h -o main/subwnd.moc

${MOC} plot/plot.h -o plot/plot.moc
${MOC} plot/plot2d.h -o plot/plot2d.moc
${MOC} plot/plot3d.h -o plot/plot3d.moc
${MOC} plot/plot4d.h -o plot/plot4d.moc

${MOC} dialogs/ListDlg.h -o dialogs/ListDlg.moc
${MOC} dialogs/CombineDlg.h -o dialogs/CombineDlg.moc
${MOC} dialogs/SettingsDlg.h -o dialogs/SettingsDlg.moc
${MOC} dialogs/FitDlg.h -o dialogs/FitDlg.moc
${MOC} dialogs/AboutDlg.h -o dialogs/AboutDlg.moc
${MOC} dialogs/RoiDlg.h -o dialogs/RoiDlg.moc
${MOC} dialogs/ComboDlg.h -o dialogs/ComboDlg.moc
${MOC} dialogs/PsdPhaseDlg.h -o dialogs/PsdPhaseDlg.moc
${MOC} dialogs/RadialIntDlg.h -o dialogs/RadialIntDlg.moc
${MOC} dialogs/LatticeDlg.h -o dialogs/LatticeDlg.moc
${MOC} dialogs/PlotPropDlg.h -o dialogs/PlotPropDlg.moc
${MOC} dialogs/RebinDlg.h -o dialogs/RebinDlg.moc
${MOC} dialogs/ExportDlg.h -o dialogs/ExportDlg.moc
${MOC} dialogs/InfoDock.h -o dialogs/InfoDock.moc
${MOC} dialogs/NormDlg.h -o dialogs/NormDlg.moc

${MOC} tools/res/ResoDlg.h -o tools/res/ResoDlg.moc
${MOC} tools/formula/FormulaDlg.h -o tools/formula/FormulaDlg.moc


echo -e "\n"
