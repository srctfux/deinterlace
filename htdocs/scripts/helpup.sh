#!/bin/sh 

#dScaler Help Update Script
#Dan Schmelzer
#johnadcock@hotmail.com
#1/25/2001

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/d/de/deinterlace/htdocs/scripts/helpup.sh
#   /home/users/firstletterusername/firsttwolettersusername/yourusername/ <Enter>
#cd /home/users/firstletterofyourusername/firsttwolettersofusername/yourusername/ <Enter>
#sh helpup.sh <Enter>

#Instructions for subsequent runs...
#sh helpup.sh <Enter>

#A note about error messages...
#This script is noisy and verbose.  Don't worry about the error messages.

#===Getting down to business===

#Do the work from the dScaler subdirectory

cd /home/groups/d/de/deinterlace/

rm -R -f htdocs/Help
rm -R -f htdocs/Help/images

#Check out the Help files module from the dScaler CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co -d Help DScaler/Help >/dev/null

#Then change the file permissions so that anyone in the deinterlace administrative group
#can delete the directories and files

cd /home/groups/d/de/deinterlace/htdocs/Help
chmod 777 -R *

#Joy