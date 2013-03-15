#!/bin/bash



#echo -e "building uis..."

#cd ui
#for file in *.ui
#do
#        outfile=ui_${file%\.ui}.h
#
#        echo -e "${file} -> ${outfile}"
#        uic ${file} -o ${outfile}
#done




echo -e "building mocs..."

moc mainwnd.h -o mainwnd.moc
moc subwnd.h -o subwnd.moc
moc plot/plot.h -o plot/plot.moc
moc plot/plot2d.h -o plot/plot2d.moc
moc plot/plot3d.h -o plot/plot3d.moc
