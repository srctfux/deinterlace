#!/bin/sh 

#dScaler Site Clean Script
#John Adcock
#john@adcock8.freeserve.co.uk
#4/Oct/2001

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/d/de/deinterlace/htdocs/scripts/webclean.sh
#   /home/users/firstletterusername/firsttwolettersusername/yourusername/ <Enter>
#cd /home/users/firstletterofyourusername/firsttwolettersofusername/yourusername/ <Enter>
#sh webclean.sh <Enter>

#Instructions for subsequent runs...
#sh webclean.sh <Enter>

#A note about permissions and the cron...
#You will need to give the cron adequate permissions to run the script, if you
#schedule it in crontab.

#A note about error messages...
#This script is noisy and verbose.  Don't worry about the error messages.


#===Getting down to business===


#Do the work from the dScaler subdirectory

cd /home/groups/d/de/deinterlace/


#Delete the previous live site.
#Make sure you add add'l htdocs subdirectories to this list, if you add them to CVS.
#But DON'T delete the htdocs/webalizer directory, since that is not in the CVS.
#If you delete it, then you will have to reconstruct it from the logs, an extremely
#laborious process.

#Note 5/13/2001: Sourceforge no longer exposes the log files for Webalizer.  Old
#statistics are still kept on dScaler's webalizer page, but are no longer updating.
#One of these days, Sourceforge will install a complete web site stats package
#and we will again have accurate usage info, etc.

rm -f htdocs/*
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

#Check out the htdocs module from the dScaler CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co htdocs>/dev/null

#Then change the file permissions so that anyone in the deinterlace administrative group
#can delete the directories and files

cd /home/groups/d/de/deinterlace/htdocs/

chmod -R 777 *
chown -R adcockj.deinterlace *

#Joy