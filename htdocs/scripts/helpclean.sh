#!/bin/sh 

#dScaler Help clean Script
#Dan Schmelzer
#johnadcock@hotmail.com
#1/25/2001

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/d/de/deinterlace/htdocs/scripts/helpup.sh
#   /home/users/firstletterusername/firsttwolettersusername/yourusername/ <Enter>
#cd /home/users/firstletterofyourusername/firsttwolettersofusername/yourusername/ <Enter>
#sh helpclean.sh <Enter>

#Instructions for subsequent runs...
#sh helpclean.sh <Enter>

#A note about error messages...
#This script is noisy and verbose.  Don't worry about the error messages.

#===Getting down to business===

#Do the work from the dScaler subdirectory

cd /home/groups/d/de/deinterlace/htdocs

rm -R -f htdocs/Help/CVS
rm -R -f htdocs/Help/classes
rm -R -f htdocs/Help/images
rm -R -f htdocs/Help

rm -d -f htdocs/Help/CVS
rm -d -f htdocs/Help/classes
rm -d -f htdocs/Help/images
rm -d -f htdocs/Help

#Check out the Help files module from the dScaler CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co -d Help DScaler/Help >/dev/null

#Then change the file permissions so that anyone in the deinterlace administrative group
#can delete the directories and files

cd /home/groups/d/de/deinterlace/htdocs/Help

chmod -R 777 *
chown -R adcockj.deinterlace *

#Joy