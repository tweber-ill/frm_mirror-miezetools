#!/bin/bash
#
# Mieze-Tools
# @author Tobias Weber <tobias.weber@tum.de>
# @license GPLv3
#

UIC="$(which uic-qt4)"
MOC="$(which moc-qt4)"

if [ -z "$UIC" ] || [ -z "${MOC}" ];
then
	echo "Error: moc/uic tools not found!";
	exit -1;
fi



echo -e "--------------------------------------------------------------------------------"
echo -e "building uis..."

cd ui
for file in *.ui
do
        outfile=ui_${file%\.ui}.h

        echo -e "${file} -> ${outfile}"
        ${UIC} ${file} -o ${outfile}
done
cd ..


echo -e "--------------------------------------------------------------------------------"
echo -e "\n"




echo -e "--------------------------------------------------------------------------------"
echo -e "building mocs..."

declare -a hfiles=(
	"main/mainwnd.h"
	"main/subwnd.h"
	"plot/plot.h"
	"plot/plot2d.h"
	"plot/plot3d.h"
	"plot/plot4d.h"
	"dialogs/ListDlg.h"
	"dialogs/CombineDlg.h"
	"dialogs/SettingsDlg.h"
	"dialogs/FitDlg.h"
	"dialogs/AboutDlg.h"
	"dialogs/RoiDlg.h"
	"dialogs/ComboDlg.h"
	"dialogs/PsdPhaseDlg.h"
	"dialogs/RadialIntDlg.h"
	"dialogs/PlotPropDlg.h"
	"dialogs/RebinDlg.h"
	"dialogs/ExportDlg.h"
	"dialogs/InfoDock.h"
	"dialogs/NormDlg.h"
	"tools/formula/FormulaDlg.h"
)

for hfile in ${hfiles[@]}
do
        mocfile=${hfile%\.h}.moc

        echo -e "${hfile} -> ${mocfile}"
        ${MOC} ${hfile} -o ${mocfile}
done

echo -e "--------------------------------------------------------------------------------"
exit 0
