#!/bin/sh 

#dTV Module Zip Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#1/30/2001

#Why this script is needed...
#This script is needed to do the nightly module zip when we want to and to put
#it into a format that is Windows-friendly.  My unzip apps die ungracefully on the
#Sourceforge-generated nightly tarball.

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/deinterlace/htdocs/scripts/zipmodule.sh /home/users/yourusername/ <Enter>
#cd /home/users/yourusername/ <Enter>
#sh zipmodule.sh <Enter>

#Instructions for subsequent runs...
#sh zipmodule.sh <Enter>

#A note about permissions and the cron...
#You will need to give the cron adequate permissions to run the script, if you
#schedule it in crontab.


#===Getting down to business===


#Remove the old module zip from the FTP site

rm -f /home/groups/ftp/pub/deinterlace/dtvmodule.zip


#Do the remaining work from the dTV subdirectory

cd /home/groups/deinterlace/


#Check out the dTV module to temporary space from the dTV CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co dTV>/dev/null


#Zip up the module

zip -r dtvmodule.zip /home/groups/deinterlace/dTV/*


#Give the zip file group permissions in case somebody else needs to delete it

chmod 775 dtvmodule.zip


#Then move it to the anonymous FTP space

mv dtvmodule.zip /home/groups/ftp/pub/deinterlace/


#Clean up the temporary space

rm -rf dTV


#Joy
