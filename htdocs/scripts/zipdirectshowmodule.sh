#!/bin/sh 

#Deinterlace Module Zip Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#Updated 8/2/2001

#Why this script is needed...
#This script is needed to do the nightly Deinterlace module zip and to put
#it into a format that is Windows-friendly.  My unzip apps die ungracefully on the
#Sourceforge-generated nightly tarball.

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/d/de/deinterlace/htdocs/scripts/zipdirectshowmodule.sh
#   /home/users/1stletterusername/1sttwolettersusername/yourusername/ <Enter>
#cd /home/users/1stLetterusername/1sttwolettersusername/yourusername/ <Enter>
#sh zipdirectshowmodule.sh <Enter>

#Instructions for subsequent runs...
#sh zipdirectshowmodule.sh <Enter>

#A note about permissions and the cron...
#You will need to give the cron adequate permissions to run the script, if you
#schedule it in crontab.


#===Getting down to business===


#Remove the old module zip from the web site

rm -f /home/groups/d/de/deinterlace/htdocs/downloads/directshowmodule.zip


#Do the remaining work from the DScaler subdirectory

cd /home/groups/d/de/deinterlace/


#Check out the DIDMO module to temporary space from the DScaler CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co DIDMO>/dev/null



#todos doesn't have a recursive function, so you have to work with each directory individually.

cd /home/groups/d/de/deinterlace/DIDMO
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/


#Zip up the module

zip -r directshowmodule.zip DIDMO


#Give the zip file group permissions in case somebody else needs to delete it

chmod 777 directshowmodule.zip


#Then move it to the site space

mv directshowmodule.zip /home/groups/d/de/deinterlace/htdocs/downloads/


#Clean up the temporary space

rm -rf DIDMO


#Joy
