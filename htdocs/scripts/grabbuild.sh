#!/bin/sh

#dTV Nightly Build Grab Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#1/31/2001

#Instructions...
#cp /home/groups/deinterlace/htdocs/scripts/grabbuild.sh /home/users/yourusername/ <Enter>
#Schedule this update script in your cron to grab the nightly build from a remote site and
#do some related site cleanup.

#Make sure you give the cron adequate permissions to run the script.


#=======================


#Do all of the work from the nightly-builds directory

cd /home/groups/deinterlace/htdocs/nightly-builds


#Grab the build zip from the remote site, if the file exists.
#Replace the included site name with your site name

wget -q www.schmelzer.org/dtvupload/dtv_nightly_build.zip


#Give the file adequate permissions so others can delete it, if necessary

chmod 775 dtv_nightly_build.zip


#Delete the build that is one week old

rm -f *$(date +%w).zip


#Rename the just-downloaded nightly build zip to be more descriptive

rn dtv_nightly_build.zip dtv_build_$(date +%Y%m%d)d$(date +%w).zip


#Joy
