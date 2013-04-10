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

echo -e "\n"
