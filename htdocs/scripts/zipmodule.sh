#!/bin/sh 

#dScaler Module Zip Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#1/30/2001

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

rm -f /home/groups/ftp/pub/deinterlace/dscalermodule.zip


#Do the remaining work from the dTV subdirectory

cd /home/groups/d/de/deinterlace/


#Check out the dTV module to temporary space from the dTV CVS using pserver.
#You will need to log in to pserver once; after that, you should be fine, unless the CVS
#server crashes or something like that.  In a half year, I've had to log in a couple of times.
#See the instructions above for the details on logging in.

cvs -d:pserver:anonymous@cvs1:/cvsroot/deinterlace co dTV>/dev/null



#todos doesn't have a recursive function, so you have to work with each directory individually.

cd /home/groups/d/de/deinterlace/dTV/dTV
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Driver/COMMON
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Driver/DLL
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Driver/INCLUDE
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Driver/SYS
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Driver/VXD
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Debug
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Release
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Release/Docs
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_Adaptive
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_BlendedClip
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_Bob
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_EvenOnly
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_Greedy
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_Greedy2Frame
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_OddOnly
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_ScalerBob
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_TwoFrame
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_VideoBob
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_VideoWeave
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/DI_Weave
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/FLT_Gamma
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/FLT_TNoise
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Plugins/FLT_LinearCorrection
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/PlugTest
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp
cd /home/groups/d/de/deinterlace/dTV/Api
todos *.c *.h *.asm *.dsp *.dsw *.html *.htm *.txt *.rc *.ini *.cpp



cd /home/groups/d/de/deinterlace/


#Zip up the module

zip -r dscalermodule.zip dTV


#Give the zip file group permissions in case somebody else needs to delete it

chmod 777 dscalermodule.zip


#Then move it to the site space

mv dscalermodule.zip /home/groups/ftp/pub/deinterlace/


#Clean up the temporary space

rm -rf dTV


#Joy







