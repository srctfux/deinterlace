#!/bin/sh 

#DScaler Site Clean Script
#John Adcock
#john@adcock8.freeserve.co.uk
#4/Oct/2001

#This script deletes the web site.
#Must be run from your personal Sourceforge directory

#Instructions for initial run...
#svn co https://deinterlace.svn.sourceforge.net/svnroot/deinterlace/trunk/htdocs/scripts ~/scripts
#cd ~/scripts <Enter>
#sh webclean.sh <Enter>

#Instructions for subsequent runs...
#cd ~/scripts <Enter>
#svn update <Enter>
#sh webclean.sh <Enter>

#A note about permissions and the cron...
#You will need to give the cron adequate permissions to run the script, if you
#schedule it in crontab.

#===Getting down to business===


#Do the work from the DScaler subdirectory

cd /home/groups/d/de/deinterlace/

rm -f htdocs/*
rm -R -f htdocs/CVS
rm -R -f htdocs/.svn
rm -R -f htdocs/channels
rm -R -f htdocs/bugs
rm -R -f htdocs/card-support
rm -R -f htdocs/dtv-vs-windvd
rm -R -f htdocs/francais
rm -R -f htdocs/images
rm -R -f htdocs/palmovie
rm -R -f htdocs/portuguese
rm -R -f htdocs/readme
rm -R -f htdocs/reviews
rm -R -f htdocs/russian
rm -R -f htdocs/screenshots
rm -R -f htdocs/scripts
rm -R -f htdocs/setup
rm -R -f htdocs/Templates

rm -d -f htdocs/CVS
rm -d -f htdocs/.svn
rm -d -f htdocs/bugs
rm -d -f htdocs/card-support
rm -d -f htdocs/dtv-vs-windvd
rm -d -f htdocs/francais
rm -d -f htdocs/images
rm -d -f htdocs/palmovie
rm -d -f htdocs/portuguese
rm -d -f htdocs/readme
rm -d -f htdocs/reviews
rm -d -f htdocs/russian
rm -d -f htdocs/screenshots
rm -d -f htdocs/scripts
rm -d -f htdocs/setup
rm -d -f htdocs/Templates
