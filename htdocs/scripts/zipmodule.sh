#!/bin/sh 

#DScaler Module Zip Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#Updated 8/2/2001

#Why this script is needed...
#This script is needed to do the nightly module zip when we want to and to put
#it into a format that is Windows-friendly.  My unzip apps die ungracefully on the
#Sourceforge-generated nightly tarball.

#Instructions for initial run...
#cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace login <Enter> (See below for discussion)
#cp /home/groups/d/de/deinterlace/htdocs/scripts/zipmodule.sh
#   /home/users/1stletterusername/1sttwolettersusername/yourusername/ <Enter>
#cd /home/users/1stletterusername/1sttwolettersusername/yourusername/ <Enter>
#sh zipmodule.sh <Enter>

#Instructions for subsequent runs...
#sh zipmodule.sh <Enter>

#A note about permissions and the cron...
#You will need to give the cron adequate permissions to run the script, if you
#schedule it in crontab.


#===Getting down to business===


#Remove the old module zip from the FTP site

rm -f /home/groups/d/de/deinterlace/htdocs/downloads/dscalermodule.zip


#Do the remaining work from the DScaler subdirectory

cd /home/groups/d/de/deinterlace/


#Check out the DScaler module to temporary space from the DScaler CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co DScaler>/dev/null



#todos doesn't have a recursive function, so you have to work with each directory individually.

cd /home/groups/d/de/deinterlace/DScaler/API
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Debug
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Driver
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Driver/DSDrv
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Driver/DSDrv95
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Driver/DSDrvNT
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/DScaler
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/DScaler/RES
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/DScaler/Help
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Install
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Mapconv
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_Adaptive
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_BlendedClip
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_Bob
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_EvenOnly
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_Greedy
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_Greedy2Frame
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_GreedyH
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_OddOnly
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_OldGame
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_ScalerBob
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_TwoFrame
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_VideoBob
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_VideoWeave
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/DI_Weave
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/FLT_Gamma
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/FLT_LinearCorrection
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/FLT_LogoKill
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/FLT_TNoise
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Plugins/FLT_Sharpness
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/PlugTest
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Release
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf
cd /home/groups/d/de/deinterlace/DScaler/Verinc
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp *.url *.sln
todos *.hhk *.hhc *.rtf


cd /home/groups/d/de/deinterlace/


#Zip up the module

zip -r dscalermodule.zip DScaler


#Give the zip file group permissions in case somebody else needs to delete it

chmod 777 dscalermodule.zip


#Then move it to the site space

mv dscalermodule.zip /home/groups/d/de/deinterlace/htdocs/downloads/


#Clean up the temporary space

rm -rf DScaler


#Joy
