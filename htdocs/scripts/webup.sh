#!/bin/sh 

#dScaler Site Update Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#1/25/2001

#Instructions for running
#cd ~/scripts <Enter>
#svn update <Enter>
#sh webup.sh <Enter>

#Do the work from the dScaler htdocs subdirectory

cd /home/groups/d/de/deinterlace/htdocs

svn update

#Then change the file permissions so that anyone in the deinterlace administrative group
#can delete the directories and files

chmod -R 771 *
chown -R adcockj.deinterlace *

#Joy