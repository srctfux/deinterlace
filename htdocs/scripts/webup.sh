#!/bin/sh 

#dTV Site Update Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#1/25/2001

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/deinterlace/htdocs/webup.sh /home/users/yourusername/ <Enter>
#cd /home/users/yourusername/ <Enter>
#sh webup.sh <Enter>

#Instructions for subsequent runs...
#sh webup.sh <Enter>

#A note about permissions and the cron...
#You will need to give the cron adequate permissions to run the script, if you
#schedule it in crontab.  As of now, some of Sourceforge's permissions are
#screwed up, so I have been unable to schedule it in the cron to work just
#as I would like it.  This will be fixed as Sourceforge builds out its new machines.

#A note about error messages...
#This script is noisy and verbose.  Don't worry about the error messages.


#===Getting down to business===


#Do the work from the dTV subdirectory

cd /home/groups/deinterlace/


#Delete the previous live site.
#Make sure you add add'l htdocs subdirectories to this list, if you add them to CVS.
#But DON'T delete the htdocs/webalizer directory, since that is not in the CVS.
#If you delete it, then you will have to reconstruct it from the logs, an extremely
#laborious process.

rm -f htdocs/*
rm -R -f htdocs/francais
rm -R -f htdocs/images
rm -R -f htdocs/Templates
rm -R -f htdocs/distributions
rm -R -f htdocs/bugs
rm -R -f htdocs/card-support
rm -R -f htdocs/dtv-vs-windvd
rm -R -f htdocs/palmovie
rm -R -f htdocs/readme
rm -R -f htdocs/screenshots
rm -R -f htdocs/setup


#Check out the htdocs module from the dTV CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co htdocs>/dev/null


#Sometimes the attic gets cluttered with various versions of a file, because of the
#caps-sensitive nature of *nix and the caps-insensitive nature of Windows.
#For the life of me, I couldn't get CVS to recognize these two files correctly,
#so a way of cheating is putting them on a remote site and wgetting them every time
#you do a web update.  Elegance be damned.

cd /home/groups/deinterlace/htdocs/distributions
wget -q http://www.schmelzer.org/dtvupload/dTV18exe.zip
wget -q http://www.schmelzer.org/dtvupload/dTV18src.zip


#Then change the file permissions so that anyone in the deinterlace administrative group
#can delete the directories and files

cd /home/groups/deinterlace/htdocs/
chmod 775 -R *


#Joy





