#!/bin/sh

#dTV Web Site Statistics Update Script
#Dan Schmelzer
#dschmelzer@hotmail.com
#1/25/2001

#Instructions...
#cp /home/groups/deinterlace/htdocs/scripts runlogs.sh /home/users/yourusername/ <Enter>
#Schedule this update script in your crontab to update Webalizer at 15 before midnight or so each night.
#Don't run it any closer to midnight, because sometimes it takes a minute or so to run, and other traffic may step
#on you and make it the next day, etc.  Only run this script once a day to avoid corruption of stats.
#Make sure this script has adequate permissions to run from crontab.


#===Getting down to business===


#Do all of the work from the logs directory, which is parallel to the htdocs directory on the file tree.

cd /home/groups/deinterlace/log


#Gather all of the log information from the various web servers and put it into text files in the log directory.
#Now, Sourceforge has five servers.  In the future, Sourceforge may add or subtract, so make sure that this is up to date.

grep -e deinterlace.sourceforge.net /home/logs/pr-web1/$(date +%Y)/$(date +%m)/$(date +%d)/* > $(date +%Y%m%d)no1
grep -e deinterlace.sourceforge.net /home/logs/pr-web2/$(date +%Y)/$(date +%m)/$(date +%d)/* > $(date +%Y%m%d)no2
grep -e deinterlace.sourceforge.net /home/logs/pr-web3/$(date +%Y)/$(date +%m)/$(date +%d)/* > $(date +%Y%m%d)no3
grep -e deinterlace.sourceforge.net /home/logs/pr-web4/$(date +%Y)/$(date +%m)/$(date +%d)/* > $(date +%Y%m%d)no4
grep -e deinterlace.sourceforge.net /home/logs/pr-web5/$(date +%Y)/$(date +%m)/$(date +%d)/* > $(date +%Y%m%d)no5


#Concatenate these server-specific dTV logs into the full daily log text file.

cat $(date +%Y%m%d)no1 $(date +%Y%m%d)no2 $(date +%Y%m%d)no3 $(date +%Y%m%d)no4 $(date +%Y%m%d)no5 > $(date +%Y%m%d)full


#Delete the server-specific dTV logs to tidy up.

rm -f $(date +%Y%m%d)no1
rm -f $(date +%Y%m%d)no2
rm -f $(date +%Y%m%d)no3
rm -f $(date +%Y%m%d)no4
rm -f $(date +%Y%m%d)no5


#Do the remaining work from the webalizer directory, which is exposed to ROW.

cd /home/groups/deinterlace/htdocs/webalizer


#Run the Webalizer program to update the html files and to create new ones as specified in dTV's webalizer .conf
#file.

/usr/bin/webalizer /home/groups/deinterlace/log/$(date +%Y%m%d)full


#Joy